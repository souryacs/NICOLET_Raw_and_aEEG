// EEG_marking_toolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EEG_marking_tool.h"
#include "EEG_marking_toolDlg.h"
#include "typepatientreport.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
// CEEG_marking_toolDlg dialog

CEEG_marking_toolDlg::CEEG_marking_toolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEEG_marking_toolDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEEG_marking_toolDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEEG_marking_toolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEEG_marking_toolDlg)
	DDX_Control(pDX, IDC_STATIC_GRID2, m_Grid_Amp);
	DDX_Control(pDX, IDC_STATIC_GRID, m_Grid);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEEG_marking_toolDlg, CDialog)
	//{{AFX_MSG_MAP(CEEG_marking_toolDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_VIEW_NEXT, OnViewNext)
	ON_COMMAND(ID_VIEW_PREV, OnViewPrev)
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_TIMEBASE_6, OnTimebase6)
	ON_COMMAND(ID_SENSITIVITY_1, OnSensitivity1)
	ON_COMMAND(ID_SENSITIVITY_2, OnSensitivity2)
	ON_COMMAND(ID_SENSITIVITY_3, OnSensitivity3)
	ON_COMMAND(ID_SENSITIVITY_5, OnSensitivity5)
	ON_COMMAND(ID_SENSITIVITY_7, OnSensitivity7)
	ON_COMMAND(ID_SENSITIVITY_10, OnSensitivity10)
	ON_COMMAND(ID_SENSITIVITY_15, OnSensitivity15)
	ON_COMMAND(ID_SENSITIVITY_20, OnSensitivity20)
	ON_COMMAND(ID_SENSITIVITY_30, OnSensitivity30)
	ON_COMMAND(ID_SENSITIVITY_50, OnSensitivity50)
	ON_COMMAND(ID_SENSITIVITY_70, OnSensitivity70)
	ON_COMMAND(ID_SENSITIVITY_100, OnSensitivity100)
	ON_COMMAND(ID_SENSITIVITY_200, OnSensitivity200)
	ON_COMMAND(ID_SENSITIVITY_500, OnSensitivity500)
	ON_COMMAND(ID_TIMEBASE_8, OnTimebase8)
	ON_COMMAND(ID_TIMEBASE_10, OnTimebase10)
	ON_COMMAND(ID_TIMEBASE_15, OnTimebase15)
	ON_COMMAND(ID_TIMEBASE_20, OnTimebase20)
	ON_COMMAND(ID_TIMEBASE_30, OnTimebase30)
	ON_COMMAND(ID_TIMEBASE_60, OnTimebase60)
	ON_COMMAND(ID_TIMEBASE_120, OnTimebase120)
	ON_COMMAND(ID_TIMEBASE_240, OnTimebase240)
	ON_COMMAND(ID_MARKING_SEIZURE, OnMarkingSeizure)
	ON_COMMAND(ID_MARKING_BURST, OnMarkingBurst)
	ON_COMMAND(ID_MARKING_ARTIFACT, OnMarkingArtifact)
	ON_COMMAND(ID_MARKING_SLEEPSPINDLE, OnMarkingSleepspindle)
	ON_COMMAND(ID_MARKING_BURSTSUPPRESSION, OnMarkingBurstSuppression)
	ON_COMMAND(ID_MARKING_UNKNOWNMARK, OnMarkingUnknownmark)
	ON_COMMAND(ID_MARKING_UNMARK, OnMarkingUnmark)
	ON_WM_KEYDOWN()
	ON_WM_CLOSE()
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_FILE_SHOWVIDEO, OnFileShowvideo)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CAPTURECHANGED()
	ON_COMMAND(ID_TOOLS_PATIENTREPORT, OnToolsPatientreport)
	ON_COMMAND(ID_NEXT_EVENT, OnNextEvent)
	ON_COMMAND(ID_PREV_EVENT, OnPrevEvent)
	ON_WM_ERASEBKGND()
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_aEEG_FULL_SCALE, OnaEEGFULLSCALE)
	ON_COMMAND(ID_aEEG_6_CM_HR, OnaEEG6CMHR)
	ON_COMMAND(ID_aEEG_30_CM_HR, OnaEEG30CMHR)
	ON_COMMAND(ID_VIEW_NEXT_AEEG_PAGE, OnViewNextAeegPage)
	ON_COMMAND(ID_VIEW_PREV_AEEG_PAGE, OnViewPrevAeegPage)
	ON_WM_CANCELMODE()
	ON_COMMAND(ID_MARKING_NORMAL, OnMarkingNormalEEG)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEEG_marking_toolDlg message handlers

