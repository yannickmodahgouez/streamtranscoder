#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#else
#include <winsock.h>
#include <io.h>
#endif

#ifdef WIN32
#define close _close
#endif

#include <errno.h>
#include <string.h>
#include <sys/types.h>

int connect_client(char *host, int port, char *mount, char **header)
{
    int sockfd;  
    struct hostent *he;
    struct sockaddr_in their_addr;
	char	getRequest[8192] = "";
	char *p1 = NULL;
	int byteCounter = 0;
    int sent = 0;
    int received = 0;
	int loop = 1;
	int	returnHeaderSize = 8196;
	char	*returnHeader;

    if ((he=gethostbyname(host)) == NULL) {  // get the host info 
        fprintf(stderr, "Problem looking up host %s\n", host);
        return(-1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Problem creating socket\n");
        return(-1);
    }

    their_addr.sin_family = AF_INET;    // host byte order 
    their_addr.sin_port = htons((unsigned short)port);  // short, network byte order 
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "Cannot connect to http://%s:%d%s\n", host, port, mount);
        return(-1);
    }

	sprintf(getRequest, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: WinampMPEG/5.11\r\nAccept: */*\r\nIcy-MetaData:1\r\nConnection: close\r\n\r\n", mount, host);
	
    sent = send(sockfd, getRequest, strlen(getRequest), 0);

	if (sent == -1) {
        fprintf(stderr, "Problem sending request %s\n", getRequest);
		close(sockfd);
		return -1;
	}

	loop = 1;
	returnHeader = (char *)malloc(returnHeaderSize);
	memset(returnHeader, '\000', returnHeaderSize);

	p1 = (char *)returnHeader;
	byteCounter = 0;
	while (loop) {
		int received = recv(sockfd, p1, 1, 0);
		if (received == -1) {
			fprintf(stderr, "Server closed connection before receiving header\n");
			close(sockfd);
			return -1;
		}
		byteCounter++;
		if (byteCounter > 4) {
			if ((*(p1-3) == '\r') &&
			    (*(p1-2) == '\n') &&
			    (*(p1-1) == '\r') &&
			    (*(p1-0) == '\n')) {
				loop = 0;
				continue;
			}
		}
		p1++;
		byteCounter++;
	}
	*header = returnHeader;

    return(sockfd);
}
