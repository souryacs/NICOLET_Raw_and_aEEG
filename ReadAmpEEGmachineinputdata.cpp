// ReadAmpEEGmachineinputdata.cpp : implementation file
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

/*
	data read from input amp  eeg text file
*/
void CEEG_marking_toolDlg::Read_Amp_EEG_machine_input_data(void)
{
	FILE* fid;
	char textLine[MAX_LINE_LEN];
	char* token;
	int count, i;
	double temp_arr[MAX_NO_OF_CHANNELS];
	int max_channel_index = -1;

	for (i = 0; i < MAX_NO_OF_CHANNELS; i++)
	{
		temp_arr[i] = 0;
	}
	
	fid = fopen(fileName, "r");     // file open

	// following code is executed only if input raw EEG data is a unipolar one
	// i.e. we have to convert it to bipolar montage 
	if (montage_info_inbuilt == false)
	{

		// now we derive the individual channel index that occurs during the
		// channel name
		// for instance, in montage having channel C4-P4, we store index of
		// channel C4 in the structure chan1_index (because it is the 1st
		// channel) - otherwise, it is stored in the chan2_index
		// if montage contains REF string then corresponding channel index is set to 1
		for (i = 0; i < no_of_channels; i++)
		{
			if (!strcmp(chan1_1st_channel[i], "REF"))
			{
				chan1_index[i] = -1;
			}
			if (!strcmp(chan1_2nd_channel[i], "REF"))
			{
				chan2_index[i] = -1;
			}
		}

		fgets(textLine, MAX_LINE_LEN, fid);  //read the 1st line  - header information                 
		count = 0;               // string delimeter - channel name list
		token = strtok(textLine, " \t");

		//now set the channel index information
		for (; ((count < MAX_NO_OF_CHANNELS) && strcmp(token, "")); count++)   // at most MAX_NO_OF_CHANNELS no of different channels are selected
		{        
			for (i = 0; i < no_of_channels; i++)
			{
				if (!strcmp(chan1_1st_channel[i], token))
				{
					chan1_index[i] = count;
				}
				if (!strcmp(chan1_2nd_channel[i], token))
				{
					chan2_index[i] = count;
				}
			}
			token = strtok(NULL, " \t");
		}


		// find max channel index that is to be read 
		// for example, only one channel montage is recorded, that is C4-P4
		// so in the text file, reading upto max (C4_index, P4_index) is sufficient
		// this value is stored in the max channel index
		for (i = 0; i < no_of_channels; i++)
		{
			if (chan1_index[i] > max_channel_index)
			{
				max_channel_index = chan1_index[i];
			}
			if (chan2_index[i] > max_channel_index)
			{
				max_channel_index = chan2_index[i];
			}
		}
	}	//end unipolar montage check
	////////////////////////////////////

	// init the channel data size
	channel_data_size = 0;

	struct channnel_data_list *temp;	// temporary pointer to the new node 
	struct channnel_data_list *aeeg_temp;	// temporary pointer to the new node for containing filtered data
	
	// loop to read the input text data
	while (!feof (fid))
	{
		fgets(textLine, MAX_LINE_LEN, fid);
		if (isalpha(textLine[0]))
			continue;

		if ((channel_data_size % CHAN_DATA_BLOCK_SIZE) == 0)
		{
			///////////////////////////////////////////////
			// allocate one node of list which will contain channel data 
			temp = (struct channnel_data_list *)malloc(sizeof(struct channnel_data_list));
			if (temp == NULL)
			{
				AfxMessageBox("Insufficient memory - no new node alloc");	
				return;
			}
			list_nodecount++;	// increment the node count

			temp->next = NULL;
			temp->prev = NULL;
			if (head == NULL && tail == NULL)	//  this is the first node
			{
				head = tail = temp;		// assign pointer
			}
			else
			{
				temp->prev = tail;
				tail->next = temp;
				tail = temp;			// assign pointer
			}
			
			// for each channel, allocate one chunk of memory
			tail->channel_data = (double**) calloc(no_of_channels, sizeof(double*));
			if (tail->channel_data == NULL)
			{
				AfxMessageBox("Insufficient memory - basic channel data alloc");	
				return;
			}
			for (i = 0; i < no_of_channels; i++)
			{
				tail->channel_data[i] = (double*) calloc(CHAN_DATA_BLOCK_SIZE, sizeof(double));
				if (tail->channel_data[i] == NULL)
				{
					AfxMessageBox("Insufficient memory - no new channel data alloc");	
					return;
				}
			}
			///////////////////////////////////////////////
			// for amp EEG data storage
			// allocate one node of list which will contain channel data 
			aeeg_temp = (struct channnel_data_list *)malloc(sizeof(struct channnel_data_list));
			if (aeeg_temp == NULL)
			{
				AfxMessageBox("Insufficient memory - no new node alloc");	
				return;
			}

			aeeg_temp->next = NULL;
			aeeg_temp->prev = NULL;
			if (aeeg_head == NULL && aeeg_tail == NULL)	//  this is the first node
			{
				aeeg_head = aeeg_tail = aeeg_temp;		// assign pointer
			}
			else
			{
				aeeg_temp->prev = aeeg_tail;
				aeeg_tail->next = aeeg_temp;
				aeeg_tail = aeeg_temp;			// assign pointer
			}
			
			// for each channel, allocate one chunk of memory
			aeeg_tail->channel_data = (double**) calloc(no_of_channels, sizeof(double*));
			if (aeeg_tail->channel_data == NULL)
			{
				AfxMessageBox("Insufficient memory - basic channel data alloc");	
				return;
			}
			for (i = 0; i < no_of_channels; i++)
			{
				aeeg_tail->channel_data[i] = (double*) calloc(CHAN_DATA_BLOCK_SIZE, sizeof(double));
				if (aeeg_tail->channel_data[i] == NULL)
				{
					AfxMessageBox("Insufficient memory - no new channel data alloc");	
					return;
				}
			}
			// end amp eeg data alloc
			///////////////////////////////////////////////
		}

		// following code is executed only if input raw EEG data is a unipolar one
		// i.e. we have to convert it to bipolar montage 
		if (montage_info_inbuilt == false)
		{
			token = strtok(textLine, " \t");	
			i = 0;

			//read one line data and store it in temporary array
			while (!( (token == NULL) || (i >= max_channel_index /* MAX_NO_OF_CHANNELS */) ))
			{
				temp_arr[i] = ((atof(token) + 4999.84)/0.152587771233253)-32767;
				token = strtok(NULL, " \t");
				i++;
			}
			// insert 0's for any non-available data
			for (; i < max_channel_index /* MAX_NO_OF_CHANNELS */; i++)
			{
				temp_arr[i] = 0;
			}

			// now insert the line data onto channel data structure based on the channel no index obtained 
			for (i = 0; i < no_of_channels; i++)
			{
				if ((chan1_index[i] == -1) && (chan2_index[i] == -1))		//REF-REF
				{
					tail->channel_data[i][channel_data_size % CHAN_DATA_BLOCK_SIZE] = 0;
				}
				else if ((chan1_index[i] != -1) && (chan2_index[i] != -1))	//NON_REF-NON_REF
				{
					tail->channel_data[i][channel_data_size % CHAN_DATA_BLOCK_SIZE] = (temp_arr[chan1_index[i]] - temp_arr[chan2_index[i]]);
				}
				else if ((chan1_index[i] == -1) && (chan2_index[i] != -1))	//REF-NON_REF
				{
					tail->channel_data[i][channel_data_size % CHAN_DATA_BLOCK_SIZE] = -temp_arr[chan2_index[i]];
				}
				else if ((chan1_index[i] != -1) && (chan2_index[i] == -1))	//NON_REF-REF
				{
					tail->channel_data[i][channel_data_size % CHAN_DATA_BLOCK_SIZE] = temp_arr[chan1_index[i]];
				}
			}	//end one line read and assign
		}
		else	// montage info is within the generated text file
		{
			token = strtok(textLine, " \t");	
			i = 0;
			//read one line data and store it in temporary array
			while (!( (token == NULL) || (i >= no_of_channels) ))
			{
				tail->channel_data[i][channel_data_size % CHAN_DATA_BLOCK_SIZE] = ((atof(token) + 4999.84)/0.152587771233253)-32767;
				token = strtok(NULL, " \t");
				i++;
			}	//end one line read and assign
		}	// end montage existence checking condition

		// increment the counter
		channel_data_size++;

	}	// end file read loop

	// file close
	fclose(fid); 

	// following code checks whether the information text file containing the sampling rate is there or not
	// if not then we write in a text file, the sampling rate and file duration information
	CFileStatus status;
	FILE *fod;
	CString temp_file = directory_name + "duration.txt";

	if (!(CFile::GetStatus(temp_file, status)))
	{
		fod = fopen(temp_file, "w");
		fprintf(fod, "\n total recording duration: %f ", (channel_data_size / sampling_rate));
		fprintf(fod, "\n data size: %ld ", channel_data_size);
		fprintf(fod, "\n sampling rate: %f ", sampling_rate);
		fclose(fod);
	}

    
} //end function