BOOL CEEG_marking_toolDlg::OnInitDialog()
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
	
	// TODO: Add extra initialization here	
	HDC screen = ::GetDC(NULL);
	int hSize = ::GetDeviceCaps(screen,HORZSIZE);
	int hRes = ::GetDeviceCaps(screen,HORZRES);
	int vRes = ::GetDeviceCaps(screen,VERTRES);

	PixelsPerMM = hRes/hSize;   // pixels per millimeter
	//double PixelsPerInch = PixelsPerMM * 25.4; //dpi

	// initialize global data pointers as NULL
	marking_database = NULL;
	mark_database_count = NULL;

	pen.CreatePen(PS_SOLID, 5, RGB(255,0,0));		// raw EEG pen
	pen_Amp.CreatePen(PS_SOLID, 3, RGB(255,0,0));	// amp EEG draw pen

	// function to get the current executable directory
	TCHAR current_dir[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, current_dir);
	
	exec_directory = current_dir;

	// initialize color code for different marking
	// green (seizure)
	color_code[0][0] = 0;
	color_code[0][1] = 255;
	color_code[0][2] = 0;
	// yellow (burst)
	color_code[1][0] = 255;
	color_code[1][1] = 255;
	color_code[1][2] = 0;
	// cyan (artifact)
	color_code[2][0] = 0;
	color_code[2][1] = 128;
	color_code[2][2] = 255;
	// magenta (sleep spindle)
	color_code[3][0] = 128;
	color_code[3][1] = 0;
	color_code[3][2] = 0;
	// red (burst suppression)
	color_code[4][0] = 255;
	color_code[4][1] = 0;
	color_code[4][2] = 0;
	// pink (normal)
	color_code[5][0] = 255;
	color_code[5][1] = 0;
	color_code[5][2] = 255;
	// black (unknown)
	color_code[6][0] = 0;
	color_code[6][1] = 0;
	color_code[6][2] = 0;

	for (int i = 0; i < 7; i++)
	{
		xpen[i].CreatePen(PS_SOLID, 3, RGB(color_code[i][0], color_code[i][1], color_code[i][2]));
	}

	m_Grid.GetClientRect(&rect);
	hWnd = m_Grid.m_hWnd;
	hDC = m_Grid.GetDC()->m_hDC;
	SelectObject(hDC, &pen);
	screen_y_size = rect.Height();
	screen_x_size = rect.Width();	
	
	m_Grid_Amp.GetClientRect(&rect_Amp);
	hWnd_Amp = m_Grid_Amp.m_hWnd;
	hDC_Amp = m_Grid_Amp.GetDC()->m_hDC;
	SelectObject(hDC_Amp, &pen_Amp);
	screen_y_size_Amp = rect_Amp.Height();
	screen_x_size_Amp = rect_Amp.Width();

	// sourya - added
	// initialize the temporary dcmem and associated structures
	dcMem.CreateCompatibleDC(m_Grid_Amp.GetDC());
	bitmap.CreateCompatibleBitmap(m_Grid_Amp.GetDC(), rect_Amp.Width(), rect_Amp.Height());

	pDC_Amp = m_Grid_Amp.GetDC();	// sourya

	drawBrush = CreateSolidBrush(RGB(255,255,255));		// raw EEG
	drawBrush_Amp = CreateSolidBrush(RGB(255,255,255));	// amp EEG			

	repaint_on = true;

	aeeg_computation_done = 0; 

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEEG_marking_toolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CEEG_marking_toolDlg::OnPaint() 
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
    CPaintDC dc(this);
		// plot the EEG
		//if (repaint_on)
		if (aeeg_computation_done == 1)
		{
			plot_amp_EEG_signal();		
			plot_raw_EEG_signal(1);
		}	
		//CDialog::OnPaint();
	}
}


// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEEG_marking_toolDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CEEG_marking_toolDlg::OnFileOpen() 
{
	// TODO: Add your command handler code here

	// szFilters is a text string that includes two file name filters:
	//static char BASED_CODE szFilters[]= "Text Files (*.txt)|*.txt|Ascii Files (*.ascii)|*.ascii||";
	//static char BASED_CODE szFilters[]= "Text Files (*.txt)|*.txt|";
	static char BASED_CODE szFilters[]= "Text Files (*.txt)|*.txt|Binary Files (*.bindata)|*.bindata||";

	//at first initialize several parameters related to future processing
	Init_Param_EEG();


	CFileDialog fOpenDlg(TRUE, "txt", "*.txt", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, this);

	if(fOpenDlg.DoModal() == IDOK)
	{
		directory_name = fOpenDlg.GetPathName();
		fileName = fOpenDlg.GetFileName();
		directory_name.TrimRight(fileName);

		// if text format file is selected - that is, it is the nicolet machine generated text file
		if (fileName.Find(".txt") != -1)
		{
			SetWindowText("Raw AMP EEG data - text format");	

			//read amplitude EEG machine configuration parameters
			Read_Text_Amp_EEG_machine_Config();	
			
			// call the video summarization function
			Video_Summarize();
			
			//amplitude EEG file read	
			Read_Amp_EEG_machine_input_data();		
			
			// init some encoding and display parameters
			Init_Display_Marking_parameters();
			
			// call the channel detailed data read and processing function	
			Read_and_Process_EEG_Data();			
		}
		/*
		// else if binary format file is selected
		else if (fileName.Find(".bindata") != -1)
		{
			SetWindowText("Raw AMP EEG data - binary format");	

			// general binary format data read and process
			Read_Bin_Fmt_aEEG_Data();	

			// init some encoding and display parameters
			//Init_Display_Marking_parameters(true);		

			// call the channel detailed data read and processing function	
			//Read_and_Process_EEG_Data();	
		}
		*/
	}
}


