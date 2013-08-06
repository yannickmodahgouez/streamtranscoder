#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#else
#include <winsock.h>
#include <io.h>
#endif

#ifdef WIN32
#define close _close
#endif

#include "cbuffer.h"
#include "socket_client.h"
#include "liboddcast/liboddcast.h"

int sourceMainLoop = 1;
int sourceInnerLoop = 1;
int sourceThreadRunning = 0;

CBUFFER	sourceCBuffer;
CBUFFER metadataCBuffer;

extern void outputStatusCallback(void *gbl, void *pValue);
extern void inputMetadataCallback(void *gbl, void *pValue);
extern oddcastGlobals gMain;
extern oddcastGlobals *g[];

#define MP3_FORMAT  1 
#define OGGVORBIS_FORMAT  2

int	sourceMedia = MP3_FORMAT;

#define SERVER_FORMAT_ICECAST 1
#define SERVER_FORMAT_SHOUTCAST 2

int decode_pipe[2];

static  int oggvHeaderRead = 0;
static  int header_read = 0;
static  int partial_header = 0;
static  int interleavedmetaDataFlag = 0;
static  int serverFormatFlag = SERVER_FORMAT_SHOUTCAST;
static  int	icyMetaInt = 0;

pthread_mutex_t encoders_mutex;


int		client_socket = 0;

int processInterleavedMetadata(char *buf, int len, char **outbuf, int *outlen) {
	static char	metadataBucket[2046];
	char title[1024] = "";
	static char currentSong[2046] = "";

	if (!buf) {
		return 0;
	}

	LogMessage(&gMain, LOG_DEBUG, "Metadata CBuffer: %d", cbuffer_get_size(&metadataCBuffer));
	if (cbuffer_insert(&metadataCBuffer, buf, len) == BUFFER_FULL) {
		return 0;
	}

	LogMessage(&gMain, LOG_DEBUG, "Processing Interleaved Metadata, MetaInt = %d", icyMetaInt);
	if (cbuffer_get_used(&metadataCBuffer) >= (unsigned long)(icyMetaInt*2)) {
        char *pStreamData = (char*)malloc(icyMetaInt);

        cbuffer_extract(&metadataCBuffer, pStreamData, icyMetaInt);

		char c;
        cbuffer_extract(&metadataCBuffer, &c, 1);
        if (c > 0) {

            // We have metadata, lets read it!
            char *pMetaData = (char*)malloc(c*16);
            memset(pMetaData, '\000', sizeof(c*16));
            cbuffer_extract(&metadataCBuffer, pMetaData, c*16);
			LogMessage(&gMain, LOG_DEBUG, "Metadata Triggered (%s)", pMetaData);
            char *pStreamTitle = strchr(pMetaData, '\'');
            if (pStreamTitle) {
                char *pEndStreamTitle = strstr(pStreamTitle+1, "';");
                if (pEndStreamTitle) {
                    memset(title, '\000', sizeof(title));
                    pStreamTitle++;
                    strncpy(title, pStreamTitle, pEndStreamTitle-pStreamTitle);

	                if (!strcmp(currentSong, title)) {
                        ;
                    }
                    else {
                        strcpy(currentSong, title);
                        inputMetadataCallback(&gMain, title);
						pthread_mutex_lock(&encoders_mutex);
                        for(int i = 0; i < gMain.gNumEncoders; i++) {
                            setCurrentSongTitle(g[i], title);
                        }
						pthread_mutex_unlock(&encoders_mutex);
                    }
                }
            }
			if (pMetaData) {
				free(pMetaData);
			}
        }
        if (c == 0) {
            ;
        }
		*outbuf = pStreamData;
		*outlen = icyMetaInt;
		return 1;
    }
	return 0;
}

