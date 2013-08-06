// streamTranscoderUIv3.h : main header file for the STREAMTRANSCODERUIV3 application
//

#if !defined(AFX_STREAMTRANSCODERUIV3_H__0AE691DD_D200_46FB_8D69_1B1B98E55DA2__INCLUDED_)
#define AFX_STREAMTRANSCODERUIV3_H__0AE691DD_D200_46FB_8D69_1B1B98E55DA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CStreamTranscoderUIv3App:
// See streamTranscoderUIv3.cpp for the implementation of this class
//

class CStreamTranscoderUIv3App : public CWinApp
{
public:
	CStreamTranscoderUIv3App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStreamTranscoderUIv3App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CStreamTranscoderUIv3App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STREAMTRANSCODERUIV3_H__0AE691DD_D200_46FB_8D69_1B1B98E55DA2__INCLUDED_)
