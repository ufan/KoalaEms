/*
 * async_collector.c
 * 
 * created 2011-04-21 PW
 * $ZEL: async_collector.c,v 1.2 2011/08/18 22:39:27 wuestner Exp $
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

#define MAXSIZE 0xfffff

/* this describes a data block, possible partially written or read */
struct data_descr {
    int refcount;
    u_int32_t size;
    size_t space;
    u_int8_t *opaque;
};

/* this describes an entry for a list of data_descr */
struct data_ent {
    struct data_ent *next;
    struct data_descr *data;
};

struct sockdescr {
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    int p, idx, stalled;
    int maxqueue; /* maximum length of data queue (0: unlimited) */
    struct data_descr *data; /* data block which is currently written or read */
    struct data_ent *list;   /* not used by insocks*/
};

struct lsockdescr {
    char service[NI_MAXSERV];
    int p;        /* path descriptor */
    int maxqueue; /* maximum length of data queue (0: unlimited) */
};

static int queuemax=0;
static int quiet=0;
static int verbose=0;
static int no_v4=0;
static int no_v6=0;
static const char *inname=0;
static char *logname=0;
static FILE *logfile;
static FILE *log=0;

static int numinlistensocks=0;
static struct lsockdescr *inlistensocks=0;
static int numoutlistensocks=0;
static struct lsockdescr *outlistensocks=0;
static int numinsocks=0;
static struct sockdescr *insocks=0;
static int numoutsocks=0;
static struct sockdescr *outsocks=0;

static volatile int int_received;

