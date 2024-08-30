// InitParamEEGProcessBasic.cpp : implementation file
//

#include "stdafx.h"
#include "EEG_marking_tool.h"
#include "InitParamEEGProcessBasic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// InitParamEEGProcessBasic dialog


InitParamEEGProcessBasic::InitParamEEGProcessBasic(CWnd* pParent /*=NULL*/)
	: CDialog(InitParamEEGProcessBasic::IDD, pParent)
{
	//{{AFX_DATA_INIT(InitParamEEGProcessBasic)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void InitParamEEGProcessBasic::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(InitParamEEGProcessBasic)
	DDX_Control(pDX, IDC_COMBO_InitParam_Filter_HPF, m_Filter_HPF);
	DDX_Control(pDX, IDC_COMBO_Initparam_Filter_LPF, m_Filter_LPF);
	DDX_Control(pDX, IDC_InitParam_Edit_SamplingRate, m_SamplingRate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(InitParamEEGProcessBasic, CDialog)
	//{{AFX_MSG_MAP(InitParamEEGProcessBasic)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// InitParamEEGProcessBasic message handlers

void InitParamEEGProcessBasic::OnOK() 
{
	// TODO: Add extra validation here
	// TODO: Add your control notification handler code here	
	CString temp;

	// sampling rate
	m_SamplingRate.GetWindowText(temp);
	sampling_rate_buf = atof(temp);

	//strcpy(sampling_rate_buf, "");
	//strcat(sampling_rate_buf, temp);

	// filter low pass frequency
	m_Filter_LPF.GetWindowText(temp);	
	Filter_LPF_buf = atof(temp);

	//strcpy(Filter_LPF_buf, "");
	//strcat(Filter_LPF_buf, temp);
	
	// filter high pass frequency
	m_Filter_HPF.GetWindowText(temp);	
	Filter_HPF_buf = atof(temp);

	//strcpy(Filter_HPF_buf, "");
	//strcat(Filter_HPF_buf, temp);

	// set the button clicked boolean variable to true
	// and close the dialog box
	button_clicked = true;
	//CDialog::OnCancel();
  
	CDialog::OnOK();
}

BOOL InitParamEEGProcessBasic::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	// TODO: Add extra initialization here
	double t;
	CString buf;	

	// sampling rate - default initialization
	t = 125;
	buf.Format(_T("%f"), t);
	m_SamplingRate.SetWindowText(buf);

	// filter - low pass and high pass - construction

	// low pass filter - initialize the combo box elements
	t = 0.01;	buf.Format(_T("%lf"), t);	m_Filter_LPF.AddString(buf);
	t = 0.016;	buf.Format(_T("%lf"), t);	m_Filter_LPF.AddString(buf);
	t = 0.16;	buf.Format(_T("%lf"), t);	m_Filter_LPF.AddString(buf);
	t = 0.3;	buf.Format(_T("%lf"), t);	m_Filter_LPF.AddString(buf);
	t = 0.5;	buf.Format(_T("%lf"), t);	m_Filter_LPF.AddString(buf);
	t = 1;		buf.Format(_T("%lf"), t);	m_Filter_LPF.AddString(buf);
	t = 1.6;	buf.Format(_T("%lf"), t);	m_Filter_LPF.AddString(buf);
	t = 2;		buf.Format(_T("%lf"), t);	m_Filter_LPF.AddString(buf);
	t = 3;		buf.Format(_T("%lf"), t);	m_Filter_LPF.AddString(buf);
	t = 5;		buf.Format(_T("%lf"), t);	m_Filter_LPF.AddString(buf);
	m_Filter_LPF.SetCurSel(6);	// show the item corresponding to 1.6

	// high pass filter - initialize the combo box elements
	t = 10;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 15;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 25;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 30;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 35;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 40;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 50;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 60;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);	
	t = 70;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 100;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 150;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 200;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 300;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 500;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 1000;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	t = 1500;	buf.Format(_T("%lf"), t);	m_Filter_HPF.AddString(buf);
	m_Filter_HPF.SetCurSel(1);	// show the item corresponding to 15

	// finally the extract button clicked boolean info - it must be set to false
	button_clicked = false;
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