/*
	next page raw EEG display
*/
void CEEG_marking_toolDlg::OnViewNext(int flag) 
{
	// TODO: Add your command handler code here
	// flag variable - if 0, next page is displayed
	// if 1 then only 1 second displacement is done

	if (sel_raw_eeg_end_time < total_recording_duration_sec)	//end of file is not reached yet
	{
		//update the current raw EEG display boundary
		if (flag == 0)
			sel_raw_eeg_start_time = sel_raw_eeg_end_time;
		else
			sel_raw_eeg_start_time = sel_raw_eeg_start_time + 1;

		sel_raw_eeg_end_time = sel_raw_eeg_start_time + raw_eeg_per_page_duration;
		if (sel_raw_eeg_end_time > total_recording_duration_sec)	//crosses time
		{
			sel_raw_eeg_end_time = total_recording_duration_sec;
			sel_raw_eeg_start_time = sel_raw_eeg_end_time - raw_eeg_per_page_duration;
		}

		// we have to check whether the new raw EEG display boundary exceeds current aEEG display
		// boundary - if so, then we have to adjust aEEG display boundary
		if (sel_raw_eeg_end_time > sel_aeeg_end_time)
		{
			sel_aeeg_start_time = sel_aeeg_start_time + (int)((sel_aeeg_end_time - sel_aeeg_start_time) / 2);
			sel_aeeg_end_time = sel_aeeg_start_time + aeeg_per_page_duration;
			if (sel_aeeg_end_time > total_recording_duration_sec)	//crosses time
			{
				sel_aeeg_end_time = total_recording_duration_sec;
				sel_aeeg_start_time = sel_aeeg_end_time - aeeg_per_page_duration;
				if (sel_aeeg_start_time < 0)
					sel_aeeg_start_time = 0;
			}	
			aeeg_computation_done = 0;	// reset the aEEG computation flag
			plot_amp_EEG_signal();				
		}

		//call plot raw EEG
		plot_raw_EEG_signal(0);
	}
}

/*
	prev page raw EEG display
*/
void CEEG_marking_toolDlg::OnViewPrev(int flag) 
{
	// TODO: Add your command handler code here
	// flag variable - if 0, prev page is displayed
	// if 1 then only 1 second displacement is done

	if (sel_raw_eeg_start_time > 0)	//start of file is not reached yet
	{
		//update the current raw EEG display boundary
		if (flag == 0)
			sel_raw_eeg_end_time = sel_raw_eeg_start_time;
		else
			sel_raw_eeg_end_time = sel_raw_eeg_end_time -  1;

		sel_raw_eeg_start_time = sel_raw_eeg_end_time - raw_eeg_per_page_duration;
		if (sel_raw_eeg_start_time < 0)
		{
			sel_raw_eeg_start_time = 0;
			sel_raw_eeg_end_time = sel_raw_eeg_start_time + raw_eeg_per_page_duration;
		}

		// we have to check whether the new raw EEG display boundary exceeds current aEEG display
		// boundary - if so, then we have to adjust aEEG display boundary
		if (sel_raw_eeg_start_time < sel_aeeg_start_time)
		{
			sel_aeeg_end_time = sel_aeeg_end_time - (int)((sel_aeeg_end_time - sel_aeeg_start_time) / 2);
			sel_aeeg_start_time = sel_aeeg_end_time - aeeg_per_page_duration;
			if (sel_aeeg_start_time < 0)
			{
				sel_aeeg_start_time = 0;
				sel_aeeg_end_time = sel_aeeg_start_time + aeeg_per_page_duration;
				if (sel_aeeg_end_time > total_recording_duration_sec)
					sel_aeeg_end_time = total_recording_duration_sec;
			}
			aeeg_computation_done = 0;	// reset the aEEG computation flag
			plot_amp_EEG_signal();				
		}

		//call plot raw EEG
		plot_raw_EEG_signal(0);
	}	
}

