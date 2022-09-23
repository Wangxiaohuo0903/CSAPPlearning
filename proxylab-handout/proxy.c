#include "csapp.h"
#include <stdio.h>
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
struct Uri {
  char host[MAXLINE]; // hostname
  char port[MAXLINE]; //端口
  char path[MAXLINE]; //路径
};
void parse_uri(char *uri, struct Uri *uri_data);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);
void doit(int fd);
void read_requesthdrs(rio_t *rp);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);  // line:netp:tiny:doit
    Close(connfd); // line:netp:tiny:close
  }
}
/* $end tinymain */

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int connfd) {
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char server[MAXLINE];

  rio_t rio, server_rio;

  Rio_readinitb(&rio, connfd);
  Rio_readlineb(&rio, buf, MAXLINE);
  sscanf(buf, "%s %s %s", method, uri, version);

  if (strcasecmp(method, "GET")) {
    printf("Proxy does not implement the method");
    return;
  }

  /* Parse URI from GET request */
  struct Uri *uri_data = (struct Uri *)malloc(sizeof(struct Uri));
  parse_uri(uri, uri_data); // line:netp:doit:staticcheck
  int serverfd = Open_clientfd(uri_data->host, uri_data->port);
  if (serverfd < 0) {
    printf("connection failed\n");
    return;
  }
  //转发给服务器
  Rio_readinitb(&server_rio, serverfd);
  Rio_writen(serverfd, server, strlen(server));

  size_t n;
  //回复给客户端
  while ((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
    printf("proxy received %d bytes,then send\n", (int)n);
    Rio_writen(connfd, buf, n);
  }
  //关闭服务器描述符
  Close(serverfd);
}
/* $end doit */

/*
 * read_requesthdrs - read HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp) {
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  printf("%s", buf);
  while (strcmp(buf, "\r\n")) { // line:netp:readhdrs:checkterm
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */

/* $end parse_uri */

/*
 * serve_static - copy a file back to the client
 */
void parse_uri(char *uri, struct Uri *uri_data) {
  char *hostpose = strstr(uri, "//");
  //默认端口为80
  if (hostpose == NULL) {
    char *pathpose = strstr(uri, "/");
    if (pathpose != NULL)
      strcpy(uri_data->path, pathpose);
    strcpy(uri_data->port, "80");
    return;
  } else {
    char *portpose = strstr(hostpose + 2, ":");
    if (portpose != NULL) {
      int tmp;
      sscanf(portpose + 1, "%d%s", &tmp, uri_data->path);
      sprintf(uri_data->port, "%d", tmp);
      *portpose = '\0';
    } else {
      char *pathpose = strstr(hostpose + 2, "/");
      if (pathpose != NULL) {
        strcpy(uri_data->path, pathpose);
        strcpy(uri_data->port, "80");
        *pathpose = '\0';
      }
    }
    strcpy(uri_data->host, hostpose + 2);
  }
  return;
}
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);    // line:netp:servestatic:getfiletype
  sprintf(buf, "HTTP/1.0 200 OK\r\n"); // line:netp:servestatic:beginserve
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n", filesize);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: %s\r\n\r\n", filetype);
  Rio_writen(fd, buf, strlen(buf)); // line:netp:servestatic:endserve

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0); // line:netp:servestatic:open
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd,
              0);                 // line:netp:servestatic:mmap
  Close(srcfd);                   // line:netp:servestatic:close
  Rio_writen(fd, srcp, filesize); // line:netp:servestatic:write
  Munmap(srcp, filesize);         // line:netp:servestatic:munmap
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) {
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");
}
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs) {
  char buf[MAXLINE], *emptylist[] = {NULL};

  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0) { /* Child */ // line:netp:servedynamic:fork
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1); // line:netp:servedynamic:setenv
    Dup2(fd, STDOUT_FILENO);
    /* Redirect stdout to client */ // line:netp:servedynamic:dup2
    Execve(filename, emptylist, environ);
    /* Run CGI program */ // line:netp:servedynamic:execve
  }
  Wait(NULL);
  /* Parent waits for and reaps child */ // line:netp:servedynamic:wait
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg) {
  char buf[MAXLINE];

  /* Print the HTTP response headers */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n\r\n");
  Rio_writen(fd, buf, strlen(buf));

  /* Print the HTTP response body */
  sprintf(buf, "<html><title>Tiny Error</title>");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "<body bgcolor="
               "ffffff"
               ">\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
  Rio_writen(fd, buf, strlen(buf));
}
/* $end clienterror */
