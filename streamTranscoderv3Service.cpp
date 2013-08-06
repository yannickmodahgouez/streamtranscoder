#include <windows.h>
#include <stdio.h>
#include <errno.h>
#include <direct.h>
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
#endif
#include <pthread.h>
#include <string.h>


SERVICE_STATUS          ServiceStatus; 
SERVICE_STATUS_HANDLE   hStatus; 

pthread_t   sourceThread = 0;
pthread_t   decodeThread = 0;
pthread_t   reconnectThread = 0;

int showConsole = 1;
#define MAX_ENCODERS 50
oddcastGlobals          *g[MAX_ENCODERS];
oddcastGlobals          gMain;
 
void  ServiceMain(int argc, char** argv); 
void  ControlHandler(DWORD request); 
int InitService();
extern "C" int mainService(int argc, char **argv);

int InitService() 
{ 
   int result = 0;
   return(result); 
}

void installService(char *path)
{
	if (path) {
		char	fullPath[8096*2] = "";

		sprintf(fullPath, "\"%s\\streamTranscoderv3Service.exe\" \"%s\"", path, path);
		SC_HANDLE handle = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
		SC_HANDLE service = CreateService(
			handle,
			"StreamTranscoderV3",
			"Stream Transcoder V3",
			GENERIC_READ | GENERIC_EXECUTE,
			SERVICE_WIN32_OWN_PROCESS,
			SERVICE_AUTO_START,
			SERVICE_ERROR_IGNORE,
			fullPath,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		);
		CloseServiceHandle(service);
		CloseServiceHandle(handle);
		printf("Service Installed\n");
	}
}
void removeService()
{
	SC_HANDLE handle = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	SC_HANDLE service = OpenService(handle, "StreamTranscoderV3", DELETE);
	if (service) {
		DeleteService(service);
	}
	CloseServiceHandle(service);
	CloseServiceHandle(handle);

//	printf("Service Removed\n");
}
void ControlHandler(DWORD request) 
{ 
   switch(request) { 
      case SERVICE_CONTROL_STOP: 
		  /* Stop here */
		ServiceStatus.dwWin32ExitCode = 0; 
		ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
		SetServiceStatus (hStatus, &ServiceStatus);
		return; 
 
      case SERVICE_CONTROL_SHUTDOWN: 
		/* Stop Here */
		ServiceStatus.dwWin32ExitCode = 0; 
		ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
		SetServiceStatus (hStatus, &ServiceStatus);
		return; 
      default:
		break;
    } 
 
    // Report current status
    SetServiceStatus (hStatus, &ServiceStatus);
 
    return; 
}

void ServiceMain(int argc, char** argv) 
{ 
   int error; 
 
   ServiceStatus.dwServiceType = SERVICE_WIN32; 
   ServiceStatus.dwCurrentState = SERVICE_START_PENDING; 
   ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
   ServiceStatus.dwWin32ExitCode = 0; 
   ServiceStatus.dwServiceSpecificExitCode = 0; 
   ServiceStatus.dwCheckPoint = 0; 
   ServiceStatus.dwWaitHint = 0; 
 
   hStatus = RegisterServiceCtrlHandler("StreamTranscoderV3", (LPHANDLER_FUNCTION)ControlHandler); 
   if (hStatus == (SERVICE_STATUS_HANDLE)0) { 
      // Registering Control Handler failed
      return; 
   }  
   // Initialize Service 
   error = InitService(); 
   if (error) {
      // Initialization failed
      ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
      ServiceStatus.dwWin32ExitCode = -1; 
      SetServiceStatus(hStatus, &ServiceStatus); 
      return; 
   } 
   // We report the running status to SCM. 
   ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
   SetServiceStatus (hStatus, &ServiceStatus);
 
   /* Here we do the work */

   int ret = mainService(argc, (char **)argv);

	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	ServiceStatus.dwWin32ExitCode = -1;
	SetServiceStatus(hStatus, &ServiceStatus);
	return; 
}


void main(int argc, char **argv) 
{

	bool matched  = false;
	if (argv[0]) {
		if (argv[1]) {
			if (!strcmp(argv[1], "install")) {
				removeService();
				installService(argv[2]);
				matched = true;
			}
			if (!strcmp(argv[1], "remove")) {
				removeService();
				matched = true;
			}
		}
	}
	if (matched) {
		return;
	}
	_chdir(argv[1]);

	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = "Stream Transcoder V3";
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;
	// Start the control dispatcher thread for our service
	StartServiceCtrlDispatcher(ServiceTable);  
}






void inputMetadataCallback(void *gbl, void *pValue) {
;
}
void outputStatusCallback(void *gbl, void *pValue) {
;
}
void writeBytesCallback(void *gbl, void *pValue) {
;
}
void outputServerNameCallback(void *gbl, void *pValue) {
;
}
void outputBitrateCallback(void *gbl, void *pValue) {
;
}
void outputStreamURLCallback(void *gbl, void *pValue) {
;
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
    readConfigFile(g);
    return 1;
}

void usage() {
	printf("usage: streamTranscoderv3Service install Dir");
}

int mainService(int argc, char *argv[])
{
	char	sourceURL[1024] = "";
	char	configFile[1024] = "";
	int		c = 0;
	int 	numEncoders = 1;


    char    currentlogFile[1024] = "";
	char	logPrefix[255] = "streamTranscoder";

    sprintf(configFile, "%s", logPrefix);
    sprintf(currentlogFile, "%s", logPrefix);

    setgLogFile(&gMain, currentlogFile);
    setConfigFileName(&gMain, configFile);

	addConfigVariable(&gMain, "SourceURL");
	addConfigVariable(&gMain, "NumEncoders");
	addConfigVariable(&gMain, "AutomaticReconnectSecs");
	addConfigVariable(&gMain, "AutoConnect");
	addConfigVariable(&gMain, "LogLevel");
	addConfigVariable(&gMain, "LogFile");


    readConfigFile(&gMain, 0);

	strcpy(sourceURL, gMain.gSourceURL);

    for(int i = 0; i < gMain.gNumEncoders; i++) {
        if(!g[i]) {
            g[i] = (oddcastGlobals *) malloc(sizeof(oddcastGlobals));
            memset(g[i], '\000', sizeof(oddcastGlobals));
        }

        g[i]->encoderNumber = i + 1;
        setgLogFile(g[i], gMain.gLogFile);
        setConfigFileName(g[i], gMain.gConfigFileName);
		addBasicEncoderSettings(g[i]);
        initializeGlobals(g[i]);
        oddcastv3_init(g[i]);
		connectToServer(g[i]);
    }

	pthread_create(&sourceThread, NULL, &startSourceThread, sourceURL);
	pthread_create(&reconnectThread, NULL, &startReconnectThread, NULL);
	pthread_create(&decodeThread, NULL, &startDecodeThread, NULL);
	pthread_join(sourceThread, NULL);
	pthread_join(decodeThread, NULL);
	return 1;
}

