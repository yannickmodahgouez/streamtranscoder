// streamTranscoderUIv3Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "streamTranscoderUIv3.h"
#include "streamTranscoderUIv3Dlg.h"

#include "socket_client.h"
#include "sourcethread.h"
#include "decodethread.h"
#include "reconnectthread.h"
#include "liboddcast/liboddcast.h"
#include "cbuffer.h"


#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <errno.h>
#include <unistd.h>
#include <string.h>
#else
#include <winsock.h>
#include <io.h>
#endif

#ifdef WIN32
#define close _close
#endif

#include <pthread.h>
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


pthread_t   sourceThread = 0;
pthread_t   decodeThread = 0;
pthread_t   reconnectThread = 0;

extern int decoderLoop;
extern int reconnectLoop;
extern int sourceMainLoop;
extern int sourceInnerLoop;
extern int client_socket;
extern int reconnectThreadRunning;
extern int sourceThreadRunning;
extern CBUFFER	sourceCBuffer;
extern CBUFFER	metadataCBuffer;




CStreamTranscoderUIv3Dlg *pDialog;

int showConsole = 1;
int triggeredStop = 0;
#define MAX_ENCODERS 50
oddcastGlobals          *g[MAX_ENCODERS];
oddcastGlobals          gMain;


void updateEncoderText(int enc, char *value) {
		int numItems = pDialog->m_EncodersCtrl.GetItemCount();
		if(enc >= numItems) {
			pDialog->m_EncodersCtrl.InsertItem(enc, (char *) "");
		}

		pDialog->m_EncodersCtrl.SetItemText(enc, 0, (char *) value);
 }
void updateEncoderDest(int enc, char *value) {
		int numItems = pDialog->m_EncodersCtrl.GetItemCount();
		if(enc >= numItems) {
			pDialog->m_EncodersCtrl.InsertItem(enc, (char *) "");
		}

		pDialog->m_EncodersCtrl.SetItemText(enc, 1, (char *) value);
}
void inputMetadataCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
	
	int enc = g->encoderNumber;

	if (enc == 0) {
		pDialog->m_MetadataCtrl.SetWindowText((char *)pValue);
	}

    LogMessage(g, LOG_DEBUG, "Encoder %d: %s\n", g->encoderNumber, pValue);
   //mainWindow->inputMetadataCallback(g->encoderNumber, pValue);
    //fprintf(stdout, "Encoder %d: %s\n", g->encoderNumber, pValue);
}
void outputStatusCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
    //mainWindow->outputStatusCallback(g->encoderNumber, pValue);
	int enc = g->encoderNumber;

	updateEncoderText(enc, (char *)pValue);
    LogMessage(g, LOG_DEBUG, "Encoder %d: %s\n", g->encoderNumber, pValue);
} 
void writeBytesCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
    //mainWindow->writeBytesCallback(g->encoderNumber, pValue);

	/* pValue is a long */
	static long startTime[MAX_ENCODERS];
	static long endTime[MAX_ENCODERS];
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

			outputStatusCallback(g, (void *)kBPSstr);
			startTime[enc_index] = 0;
		}
	}

} 
void outputServerNameCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
    //mainWindow->outputServerNameCallback(g->encoderNumber, pValue);
	int enc = g->encoderNumber;

	updateEncoderText(enc, (char *)pValue);
//    fprintf(stdout, "Encoder %d: %s\n", g->encoderNumber, pValue);
}
void outputBitrateCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
    //mainWindow->outputBitrateCallback(g->encoderNumber, pValue);
	int enc = g->encoderNumber;

	char	msg[1024] = "";

	sprintf(msg, "%s:%s%s %s", g->gServer, g->gPort, g->gMountpoint, pValue);
	updateEncoderDest(enc, (char *)msg);