void parseIcecastHeader(char *http_header) {
	char *p2 = 0;
	char *p3 = 0;
	char	tmpBuf[1024] = "";
	char	icyName[1024] = "";
	char	icyMetaIntstr[1024] = "";


	serverFormatFlag = SERVER_FORMAT_ICECAST;
	p2 = strstr(http_header, "icy-name:");
	if (p2) {
		p2 = p2 + strlen("icy-name:");
		p3 = strstr(p2, "\r\n");
		if (p3) {
			memset(icyName, '\000', sizeof(icyName));
			strncpy(icyName, p2, p3-p2);
		}
		outputStatusCallback(&gMain, (void *)icyName);
	}
	p2 = strstr(http_header, "icy-metaint:");
	if (p2) {
		p2 = p2 + strlen("icy-metaint:");
		p3 = strstr(p2, "\r\n");
		if (p3) {
			icyMetaInt = 0;
			memset(icyMetaIntstr, '\000', sizeof(icyMetaIntstr));
			strncpy(icyMetaIntstr, p2, p3-p2);
			icyMetaInt = atoi(icyMetaIntstr);
			interleavedmetaDataFlag = 1;
		}
	}
	outputStatusCallback(&gMain, (void *)"Icecast");
	p2 = strstr(http_header, "Content-Type: ");
	if (p2) {
		p2 = p2 + strlen("Content-Type: ");
		p3 = strstr(p2, "\r\n");
		if (p3) {
			memset(tmpBuf, '\000', sizeof(tmpBuf));
			strncpy(tmpBuf, p2, p3-p2);
			if (!strcmp(tmpBuf, "application/ogg")) {
				sourceMedia = OGGVORBIS_FORMAT;
				outputStatusCallback(&gMain, (void *)"Vorbis");
			}
			else {
				sourceMedia = MP3_FORMAT;
				outputStatusCallback(&gMain, (void *)"MP3");
			}
		}
	}

}

void parseShoutcastHeader(char *http_header) {
	char *p2 = 0;
	char *p3 = 0;
	char	icyName[1024] = "";
	char	icyMetaIntstr[1024] = "";

	interleavedmetaDataFlag = 0;

	p2 = strstr(http_header, "icy-name:");
	if (p2) {
		p2 = p2 + strlen("icy-name:");
		p3 = strstr(p2, "\r\n");
		if (p3) {
			memset(icyName, '\000', sizeof(icyName));
			strncpy(icyName, p2, p3-p2);
		}
	}
	p2 = strstr(http_header, "icy-metaint:");
	if (p2) {
		p2 = p2 + strlen("icy-metaint:");
		p3 = strstr(p2, "\r\n");
		if (p3) {
			icyMetaInt = 0;
			memset(icyMetaIntstr, '\000', sizeof(icyMetaIntstr));
			strncpy(icyMetaIntstr, p2, p3-p2);
			icyMetaInt = atoi(icyMetaIntstr);
			interleavedmetaDataFlag = 1;
		}
	}

	serverFormatFlag = SERVER_FORMAT_SHOUTCAST;
	sourceMedia = MP3_FORMAT;

	outputStatusCallback(&gMain, (void *)icyName);
	outputStatusCallback(&gMain, (void *)"Shoutcast");
	outputStatusCallback(&gMain, (void *)"MP3");
}

