#include "socket_client.h"
#include "sourcethread.h"
#include "decodethread.h"
#include "reconnectthread.h"
#include "liboddcast/liboddcast.h"

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <string.h>

pthread_t   sourceThread = 0;
pthread_t   decodeThread = 0;
pthread_t   reconnectThread = 0;

bool nostdout = false;

int showConsole = 1;
#define MAX_ENCODERS 50
oddcastGlobals          *g[MAX_ENCODERS];
oddcastGlobals          gMain;

int file_exists(char *filename) {
	struct stat buf;
	int i = stat(filename, &buf);
	if (i == 0) {
		return 1;
	}
	return 0;
}
void inputMetadataCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
	if (g->encoderNumber == 0) {
		if (!nostdout) {
			printf("Metadata Updated: %s\n", (char *)pValue);
		}
    	LogMessage(g, LOG_INFO, "Metadata Updated: %s\n", (char *)pValue);
	}
    //mainWindow->inputMetadataCallback(g->encoderNumber, pValue);
}
void outputStatusCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
    //mainWindow->outputStatusCallback(g->encoderNumber, pValue);
	if (g->encoderNumber == 0) {
		if (!strcmp((char *)pValue, "Connected")) {
			if (!nostdout) {
				printf("Source is connected\n");
			}
		}
    	LogMessage(g, LOG_INFO, "Source : %s\n", pValue);
	}
	else {
		if (!strcmp((char *)pValue, "Socket connected")) {
			if (!nostdout) {
				printf("Encoder %d is connected\n", g->encoderNumber);
			}
		}
    	LogMessage(g, LOG_INFO, "Encoder %d: %s\n", g->encoderNumber, pValue);
	}
} 
void writeBytesCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
    //mainWindow->writeBytesCallback(g->encoderNumber, pValue);

	/* pValue is a long */
	static time_t startTime[MAX_ENCODERS];
	static time_t endTime[MAX_ENCODERS];
	static long bytesWrittenInterval[MAX_ENCODERS];
	static long totalBytesWritten[MAX_ENCODERS];
	static int  initted = 0;
	char        kBPSstr[255] = "";

	if(!initted) {
		initted = 1;
		memset(&startTime, '\000', sizeof(startTime));
		memset(&endTime, '\000', sizeof(endTime));
		memset(&bytesWrittenInterval, '\000', sizeof(bytesWrittenInterval));
		memset(&totalBytesWritten, '\000', sizeof(totalBytesWritten));
	}

	if(g->encoderNumber != 0) {
		int     enc_index = g->encoderNumber - 1;
		long    bytesWritten = (long) pValue;

		if(bytesWritten == -1) {
			strcpy(kBPSstr, "");
			outputStatusCallback(g, (void *)kBPSstr);
			startTime[enc_index] = 0;
			return;
		}

		if(startTime[enc_index] == 0) {
			startTime[enc_index] = time(&(startTime[enc_index]));
			bytesWrittenInterval[enc_index] = 0;
		}

		bytesWrittenInterval[enc_index] += bytesWritten;
		totalBytesWritten[enc_index] += bytesWritten;
		endTime[enc_index] = time(&(endTime[enc_index]));
		if((endTime[enc_index] - startTime[enc_index]) > 4) {
			int     bytespersec = bytesWrittenInterval[enc_index] / (endTime[enc_index] - startTime[enc_index]);
			long    kBPS = (bytespersec * 8) / 1000;
			if(strlen(g->gMountpoint) > 0) {
				sprintf(kBPSstr, "%ld Kbps (%s)", kBPS, g->gMountpoint);
			}
			else {
				sprintf(kBPSstr, "%ld Kbps", kBPS);
			}

			LogMessage(g, LOG_DEBUG, kBPSstr);
			startTime[enc_index] = 0;
		}
	}

} 
void outputServerNameCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
    //mainWindow->outputServerNameCallback(g->encoderNumber, pValue);
    LogMessage(g, LOG_INFO, "Encoder %d: %s\n", g->encoderNumber, pValue);
}
void outputBitrateCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
    //mainWindow->outputBitrateCallback(g->encoderNumber, pValue);
    LogMessage(g, LOG_INFO, "Encoder %d: %s\n", g->encoderNumber, pValue);
} 
void outputStreamURLCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
    //mainWindow->outputStreamURLCallback(g->encoderNumber, pValue);
    LogMessage(g, LOG_INFO, "Encoder %d: %s\n", g->encoderNumber, pValue);
} 

int oddcastv3_init(oddcastGlobals *g)
{
    int printConfig = 0;
   
    
    setServerStatusCallback(g, outputStatusCallback);
    setGeneralStatusCallback(g, NULL);
    setWriteBytesCallback(g, writeBytesCallback);
    setBitrateCallback(g, outputBitrateCallback);
    setServerNameCallback(g, outputServerNameCallback);
    setDestURLCallback(g, outputStreamURLCallback);
    readConfigFile(g, 0);
    return 1;
}


void usage() {
    fprintf(stdout, "usage: streamTranscoderv3 -b -n -h\n");
    fprintf(stdout, "where :\n");
    fprintf(stdout, "   -b = start in background\n");
    fprintf(stdout, "   -n = no info to stdout\n");
    fprintf(stdout, "   -h = help\n");
    fprintf(stdout, "\n");

    exit(1);
}