//    fprintf(stdout, "Encoder %d: %s\n", g->encoderNumber, pValue);
} 
void outputStreamURLCallback(void *gbl, void *pValue) {
    oddcastGlobals *g = (oddcastGlobals *)gbl;
    //mainWindow->outputStreamURLCallback(g->encoderNumber, pValue);
	int enc = g->encoderNumber;

	updateEncoderText(enc, (char *)pValue);
//    fprintf(stdout, "Encoder %d: %s\n", g->encoderNumber, pValue);
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


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStreamTranscoderUIv3Dlg dialog

CStreamTranscoderUIv3Dlg::CStreamTranscoderUIv3Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStreamTranscoderUIv3Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStreamTranscoderUIv3Dlg)
	m_SourceURL = _T("");
	m_NumEncoders = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CStreamTranscoderUIv3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStreamTranscoderUIv3Dlg)
	DDX_Control(pDX, IDC_METADATA, m_MetadataCtrl);
	DDX_Control(pDX, IDC_SOURCEURL, m_SourceURLCtrl);
	DDX_Control(pDX, IDC_NUMENCODERS, m_NumEncodersCtrl);
	DDX_Control(pDX, IDC_CONNECT, m_Connect);
	DDX_Control(pDX, IDC_ENCODERS, m_EncodersCtrl);
	DDX_Text(pDX, IDC_SOURCEURL, m_SourceURL);
	DDX_Text(pDX, IDC_NUMENCODERS, m_NumEncoders);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CStreamTranscoderUIv3Dlg, CDialog)
	//{{AFX_MSG_MAP(CStreamTranscoderUIv3Dlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_EN_CHANGE(IDC_NUMENCODERS, OnChangeNumencoders)
	ON_EN_KILLFOCUS(IDC_NUMENCODERS, OnKillfocusNumencoders)
	ON_EN_KILLFOCUS(IDC_SOURCEURL, OnKillfocusSourceurl)
	ON_NOTIFY(NM_DBLCLK, IDC_ENCODERS, OnDblclkEncoders)
	ON_WM_SIZE()
	ON_NOTIFY(UDN_DELTAPOS, IDC_ENCODERSPIN, OnDeltaposEncoderspin)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStreamTranscoderUIv3Dlg message handlers

BOOL CStreamTranscoderUIv3Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	pDialog = this;

	m_EncodersCtrl.InsertColumn(0, "Encoder Status");
	m_EncodersCtrl.SetColumnWidth(0, 150);
	m_EncodersCtrl.InsertColumn(1, "Destination");
	m_EncodersCtrl.SetColumnWidth(1, 400);
	m_EncodersCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	char    currentlogFile[1024] = "";
	char	logPrefix[255] = "streamTranscoder";
	char	configFile[1024] = "";
	
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


    readConfigFile(&gMain, 0);

	m_NumEncoders = gMain.gNumEncoders;
	m_SourceURL = gMain.gSourceURL;

	outputStatusCallback(&gMain, "Ready to connect");

	reloadConfigs();
	UpdateData(FALSE);
	// TODO: Add extra initialization here
	
	isConnected = false;
	GetClientRect(&m_MainWindowPos);
	m_EncodersCtrl.GetClientRect(&m_EncodersCtrlPos);
	m_NumEncodersCtrl.GetClientRect(&m_SourceURLCtrlPos);
	m_NumEncodersCtrl.GetClientRect(&m_NumEncodersCtrlPos);

	if (gMain.autoconnect) {
		outputStatusCallback(&gMain, "Autoconnecting in 5 seconds");
		SetTimer(10, 5000, NULL);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CStreamTranscoderUIv3Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CStreamTranscoderUIv3Dlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CStreamTranscoderUIv3Dlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CStreamTranscoderUIv3Dlg::OnConnect() 
{
	// TODO: Add your control notification handler code here

	CWaitCursor waitCursor;

	if (isConnected) {
		triggeredStop = 0;
		SetTimer(11, 300, NULL);
	}
	else {

		for(int i = 0; i < gMain.gNumEncoders; i++) {
			if (!connectToServer(g[i])) {
				char buf[255] = "";
				sprintf(buf, "Disconnected from server");
				g[i]->forcedDisconnect = true;
				g[i]->forcedDisconnectSecs = time(&(g[i]->forcedDisconnectSecs));
				g[i]->serverStatusCallback(g[i], (void *) buf);
			}
		}

		pthread_create(&sourceThread, NULL, &startSourceThread, (char *)LPCSTR(m_SourceURL));
		pthread_create(&reconnectThread, NULL, &startReconnectThread, NULL);
		pthread_create(&decodeThread, NULL, &startDecodeThread, NULL);
		isConnected = true;
		m_Connect.SetWindowText("Stop");
		m_NumEncodersCtrl.EnableWindow(FALSE);
		m_SourceURLCtrl.EnableWindow(FALSE);
	}
}

void CStreamTranscoderUIv3Dlg::OnChangeNumencoders() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

	
}

