#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <unistd.h>
#endif
#include "liboddcast/liboddcast.h"

extern oddcastGlobals          *g[];
extern oddcastGlobals          gMain;

extern void outputStatusCallback(void *gbl, void *pValue);

int reconnectLoop = 1;
int reconnectThreadRunning = 0;

void * startReconnectThread(void *url) {
	
	reconnectLoop = 1;
	reconnectThreadRunning = 1;
	
	while (reconnectLoop) {
		time_t  currentTime;

		currentTime = time(&currentTime);
		for(int i = 0; i < gMain.gNumEncoders; i++) {
			if(g[i]->forcedDisconnect) {
				LogMessage(g[i], LOG_DEBUG, "Reconnecting disconnected encoder.");

				int timeout = getReconnectSecs(g[i]);
				int timediff = currentTime - g[i]->forcedDisconnectSecs;
				if(timediff > timeout) {
					g[i]->forcedDisconnect = false;
					if(!g[i]->weareconnected) {
						setForceStop(g[i], 0);
						if(!connectToServer(g[i])) {
							g[i]->forcedDisconnect = true;
							g[i]->forcedDisconnectSecs = time(&(g[i]->forcedDisconnectSecs));
						}
						LogMessage(g[i], LOG_DEBUG, "Done Reconnecting disconnected encoder.");
					}
				}
				else {
					char    buf[255] = "";
					sprintf(buf, "Connecting in %d seconds", timeout - timediff);
					outputStatusCallback(g[i], buf);
				}
			}
		}
#ifdef WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	}
	reconnectThreadRunning = 0;
	return(NULL);
}
