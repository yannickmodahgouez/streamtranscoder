// streamTranscoderUIv3Dlg.h : header file
//

#if !defined(AFX_STREAMTRANSCODERUIV3DLG_H__D30EE60E_2F96_42A2_82BD_F482BCF8C031__INCLUDED_)
#define AFX_STREAMTRANSCODERUIV3DLG_H__D30EE60E_2F96_42A2_82BD_F482BCF8C031__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "liboddcast/liboddcast.h"

/////////////////////////////////////////////////////////////////////////////
// CStreamTranscoderUIv3Dlg dialog

class CStreamTranscoderUIv3Dlg : public CDialog
{
// Construction
public:
	boolean isConnected;
	void addConfigVariables(oddcastGlobals *g);
	void reloadConfigs();
	CStreamTranscoderUIv3Dlg(CWnd* pParent = NULL);	// standard constructor

	RECT	m_EncodersCtrlPos;
	RECT	m_SourceURLCtrlPos;
	RECT	m_NumEncodersCtrlPos;
	RECT	m_MainWindowPos;

// Dialog Data
	//{{AFX_DATA(CStreamTranscoderUIv3Dlg)
	enum { IDD = IDD_STREAMTRANSCODERUIV3_DIALOG };
	CStatic	m_MetadataCtrl;
	CEdit	m_SourceURLCtrl;
	CEdit	m_NumEncodersCtrl;
	CButton	m_Connect;
	CListCtrl	m_EncodersCtrl;
	CString	m_SourceURL;
	int		m_NumEncoders;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStreamTranscoderUIv3Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CStreamTranscoderUIv3Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnConnect();
	afx_msg void OnChangeNumencoders();
	afx_msg void OnKillfocusNumencoders();
	afx_msg void OnKillfocusSourceurl();
	virtual void OnOK();
	afx_msg void OnDblclkEncoders(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDeltaposEncoderspin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STREAMTRANSCODERUIV3DLG_H__D30EE60E_2F96_42A2_82BD_F482BCF8C031__INCLUDED_)
