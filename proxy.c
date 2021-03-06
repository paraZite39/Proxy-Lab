#include "csapp.h"
#include "sbuf.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define MAX_THREADS 4
#define SBUFSIZE 16

/* Header elements */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection = "Connection: close";
static const char *proxy_connection = "Proxy-connection: close";

/* Function prototypes */
void doit(int fd);
void read_reqheaders(rio_t *rio);
int parse_uri(char *uri, char *hostname, char *filename, int *port);
void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void build_header(char *header, char *hostname, char *path, rio_t *client_rio);
void *thread (int *cvargs);

sbuf_t sbuf;

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    pthread_t tid;

    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
    } 

    /* create listening socket file descriptor */
    listenfd = Open_listenfd(argv[1]);
    
    /* open MAX_THREADS threads */
    int i;
    sbuf_init(&sbuf, SBUFSIZE);
    for(i = 0; i < MAX_THREADS; i++)
    {
        Pthread_create(&tid, NULL, thread, NULL);
    }

    while(1)
    {
        /* Accept connection from client, create filedescriptor and insert it into buffer */
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        sbuf_insert(&sbuf, connfd);
    }
}

/* function done by each thread */
void *thread(void *vargp)
{
    Pthread_detach(pthread_self());
    while(i) {
        int connfd = sbuf_remove(sbuf);
        doit(connfd);
        Close(connfd);
    }
}

void doit(int connfd)
{
    rio_t client_rio, server_rio;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filepath[MAXLINE], hostname[MAXLINE];
    char end_header[MAXLINE];
    int end_serverfd, port = 80;
    char portbuf[100];
    int n;

    /* Read headers */
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);

    /* Only GET requests are implemented */
    if(strcasecmp(method, "GET") != 0)
    {
        client_error(fd, method, "501", "Not implemented", "Proxy does not support this method");
        return;
    }

    /* Split request into hostname, URI, filepath and port */
    parse_uri(uri, hostname, filepath, &port);

    /* Build header with elements described above */
    build_header(end_header, hostname, filepath, client_rio);

    /* Connect to end server */
    sprintf(portstr, "%d", port);
    end_serverfd = Open_clientfd(hostname, portstr);
    
    if(end_serverfd < 0)
    {
        printf("Error: Connection failed.\n");
        return;
    }

    /* Send header to end server */
    Rio_readinitb(&server_rio, end_serverfd);
    Rio_writen(end_serverfd, end_header);

    /* Read bytes received from server, send to client */
    while(n = Rio_readlineb(&server_rio, buf, MAXLINE) != 0)
    {
        print("Received %s bytes from server, sending to client...", strlen(buf));
        Rio_writen(connfd, buf, n);
    }

    Close(end_serverfd);
}

/* function to split uri into hostname, filepath and port */
void parse_uri(char *uri, char *hostname, char *filepath, int *port)
{
    char* ptr = strstr(uri,"//");

    ptr = ptr != NULL? ptr+2 : uri;

    char *temp = strstr(ptr, ":");
    if(temp != NULL) {
        *temp = '\0';
        sscanf(ptr, "%s", hostname);
        sscanf(temp + 1, "%d%s", port, filepath);
    }
    else {
        temp = strstr(ptr, "/");
        if(temp != NULL) {
            *temp = '\0';
            sscanf(ptr, "%s", hostname);
            *temp = '/';
            sscanf(temp, "%s", filepath);
        }
        else {
            sscanf(ptr, "%s", hostname);
        }
    }
    return;
}

/* create full request header to send to endserver */
void build_header(char *header, char *hostname, char *path, rio_t *client_rio)
{
    sprintf(header, "GET %s HTTP/1.0\r\n", path);
    sprintf(header, "%sHost: %s\r\n", header, hostname);
    sprintf(header, "%s%s\r\n", header, user_agent_hdr);
    sprintf(header, "%s%s\r\n", header, connection);
    sprintf(header, "%s%s\r\n", header, proxy_connection);
}

/* error function */
void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    sprintf(body, "<html><title>Title Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web Server</em>\r\n", body);

    sprintf(buf, "HTTP/1.0 %s.%s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

void read_reqheaders(rio_t *rio)
{
    char buf[MAXLINE];

    Rio_readlineb(rio, buf, MAXLINE);
    while(strcmp(buf, "\r\n"))
    {
        Rio_readlineb(rio, buf, MAXLINE);
        printf("%s", buf);
    }

    return;
}