/*
	mouse click handler event
*/
void CEEG_marking_toolDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	int startx;

	// TODO: Add your message handler code here and/or call default
	startx = point.x;

	// if the clicked point lies outside the interval of aEEG display then we just consider 
	// the last portion
	if (startx > (screen_x_size_Amp - SCREEN_X_AMP_OFFSET))
		startx = (screen_x_size_Amp - SCREEN_X_AMP_OFFSET);
	
	// when user clicks on the amp eeg display window, we caluclate the current region time by taking 
	// note on the displayed aeeg section length and start time offset
	sel_raw_eeg_start_time = sel_aeeg_start_time + ((startx * 1.0) / (screen_x_size_Amp - SCREEN_X_AMP_OFFSET)) * aeeg_per_page_duration;
	sel_raw_eeg_end_time = sel_raw_eeg_start_time + raw_eeg_per_page_duration;

	// now check whether the raw EEG end time is within aEEG display range - if not so, then shift it
	if (sel_raw_eeg_end_time > sel_aeeg_end_time)
	{
		sel_raw_eeg_end_time = sel_aeeg_end_time;
		sel_raw_eeg_start_time = sel_raw_eeg_end_time - raw_eeg_per_page_duration;
	}

	// check whether raw EEG display time exceeds the total recording duration
	// if so then adjust the display boundary
	if (sel_raw_eeg_end_time > total_recording_duration_sec)
	{
		sel_raw_eeg_end_time = total_recording_duration_sec;
		sel_raw_eeg_start_time = sel_raw_eeg_end_time - raw_eeg_per_page_duration;
	}

	//call plot raw EEG
	plot_raw_EEG_signal(0);

	/////////////////
	CDialog::OnLButtonDown(nFlags, point);
}

/*
	this function is called when we call marking data save 
*/
void CEEG_marking_toolDlg::OnFileSave() 
{
	// TODO: Add your command handler code here
	FILE* fid;
	CString outfile = directory_name + output_text_filename; 
	struct data_mark_format* temp_data_mark;	

	//we save the marking database in one excel file
	int channel_no;

	fid = fopen(outfile, "w");	//file open	

	for (channel_no = 0; channel_no < no_of_channels; channel_no++)
	{
		// initialize with the header pointer of current channel event list
		temp_data_mark = marking_database[channel_no];
		while (temp_data_mark != NULL)
		{
			if (temp_data_mark->mark_start_time != -1)	// valid entry
				fprintf(fid, "%lf\t%lf\t%d\t%d\n", temp_data_mark->mark_start_time, temp_data_mark->mark_end_time, temp_data_mark->mark_event, channel_no);

			// advance to the next node
			temp_data_mark = temp_data_mark->next;
		}
	}
	fclose(fid);
	AfxMessageBox("File Saved !");
}

/*
   common timebase change event function
*/
void CEEG_marking_toolDlg::TimebaseChangeFunc(int timebase) 
{
	// raw EEG displays according to screen size and time base
    raw_eeg_per_page_duration = (int)((screen_x_size - SCREEN_X_OFFSET) / (PixelsPerMM * timebase));   // (in Sec) time
  if ( raw_eeg_per_page_duration > total_recording_duration_sec )
	{
    raw_eeg_per_page_duration = (int)total_recording_duration_sec;
	}   
  sel_raw_eeg_end_time = sel_raw_eeg_start_time + raw_eeg_per_page_duration;
	if (sel_raw_eeg_end_time > total_recording_duration_sec)
	{
		sel_raw_eeg_end_time = total_recording_duration_sec;
		sel_raw_eeg_start_time = sel_raw_eeg_end_time - raw_eeg_per_page_duration;
	}
	plot_raw_EEG_signal(0);
}

/* 
	event handling
*/
void CEEG_marking_toolDlg::OnTimebase6() 
{
	// TODO: Add your command handler code here
	timebase = 6;		
	TimebaseChangeFunc(timebase);
}

void CEEG_marking_toolDlg::OnSensitivity1() 
{
	// TODO: Add your command handler code here
	sensitivity = 1;   
	plot_raw_EEG_signal(0);
}

void CEEG_marking_toolDlg::OnSensitivity2() 
{
	// TODO: Add your command handler code here
	sensitivity = 2;   
	plot_raw_EEG_signal(0);	
}

