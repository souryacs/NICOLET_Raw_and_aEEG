// RectifyAmpEEGData.cpp : implementation file
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
	this function implements the moving average function for the amplitude EEG data
	subsequently it also does the semilogarithmic scale implementation
*/
void CEEG_marking_toolDlg::rectify_amp_eeg_data(void)
{
	int ch_no;
	struct channnel_data_list* aeeg_write;	// temporary pointer to the new node for amp eeg data
	int node_count;
	int end_node_offset;
	int i;
	
	// smoothing operation is performed taking no of samples equal to half of sampling frequency
	int smooth_len = (int)(0.5 * sampling_rate);		
	 
	// in fast smooth algorithm, we use 3 time smoothing - this is the smooth counter
	int smooth_counter;	
	double SumPoints;
	double *s;
	double *SmoothY;
	int halfw, L, k;
	int startpoint;
	double temp_sum;

	// apply for data of all the channels
	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		aeeg_write = aeeg_head;
		for (node_count = 0; node_count < list_nodecount; node_count++)	
		{
			if (node_count == (list_nodecount - 1))		//last node
				end_node_offset = ((int)(total_recording_duration_sec * sampling_rate - 1)) % CHAN_DATA_BLOCK_SIZE;
			else
				end_node_offset = CHAN_DATA_BLOCK_SIZE - 1;

			///////////////////////////////////////
			// length parameters
			L = CHAN_DATA_BLOCK_SIZE;	//(end_node_offset + 1);	//sourya
			halfw = (int)((smooth_len / 2));

			// allocate temporary structures
			// this variable stores the smooth output
			SmoothY = (double *) calloc(L, sizeof(double));	
			// this is another variable storing temp sum
			s = (double *) calloc(L, sizeof(double));	

			/////////////////////////////////////////
			// for smoothing of the filtered data
			// here we replicate the fastsmooth algorithm (originally developed in MATLAB)
			
			for (smooth_counter = 1; smooth_counter <= 3; smooth_counter++)
			{
				SumPoints = 0;
				for (i = 0; i < smooth_len; i++)
				{
					SumPoints += aeeg_write->channel_data[ch_no][i]; 
				}

				for (k = 0; k < (L - smooth_len); k++)
				{
					s[k + halfw] = SumPoints;
					SumPoints -= aeeg_write->channel_data[ch_no][k];
					SumPoints += aeeg_write->channel_data[ch_no][k + smooth_len];
				}

				s[L - smooth_len + halfw] = 0;
				for (k = (L - smooth_len); k < L; k++)
				{
					s[L - smooth_len + halfw] += aeeg_write->channel_data[ch_no][k];
				}
				
				for (k = 0; k < L; k++)
					SmoothY[k] = (s[k] / smooth_len);

				startpoint = (int)((smooth_len + 1)/2);
				SmoothY[0] = (aeeg_write->channel_data[ch_no][0] + aeeg_write->channel_data[ch_no][1]) / 2;
				for (k = 1; k < startpoint; k++)
				{
					temp_sum = 0;
					for (i = 0; i < (2*k-1); i++)
						temp_sum += aeeg_write->channel_data[ch_no][i];

					SmoothY[k] = temp_sum / (2*k - 1); 
					
					temp_sum = 0;
					for (i = (L - 2*k - 1); i <= (L - 1); i++)
						temp_sum += aeeg_write->channel_data[ch_no][i];

					SmoothY[L - k - 1] = temp_sum / (2*k - 1); 
				}
				SmoothY[L-1] = (aeeg_write->channel_data[ch_no][L-1] + aeeg_write->channel_data[ch_no][L-2]) / 2;

				//now copy the smoothed data back to the aEEG channel data
				for (i = 0; i < L; i++)
					aeeg_write->channel_data[ch_no][i] = SmoothY[i];
			}

			// free the temporary structure
			free(s);
			free(SmoothY);

			/////////////////////////////////////////
/*
			//semilogarithmic adaptation
			for (i = 0; i < CHAN_DATA_BLOCK_SIZE; i++)		// L 
			{
				// semilogarithmic scale
				if (aeeg_write->channel_data[ch_no][i] > 10)	// 0-10v linear, 10-100v non linear
				{
					aeeg_write->channel_data[ch_no][i] = 10 * log10(aeeg_write->channel_data[ch_no][i]);
					// clip the data over 20 (that is, original data greater than 100) 
					// for distinct visual display
					if (aeeg_write->channel_data[ch_no][i] > 20)
						aeeg_write->channel_data[ch_no][i] = 20;
				}
				// for VC++ drawing, sign is to be reversed
				//aeeg_write->channel_data[ch_no][i] = aeeg_write->channel_data[ch_no][i] * (-1);
			}
*/

			// advance the node pointer
			aeeg_write = aeeg_write->next;
		} // end of for loop for 1 channel
	}	// end of channel loop


}	//end function


/*
	this function implements the semilogarithmic adaptation of the 
	aEEG data
*/
double CEEG_marking_toolDlg::SemilogAdapt(double val)
{
	double out_val;
	
	// apply the scale reduction here
	val = val / 2.0;	// scale by 4 - check the MATLAB implementation

	if (val <= 10)	// 0-10v linear, 10-100v non linear
		out_val = val;
	else
	{
		out_val = 10 * log10(val);
		// clip the data over 20 (that is, original data greater than 100) 
		// for distinct visual display
		if (out_val > 20)
			out_val = 20;
	}
	return out_val;
}