void * startSourceThread(void *url) {

	char	requestURL[1024] = "";
	char 	host[1024] = "";
	char 	port_str[255] = "";
	int		port = 0;
	char 	mount[1024] = "";
	char	*p1, *p2, *p3;
	char	*pSourceURLHeaders = NULL;

	cbuffer_init(&sourceCBuffer, 16384*20);

	pthread_mutex_init(&encoders_mutex, NULL);

	strncpy(requestURL, (char *)url, sizeof(requestURL)-1);

	memset(host, '\000', sizeof(host));
	memset(port_str, '\000', sizeof(port_str));
	memset(mount, '\000', sizeof(mount));

	
	p1 = strstr(requestURL, "http://");
	if (!p1) {
		fprintf(stderr, "bad URL : %s\n", requestURL);
		pthread_exit((void *)1);
		return(NULL);
	}
	p1 = p1 + strlen("http://");
	p2 = strchr(p1, ':');
	if (!p2) {
		fprintf(stderr, "bad URL : %s\n", requestURL);
		pthread_exit((void *)1);
		return(NULL);
	}
	p2++;
	p3 = strchr(p2, '/');
	if (!p3) {
		strcpy(mount, "/");
		strncpy(port_str, p2, sizeof(port_str)-1);
	}
	else {
		strncpy(mount, p3, sizeof(mount)-1);
		strncpy(port_str, p2, p3-p2);
	}

	strncpy(host, p1, p2-1-p1);

	port = atoi(port_str);

	char buf[1024] = "";
	sprintf(buf, "Connecting to %s:%d%s", host, port, mount);
	outputStatusCallback(&gMain, (void *)"Ready to connect");


	sourceMainLoop = 1;
	sourceThreadRunning = 1;

	while (sourceMainLoop) {
		LogMessage(&gMain, LOG_DEBUG, "Connecting source thread");

		client_socket = connect_client(host, port, mount, &pSourceURLHeaders);	

		if (client_socket != -1) {
			if (!strncmp(pSourceURLHeaders, "ICY 200 OK", strlen("ICY 200 OK"))) { 
				parseShoutcastHeader(pSourceURLHeaders);
			}
			else {
				parseIcecastHeader(pSourceURLHeaders);
			}
			outputStatusCallback(&gMain, (void *)"Connected");
			if (pSourceURLHeaders) {
				free(pSourceURLHeaders);
			}

			sourceInnerLoop = 1;

#define SOCK_BLOCK 0
#ifdef WIN32
			u_long varblock = SOCK_BLOCK;
#else
			int varblock = SOCK_BLOCK;
#endif

#ifdef WIN32
			ioctlsocket(client_socket, FIONBIO, &varblock);
#else
			fcntl(client_socket, F_SETFL, (varblock == SOCK_BLOCK) ? 0 : O_NONBLOCK);
#endif

			if (interleavedmetaDataFlag) {
				cbuffer_destroy(&metadataCBuffer);
				cbuffer_init(&metadataCBuffer, icyMetaInt*4);
			}

			fd_set readyfds;
			fd_set errorfds;
			int	fd_max = client_socket;
			FD_ZERO(&readyfds);
			FD_ZERO(&errorfds);
			FD_SET(client_socket, &readyfds);
			FD_SET(client_socket, &errorfds);
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 200000;
			//FILE *dump = fopen("dump.mp3", "wb");
			while (sourceInnerLoop) {
				char buf[4096];
				int	read = 0;
				

				int selret = 0;

				selret = select(fd_max+1, NULL, &readyfds, &errorfds, &tv);

				if (selret == 0) {
					/* Nothing ready */
	#ifdef WIN32
					Sleep(100);
	#else
					usleep(1000);
	#endif
				}
				else {
					if (selret < 0) {
						/* Problem here */
						sourceInnerLoop = 0;
						continue;
					}
					
					/* Check for any errors */
					if (FD_ISSET(client_socket, &errorfds)) {
						sourceInnerLoop = 0;
						closesocket(client_socket);
						continue;
					}
					read = recv(client_socket, buf, sizeof(buf), 0);
					if (sourceInnerLoop == 0) {
						break;
					}
					if (read <= 0) {
						outputStatusCallback(&gMain, (void *)"Disconnected from source socket...");
						sourceInnerLoop = 0;
					}
					if (read == 0) {
#ifdef WIN32
						Sleep(100);
#else
						usleep(1000);
#endif
					}
					else {
						char *pData = buf;
						int	dataLen = read;
						char *pNew = NULL;
						int newLen = 0;
						bool writeit = true;

						if (interleavedmetaDataFlag) {
							if (processInterleavedMetadata(buf, read, &pNew, &newLen)) {
								pData = pNew;
								dataLen = newLen;
							}
							else {
								writeit = false;
							}
						}
						
						if (writeit) {
							LogMessage(&gMain, LOG_DEBUG, "Source CBuffer: %d", cbuffer_get_free(&sourceCBuffer));

							if (cbuffer_insert(&sourceCBuffer, pData, dataLen) == BUFFER_FULL) {
								outputStatusCallback(&gMain, (void *)"Circular buffer is full...");
								sourceInnerLoop = 0;
							}
							else {
								;
							}
						}
						if (pData == pNew) {
							free(pNew);
						}
					}			
				}
				

				//usleep(500000);
			}
			LogMessage(&gMain, LOG_DEBUG, "Broke out of source inner loop.");

			closesocket(client_socket);
			if (sourceMainLoop) {
				outputStatusCallback(&gMain, (void *)"Disconnected");
			}
		}
		int countdown = gMain.gReconnectSec;
		for (int i=0;i<gMain.gReconnectSec;i++) {
			char title[255] = "";
			sprintf(title, "Source reconnect in %d seconds", countdown);
			countdown--;
			outputStatusCallback(&gMain, title);
#ifdef WIN32
			Sleep(1000);
#else
			sleep(1);
#endif
		}
	}
	sourceThreadRunning = 0;
	pthread_exit((void *)1);
	return(NULL);
}
