// EEG_marking_tool.h : main header file for the EEG_MARKING_TOOL application
//

#if !defined(AFX_EEG_MARKING_TOOL_H__D6011924_49D5_4177_9F8D_D9BADC724C2E__INCLUDED_)
#define AFX_EEG_MARKING_TOOL_H__D6011924_49D5_4177_9F8D_D9BADC724C2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CEEG_marking_toolApp:
// See EEG_marking_tool.cpp for the implementation of this class
//

class CEEG_marking_toolApp : public CWinApp
{
public:
	CEEG_marking_toolApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEEG_marking_toolApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CEEG_marking_toolApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EEG_MARKING_TOOL_H__D6011924_49D5_4177_9F8D_D9BADC724C2E__INCLUDED_)
