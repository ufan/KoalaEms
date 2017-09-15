/*
 * event_distributor.c
 * 
 * created before 2003
 * $ZEL: event_distributor_old.c,v 1.1 2010/09/10 22:46:32 wuestner Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

typedef int uint_32;

typedef struct
  {
  size_t size;
  char* data;
  int number_of_users;
  } event_buffer_struct;

typedef struct
  {
  event_buffer_struct* event_buffer;
  uint_32 head[2];
  size_t position;
  int valid;
  } input_event_buffer_struct;

typedef struct
  {
  event_buffer_struct* event_buffer;
  size_t position;
  struct sockaddr_in address;
  int p;
  } sock_struct;

typedef enum {intype_connect, intype_accept, intype_stdin} intypes;
typedef enum {swaptype_check, swaptype_always, swaptype_never} swaptypes;
sock_struct** socks;
int num_socks, max_socks;
int quiet, old, close_old;
intypes intype;
swaptypes swaptype;
int inport, outport;
unsigned int inhostaddr;

/******************************************************************************/
unsigned int swap_bytes (unsigned int n)
{
return ((n & 0xff000000) >> 24) |
       ((n & 0x00ff0000) >>  8) |
       ((n & 0x0000ff00) <<  8) |
       ((n & 0x000000ff) << 24);
}
/******************************************************************************/
void sigpipe (int num)
{
if (!quiet) fprintf(stderr, "sigpipe received\n");
}
/******************************************************************************/
const char* conn2string(struct sockaddr_in* addr)
{
static char s[1024];
snprintf(s, 1024, "%s:%d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
return s;
}
/******************************************************************************/
int do_accept(int p, struct sockaddr_in* address, char *dir)
{
int ns, len;

len = sizeof(struct sockaddr_in);
ns = accept(p, (struct sockaddr*)address, &len);
if (ns<0)
  {
  perror("accept");
  return -1;
  }
if (!quiet) fprintf(stderr, "%s accepted from %s\n", dir, conn2string(address));
if (fcntl(ns, F_SETFL, O_NDELAY)==-1)
  {
  perror("fcntl O_NDELAY");
  close(ns);
  return -1;
  }
return ns;
}
/******************************************************************************/
void clear_new_event(input_event_buffer_struct* buf)
{
if (buf->event_buffer && !buf->event_buffer->number_of_users)
  {
  free(buf->event_buffer->data);
  free(buf->event_buffer);
  }
buf->event_buffer=0;
buf->position=0;
buf->valid=0;
}
/******************************************************************************/
event_buffer_struct* create_event_buffer(size_t size)
{
event_buffer_struct* buf;
if ((buf=(event_buffer_struct*)malloc(sizeof(event_buffer_struct)))==0)
  {
  perror("malloc event_buffer_struct:");
  return 0;
  }
if ((buf->data=(char*)malloc(size))==0)
  {
  fprintf(stderr, "malloc data (%d byte): %s\n", size, strerror(errno));
  free(buf);
  return 0;
  }
buf->number_of_users=0;
return buf;
}
/******************************************************************************/
int do_read(int pin, input_event_buffer_struct* buf)
{
int res;
size_t n;
int HEADERSIZE=old?sizeof(uint_32):2*sizeof(uint_32);

if (!buf->event_buffer) /* read header and prepare buffer */
  {
  res=read(pin, ((char*)buf->head)+buf->position, HEADERSIZE-buf->position);
  if (res<0)
    {
    if (errno==EINTR) return 0; /* try later */
    if (!quiet) fprintf(stderr, "read header: %s\n", strerror(errno));
    return -1; /* error, don't try again */
    }
  else if (res==0)
    {
    if (!quiet) fprintf(stderr, "read header: %s\n", strerror(EPIPE));
    return -1; /* error, don't try again */
    }
  buf->position+=res;
  if (buf->position<HEADERSIZE) return 0; /* try later */

  if (old)
    {
    switch (swaptype)
      {
      case swaptype_check:
          n=(buf->head[0]&0xffff0000)?swap_bytes(buf->head[0]):buf->head[0];
          break;
      case swaptype_always: n=swap_bytes(buf->head[0]); break;
      case swaptype_never:  n=buf->head[0]; break;
      }
    }
  else
    {
    switch (buf->head[1])
      {
      case 0x12345678: n=buf->head[0]; break;
      case 0x78563412: n=swap_bytes(buf->head[0]); break;
      default:
        fprintf(stderr, "unknown endien 0x%08x\n", buf->head[1]);
        return -1;
      }
    }

  n=(n+1)*sizeof(uint_32);
  if ((buf->event_buffer=create_event_buffer(n))==0)
    {
    return -1;
    }

  buf->event_buffer->size=n;
  buf->position=HEADERSIZE;
  bcopy((char*)buf->head, buf->event_buffer->data, HEADERSIZE);
  }

/* header is complete, read the data */
res=read(pin, buf->event_buffer->data+buf->position,
    buf->event_buffer->size-buf->position);
if (res<0)
  {
  if ((errno==EINTR)||(errno==EWOULDBLOCK)) return 0; /* try later */
  if (!quiet) fprintf(stderr, "read data: %s\n", strerror(errno));
  return -1; /* error, don't try again */
  }
else if (res==0)
  {
  if (!quiet) fprintf(stderr, "read data returns 0\n");
  return -1;
  }
buf->position+=res;
if (buf->position<buf->event_buffer->size) return 0; /* try later */

/* data are complete */
buf->valid=1;
return 0;
}
/******************************************************************************/
int do_write(sock_struct* sock)
{
int res;

res=write(sock->p, sock->event_buffer->data+sock->position,
    sock->event_buffer->size-sock->position);
if (res<0)
  {
  if (errno==EINTR) return 0; /* try later */
  if (!quiet) fprintf(stderr, "write to %s: %s\n", conn2string(&sock->address),
      strerror(errno));
  return -1; /* error, don't try again */
  }
else if (res==0)
  {
  if (!quiet) fprintf(stderr, "write to %s returns 0\n",
      conn2string(&sock->address));
  return -1; /* error, don't try again */
  }
sock->position+=res;
if (sock->position<sock->event_buffer->size) return 0; /* try later */

/* all sent */

if (!--sock->event_buffer->number_of_users)
  {
  free(sock->event_buffer->data);
  free(sock->event_buffer);
  }
sock->event_buffer=0;
return 0;
}
/******************************************************************************/
int create_listening_socket (int listening_port)
{
int listening_socket;
struct sockaddr_in address;

if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0))<0)
  {
  perror("create listening_socket");
  return -2;
  }

