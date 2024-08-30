// ReadTextAmpEEGmachineConfig.cpp : implementation file
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

void CEEG_marking_toolDlg::Read_Text_Amp_EEG_machine_Config(void)
{
	char* token;
	char textLine[MAX_LINE_LEN];
	CString channel_info_filename;
	FILE* fid;
	CFileStatus status;

  // set the channel montage
  // read total no of channels
  channel_info_filename = directory_name + "montage.txt"; 
	if (CFile::GetStatus(channel_info_filename, status))
	{
		montage_info_inbuilt = false;
		fid = fopen(channel_info_filename, "r");	//file open
    no_of_channels = 0;
    while (!feof (fid))
		{
			fgets(textLine, MAX_LINE_LEN, fid);
			token = strtok(textLine, "\n");
      channel_name[no_of_channels] = textLine;
      token = strtok(textLine, "-\n");
      chan1_1st_channel[no_of_channels] = token;
      token = strtok(NULL, "-\n");     
      chan1_2nd_channel[no_of_channels] = token;
			no_of_channels++;
		} // end while
    fclose(fid);	//file close
	}
	else
	{
		montage_info_inbuilt = true;
		fid = fopen(fileName, "r");	//file open	
		no_of_channels = 0;
		fgets(textLine, MAX_LINE_LEN, fid);       
		token = strtok(textLine, " \t\n");
		while ((token != NULL) && (token != ""))
		{
			channel_name[no_of_channels] = token;
			no_of_channels++;
			token = strtok(NULL, " \t\n");   
		}
		fclose(fid);	//file close
	}

} //end function