void CEEG_marking_toolDlg::OnSensitivity3() 
{
	// TODO: Add your command handler code here
	sensitivity = 3;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity5() 
{
	// TODO: Add your command handler code here
	sensitivity = 5;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity7() 
{
	// TODO: Add your command handler code here
	sensitivity = 7;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity10() 
{
	// TODO: Add your command handler code here
	sensitivity = 10;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity15() 
{
	// TODO: Add your command handler code here
	sensitivity = 15;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity20() 
{
	// TODO: Add your command handler code here
	sensitivity = 20;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity30() 
{
	// TODO: Add your command handler code here
	sensitivity = 30;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity50() 
{
	// TODO: Add your command handler code here
	sensitivity = 50;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity70() 
{
	// TODO: Add your command handler code here
	sensitivity = 70;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity100() 
{
	// TODO: Add your command handler code here
	sensitivity = 100;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity200() 
{
	// TODO: Add your command handler code here
	sensitivity = 200;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnSensitivity500() 
{
	// TODO: Add your command handler code here
	sensitivity = 500;   
	plot_raw_EEG_signal(0);		
}

void CEEG_marking_toolDlg::OnTimebase8() 
{
	// TODO: Add your command handler code here
	timebase = 8;		
	TimebaseChangeFunc(timebase);
}

void CEEG_marking_toolDlg::OnTimebase10() 
{
	// TODO: Add your command handler code here
	timebase = 10;		
	TimebaseChangeFunc(timebase);
}

void CEEG_marking_toolDlg::OnTimebase15() 
{
	// TODO: Add your command handler code here
	timebase = 15;		
	TimebaseChangeFunc(timebase);
}

void CEEG_marking_toolDlg::OnTimebase20() 
{
	// TODO: Add your command handler code here
	timebase = 20;		
	TimebaseChangeFunc(timebase);
}

void CEEG_marking_toolDlg::OnTimebase30() 
{
	// TODO: Add your command handler code here
	timebase = 30;		
	TimebaseChangeFunc(timebase);
}

void CEEG_marking_toolDlg::OnTimebase60() 
{
	// TODO: Add your command handler code here
	timebase = 60;		
	TimebaseChangeFunc(timebase);
}

void CEEG_marking_toolDlg::OnTimebase120() 
{
	// TODO: Add your command handler code here
	timebase = 120;		
	TimebaseChangeFunc(timebase);
}

void CEEG_marking_toolDlg::OnTimebase240() 
{
	// TODO: Add your command handler code here
	timebase = 240;		
	TimebaseChangeFunc(timebase);	
}

void CEEG_marking_toolDlg::OnMarkingSeizure() 
{
	// TODO: Add your command handler code here
	mark_on = SEIZURE_MARK;
}

void CEEG_marking_toolDlg::OnMarkingBurst() 
{
	// TODO: Add your command handler code here
	mark_on = BURST_MARK;
}

void CEEG_marking_toolDlg::OnMarkingArtifact() 
{
	// TODO: Add your command handler code here
	mark_on = ARTIFACT_MARK;
}

void CEEG_marking_toolDlg::OnMarkingSleepspindle() 
{
	// TODO: Add your command handler code here
	mark_on = SLEEP_SPINDLE_MARK;
}

void CEEG_marking_toolDlg::OnMarkingBurstSuppression() 
{
	// TODO: Add your command handler code here
	mark_on = BURST_SUPPRESSION_MARK;
}

void CEEG_marking_toolDlg::OnMarkingNormalEEG() 
{
	// TODO: Add your command handler code here
	mark_on = NORMAL_MARK;
	
}

void CEEG_marking_toolDlg::OnMarkingUnknownmark() 
{
	// TODO: Add your command handler code here
	mark_on = UNKNOWN_MARK;
}

void CEEG_marking_toolDlg::OnMarkingUnmark() 
{
	// TODO: Add your command handler code here
	mark_on = UNMARK_EXISTING_MARK;
}

void CEEG_marking_toolDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default

	if (nChar == VK_F2)	// F2 key is pressed - next
	{
		CEEG_marking_toolDlg::OnViewNext(0);
	}
	else if (nChar == VK_F3)	// F3 key is pressed - prev
	{
		CEEG_marking_toolDlg::OnViewPrev(0);
	}
	else if (nChar == VK_RIGHT)	// right arrow key is pressed - next
	{
		CEEG_marking_toolDlg::OnViewNext(1);
	}
	else if (nChar == VK_LEFT)	// left arrow key is pressed - prev
	{
		CEEG_marking_toolDlg::OnViewPrev(1);
	}
	else
	{
		CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

BOOL CEEG_marking_toolDlg::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)
			pMsg->wParam=NULL ;
	}
	if(pMsg->message==WM_KEYUP)
	{
		if(pMsg->wParam==VK_F2)
		{
//			AfxMessageBox("F2 Pressed......");
			CEEG_marking_toolDlg::OnViewNext(0);
		}
		else if(pMsg->wParam==VK_F3)
		{
//			AfxMessageBox("F3 Pressed......");
			CEEG_marking_toolDlg::OnViewPrev(0);
		}
		else if(pMsg->wParam==VK_RIGHT)
		{
			CEEG_marking_toolDlg::OnViewNext(1);
		}
		else if(pMsg->wParam==VK_LEFT)
		{
			CEEG_marking_toolDlg::OnViewPrev(1);
		}
		else if(pMsg->wParam==VK_F5)
		{
			CEEG_marking_toolDlg::OnNextEvent();
		}
		else if(pMsg->wParam==VK_F6)
		{
			CEEG_marking_toolDlg::OnPrevEvent();
		}
		else if(pMsg->wParam==VK_F7)
		{
			CEEG_marking_toolDlg::OnViewNextAeegPage();
		}
		else if(pMsg->wParam==VK_F8)
		{
			CEEG_marking_toolDlg::OnViewPrevAeegPage();
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CEEG_marking_toolDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	int i, count;
	struct channnel_data_list *temp;
	struct channnel_data_list *aeeg_temp;
	struct data_mark_format* temp_data_mark;	// pointer to new allocated node

	repaint_on = false;	//set repaint false

	if (list_nodecount > 0)
	{
		// free raw EEG data
		for (count = 0; count < list_nodecount; count++)
		{
			temp = head;
			head = head->next;
			temp->prev = NULL;
			temp->next = NULL;
			if (head != NULL)
			{
				head->prev = NULL;
			}
			// for each channel, free its memory
			for (i = 0; i < no_of_channels; i++)
			{
				if (temp->channel_data[i] != NULL)
					free(temp->channel_data[i]);
			}
			if (temp->channel_data != NULL)
				free(temp->channel_data);		// free channel data global pointer
		}

		//free amplitude EEG data
		for (count = 0; count < list_nodecount; count++)
		{
			aeeg_temp = aeeg_head;
			aeeg_head = aeeg_head->next;
			aeeg_temp->prev = NULL;
			aeeg_temp->next = NULL;
			if (aeeg_head != NULL)
			{
				aeeg_head->prev = NULL;
			}
			// for each channel, free its memory
			for (i = 0; i < no_of_channels; i++)
			{
					if (aeeg_temp->channel_data[i] != NULL)
				free(aeeg_temp->channel_data[i]);
			}
				if (aeeg_temp->channel_data != NULL)
			free(aeeg_temp->channel_data);		// free channel data global pointer
		}
		list_nodecount = 0;		//reset
	}

	//initialize the marking database 
	if (marking_database != NULL)
	{
		for (i = 0; i < no_of_channels; i++)
		{
			while (marking_database[i] != NULL)
			{
				temp_data_mark = marking_database[i];
				marking_database[i] = temp_data_mark->next;	
				temp_data_mark->next = NULL;
				free(temp_data_mark);
			}
		}
		free(marking_database);		// free marking database global pointer
	}
	
	// initialize the database count
	free(mark_database_count);

	CDialog::OnClose();
}

void CEEG_marking_toolDlg::OnFileExit() 
{
	// TODO: Add your command handler code here
	CEEG_marking_toolDlg::OnClose();
}

/*
	show video menu 
	when on, we have to show the video of current marked portion
*/	
void CEEG_marking_toolDlg::OnFileShowvideo() 
{
	// TODO: Add your command handler code here
	video_display = !video_display;	
	if ((video_display) && (no_of_video_files > 0))
		plot_video_data();		//plot the video data 
}


void CEEG_marking_toolDlg::OnSetFocus(CWnd* pOldWnd) 
{
	//CDialog::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here

	//CEEG_marking_toolDlg::OnPaint();
}

void CEEG_marking_toolDlg::OnKillFocus(CWnd* pNewWnd) 
{
	// CDialog::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here

	// CEEG_marking_toolDlg::OnPaint();
}

void CEEG_marking_toolDlg::OnCaptureChanged(CWnd *pWnd) 
{
	// TODO: Add your message handler code here

	// CEEG_marking_toolDlg::OnPaint();

	// CDialog::OnCaptureChanged(pWnd);
}

void CEEG_marking_toolDlg::OnToolsPatientreport() 
{
	// TODO: Add your command handler code here

	// declare the other dialog class instance
	typepatientreport tpr;

	// call this function to view the dialog
	tpr.DoModal();
}


void CEEG_marking_toolDlg::OnNextEvent() 
{
	// TODO: Add your command handler code here
	int i, ch_no;
	double least_higher_mark_end_time;
	struct data_mark_format* temp_data_mark;	
	// select the event whose end time is least higher than current EEG page end time

	// at first initialize that end time variable to total recording duration
	least_higher_mark_end_time = total_recording_duration_sec;

	// search the marking database
	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		// initialize the marking pointer
		temp_data_mark = marking_database[ch_no];

		for (i = 0; i < mark_database_count[ch_no]; i++)
		{
			if ((temp_data_mark->mark_end_time != -1) && (temp_data_mark->mark_end_time > sel_raw_eeg_end_time) && (least_higher_mark_end_time > temp_data_mark->mark_end_time))
			{
				// update the least maximum end time containing marking information 
				// (according to current displayed page)
				least_higher_mark_end_time = temp_data_mark->mark_end_time;
			}

			// advance the node pointer
			temp_data_mark = temp_data_mark->next;
		}
	}

	// update the page display boundary
	sel_raw_eeg_end_time = ceil(least_higher_mark_end_time);
	if (sel_raw_eeg_end_time > total_recording_duration_sec)	//crosses time
	{
		sel_raw_eeg_end_time = total_recording_duration_sec;
	}
	sel_raw_eeg_start_time = sel_raw_eeg_end_time - raw_eeg_per_page_duration;

	plot_raw_EEG_signal(0);	//call plot raw EEG		
}

void CEEG_marking_toolDlg::OnPrevEvent() 
{
	// TODO: Add your command handler code here
	int i, ch_no;
	double greatest_lower_mark_start_time;
	struct data_mark_format* temp_data_mark;

	// select the event whose end time is least higher than current EEG page end time

	// at first initialize that end time variable to zero
	greatest_lower_mark_start_time = 0;

	// search the marking database
	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		// initialize the marking pointer
		temp_data_mark = marking_database[ch_no];

		for (i = 0; i < mark_database_count[ch_no]; i++)
		{
			if ((temp_data_mark->mark_start_time != -1) && (temp_data_mark->mark_start_time < sel_raw_eeg_start_time) && (greatest_lower_mark_start_time < temp_data_mark->mark_start_time))
			{
				// update the greatest minimum start time containing marking information (according to current displayed page)
				greatest_lower_mark_start_time = temp_data_mark->mark_start_time;
			}

			// advance the node pointer
			temp_data_mark = temp_data_mark->next;
		}
	}

	// update the page display boundary
	sel_raw_eeg_start_time = floor(greatest_lower_mark_start_time);
	if (sel_raw_eeg_start_time < 0)	//crosses time
	{
		sel_raw_eeg_start_time = 0;
	}
	sel_raw_eeg_end_time = sel_raw_eeg_start_time + raw_eeg_per_page_duration;
	
	plot_raw_EEG_signal(0);	//call plot raw EEG			
}

BOOL CEEG_marking_toolDlg::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	//return false;	// sourya

	return CDialog::OnEraseBkgnd(pDC);
}

void CEEG_marking_toolDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	
	// TODO: Add your message handler code here
	
}

void CEEG_marking_toolDlg::OnaEEGFULLSCALE() 
{
	// TODO: Add your command handler code here
	
	// adjust aEEG parameters for displaying aEEG with full scale duration
	sel_aeeg_start_time = 0;
	aeeg_per_page_duration = total_recording_duration_sec;		
	sel_aeeg_end_time = aeeg_per_page_duration;		

    // raw EEG displays according to screen size and time base
    raw_eeg_per_page_duration = (int)((screen_x_size - SCREEN_X_OFFSET) / (PixelsPerMM * timebase));   // (in Sec) time
    if ( raw_eeg_per_page_duration > total_recording_duration_sec )
	{
        raw_eeg_per_page_duration = (int)total_recording_duration_sec;
	}   
    sel_raw_eeg_start_time = 0;
    sel_raw_eeg_end_time = raw_eeg_per_page_duration;

	aeeg_computation_done = 0;	// reset the aEEG computation flag
	plot_amp_EEG_signal();
	plot_raw_EEG_signal(0);	//call plot raw EEG	
}

void CEEG_marking_toolDlg::OnaEEG6CMHR() 
{
	// TODO: Add your command handler code here

	// adjust aEEG parameters for displaying aEEG with display scale 6 cm / hr
	aeeg_per_page_duration = (int) ((screen_x_size_Amp - SCREEN_X_AMP_OFFSET) * 1.0 / PixelsPerMM * 60);	// 1 min / mm
	sel_aeeg_start_time = 0;
	sel_aeeg_end_time = sel_aeeg_start_time + aeeg_per_page_duration;
	if (sel_aeeg_end_time > total_recording_duration_sec)
	{
		sel_aeeg_end_time = total_recording_duration_sec;
		sel_aeeg_start_time = sel_aeeg_end_time - aeeg_per_page_duration;
		if (sel_aeeg_start_time < 0)
			sel_aeeg_start_time = 0;
	}

    // raw EEG displays according to screen size and time base
    raw_eeg_per_page_duration = (int)((screen_x_size - SCREEN_X_OFFSET) / (PixelsPerMM * timebase));   // (in Sec) time
    if ( raw_eeg_per_page_duration > total_recording_duration_sec )
	{
        raw_eeg_per_page_duration = (int)total_recording_duration_sec;
	}   
    sel_raw_eeg_start_time = 0;
    sel_raw_eeg_end_time = raw_eeg_per_page_duration;

	aeeg_computation_done = 0;	// reset the aEEG computation flag
	plot_amp_EEG_signal();	
	plot_raw_EEG_signal(0);	//call plot raw EEG	
}

void CEEG_marking_toolDlg::OnaEEG30CMHR() 
{
	// TODO: Add your command handler code here

	// adjust aEEG parameters for displaying aEEG with display scale 30 cm / hr
	aeeg_per_page_duration = (int) ((screen_x_size_Amp - SCREEN_X_AMP_OFFSET) * 1.0 / PixelsPerMM * 12);	// 0.2 min / 1 mm
	sel_aeeg_start_time = 0;
	sel_aeeg_end_time = sel_aeeg_start_time + aeeg_per_page_duration;
	if (sel_aeeg_end_time > total_recording_duration_sec)
	{
		sel_aeeg_end_time = total_recording_duration_sec;
		sel_aeeg_start_time = sel_aeeg_end_time - aeeg_per_page_duration;
		if (sel_aeeg_start_time < 0)
			sel_aeeg_start_time = 0;
	}

    // raw EEG displays according to screen size and time base
    raw_eeg_per_page_duration = (int)((screen_x_size - SCREEN_X_OFFSET) / (PixelsPerMM * timebase));   // (in Sec) time
    if ( raw_eeg_per_page_duration > total_recording_duration_sec )
	{
        raw_eeg_per_page_duration = (int)total_recording_duration_sec;
	}   
    sel_raw_eeg_start_time = 0;
    sel_raw_eeg_end_time = raw_eeg_per_page_duration;

	aeeg_computation_done = 0;	// reset the aEEG computation flag
	plot_amp_EEG_signal();	
	plot_raw_EEG_signal(0);	//call plot raw EEG	
}

void CEEG_marking_toolDlg::OnViewNextAeegPage() 
{
	// TODO: Add your command handler code here
	if (sel_aeeg_end_time < total_recording_duration_sec)	//end of file is not reached yet
	{
		//update the current aEEG display boundary
		sel_aeeg_start_time = sel_aeeg_end_time;
		sel_aeeg_end_time = sel_aeeg_start_time + aeeg_per_page_duration;
		if (sel_aeeg_end_time > total_recording_duration_sec)	//crosses time
		{
			sel_aeeg_end_time = total_recording_duration_sec;
			sel_aeeg_start_time = sel_aeeg_end_time - aeeg_per_page_duration;
			if (sel_aeeg_start_time < 0)
				sel_aeeg_start_time = 0;
		}

		// now update the raw EEG display boundary
		sel_raw_eeg_start_time = sel_aeeg_start_time;	
		sel_raw_eeg_end_time = sel_raw_eeg_start_time + raw_eeg_per_page_duration;

		aeeg_computation_done = 0;	// reset the aEEG computation flag
		plot_amp_EEG_signal();	
		plot_raw_EEG_signal(0);	//call plot raw EEG	
	}
}

void CEEG_marking_toolDlg::OnViewPrevAeegPage() 
{
	// TODO: Add your command handler code here
	if (sel_aeeg_start_time > 0)	//start of file is not reached yet
	{
		//update the current aEEG display boundary
		sel_aeeg_end_time = sel_aeeg_start_time;
		sel_aeeg_start_time = sel_aeeg_end_time - aeeg_per_page_duration;
		if (sel_aeeg_start_time < 0)
		{
			sel_aeeg_start_time = 0;
			sel_aeeg_end_time = sel_aeeg_start_time + aeeg_per_page_duration;
			if (sel_aeeg_end_time > total_recording_duration_sec)
				sel_aeeg_end_time = total_recording_duration_sec;
		}
		// now update the raw EEG display boundary
		sel_raw_eeg_start_time = sel_aeeg_start_time;	
		sel_raw_eeg_end_time = sel_raw_eeg_start_time + raw_eeg_per_page_duration;

		aeeg_computation_done = 0;	// reset the aEEG computation flag
		plot_amp_EEG_signal();	
		plot_raw_EEG_signal(0);	//call plot raw EEG	
	}	
}