bzero((void*)&address, sizeof(struct sockaddr_in));
address.sin_family=AF_INET;
address.sin_port=htons(listening_port);
address.sin_addr.s_addr=htonl(INADDR_ANY);

if ((bind(listening_socket, (struct sockaddr*)&address,
    sizeof(struct sockaddr_in)))<0)
  {
  perror("bind listening_socket");
  return -3;
  }

if ((listen(listening_socket, 1))<0)
  {
  perror("listen listening_socket");
  return -4;
  }

return listening_socket;
}
/******************************************************************************/
int create_connecting_socket (unsigned int host, int port)
{
int sock, res;
struct sockaddr_in address;

if ((sock=socket(AF_INET, SOCK_STREAM, 0))<0)
  {
  perror("create connecting_socket");
  return -1;
  }

if (fcntl(sock, F_SETFL, O_NDELAY)==-1)
  {
  perror("fcntl(O_NDELAY) connecting_socket");
  return -1;
  }

bzero((void*)&address, sizeof(struct sockaddr_in));
address.sin_family=AF_INET;
address.sin_port=htons(port);
address.sin_addr.s_addr=host;

res=connect(sock, (struct sockaddr*)&address, sizeof(struct sockaddr_in));
perror("connect inputsocket");
if ((res==0) || (errno==EINPROGRESS)) return sock;

perror("connect");
close(sock);
return -1;
}
/******************************************************************************/
int is_connected(int sock)
{
int opt, len, res;
len=sizeof(opt);
if ((res=getsockopt(sock, SOL_SOCKET, SO_ERROR, &opt, &len))<0)
  {
  perror("getsockopt(SO_ERROR)");
  return -1;
  }
if (opt==0)
  {
  if (!quiet) fprintf(stderr, "insocket connected.\n");
  return 1;
  }
if (!quiet) fprintf(stderr, "connect insocket: %s\n", strerror(opt));
return -1;
}
/******************************************************************************/
int accept_insock(int insock_l)
{
struct sockaddr_in address;
return do_accept(insock_l, &address, "insocket");
}
/******************************************************************************/
void accept_outsock(int outsock_l)
{
struct sockaddr_in address;
int ns;
sock_struct* sock;

ns=do_accept(outsock_l, &address, "outsocket");
if (ns<0) return;

sock=malloc(sizeof(sock_struct));
if (sock==0)
  {
  perror("malloc sock_struct");
  close(ns);
  return;
  }

sock->event_buffer=0;
sock->address=address;
sock->p=ns;

if (num_socks==max_socks)
  {
  int i;
  sock_struct** help=malloc((max_socks+10)*sizeof(sock_struct*));
  if (help==0)
    {
    fprintf(stderr, "malloc %d sock_struct*: %s\n", max_socks+10,
        strerror(errno));
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
void delete_outsock(int idx)
{
int i;

close(socks[idx]->p);
if (socks[idx]->event_buffer && !--socks[idx]->event_buffer->number_of_users)
  {
  free(socks[idx]->event_buffer->data);
  free(socks[idx]->event_buffer);
  }
free(socks[idx]);
num_socks--;
for (i=idx; i<num_socks; i++) socks[i]=socks[i+1];
}
/******************************************************************************/
void main_loop(int insock_l, int outsock_l)
{
input_event_buffer_struct ibs;

fd_set readfds, writefds;
int insock;
struct timeval last;

last.tv_sec=0;

ibs.event_buffer=0;
clear_new_event(&ibs);
num_socks=0; max_socks=0; socks=0;
if (intype==intype_stdin)
  insock=0;
else
  insock=-1;

while(1)
  {
  int nfds, idx, need_data, i, res;
  struct timeval to, *timeout;

  timeout=0;
  nfds=-1;
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  if (intype==intype_accept)
    {
    FD_SET(insock_l, &readfds);
    nfds=insock_l;
    }
  FD_SET(outsock_l, &readfds);
  if (outsock_l>nfds) nfds=outsock_l;

  need_data=0;
  for (idx=0; idx<num_socks; idx++)
    {
    if (socks[idx]->event_buffer)
      {
      FD_SET(socks[idx]->p, &writefds);
      if (socks[idx]->p>nfds) nfds=socks[idx]->p;
      }
    else
      need_data=1;
    }
  if (need_data)
    {
    if (insock>=0)
      {
      FD_SET(insock, &readfds);
      if (insock>nfds) nfds=insock;
      }
    else if (intype==intype_connect)
      {
      if (insock_l<0)
        {
        struct timeval now;
        gettimeofday(&now, 0);
        if (now.tv_sec-last.tv_sec>60)
          {
          last=now;
          insock_l=create_connecting_socket(inhostaddr, inport);
          }
        else
          {
          to.tv_sec=10; to.tv_usec=0; timeout=&to;
          }
        }
      if (insock_l>=0)
        {
        FD_SET(insock_l, &writefds);
        if (insock_l>nfds) nfds=insock_l;
        }
      }
    }

  res=select(nfds+1, &readfds, &writefds, 0, timeout);
  if (res<0)
    {
    if (errno==EINTR) continue;
    perror("select");
    return;
    }
  if (res==0) continue;

  /* output */
  for (idx=0; idx<num_socks; idx++)
    {
    if (FD_ISSET(socks[idx]->p, &writefds))
      {
      if (do_write(socks[idx])<0)
        {
        delete_outsock(idx);
        idx--;
        }
      }
    }

  /* new connections */
  if (insock_l>=0)
    {
    if (intype==intype_accept)
      {
      if (FD_ISSET(insock_l, &readfds))
        {
        int ns=accept_insock(insock_l);
        if (ns>=0)
          {
          if (insock<0)
            insock=ns;
          else
            {
            if (close_old)
              {
              if (!quiet) fprintf(stderr, "new insocket accepted.\n");
              clear_new_event(&ibs);
              close(insock);
              insock=ns;
              }
            else
              {
              if (!quiet) fprintf(stderr, "insocket already connected.\n");
              close(ns);
              }
            }
          }
        }
      }
    else if (intype==intype_connect)
      {
      if (FD_ISSET(insock_l, &writefds))
        {
        switch (is_connected(insock_l))
          {
          case 1: insock=insock_l; insock_l=-1; break;
          case -1: close(insock_l); insock_l=-1;break;
          /* default: still in progress */
          }
        }
      }
    }
  if (FD_ISSET(outsock_l, &readfds)) accept_outsock(outsock_l);
  /* input? */
  if ((insock>=0) && FD_ISSET(insock, &readfds))
    {
    res=do_read(insock, &ibs);
    if (res<0)
      {
      clear_new_event(&ibs);
      if (intype==intype_stdin)
        {
        fprintf(stderr, "error reading stdin; giving up.\n");
        return;
        }
      else
        {
        close(insock);
        insock=-1;
        }
      }
    else
      {
      if (ibs.valid)
        {
        for (idx=0; idx<num_socks; idx++)
          {
          if (!socks[idx]->event_buffer)
            {
            socks[idx]->event_buffer=ibs.event_buffer;
            socks[idx]->position=0;
            ibs.event_buffer->number_of_users++;
            }
          }
        if (!ibs.event_buffer->number_of_users)
            fprintf(stderr, "new event_buffer unused!\n");
        clear_new_event(&ibs);
        }
      }
    }
  }
}
/******************************************************************************/
void printusage(char* argv0)
{
fprintf(stderr, "usage: %s [-h] [-d] [-q] [-o] [-c] [-s 0|1] [inport [outport]]\n",
    basename(argv0));
fprintf(stderr, "  -h: this help\n");
fprintf(stderr, "  -q: quiet (no informational output)\n");
fprintf(stderr, "  -c: close old datainput if new connection arrives\n");
fprintf(stderr, "  -o: use old event format\n");
fprintf(stderr, "  -s 0: swap never 1: swap always (old format only)\n");
fprintf(stderr, "  inport: [hostname:](portnum|portname) or '-' for stdin\n");
fprintf(stderr, "    default is \"evdistin\"\n");
fprintf(stderr, "  outport: portnum|portname\n");
fprintf(stderr, "    default is \"evdistout\"\n");
}
/******************************************************************************/
int portname2portnum(char* name)
{
struct servent* service;
int port;

if ((service=getservbyname(name, "tcp")))
  port=ntohs(service->s_port);
else
  {
  char* end;
  port=strtol(name, &end, 0);
  if (*end!=0)
    {
    fprintf(stderr, "cannot convert \"%s\" to integer.\n", name);
    port=-1;
    }
  }
return port;
}
/******************************************************************************/
main(int argc, char **argv)
{
int insock_l, outsock_l;
int c, err;
char *inname, *outname, *p;
struct servent* service;

quiet=old=close_old=0;
swaptype=swaptype_check;
err=0;
while (!err && ((c=getopt(argc, argv, "hqocs:")) != -1))
  {
  switch (c)
    {
    case 'h': printusage(argv[0]); return 1;
    case 'q': quiet=1; break;
    case 'o': old=1; break;
    case 'c': close_old=1; break;
    case 's':
      {
      char* end;
      int swap=strtol(optarg, &end, 0);
      if (*end!=0)
        {
        fprintf(stderr, "cannot convert \"%s\" to integer.\n", optarg);
        printusage(argv[0]); return 1;
        }
      swaptype=swap?swaptype_always:swaptype_never;
      }
      break;
    default: err=1;
    }
  }
if (err) {printusage(argv[0]); return 1;}

if (optind<argc)
  inname=argv[optind];
else
  inname="evdistin";
optind++;
if (optind<argc)
  outname=argv[optind];
else
  outname="evdistout";

fprintf(stderr, "inname=%s; outname=%s\n", inname, outname);

if (strcmp(inname, "-")==0)
  intype=intype_stdin;
else if ((p=index(inname, ':')))
  {
  struct hostent *host;
  char hostname[1024];
  int idx=p-inname;
  intype=intype_connect;
  strncpy(hostname, inname, idx);
  hostname[idx]=0;
  if ((inport=portname2portnum(p+1))<0) return 1;
  host=gethostbyname(hostname);
  if (host!=0)
    inhostaddr=*(unsigned int*)(host->h_addr_list[0]);
  else
    {
    inhostaddr=inet_addr(hostname);
    if (inhostaddr==-1)
      {
      fprintf(stderr, "unknown host: %s\n", hostname);
      return 1;
      }
    }
  }
else
  {
  intype=intype_accept;
  if ((inport=portname2portnum(inname))<0) return 1;
  }

if ((outport=portname2portnum(outname))<0) return 1;

if (!quiet)
  {
  fprintf(stderr, "using ");
  switch (intype)
    {
    case intype_stdin:
      fprintf(stderr, "stdin");
      break;
    case intype_connect:
      fprintf(stderr, "%d.%d.%d.%d:%d",
        inhostaddr&0xff, (inhostaddr>>8)&0xff,
        (inhostaddr>>16)&0xff, (inhostaddr>>24)&0xff, inport);
      break;
    case intype_accept:
      fprintf(stderr, "port %d", inport);
      break;
    }
  fprintf(stderr, " for input and port %d for output\n", outport);
  }

/*signal (SIGINT, *sigact);*/
/*signal (SIGPIPE, SIG_IGN);*/
signal (SIGPIPE, sigpipe);

if (intype==intype_accept)
  {
  if ((insock_l=create_listening_socket(inport))<0)
    {
    perror("create_listening_socket input");
    return insock_l;
    }
  }
else
  insock_l=-1;
if ((outsock_l = create_listening_socket(outport))<0)
  {
  perror("create_listening_socket output");
  return outsock_l;
  }

main_loop (insock_l, outsock_l);
}
/******************************************************************************/