void CStreamTranscoderUIv3Dlg::reloadConfigs()
{
    for(int i = 0; i < gMain.gNumEncoders; i++) {
        if(!g[i]) {
            g[i] = (oddcastGlobals *) malloc(sizeof(oddcastGlobals));
            memset(g[i], '\000', sizeof(oddcastGlobals));
        	initializeGlobals(g[i]);

			addConfigVariables(g[i]);
        }
		else {
        	initializeGlobals(g[i]);
		}


        g[i]->encoderNumber = i + 1;
		char    currentlogFile[1024] = "";
		char	logPrefix[255] = "streamTranscoder";

		sprintf(currentlogFile, "%s_%d", logPrefix, g[i]->encoderNumber);

		setDefaultLogFileName(currentlogFile);
        setgLogFile(g[i], currentlogFile);
        setConfigFileName(g[i], gMain.gConfigFileName);
        oddcastv3_init(g[i]);
    }
	outputStatusCallback(&gMain, "Ready to connect");
}

void CStreamTranscoderUIv3Dlg::addConfigVariables(oddcastGlobals *g)
{
	addBasicEncoderSettings(g);
}

void CStreamTranscoderUIv3Dlg::OnKillfocusNumencoders() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	gMain.gNumEncoders = m_NumEncoders;
	config_write(&gMain);
	writeConfigFile(&gMain);

	m_EncodersCtrl.DeleteAllItems();
	reloadConfigs();
}

void CStreamTranscoderUIv3Dlg::OnKillfocusSourceurl() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	setSourceURL(&gMain, (char *)(LPCSTR)m_SourceURL);
	config_write(&gMain);
	writeConfigFile(&gMain);

}

void CStreamTranscoderUIv3Dlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);	
	config_write(&gMain);
	writeConfigFile(&gMain);
	CDialog::OnOK();
}

void CStreamTranscoderUIv3Dlg::OnDblclkEncoders(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
	int iItem = m_EncodersCtrl.GetNextItem(-1, LVNI_SELECTED);

	if (iItem >= 0) {
		if (isConnected) {
			MessageBox("Cannot edit settings while connected...", "Msg", MB_OK);
			return;
		}
		char	logPrefix[255] = "streamTranscoder";
		char	configFile[1024] = "";
		char	cmd[1024] = "";

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		
		
		sprintf(configFile, "%s_%d.cfg", logPrefix, iItem);

		sprintf(cmd, "notepad %s", configFile);

		int ok = 1;
		if( !CreateProcess( NULL, // No module name (use command line). 
				cmd, // Command line. 
				NULL,             // Process handle not inheritable. 
				NULL,             // Thread handle not inheritable. 
				FALSE,            // Set handle inheritance to FALSE. 
				0,                // No creation flags. 
				NULL,             // Use parent's environment block. 
				NULL,             // Use parent's starting directory. 
				&si,              // Pointer to STARTUPINFO structure.
				&pi )             // Pointer to PROCESS_INFORMATION structure.
			) {
			   ok = 0;
		}

			// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		reloadConfigs();
	}
}