#define LOG(fmt, arg...)           \
    do {                           \
        fprintf(log, fmt, ## arg); \
        fflush(log);               \
    } while (0);

/******************************************************************************/
static void
printusage(char* argv0)
{
    LOG("usage: %s [-h] [-q] [-v] [-l <logname>] [-4] "
                 "-o <outport> [-n <num>] -i <inport>\n", argv0);
    LOG("  -h: this help\n");
    LOG("  -q: quiet (no informational output)\n");
    LOG("  -4: use IPv4 only\n");
    LOG("  -i: inport (portnum|portname)\n");
    LOG("  -o: outport (portnum|portname)\n");
    LOG("  -n: number of queued data blocks for the following outports\n"
                 "      default: 0 (unlimited)\n");
    LOG("  -l <filename>: logfile\n");
    LOG("  -v: verbose (disables quiet)\n"
                 "      may be used multiple times:\n"
                 "          0: 'normal' output only\n"
                 "          1: some more information\n"
                 "         >1: debug information\n");
    LOG("      pairs of -n .. -o .. can be given multiple times\n");
}
/******************************************************************************/
static int
readargs(int argc, char* argv[])
{
    int c, err;
    err=0;

    while (!err && ((c=getopt(argc, argv, "hq46i:o:n:l:v")) != -1)) {
        switch (c) {
        case 'h': printusage(argv[0]); return 1;
        case 'q': quiet=1; verbose=0; break;
        case 'v': verbose++; quiet=0; break;
        case '4': no_v6=1; break;
        case '6': no_v4=1; break;
        case 'i': inname=optarg; break;
        case 'n': queuemax=atoi(optarg); break;
        case 'o': {
            outlistensocks=realloc(outlistensocks,
                    (numoutlistensocks+1)*sizeof(struct lsockdescr));
            if (!outlistensocks) {
                LOG("alloc space for %d listensockets: %s\n",
                        numoutlistensocks+1, strerror(errno));
                return -1;
            }
            strncpy(outlistensocks[numoutlistensocks].service, optarg,
                    NI_MAXSERV);
            outlistensocks[numoutlistensocks].service[NI_MAXSERV-1]=0;
            outlistensocks[numoutlistensocks].p=-1;
            outlistensocks[numoutlistensocks].maxqueue=queuemax;
            numoutlistensocks++;
        } break;
        case 'l':
            logname=optarg;
            logfile=fopen(logname, "a");
            if (!logfile) {
                LOG("open logfile \"%s\": %s\n",
                        logname, strerror(errno));
                return -1;
            }
            log=logfile;
            break;
        default: err=1;
        }
    }
    if (err || argc-optind!=0) {
        printusage(argv[0]);
        return -1;
    }

    if (!inname || numoutlistensocks==0) {
        printusage(argv[0]);
        return -1;
    }

    return 0;
}
/******************************************************************************/
static void
accept_socket(struct lsockdescr *descr, struct sockdescr **socklist, int *num)
{
    struct sockaddr_storage addr;
    socklen_t addr_len;
    struct sockdescr *socklist_=*socklist;
    struct sockdescr *newdescr;
    int np, res, i;

    if (verbose>0) {
        LOG("connection on port %s\n", descr->service);
    }
    memset(&addr, 0, sizeof(struct sockaddr_storage));
    addr_len=sizeof(struct sockaddr_storage);
    np=accept(descr->p, (struct sockaddr*)&addr, &addr_len);
    if (np<0) {
        LOG("accept: %s\n", strerror(errno));
        return;
    }

    if (fcntl(np, F_SETFL, O_NDELAY)<0) {
        LOG("accept:fcntl(O_NDELAY): %s\n", strerror(errno));
        close(np);
        return;
    }

    /* try to use an old entry */
    newdescr=0;
    for (i=0; i<*num; i++) {
        if (socklist_[i].p<0) {
            if (verbose>1)
                LOG("accept_socket: will reuse entry %d\n", i);
            newdescr=socklist_+i;
            break;
        }
    }
    /* create a new entry */
    if (!newdescr) {
        socklist_=realloc(*socklist, (*num+1)*sizeof(struct sockdescr));
        if (!socklist_)   {
            LOG("accept_socket:realloc: %s\n", strerror(errno));
            return;
        }  
        newdescr=socklist_+*num;
        *socklist=socklist_;
        (*num)++;
    }

    /* fill out the new or recycled entry */
    memset(newdescr, 0, sizeof(*newdescr));
    newdescr->p=np;
    newdescr->maxqueue=descr->maxqueue;
    newdescr->data=0;
    newdescr->list=0;
    newdescr->idx=0;
    newdescr->stalled=0;

    res=getnameinfo((struct sockaddr*)&addr, addr_len,
            newdescr->host, NI_MAXHOST,
            newdescr->service, NI_MAXSERV, NI_NUMERICSERV);
    if (res) {
        LOG("accept:getnameinfo: %s\n", gai_strerror(res));
        strcpy(newdescr->host, "unknown");
        strcpy(newdescr->service, "unknown");
    } else if (verbose>0) {
        LOG("port %s accepted %s:%s\n",
                descr->service, newdescr->host, newdescr->service);
    }
}
/******************************************************************************/
static int
open_sockets(const char *name, struct lsockdescr **socklist, int *num,
        int maxqueue)
{
    struct addrinfo *addr, *a;
    struct addrinfo hints;
    struct lsockdescr *socklist_=*socklist;
    int n=0, res;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family=no_v6?AF_INET:AF_INET6;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_protocol=0;
    hints.ai_flags=AI_PASSIVE;
    if ((res=getaddrinfo(0, name, &hints, &addr))) {
        LOG("request addrinfo for \"%s\": %s\n",
            name, gai_strerror(res));
        goto error;
    }

    for (a=addr; a; a=a->ai_next) {
        int sock;
        if ((sock=socket(a->ai_family, a->ai_socktype, a->ai_protocol))<0) {
            LOG("create socket: %s\n", strerror(errno));
            continue;
        }

        if (bind(sock, a->ai_addr, a->ai_addrlen)<0) {
            LOG("bind socket to port %s: %s\n", name, strerror(errno));
            close(sock);
            continue;
        }

        if (listen(sock, 32)<0) {
            LOG("listen on port %s: %s\n", name, strerror(errno));
            close(sock);
            continue;
        }

        socklist_=realloc(*socklist, (*num+1)*sizeof(struct lsockdescr));
        if (!socklist_) {
            LOG("open_sockets:realloc: %s\n", strerror(errno));
            goto error;
        }
        *socklist=socklist_;
        strcpy((*socklist)[*num].service, name);
        (*socklist)[*num].p=sock;
        (*socklist)[*num].maxqueue=maxqueue;
        (*num)++;
        n++;
        if (verbose>0)
            LOG("port %s successfull opened\n", name);
        break;
    }

    freeaddrinfo(addr);

error:
    if (!n) {
        LOG("could not open any listening socket for port %s\n", name);
        return -1;
    } else {
        return 0;
    }
}
/******************************************************************************/
static struct data_descr*
alloc_data(void)
{
    struct data_descr *data;

    data=calloc(1, sizeof(struct data_descr));
    if (!data) {
        LOG("alloc data block: %s\n", strerror(errno));
        return 0;
    }
    data->opaque=malloc(4);
    if (!data->opaque) {
        LOG("alloc data block: %s\n", strerror(errno));
        free(data);
        return 0;
    }
    if (verbose>1)
        LOG("alloced: data=%p data->opaque=%p\n", data, data->opaque);
    data->space=4;
    return data;
}
/******************************************************************************/
static void
free_data(struct data_descr *data, const char *text)
{
    if (verbose>1) {
        LOG("will free data->opaque=%p data%p (%s)\n",
                data->opaque, data, text);
    }
    free(data->opaque);
    free(data);
}
/******************************************************************************/
/*
 * append a new data_descr to the end of list
 * if the list is now longer than maxqueue remove the first (oldest) entry
 */
static void
add_data_descr(struct sockdescr *sock, struct data_descr *entry)
{
    struct data_ent *ent;
    int n=0;

    ent=calloc(1, sizeof(struct data_ent));
    if (!ent) {
        LOG("alloc data_ent: %s\n", strerror(errno));
        return;
    }
    ent->data=entry;
    entry->refcount++;

    if (!sock->list) { /* list is empty */
        sock->list=ent;
    } else { /* list is not empty */
        struct data_ent *last_ent;
        /* find last entry */
        last_ent=sock->list; /* first entry */
        while (last_ent->next) {
            last_ent=last_ent->next;
            n++;
        }
        last_ent->next=ent;
    }
    if (sock->maxqueue>0 && n>=sock->maxqueue) { /* remove oldest entry */
        if (!sock->stalled) {
            if (verbose>1)
                LOG("output queue to %s:%s stalled\n",
                        sock->host, sock->service);
            sock->stalled=1;
        } 
        ent=sock->list;
        ent->data->refcount--;
        if (!ent->data->refcount)
            free_data(ent->data, "maxqueue");
        sock->list=ent->next;
        free(ent);
    }
}
/******************************************************************************/
/*
 * store a pointer to the new data in the list of each outsock descriptor
 */
static void
store_data(struct data_descr *data)
{
    int i;

    for (i=0; i<numoutsocks; i++) {
        if (outsocks[i].p>=0)
            add_data_descr(outsocks+i, data);
    }

    if (verbose>1)
        LOG("store_data: refcount=%d\n", data->refcount);
    if (!data->refcount)
        free_data(data, "store_data");
}
/******************************************************************************/
static void
do_read(struct sockdescr *sock)
{
    struct data_descr *data;
    ssize_t res;

    if (verbose>1) {
        LOG("read from %s:%s, data=%p\n",
                sock->host, sock->service, sock->data);
        if (sock->data) {
             LOG(" size=%d idx=%d space=%llu opaque=%p\n",
                sock->data->size,
                sock->idx,
                (unsigned long long)sock->data->space,
                sock->data->opaque);
        }
    }

    /* if necessary: alloc initial data block */
    if (!sock->data) {
        sock->idx=0;
        sock->data=alloc_data();
        if (!sock->data)
            goto error;
    }

    data=sock->data;

    /* if necessary: read size */
    if (!data->size) { /* 4 bytes for size are preallocated */
        if (verbose>1)
            LOG("will read %d bytes for size\n", 4-sock->idx);
        res=read(sock->p, data->opaque+sock->idx, 4-sock->idx);
        if (res<=0) {
            if (res==0) {
                errno=EPIPE;
                res=-1;
            }
            if (errno==EINTR || errno==EAGAIN)
                return;
            if (verbose || errno!=EPIPE)
                LOG("read head from %s:%s: %s\n",
                        sock->host, sock->service, strerror(errno));
            goto error;
        }
        if (verbose>1)
            LOG("got %lld bytes\n", (long long)res);
        sock->idx+=res;
        if (sock->idx==4) {
            data->size=
                (data->opaque[0]<<24) |
                (data->opaque[1]<<16) |
                (data->opaque[2]<<8) |
                data->opaque[3];
            /* round to next 4 byte */
            if (verbose>1)
                LOG("opaque size is %d\n", data->size);
            if (data->size & ~MAXSIZE) {
                LOG("size %d is too large\n", data->size);
                goto error;
            }
            data->size=((data->size+3)/4)*4;
            if (verbose>1)
                LOG("have to read %d bytes\n", data->size);
            if (!data->size) { /* empty block ignored */
                sock->idx=0;
                return;
            }
        } else {
            return;
        }
    }

    /* if necessary: alloc space for data */
    if (data->space<data->size+4) {
        u_int8_t *opaque;
        if (verbose>1)
            LOG("realloc %d bytes\n", data->size+4);
        opaque=realloc(data->opaque, data->size+4);
        if (verbose>1)
            LOG("realloc data: %p --> %p\n", data->opaque, opaque);
        if (!opaque) {
            LOG("alloc %d bytes for %s:%s: %s\n",
                    data->size+4,
                    sock->host, sock->service,
                    strerror(errno));
            goto error;
        }
        data->opaque=opaque;
        data->space=data->size+4;
    }

    /* read data */
    if (verbose>1)
        LOG("will read %d bytes\n", data->size-(sock->idx-4));
    res=read(sock->p, data->opaque+sock->idx, data->size-(sock->idx-4));
    if (res<=0) {
        if (res==0) {
            errno=EPIPE;
            res=-1;
        }
        if (errno==EINTR || errno==EAGAIN)
            return;
        if (verbose || errno!=EPIPE)
            LOG("read data from %s:%s: %s\n",
                    sock->host, sock->service, strerror(errno));
        goto error;
    }
    sock->idx+=res;
    if (sock->idx==data->size+4) { /* data complete */
        if (verbose>1) {
            LOG("received from %s:%s: %d bytes\n",
                    sock->host, sock->service, data->size);
            //LOG("text: >%s<\n", data->opaque+4);
        }
        store_data(data);
        sock->data=0;
    }

    return;

error:
    close(sock->p);
    free_data(data, "do_read/(error or broken)");
    sock->p=-1;
}
/******************************************************************************/
static void
do_write(struct sockdescr *sock)
{
    int res;

    if (!sock->data) {
        struct data_ent *ent=sock->list;
        if (!ent) {
            if (verbose>1)
                LOG("do_write: no data available\n");
                sleep(5);
                *(int*)0=0;
            return;
        }
        sock->data=ent->data;
        sock->list=ent->next;
        free(ent);
        sock->idx=0; /* no bytes written yet */
    }

    res=write(sock->p, sock->data->opaque+sock->idx,
            sock->data->size+4-sock->idx);
    if (res<=0) {
        if (errno==EINTR || errno==EAGAIN) {
            return;
        } else {
            if (verbose || errno!=EPIPE)
                LOG("write to %s:%s: %s\n", sock->host, sock->service,
                        strerror(errno));
            sock->data->refcount--;
            if (!sock->data->refcount)
                free_data(sock->data, "do_write/error/data");
            while (sock->list) {
                struct data_ent *ent=sock->list;
                ent->data->refcount--;
                if (!ent->data->refcount)
                    free_data(ent->data, "do_write/error/list");
                sock->list=ent->next;
                free(ent);
                
            }
            close(sock->p);
            sock->p=-1;
            return;
        }
    }
    sock->idx+=res;
    sock->stalled=0;

    if (sock->idx==sock->data->size+4) { /* all data sent */
        sock->data->refcount--;
        if (!sock->data->refcount)
            free_data(sock->data, "do_write/finished");
        sock->data=0;
    }
}
/******************************************************************************/
static void
do_select(void)
{
    fd_set rfds, wfds;
    int nfds=0, res, i;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    for (i=0; i<numinlistensocks; i++) {
        FD_SET(inlistensocks[i].p, &rfds);
        if (inlistensocks[i].p>nfds)
            nfds=inlistensocks[i].p;
    }
    for (i=0; i<numoutlistensocks; i++) {
        FD_SET(outlistensocks[i].p, &rfds);
        if (outlistensocks[i].p>nfds)
            nfds=outlistensocks[i].p;
    }
    for (i=0; i<numinsocks; i++) {
        if (insocks[i].p>=0) {
            FD_SET(insocks[i].p, &rfds);
            if (insocks[i].p>nfds)
                nfds=insocks[i].p;
        }
    }
    for (i=0; i<numoutsocks; i++) {
        if (outsocks[i].p>=0 && (outsocks[i].data || outsocks[i].list)) {
            FD_SET(outsocks[i].p, &wfds);
            if (outsocks[i].p>nfds)
                nfds=outsocks[i].p;
        }
    }
    nfds++;

    res=select(nfds, &rfds, &wfds, 0, 0);
    if (res<0) {
        LOG("select: %s\n", strerror(errno));
        return;
    }

    for (i=0; i<numinlistensocks; i++) {
        if (FD_ISSET(inlistensocks[i].p, &rfds)) {
            accept_socket(inlistensocks+i, &insocks, &numinsocks);
        }
    }
    for (i=0; i<numoutlistensocks; i++) {
        if (FD_ISSET(outlistensocks[i].p, &rfds)) {
            accept_socket(outlistensocks+i, &outsocks, &numoutsocks);
        }
    }
    for (i=0; i<numinsocks; i++) {
        if (insocks[i].p>=0 && FD_ISSET(insocks[i].p, &rfds)) {
                do_read(insocks+i);
        }
    }
    for (i=0; i<numoutsocks; i++) {
        if (outsocks[i].p>=0) {
            if (FD_ISSET(outsocks[i].p, &wfds))
                do_write(outsocks+i);
        }
    }
}
/******************************************************************************/
#if 0
static void
close_all_sockets(void)
{
    int i;

    for (i=0; i<numinlistensocks; i++)
        if (shutdown(inlistensocks[i].p, SHUT_RDWR)<0)
            perror("shutdown");
    for (i=0; i<numoutlistensocks; i++)
        if (shutdown(outlistensocks[i].p, SHUT_RDWR)<0)
            perror("shutdown");
    for (i=0; i<numinsocks; i++)
        if (shutdown(insocks[i].p, SHUT_RDWR)<0)
            perror("shutdown");
    for (i=0; i<numoutsocks; i++)
        if (shutdown(outsocks[i].p, SHUT_RDWR)<0)
            perror("shutdown");

    sleep(2);

    for (i=0; i<numinlistensocks; i++)
        if (close(inlistensocks[i].p)<0)
            perror("close");
    for (i=0; i<numoutlistensocks; i++)
        if (close(outlistensocks[i].p)<0)
            perror("close");
    for (i=0; i<numinsocks; i++)
        if (close(insocks[i].p)<0)
            perror("close");
    for (i=0; i<numoutsocks; i++)
        if (close(outsocks[i].p)<0)
            perror("close");
}
#endif
/******************************************************************************/
static void
sighand(int sig)
{
    int_received=1;
}
/******************************************************************************/
int
main(int argc, char *argv[])
{
    struct sigaction sa;
    int res;

    log=stderr;

    if ((res=readargs(argc, argv)))
        return res<0?1:0;

    /* install handlers for SIGINT and SIGTERM signal */
    int_received=0;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sighand;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        LOG("Could not install SIGINT handler.\n");
        return 2;
    }
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        LOG("Could not install SIGTERM handler.\n");
        return 2;
    }
    /* install an ignore handler for SIGPIPE. */
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) < 0) {
        LOG("Could not install SIGPIPE handler.\n");
        return 5;
    }

    inlistensocks=0;
    numinlistensocks=0;
    if (open_sockets(inname, &inlistensocks, &numinlistensocks, 0)<0)
        return 2;

    {
        int numsocks=numoutlistensocks, i;
        struct lsockdescr *socks=outlistensocks;

        numoutlistensocks=0;
        outlistensocks=0;

        for (i=0; i<numsocks; i++) {
            if (open_sockets(socks[i].service, &outlistensocks,
                    &numoutlistensocks, socks[i].maxqueue)<0)
                return 2;
        }
        free(socks);
    }

    while (!int_received) {
        do_select();
    }

    LOG("main loop finished\n");
#if 0
    close_all_sockets();
#endif

    return 0;
}
/******************************************************************************/
/******************************************************************************/