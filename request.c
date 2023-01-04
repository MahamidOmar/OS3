//
// request.c: Does the bulk of the work for the web server.
// 

#include "segel.h"
#include "request.h"


struct request_m{
   int connection_fd;
   struct timeval *stat_req_arrival;
   struct timeval stat_req_dispatch;
   StatThread stat_thread;
};

// TODO: update function
Request createRequest(int fd, struct timeval *arrival){
   Request request = malloc(sizeof(*request));
   if(!request){
      return NULL;
   }
   request -> connection_fd = fd;
   request -> stat_req_arrival = arrival;
   request -> stat_thread = NULL;
   return request;
}

// TODO: update function
void* copyRequest(void* request){
   if(!request){
      return NULL;
   }
   return createRequest(getFdRequest(request), getArriveTimeRequest(request));
}

void destroyRequest(void* request){
   free(request);
}

int getFdRequest(Request request){
   return request -> connection_fd;
}

struct timeval* getArriveTimeRequest(Request request){
   return request -> stat_req_arrival;
}

StatThread getThreadRequest(Request request){
   return request -> stat_thread;
}

void setDispatchRequest(Request request, struct timeval* new_dispatch){
  struct timeval *dispatch= malloc(sizeof(*dispatch));
 
   timersub (new_dispatch , request->stat_req_arrival, dispatch);
   
   request -> stat_req_dispatch.tv_sec = dispatch->tv_sec;
   request -> stat_req_dispatch.tv_usec = dispatch->tv_usec;
}

void requestSetThread(Request request, StatThread new_thread){
   request -> stat_thread = new_thread;
}

// requestError(      fd,    filename,        "404",    "Not found", "OS-HW3 Server could not find this file");
void requestError(Request request, char *cause, char *errnum, char *shortmsg, char *longmsg) 
{
   int fd = request -> connection_fd;
   char buf[MAXLINE], body[MAXBUF];

   // Create the body of the error message
   sprintf(body, "<html><title>OS-HW3 Error</title>");
   sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
   sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
   sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
   sprintf(body, "%s<hr>OS-HW3 Web Server\r\n", body);

   // Write out the header information for this response
   sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);


   sprintf(buf, "Stat-Req-Arrival:: %lu.%06lu\r\n", request->stat_req_arrival->tv_sec, request->stat_req_arrival->tv_usec);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Stat-Req-Dispatch:: %lu.%06lu\r\n", request->stat_req_dispatch.tv_sec, request->stat_req_dispatch.tv_usec);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Stat-Thread-Id:: %d\r\n",  getThreadId(request -> stat_thread));
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Stat-Thread-Count:: %d\r\n", getThreadCount(request -> stat_thread));
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Stat-Thread-Static:: %d\r\n", getThreadStaticCount(request -> stat_thread));
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Stat-Thread-Dynamic:: %d\r\n", getThreadDynamicCount(request -> stat_thread));
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Type: text/html\r\n");
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   // Write out the content
   Rio_writen(fd, body, strlen(body));
   printf("%s", body);

}


