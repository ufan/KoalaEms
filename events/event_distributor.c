/*
 * event_distributor.c
 * 
 * created before 2003
 * $ZEL: event_distributor.c,v 1.5 2013/01/14 16:48:49 wuestner Exp $
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

typedef unsigned int ems_u32;

// Macro for swap the byte order
#define SWAP_32(n) (((n & 0xff000000) >> 24) | \
    ((n & 0x00ff0000) >>  8) | \
    ((n & 0x0000ff00) <<  8) | \
    ((n & 0x000000ff) << 24))

struct event_buffer {
  size_t size; // size of the cluster, including the header
  char* data; // physical memory for buffering
  int number_of_users; // number of users waiting for the data of this cluster
};

struct input_event_buffer {
  struct event_buffer* event_buffer; // buffer for the cluster
  ems_u32 head[2]; // store the cluster header: size + 0x12345678
  size_t position; // pointer poisition for the next reading in the buffer
  int valid; // whether a full cluster has been read into the buffer
};

struct sock {
  struct event_buffer* event_buffer; // the cluster buffer currently associated with this output socket
  size_t position; // the pointer position for the next writing to the ouput socket in the buffer
  struct sockaddr_in address; // the address associated with this output socket
  int p; // the file descriptor associated with this output socket
};

// Three types of input stream are implemented.
// 1) intype_connect: serving as a client, need hostname+portnum to connect to the input data stream server actively
// 2) intype_accept: serving as a server, only need portnum to open a service in this host and waiting for the input data stream to connect passively
// 3) intype_stdin: the input data stream coming from stdin (file descriptor = 0)
typedef enum {intype_connect, intype_accept, intype_stdin} intypes;
typedef enum {swaptype_check, swaptype_always, swaptype_never} swaptypes;
struct sock** socks; // array of the output sockets
int num_socks, max_socks; // number of the current length of socks defined above

// program options:
// 1) quiet: serving as a verbose option, default is off
// 2) old:   using old format, default is off
// 3) close_old: in intype_accept mode, closing old input socket when a new request accepted, the default is off
// 4) swaptype: The program will interpret the header words to get the cluster size, thus need to care about the byte order. For the new format (2 words header, with 0x12345678 flag), the byte order is self-explanatory. For the old format, the user need to set the swaptype to decide wether do the byte swap or not.
// 5) intype: type of the datain stream, it depends on the no-option arguments that user passes in. If the inport contain hostname:port, then it's intype_connect; if the inport contains only portnum or portname, then its intype_accept; if the inport is "-", then it's intype_stdin.
int quiet, old, close_old;
intypes intype;
swaptypes swaptype;

// variables to store the in/out socket parameters.
char *inname, *outname;
int inport, outport;
unsigned int inhostaddr;

/******************************************************************************/
static void
printusage(char* argv0)
{
    printf("usage: %s [-h] [-d] [-q] [-o] [-c] [-s 0|1] [inport [outport]]\n",
        basename(argv0));
    printf("  -h: this help\n");
    printf("  -q: quiet (no informational output)\n");
    printf("  -c: close old datainput if new connection arrives\n");
    printf("  -o: use old event format\n");
    printf("  -s 0: swap never 1: swap always (old format only)\n");
    printf("  inport: [hostname:](portnum|portname) or '-' for stdin\n");
    printf("    default is \"evdistin\"\n");
    printf("  outport: portnum|portname\n");
    printf("    default is \"evdistout\"\n");
}
/******************************************************************************/
static int
readargs(int argc, char* argv[])
{
    int c, err;
    //struct servent* service;

    quiet=old=close_old=0;
    swaptype=swaptype_check;
    err=0;
    while (!err && ((c=getopt(argc, argv, "hqocs:")) != -1)) {
        switch (c) {
        case 'h': printusage(argv[0]); return 1;
        case 'q': quiet=1; break;
        case 'o': old=1; break;
        case 'c': close_old=1; break;
        case 's': {
                char* end;
                int swap=strtol(optarg, &end, 0);
                if (*end!=0) {
                    printf("cannot convert \"%s\" to integer.\n", optarg);
                    printusage(argv[0]); return 1;
                }
                swaptype=swap?swaptype_always:swaptype_never;
            }
            break;
        default: err=1;
        }
    }
    if (err) {
        printusage(argv[0]);
        return -1;
    }

    if (optind<argc)
        inname=argv[optind];
    else
        inname="evdistin";
    optind++;
    if (optind<argc)
        outname=argv[optind];
    else
        outname="evdistout";
    return 0;
}
/******************************************************************************/
// signal handler for SIGPIPE. Just print out the verbose message to tell the user
// that no connected output sockets are available right now, when the datain is active
static void
sigpipe(int num)
{
    if (!quiet) printf("sigpipe received\n");
}
/******************************************************************************/
static const char*
conn2string(struct sockaddr_in* addr)
{
    static char s[1024];
    snprintf(s, 1024, "%s:%d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
    return s;
}
/******************************************************************************/
static int
do_accept(int p, struct sockaddr_in* address, char *dir)
{
    unsigned int len;
    int ns;

    len=sizeof(struct sockaddr_in);
    ns=accept(p, (struct sockaddr*)address, &len);
    if (ns<0) {
        printf("accept: %s\n", strerror(errno));
        return -1;
    }
    if (!quiet) printf("%s accepted from %s\n", dir, conn2string(address));
    if (fcntl(ns, F_SETFL, O_NDELAY)==-1) {
        printf("fcntl O_NDELAY: %s\n", strerror(errno));
        close(ns);
        return -1;
    }
    return ns;
}
/******************************************************************************/
static void
clear_new_event(struct input_event_buffer* buf)
{
    if (buf->event_buffer && !buf->event_buffer->number_of_users) {
        free(buf->event_buffer->data);
        free(buf->event_buffer);
    }
    buf->event_buffer=0;
    buf->position=0;
    buf->valid=0;
}
/******************************************************************************/
static struct event_buffer*
create_event_buffer(size_t size)
{
    struct event_buffer* buf;
    if ((buf=(struct event_buffer*)malloc(sizeof(struct event_buffer)))==0) {
        printf("malloc struct event_buffer: %s\n", strerror(errno));
        return 0;
    }
    if ((buf->data=(char*)malloc(size))==0) {
        printf("malloc data (%llu byte): %s\n", (unsigned long long)size,
                strerror(errno));
        free(buf);
        return 0;
    }
    buf->number_of_users=0;
    return buf;
}
/******************************************************************************/
static int
do_read(int pin, struct input_event_buffer* buf)
{
    int res;
    size_t n=0;
    int HEADERSIZE=old?sizeof(ems_u32):2*sizeof(ems_u32);

    if (!buf->event_buffer) { /* read header and prepare buffer */
        res=read(pin, ((char*)buf->head)+buf->position, HEADERSIZE-buf->position);
        if (res<0) {
            if (errno==EINTR) return 0; /* try later */
            if (!quiet) printf("read header: %s\n", strerror(errno));
            return -1; /* error, don't try again */
        } else if (res==0) {
            if (!quiet) printf("read header: %s\n", strerror(EPIPE));
            return -1; /* error, don't try again */
        }
        buf->position+=res;
        if (buf->position<HEADERSIZE) return 0; /* try later */

        if (old) {
            switch (swaptype) {
            case swaptype_check:
                n=(buf->head[0]&0xffff0000)?
                        SWAP_32(buf->head[0]):
                        buf->head[0];
                break;
            case swaptype_always:
                n=SWAP_32(buf->head[0]);
                break;
            case swaptype_never:
                n=buf->head[0];
                break;
            }
        } else {
            switch (buf->head[1]) {
            case 0x12345678: n=buf->head[0]; break;
            case 0x78563412: n=SWAP_32(buf->head[0]); break;
            default:
                printf("unknown endien 0x%08x\n", buf->head[1]);
                return -1;
            }
        }

        n=(n+1)*sizeof(ems_u32);
        if ((buf->event_buffer=create_event_buffer(n))==0) {
            return -1;
        }

        buf->event_buffer->size=n;
        buf->position=HEADERSIZE;
        bcopy((char*)buf->head, buf->event_buffer->data, HEADERSIZE);
    }

    /* header is complete, read the data */
    res=read(pin, buf->event_buffer->data+buf->position,
    buf->event_buffer->size-buf->position);
    if (res<0) {
        if ((errno==EINTR)||(errno==EWOULDBLOCK)) return 0; /* try later */
        if (!quiet) printf("read data: %s\n", strerror(errno));
        return -1; /* error, don't try again */
    } else if (res==0) {
        if (!quiet) printf("read data returns 0\n");
        return -1;
    }
    buf->position+=res;
    if (buf->position<buf->event_buffer->size) return 0; /* try later */

    /* data are complete */
    buf->valid=1;
    return 0;
}
/******************************************************************************/
static int
do_write(struct sock* sock)
{
    int res;

    res=write(sock->p, sock->event_buffer->data+sock->position,
            sock->event_buffer->size-sock->position);
    if (res<0) {
        if (errno==EINTR) return 0; /* try later */
        if (!quiet) printf("write to %s: %s\n", conn2string(&sock->address),
                strerror(errno));
        return -1; /* error, don't try again */
    } else if (res==0) {
        if (!quiet) printf("write to %s returns 0\n",
                conn2string(&sock->address));
        return -1; /* error, don't try again */
    }
    sock->position+=res;
    if (sock->position<sock->event_buffer->size) return 0; /* try later */

    /* all sent */

    if (!--sock->event_buffer->number_of_users) {
        free(sock->event_buffer->data);
        free(sock->event_buffer);
    }
    sock->event_buffer=0;
    return 0;
}
/******************************************************************************/
static int
create_listening_socket (int listening_port)
{
    int listening_socket;
    struct sockaddr_in address;

    if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0))<0) {
        printf("create listening_socket: %s\n", strerror(errno));
        return -2;
    }

    bzero((void*)&address, sizeof(struct sockaddr_in));
    address.sin_family=AF_INET;
    address.sin_port=htons(listening_port);
    address.sin_addr.s_addr=htonl(INADDR_ANY);

    if ((bind(listening_socket, (struct sockaddr*)&address,
                sizeof(struct sockaddr_in)))<0) {
        printf("bind listening_socket: %s\n", strerror(errno));
        return -3;
    }

    if ((listen(listening_socket, 1))<0) {
        printf("listen listening_socket: %s\n", strerror(errno));
        return -4;
    }

    return listening_socket;
}
/******************************************************************************/
static int
create_connecting_socket (unsigned int host, int port)
{
    int sock, res;
    struct sockaddr_in address;

    if ((sock=socket(AF_INET, SOCK_STREAM, 0))<0) {
        printf("create connecting_socket: %s\n", strerror(errno));
        return -1;
    }

    if (fcntl(sock, F_SETFL, O_NDELAY)==-1) {
        printf("fcntl(O_NDELAY) connecting_socket: %s\n", strerror(errno));
        return -1;
    }

    bzero((void*)&address, sizeof(struct sockaddr_in));
    address.sin_family=AF_INET;
    address.sin_port=htons(port);
    address.sin_addr.s_addr=host;

    res=connect(sock, (struct sockaddr*)&address, sizeof(struct sockaddr_in));
    if ((res==0) || (errno==EINPROGRESS)) return sock;

    printf("connect inputsocket: %s\n", strerror(errno));
    close(sock);
    return -1;
}
/******************************************************************************/
static int
is_connected(int sock)
{
    int opt, res;
    unsigned int len;

    len=sizeof(opt);
    if ((res=getsockopt(sock, SOL_SOCKET, SO_ERROR, &opt, &len))<0) {
        printf("getsockopt(SO_ERROR): %s\n", strerror(errno));
        return -1;
    }
    if (opt==0) {
        if (!quiet) printf("insocket connected.\n");
        return 1;
    }
    if (!quiet) printf("connect insocket: %s\n", strerror(opt));
    return -1;
}
/******************************************************************************/
static int
accept_insock(int insock_l)
{
    struct sockaddr_in address;
    return do_accept(insock_l, &address, "insocket");
}
/******************************************************************************/
static void
accept_outsock(int outsock_l)
{
    struct sockaddr_in address;
    int ns;
    struct sock* sock;

    ns=do_accept(outsock_l, &address, "outsocket");
    if (ns<0) return;

    sock=malloc(sizeof(struct sock));
    if (sock==0) {
        printf("malloc struct sock: %s\n", strerror(errno));
        close(ns);
        return;
    }

    sock->event_buffer=0;
    sock->address=address;
    sock->p=ns;

    if (num_socks==max_socks) {
        int i;
        struct sock** help=malloc((max_socks+10)*sizeof(struct sock*));
        if (help==0) {
            printf("malloc %d sock_struct*: %s\n", max_socks+10, strerror(errno));
            close(sock->p);
            free(sock);
            return;
        }
        for (i=0; i<num_socks; i++) help[i]=socks[i];
        free(socks);
        socks=help;
    }
    socks[num_socks++]=sock;
}
/******************************************************************************/
static void
delete_outsock(int idx)
{
    int i;

    close(socks[idx]->p);
    if (socks[idx]->event_buffer && !--socks[idx]->event_buffer->number_of_users) {
        free(socks[idx]->event_buffer->data);
        free(socks[idx]->event_buffer);
    }
    free(socks[idx]);
    num_socks--;
    for (i=idx; i<num_socks; i++) socks[i]=socks[i+1];
}
/******************************************************************************/
static void
main_loop(int insock_l, int outsock_l)
{
    fd_set readfds, writefds;

    struct timeval last;
    last.tv_sec=0;

    // ibs is the intial buffer used to store the lates cluster data from datain
    struct input_event_buffer ibs;
    ibs.event_buffer=0; clear_new_event(&ibs);

    // insock is the real connected socket descriptor, which can be used for input/output.
    // It's also an flag showing whether a valid input connection exists (insock >= 0), if the connection does not exist or malfunction insock=-1.
    // insock_l is just the listening socket in the intype_accept mode. insock_l=-1 in intype_connect and intype_stdin mode. Also, in insock_connect type, if insock_l>0, it is a socket descriptor which has sent out a connection request and waited for the answer currently; if insock_l=-1, depending on the value of insock, it means the no connection request is sent out or the connection has already established.
    int insock; 
    if (intype==intype_stdin)
        insock=0;
    else
        insock=-1;

    num_socks=0; max_socks=0; socks=0;

    while(1) {
        int nfds, idx, need_data, res;
        struct timeval to, *timeout;

        ///////////////////////////////////////////////////////////////
        // Configure the checking conditions for the select function //

        // intialization for select function
        timeout=0;
        nfds=-1;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        // always check whether new connection request is pending for the output socket and for the datain socket in the intype_accept mode
        if (intype==intype_accept) {
            FD_SET(insock_l, &readfds);
            nfds=insock_l;
        }
        FD_SET(outsock_l, &readfds);
        if (outsock_l>nfds) nfds=outsock_l;

        // Checking whether need new data or not:
        // First, check whether there're output sockets which has not finished transferring the current cluster
        // In this process, also check whether there is output socket finishing the transfering. If there exists any one of the output sockets which has finished transfering the current cluster, the flag "need_data" is set so that a new cluster will be read from the datain socket.
        // Second, in intyp_connect mode, if the input connection has not been established yet, the connection request is sent out in this step; if the the request has been sent, then set the flag to check whether the connection request has been accepted or not.
        need_data=0;
        for (idx=0; idx<num_socks; idx++) {
            if (socks[idx]->event_buffer) { // if the associated buffer exists, then there is remaining data to be sent
                FD_SET(socks[idx]->p, &writefds);
                if (socks[idx]->p>nfds) nfds=socks[idx]->p;
            } else {
                need_data=1; // if any one output socket has finished tranfering one cluster, then new cluster should be requested from the datain socket.
            }
        }
        if (need_data) {
            if (insock>=0) {// valid input connection has already been established
                FD_SET(insock, &readfds);
                if (insock>nfds) nfds=insock;
            } else if (intype==intype_connect) { // valid input connection does not exist and the mode is intype_connet
                if (insock_l<0) { // the connection request has not been sent out
                    struct timeval now;
                    gettimeofday(&now, 0);
                    if (now.tv_sec-last.tv_sec>60) { // send the connection request every 60s
                        last=now;
                        insock_l=create_connecting_socket(inhostaddr, inport);
                    } else {
                        to.tv_sec=10; to.tv_usec=0; timeout=&to; // if the connection request has been sent out, the block 10s to wait for the result of this request
                    }
                }
                if (insock_l>=0) { // the connection request has been sent out successfully
                    FD_SET(insock_l, &writefds);
                    if (insock_l>nfds) nfds=insock_l;
                }
            }
        }

        // check the status of all the related sockets
        // if timeout=0, select will return immediately; otherwise, it will block at most timeout time.
        res=select(nfds+1, &readfds, &writefds, 0, timeout);
        if (res<0) {
            if (errno==EINTR) continue;
            printf("select: %s\n", strerror(errno));
            return;
        }
        if (res==0) continue;

        ////////////////////////////////////////////////////
        // Operations when the sockets have status change //

        /* Output realted operations */
        for (idx=0; idx<num_socks; idx++) {
            if (FD_ISSET(socks[idx]->p, &writefds)) {
                if (do_write(socks[idx])<0) {
                    delete_outsock(idx);
                    idx--;
                }
            }
        }

        /* Connections related operations*/
        if (insock_l>=0) {
            if (intype==intype_accept) {
              if (FD_ISSET(insock_l, &readfds)) { // check whether there exist connection request in intyp_accept mode
                    int ns=accept_insock(insock_l);
                    if (ns>=0) {
                        if (insock<0) { // accept this socket as the datain socket if the connection has not been established before
                            insock=ns;
                        } else { // a connection has already existed
                            if (close_old) { // if close_old flag is set, the previous connection will be replaced by this new connection. And the associated buffer will be cleaned as well.
                                if (!quiet) printf("new insocket accepted.\n");
                                clear_new_event(&ibs);
                                close(insock);
                                insock=ns;
                            } else { // ignore the new connection, keep the original one
                                if (!quiet) printf("insocket already connected.\n");
                                close(ns);
                            }
                        }
                    }
                }
            } else if (intype==intype_connect) { // check whether the connection request answer in intyp_connect mode
                if (FD_ISSET(insock_l, &writefds)) {
                    switch (is_connected(insock_l)) {
                    case 1: insock=insock_l; insock_l=-1; break; // if connected, insock is updated with the valid socket discriptor and insock_l back to -1
                    case -1: close(insock_l); insock_l=-1;break;
                    /* default: still in progress */
                    }
                }
            }
        }
        // Always accecpt new output socket connection.
        // Once a new output socket connection is accepted and established, a new sock data structure will be created and the num_socks will increase by 1.
        if (FD_ISSET(outsock_l, &readfds)) accept_outsock(outsock_l);

        /* Input related operations */
        // 1. If need_data is true, the new cluster data will be read from datain and stored in a new buffer at this step
        // 2. When the new full cluster has been readout completely (ibs.valid=1), the output sockets, which have finished transfering the previous cluster at this point, will be assigned this new buffer. CAUTION: the output sockets which has not finished the previous transfering will never get this new cluster data.
        // 3. The ownership of the newly created buffer is transferred to the group of ouput sockets which are associated with this new buffer immediately. So the data buffer of the input socket is only valid in this step. In other times, the input data buffer is empty or partial filled.
        // 4. If the datain socket is datain, then it's not error-tolerant. If the datain socket is network socket, then it will close the current socket and wait for the next connection if error occured.
        if ((insock>=0) && FD_ISSET(insock, &readfds)) {
            res=do_read(insock, &ibs);
            if (res<0) {
                clear_new_event(&ibs);
                if (intype==intype_stdin) {
                    printf("error reading stdin; giving up.\n");
                    return;
                } else {
                    close(insock);
                    insock=-1;
                }
            } else {
                if (ibs.valid) {
                    for (idx=0; idx<num_socks; idx++) {
                        if (!socks[idx]->event_buffer) {
                            socks[idx]->event_buffer=ibs.event_buffer;
                            socks[idx]->position=0;
                            ibs.event_buffer->number_of_users++;
                        }
                    }
                    if (!ibs.event_buffer->number_of_users)
                            printf("new event_buffer unused!\n");
                    clear_new_event(&ibs);
                }
            }
        }
    }
}
/******************************************************************************/
static int
portname2portnum(char* name)
{
    struct servent* service;
    int port;

    if ((service=getservbyname(name, "tcp"))) {
        port=ntohs(service->s_port);
    } else {
        char* end;
        port=strtol(name, &end, 0);
        if (*end!=0) {
            printf("cannot convert \"%s\" to integer.\n", name);
            port=-1;
        }
    }
    return port;
}
/******************************************************************************/
int
main(int argc, char *argv[])
{
    int insock_l, outsock_l;
    char *p;

    if (readargs(argc, argv)) return 1;

    printf("inname=%s; outname=%s\n", inname, outname);

    if (strcmp(inname, "-")==0) {
        intype=intype_stdin;
    } else if ((p=index(inname, ':'))) {
        struct hostent *host;
        char hostname[1024];
        int idx=p-inname;
        intype=intype_connect;
        strncpy(hostname, inname, idx);
        hostname[idx]=0;
        if ((inport=portname2portnum(p+1))<0) return 1;
        host=gethostbyname(hostname);
        if (host!=0) {
            inhostaddr=*(unsigned int*)(host->h_addr_list[0]);
        } else {
            inhostaddr=inet_addr(hostname);
            if (inhostaddr==-1) {
                printf("unknown host: %s\n", hostname);
                return 1;
            }
        }
    } else {
        intype=intype_accept;
        if ((inport=portname2portnum(inname))<0) return 1;
    }

    if ((outport=portname2portnum(outname))<0) return 1;

    if (!quiet) {
        printf("using ");
        switch (intype) {
        case intype_stdin:
            printf("stdin");
            break;
        case intype_connect:
            printf("%d.%d.%d.%d:%d",
                    inhostaddr&0xff, (inhostaddr>>8)&0xff,
                    (inhostaddr>>16)&0xff, (inhostaddr>>24)&0xff, inport);
            break;
        case intype_accept:
            printf("port %d", inport);
            break;
        }
        printf(" for input and port %d for output\n", outport);
    }

    /*signal (SIGINT, *sigact);*/
    /*signal (SIGPIPE, SIG_IGN);*/
    signal (SIGPIPE, sigpipe);

    if (intype==intype_accept) {
        if ((insock_l=create_listening_socket(inport))<0) {
            printf("create_listening_socket input: %s\n", strerror(errno));
            return insock_l;
        }
    } else {
        insock_l=-1;
    }
    if ((outsock_l = create_listening_socket(outport))<0) {
        printf("create_listening_socket output: %s\n", strerror(errno));
        return outsock_l;
    }

    main_loop (insock_l, outsock_l);
    return 0;
}
/******************************************************************************/
/******************************************************************************/