int main(int argc, char *argv[])
{
	char	sourceURL[1024] = "";
	char	configFile[1024] = "";
	int		c = 0;
	int 	numEncoders = 1;
	bool	background = false;

	while ((c = getopt(argc, argv, "bnh")) != -1) {
		switch (c) {
			case 'h':
				usage();
				break;
			case 'b':
				background = true;
				nostdout = true;
			case 'n':
				nostdout = true;
				break;
			default:
				usage();
		}
	}

	if (background) {
		fprintf(stderr, "Backgrounding process...\n");
		/* Shamelessly ripped from ICES */
		int ret = 0;
		/* Start up new session, to lose old session and process group */
		switch (fork()) {
			case 0: 	break; /* child continues */
			case -1: 	perror ("fork"); 
						ret = -1;
			default:
						exit (ret);
		}

		/* Disassociate process group and controlling terminal */
		setsid();

		/* Become a NON-session leader so that a */
		/* control terminal can't be reacquired */
		switch (fork()) {
			case 0: 	break; /* child continues */
			case -1: 	perror ("fork"); 
						ret = -1;
			default:
						exit (ret);
		}
	}

    char    currentlogFile[1024] = "";
    char    logPrefix[255] = "streamTranscoder";

	int		configFilesCreated = 0;
	int		encoderconfigFilesCreated = 0;

    
    sprintf(configFile, "%s", logPrefix);
    sprintf(currentlogFile, "%s", logPrefix);

    setDefaultLogFileName(currentlogFile);
    setgLogFile(&gMain, currentlogFile);
    setConfigFileName(&gMain, configFile);
    addConfigVariable(&gMain, "SourceURL");
    addConfigVariable(&gMain, "NumEncoders");
    addConfigVariable(&gMain, "AutomaticReconnectSecs");
    addConfigVariable(&gMain, "AutoConnect");
    addConfigVariable(&gMain, "LogLevel");
    addConfigVariable(&gMain, "LogFile");

    char    tmpLog[1024] = "";
	sprintf(tmpLog, "%s_%d.cfg", logPrefix, 0);
	if (!file_exists(tmpLog)) {
		printf("Creating config file %s\n", tmpLog);
		configFilesCreated = 1;
	}
	
    readConfigFile(&gMain, 0);


    for(int i = 0; i < gMain.gNumEncoders; i++) {
        if(!g[i]) {
            g[i] = (oddcastGlobals *) malloc(sizeof(oddcastGlobals));
            memset(g[i], '\000', sizeof(oddcastGlobals));
        	initializeGlobals(g[i]);
            addBasicEncoderSettings(g[i]);
        }
		else {
        	initializeGlobals(g[i]);
		}
    

        g[i]->encoderNumber = i + 1;
        char    currentlogFile[1024] = "";
        char    logPrefix[255] = "streamTranscoder";

        sprintf(currentlogFile, "%s_%d", logPrefix, g[i]->encoderNumber);

        setDefaultLogFileName(currentlogFile);
        setgLogFile(g[i], currentlogFile);
        setConfigFileName(g[i], gMain.gConfigFileName);

		sprintf(tmpLog, "%s.cfg", currentlogFile);
		if (!file_exists(tmpLog)) {
			printf("Creating config file %s\n", tmpLog);
			configFilesCreated = 1;
			encoderconfigFilesCreated = 1;
		}

        oddcastv3_init(g[i]);
    }

	if (configFilesCreated) {
		printf("One or more config files were created.\n");
		printf("Source settings are contained in %s_0.cfg\n", logPrefix);
		printf("All encoder settings are contained in %s_X.cfg, where X is the encoder number.\n", logPrefix);
		printf("\nYou must manually edit these config files before continuing.\n");
		if (encoderconfigFilesCreated) {
			printf("Your next step should probably be to ****edit %s_X.cfg where X = 1 to number of encoders**** you've defined, then rerun streamTranscoder\n", logPrefix);
		}
		else {
			printf("Your next step should probably be to ****edit %s_0.cfg and change the number of encoders****, then rerun streamTranscoder\n", logPrefix);
		}
		exit(1);
	}
    outputStatusCallback(&gMain, (void *)"Ready to connect");

	for(int i = 0; i < gMain.gNumEncoders; i++) {
		if (!connectToServer(g[i])) {
			char buf[255] = "";
			sprintf(buf, "Disconnected from server");
			g[i]->forcedDisconnect = true;
			g[i]->forcedDisconnectSecs = time(&(g[i]->forcedDisconnectSecs));
			g[i]->serverStatusCallback(g[i], (void *) buf);
		}
	}


	pthread_create(&sourceThread, NULL, &startSourceThread, gMain.gSourceURL);
	pthread_create(&reconnectThread, NULL, &startReconnectThread, NULL);
	pthread_create(&decodeThread, NULL, &startDecodeThread, NULL);

	sleep(2);
	if (!nostdout) {
		printf("streamTranscoder started.\n");
	}
	pthread_join(sourceThread, NULL);
	pthread_join(decodeThread, NULL);
	if (!nostdout) {
		fprintf(stdout, "Done...\n");
	}
	
}

