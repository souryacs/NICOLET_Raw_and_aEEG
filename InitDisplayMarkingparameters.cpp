// InitDisplayMarkingparameters.cpp : implementation file
//

#include "stdafx.h"
#include "EEG_marking_tool.h"
#include "EEG_marking_toolDlg.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// init parameters based on data read
void CEEG_marking_toolDlg::Init_Display_Marking_parameters()
{
	int i;
	FILE *fid;
	CString outfile;
	CFileStatus status;
	double start_time, end_time;
	int ch_no, mark;

	time_interval = (1.0 / sampling_rate);
	total_recording_duration_sec = (channel_data_size / sampling_rate);  

  // raw EEG displays according to screen size and time base
  raw_eeg_per_page_duration = (int)((screen_x_size - SCREEN_X_OFFSET) / (PixelsPerMM * timebase));   // (in Sec) time
  if ( raw_eeg_per_page_duration > total_recording_duration_sec )
	{
    raw_eeg_per_page_duration = (int)total_recording_duration_sec;
	}   
  sel_raw_eeg_start_time = 0;
  sel_raw_eeg_end_time = raw_eeg_per_page_duration;

	// standardize the aEEG display settings
	// full scale display settings as the default
	sel_aeeg_start_time = 0;
	aeeg_per_page_duration = total_recording_duration_sec;		
	sel_aeeg_end_time = aeeg_per_page_duration;		

	/////////////////////////////
	//initialize the marking database count
	// it stores the marked event counts for the individual channel data
	mark_database_count = (int*) malloc(no_of_channels * sizeof(int));
	for (i = 0; i < no_of_channels; i++)
	{
		mark_database_count[i] = 0;
	}

	// initialize the marking database 
	// it points to the pointers of individual channel data
	// those pointers are initialized as NULL
	marking_database = (struct data_mark_format**) malloc(no_of_channels * sizeof(struct data_mark_format*));
	for (i = 0; i < no_of_channels; i++)
	{
		marking_database[i] = NULL;
	}

	/////////////////////////////
	// now open the output text file if it already exists
	outfile = directory_name + output_text_filename; 	

	// if we read text file as input then we search for the complete mark database
	// if we read binary data file with its status as continuous extracted EEG data then also
	// we search for the complete mark database
	if (CFile::GetStatus(outfile, status))	//if file exists
	{
		fid = fopen(outfile, "r");	//file open	in read mode
		while (!feof (fid))	
		{
			fscanf(fid, "%lf %lf %d %d", &start_time, &end_time, &mark, &ch_no);
			// here we read text file
			// so we read all marking information for all channels
			AddMarkData(ch_no, start_time, end_time, mark);
		}
		fclose(fid);
	}

	////////////////////////////////
	// initialize the display labels for both amp eeg and raw eeg display
	raw_eeg_channel_label = (CStatic**) malloc(no_of_channels * sizeof(CStatic*));
	amp_eeg_channel_label = (CStatic**) malloc(no_of_channels * sizeof(CStatic*));
	for (i = 0; i < no_of_channels; i++) 
	{
		raw_eeg_channel_label[i] = new CStatic;
		amp_eeg_channel_label[i] = new CStatic; 
	}	
}

// following function converts the time label (int) to the string for display
CString CEEG_marking_toolDlg::convert_time_to_str(int curr_time)
{
	CString time_label, temp;
	int hr = curr_time / 3600;
	int min = (curr_time - hr * 3600) / 60;
	int sec = (curr_time - hr * 3600 - min * 60);

	time_label = "";

	temp.Format(_T("%d"), hr);
	if (hr < 10)
		time_label = time_label + "0";
	time_label = time_label + temp;
	time_label = time_label + ":";
	temp.Format(_T("%d"), min);
	if (min < 10)
		time_label = time_label + "0";
	time_label = time_label + temp;
	time_label = time_label + ":";
	temp.Format(_T("%d"), sec);
	if (sec < 10)
		time_label = time_label + "0";
	time_label = time_label + temp;
	return time_label;
}