//
// Reads and discards everything up to an empty text line
//
void requestReadhdrs(rio_t *rp)
{
   char buf[MAXLINE];

   Rio_readlineb(rp, buf, MAXLINE);
   while (strcmp(buf, "\r\n")) {
      Rio_readlineb(rp, buf, MAXLINE);
   }
   return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int requestParseURI(char *uri, char *filename, char *cgiargs) 
{
   char *ptr;

   if (strstr(uri, "..")) {
      sprintf(filename, "./public/home.html");
      return 1;
   }

   if (!strstr(uri, "cgi")) {
      // static
      strcpy(cgiargs, "");
      sprintf(filename, "./public/%s", uri);
      if (uri[strlen(uri)-1] == '/') {
         strcat(filename, "home.html");
      }
      return 1;
   } else {
      // dynamic
      ptr = index(uri, '?');
      if (ptr) {
         strcpy(cgiargs, ptr+1);
         *ptr = '\0';
      } else {
         strcpy(cgiargs, "");
      }
      sprintf(filename, "./public/%s", uri);
      return 0;
   }
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype)
{
   if (strstr(filename, ".html")) 
      strcpy(filetype, "text/html");
   else if (strstr(filename, ".gif")) 
      strcpy(filetype, "image/gif");
   else if (strstr(filename, ".jpg")) 
      strcpy(filetype, "image/jpeg");
   else 
      strcpy(filetype, "text/plain");
}

void requestServeDynamic(Request request, char *filename, char *cgiargs)
{
   int fd = request -> connection_fd;
   char buf[MAXLINE], *emptylist[] = {NULL};

   // increment dynamic counter
   increaseDynamicCount(request->stat_thread);

   // The server does only a little bit of the header.  
   // The CGI script has to finish writing out the header.
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);

   sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, request->stat_req_arrival->tv_sec, request->stat_req_arrival->tv_usec);
   sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, request->stat_req_dispatch.tv_sec, request->stat_req_dispatch.tv_usec);
   sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf,  getThreadId(request -> stat_thread));
   sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, getThreadCount(request -> stat_thread));
   sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, getThreadStaticCount(request -> stat_thread));
   sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n", buf, getThreadDynamicCount(request -> stat_thread));

   Rio_writen(fd, buf, strlen(buf));

   pid_t pid = Fork();
   if (pid == 0) {
      /* Child process */
      Setenv("QUERY_STRING", cgiargs, 1);
      /* When the CGI process writes to stdout, it will instead go to the socket */
      Dup2(fd, STDOUT_FILENO);
      Execve(filename, emptylist, environ);
   }
   WaitPid(pid, NULL, 0);
}


void requestServeStatic(Request request, char *filename, int filesize) 
{
   int fd = request -> connection_fd;
   int srcfd;
   char *srcp, filetype[MAXLINE], buf[MAXBUF];

   // increment static counter
   increaseStaticCount(request->stat_thread);

   requestGetFiletype(filename, filetype);

   srcfd = Open(filename, O_RDONLY, 0);

   // Rather than call read() to read the file into memory, 
   // which would require that we allocate a buffer, we memory-map the file
   srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
   Close(srcfd);

   // put together response
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
   sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
   sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);
   sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, request->stat_req_arrival->tv_sec, request->stat_req_arrival->tv_usec);
   sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, request->stat_req_dispatch.tv_sec, request->stat_req_dispatch.tv_usec);
   sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf,  getThreadId(request -> stat_thread));
   sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, getThreadCount(request -> stat_thread));
   sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, getThreadStaticCount(request -> stat_thread));
   sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, getThreadDynamicCount(request -> stat_thread));

  

   Rio_writen(fd, buf, strlen(buf));

   //  Writes out to the client socket the memory-mapped file 
   Rio_writen(fd, srcp, filesize);
   Munmap(srcp, filesize);

}

// handle a request
void requestHandle(Request request)
{
   int fd = request -> connection_fd;
   int is_static;
   struct stat sbuf;
   char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
   char filename[MAXLINE], cgiargs[MAXLINE];
   rio_t rio;

   Rio_readinitb(&rio, fd);
   Rio_readlineb(&rio, buf, MAXLINE);
   sscanf(buf, "%s %s %s", method, uri, version);

   printf("%s %s %s\n", method, uri, version);

   if (strcasecmp(method, "GET")) {
      requestError(request, method, "501", "Not Implemented", "OS-HW3 Server does not implement this method");
      return;
   }
   requestReadhdrs(&rio);

   is_static = requestParseURI(uri, filename, cgiargs);
   if (stat(filename, &sbuf) < 0) {
      requestError(request, filename, "404", "Not found", "OS-HW3 Server could not find this file");
      return;
   }

   if (is_static) {
      if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
         requestError(request, filename, "403", "Forbidden", "OS-HW3 Server could not read this file");
         return;
      }
      requestServeStatic(request, filename, sbuf.st_size);
   } else {
      if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
         requestError(request, filename, "403", "Forbidden", "OS-HW3 Server could not run this CGI program");
         return;
      }
      requestServeDynamic(request, filename, cgiargs);
   }
}