void CStreamTranscoderUIv3Dlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	return;
	RECT m_MainWindowScreenPos;

	GetWindowRect(&m_MainWindowScreenPos);

	int currentWidth = m_MainWindowPos.right - m_MainWindowPos.left;
	int currentHeight = m_MainWindowPos.bottom - m_MainWindowPos.top;

	int deltaWidth = cx - currentWidth;
	int deltaHeight = cy - currentHeight;

	if (m_EncodersCtrl) {
		RECT m_EncodersCtrlPos;
//		m_EncodersCtrl.GetWindowRect(&m_EncodersCtrlScreenPos);
		m_EncodersCtrl.GetWindowRect(&m_EncodersCtrlPos);

		m_EncodersCtrlPos.left = m_EncodersCtrlPos.left - m_MainWindowScreenPos.left;
		m_EncodersCtrlPos.top = m_EncodersCtrlPos.top - m_MainWindowScreenPos.top;

		m_EncodersCtrlPos.right = m_EncodersCtrlPos.right + deltaWidth;
		m_EncodersCtrlPos.bottom = m_EncodersCtrlPos.bottom + deltaHeight;
		m_EncodersCtrl.MoveWindow(&m_EncodersCtrlPos, TRUE);

//		m_EncodersCtrl.SetWindowPos(this, m_EncodersCtrlPos.left, m_EncodersCtrlPos.top, 
//									m_EncodersCtrlPos.right-m_EncodersCtrlPos.left, 
//									m_EncodersCtrlPos.bottom-m_EncodersCtrlPos.top, SWP_NOZORDER);
	}
	if (m_SourceURLCtrl) {
		m_SourceURLCtrl.GetClientRect(&m_SourceURLCtrlPos);
		m_SourceURLCtrlPos.right = m_SourceURLCtrlPos.right + deltaWidth;
		//m_SourceURLCtrl.MoveWindow(&m_SourceURLCtrlPos, TRUE);
	//	m_SourceURLCtrl.SetWindowPos(NULL, m_SourceURLCtrlPos.left, 
	//								m_SourceURLCtrlPos.top, 
	//								m_SourceURLCtrlPos.right-m_SourceURLCtrlPos.left, 
	//								m_SourceURLCtrlPos.bottom-m_SourceURLCtrlPos.top, SWP_NOZORDER);
	}
	if (m_NumEncodersCtrl) {
		m_NumEncodersCtrl.GetClientRect(&m_NumEncodersCtrlPos);
		m_NumEncodersCtrlPos.right = m_NumEncodersCtrlPos.right + deltaWidth;
		//m_NumEncodersCtrl.MoveWindow(&m_NumEncodersCtrlPos, TRUE);
		//m_NumEncodersCtrl.SetWindowPos(NULL, m_NumEncodersCtrlPos.left, 
	//								m_NumEncodersCtrlPos.top, 
	//								m_NumEncodersCtrlPos.right-m_NumEncodersCtrlPos.left, 
	//								m_NumEncodersCtrlPos.bottom-m_NumEncodersCtrlPos.top, SWP_NOZORDER);
	}




	// TODO: Add your message handler code here
	
}

void CStreamTranscoderUIv3Dlg::OnDeltaposEncoderspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	if (0 < pNMUpDown->iDelta) {
		if (m_NumEncoders > 0) {
	        m_NumEncoders--;
		}
	}
    else {
		m_NumEncoders++;
	}
	UpdateData(FALSE);

	OnKillfocusNumencoders();
	*pResult = 0;
}

void CStreamTranscoderUIv3Dlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 10) {
		LogMessage(&gMain, LOG_DEBUG, "Connect called from OnTimer");

		OnConnect();
		KillTimer(10);
	}
	if (nIDEvent == 11) {
		void *ret = 0;

		if ((!reconnectThreadRunning) && (!sourceThreadRunning)) {
			LogMessage(&gMain, LOG_DEBUG, "Disconnect called from OnTimer");
			
			isConnected = false;
			m_Connect.SetWindowText("Connect");
			m_NumEncodersCtrl.EnableWindow(TRUE);
			m_SourceURLCtrl.EnableWindow(TRUE);
			outputStatusCallback(&gMain, (void *)"Source Disconnected");
			for(int i = 0; i < gMain.gNumEncoders; i++) {
				outputStatusCallback(g[i], "Disconnected");
			}
			cbuffer_destroy(&sourceCBuffer);
			cbuffer_destroy(&metadataCBuffer);
			KillTimer(11);
		}
		if (!triggeredStop) {
			triggeredStop = 1;
			LogMessage(&gMain, LOG_DEBUG, "Stopping Reconnect thread");
			outputStatusCallback(&gMain, "Requesting stop");
			reconnectLoop = 0;

			/* Stop all connected encoders */
			for(int i = 0; i < gMain.gNumEncoders; i++) {
				if (g[i]->weareconnected) {
					disconnectFromServer(g[i]);
				}
			}

			decoderLoop = 0;
			/* Lets stuff the cbuffer so we exit out of the loop */
			char	pData[1024] = "";
			cbuffer_insert(&sourceCBuffer, pData, sizeof(pData));
			sourceMainLoop = 0;
			sourceInnerLoop = 0;
		}


//		KillTimer(11);

	}
	CDialog::OnTimer(nIDEvent);
}
