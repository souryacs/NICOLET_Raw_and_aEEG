// BurstFollowedBySuppressionDetection.cpp : implementation file
//.

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
	this function derives the suppression intervals within the recorded EEG
*/
void CEEG_marking_toolDlg::Suppression_Detection(int ch_no)
{
	// suppression voltage threshold
	double SUPPRESSION_VOLTAGE_THRESHOLD = 5;	

	// this boolean variable signifies whether suppression is ongoing or not
	bool suppression_status = false;

	// suppression signifying timing variables
	double suppression_start_time, suppression_end_time;

	double time_elapsed = 0;	// marks the time elapsed during data analysis 	
	int start_index, end_index;		// location boundary of input signal segment
	int inp_sig_len = (int)(ceil(sampling_rate * TIME_WINDOW_LENGTH));	// 1 sec signal length
	int i;	

	// allocate memory for the input signal data
	double *input_signal = (double *) calloc(inp_sig_len, sizeof(double));	

	// loop 
	while((time_elapsed + TIME_WINDOW_LENGTH) <= total_recording_duration_sec)
	{ 
		// calculate the segment boundary
		start_index = (int) floor(time_elapsed * sampling_rate);
		end_index = start_index + (int) floor(TIME_WINDOW_LENGTH * sampling_rate) - 1;
		if (end_index >= channel_data_size)
			end_index = (channel_data_size - 1);

		// read current signal segment
		Read_Signal_Segment(ch_no, start_index, end_index, input_signal, inp_sig_len, false);

		// if there is persistent voltage less than 5V then we mark as suppression
		// we check the value of each sample and then mark the suppression start (start of low voltage) and 
		// end (end of low voltage) accordingly
		for (i = 0; (i < inp_sig_len); i++)
		{
			// input signal sample is less than the suppression threshold
			if (fabs(input_signal[i]) <= SUPPRESSION_VOLTAGE_THRESHOLD)
			{
				// check if this is the start of low voltage
				// then mark it as a possible suppression start time
				// and also set the suppression signifying boolean variable to true
				if (suppression_status == false)
				{	
					suppression_status = true;
					suppression_start_time = time_elapsed + (((i * 1.0) / inp_sig_len) * TIME_WINDOW_LENGTH);
				}

				// else there is ongoing suppression and we dont need to change anything
			}
			else	// input signal sample of comparatively high voltage
			{
				// check if this is the end of a previous suppression
				// then mark it as a possible suppression end time
				// and also set the suppression signifying boolean variable to false
				if (suppression_status == true)
				{	
					suppression_status = false;
					suppression_end_time = time_elapsed + (((i * 1.0) / inp_sig_len) * TIME_WINDOW_LENGTH);

					// suppressions should be at least of length MIN_SUPPRESSION_LEN at a stretch
					// in that case, add the suppression mark in the marking database
					if ((suppression_end_time - suppression_start_time) >= MIN_SUPPRESSION_LEN)
					{
						AddMarkData(ch_no, suppression_start_time, suppression_end_time, SUPPRESSION_MARK);
					}
				}	// end check on going suppression

				// if there is no such ongoing suppression then we dont need to do anything
			}
		}	// end of signal segment traverse loop

        // update time counter 
        time_elapsed = time_elapsed + TIME_WINDOW_LENGTH;  		

	}	// end time loop

} // end of function

////////////////////////////////////////////////////////

/*
	detects burst followed by suppression pattern 
*/
void CEEG_marking_toolDlg::burst_followed_by_suppression_detection()
{
	int ch_no;
	int mark_count, i;
	struct data_mark_format* temp_burst_mark;	// pointer to burst mark
	struct data_mark_format* temp_supp_mark;	// pointer to suppression mark

	// this variable is set true when a burst mark follows a suppression mark within mentioned relative distance
	bool prev_supp_mark_within_range;

	// this variable is set true when a burst mark preceds a suppression mark within mentioned relative distance
	bool next_supp_mark_within_range;

	// max distance between a burst and a suppression pattern (in either way) in seconds
	double DIST_SUPP_PRECEDS_BURST = 5;
	double DIST_SUPP_FOLLOWS_BURST = 5;

	double prev_supp_start_time;
	double next_supp_end_time;

	// traverse through all the channels
	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		// initialize the burst mark pointer with the header node of the current channel
		temp_burst_mark = marking_database[ch_no];

		// traverse through burst markings
		for (mark_count = 0; ((mark_count < mark_database_count[ch_no]) && 
			(temp_burst_mark->mark_end_time <= total_recording_duration_sec)); mark_count++)
		{
			// if current mark is not a burst mark or if
			// invalid start or end time of the mark - continue
			if ((temp_burst_mark->mark_start_time == -1) || (temp_burst_mark->mark_end_time == -1)
				|| (temp_burst_mark->mark_event != BURST_MARK))	
			{
				temp_burst_mark = temp_burst_mark->next;	// advance the node pointer
				continue;	
			}

			// reset the boolean variables
			prev_supp_mark_within_range = false;
			next_supp_mark_within_range = false;

			// if current mark pointed by the 1st pointer is a burst mark
			// then initialize the suppression mark pointer with the header node of the current channel
			temp_supp_mark = marking_database[ch_no];

			for (i = 0; ((i < mark_database_count[ch_no]) && 
				(temp_supp_mark->mark_end_time <= total_recording_duration_sec)); i++)
			{
				// if the mark pointed by second pointer is identical to the mark pointed by first pointer
				// or if the mark has invalid start or end time
				// or if the mark is not a suppression, then skip any comparison
				if ((i == mark_count) || 
					(temp_supp_mark->mark_event != SUPPRESSION_MARK) ||
					(temp_supp_mark->mark_start_time == -1) || 
					(temp_supp_mark->mark_end_time == -1))	
				{
					temp_supp_mark = temp_supp_mark->next;	// advance the node pointer
					continue;
				}

				/////////////////////////////////////////
				// we check whether the end time of the current suppression mark exceeds the start time of the current burst mark
				// without setting the boolean variable prev_supp_mark_within_range
				// in that case there is no point in checking further for the current burst mark
				if ((temp_supp_mark->mark_start_time > temp_burst_mark->mark_start_time) 
					&& (prev_supp_mark_within_range == false))
				{
					break;
				}

				// otherwise if the start time of the current suppression mark exceeds the burst end time
				// by the specified distance then also terminate
				if (temp_supp_mark->mark_start_time > (temp_burst_mark->mark_end_time + DIST_SUPP_FOLLOWS_BURST)) 
					break;


				/////////////////////////////////////////
				// consider certain cases
				// case 1 - if one suppression end time just crosses another burst start time
				// but its start time is way before the burst start time
				// in that case, suppression end time = burst start time
				if ((temp_supp_mark->mark_end_time > temp_burst_mark->mark_start_time) &&
					(temp_supp_mark->mark_end_time < temp_burst_mark->mark_end_time) &&
					(temp_supp_mark->mark_start_time < temp_burst_mark->mark_start_time))
				{
					temp_supp_mark->mark_end_time = temp_burst_mark->mark_start_time;
				}

				// case 2 - if one suppression start time just preceds another burst end time 
				// but its end time is way after the burst end time
				// in that case, suppression start time = burst end time
				if ((temp_supp_mark->mark_start_time < temp_burst_mark->mark_end_time) &&
					(temp_supp_mark->mark_start_time > temp_burst_mark->mark_start_time) &&
					(temp_supp_mark->mark_end_time > temp_burst_mark->mark_end_time))
				{
					temp_supp_mark->mark_start_time = temp_burst_mark->mark_end_time;
				}

				/////////////////////////////////////////	

				// if current mark is a suppression mark and it preceds the ongoing burst mark
				// by marginal distance then mark them together 
				if (fabs(temp_burst_mark->mark_start_time - temp_supp_mark->mark_end_time) <= DIST_SUPP_PRECEDS_BURST)
				{
					prev_supp_mark_within_range = true;
					
					// store the start time of suppression mark
					prev_supp_start_time = temp_supp_mark->mark_start_time;		
					
					// delete the suppression mark
					temp_supp_mark->mark_start_time = -1;
					temp_supp_mark->mark_end_time = -1;
					temp_supp_mark->mark_event = -1;
				}
				if (fabs(temp_supp_mark->mark_start_time - temp_burst_mark->mark_end_time) <= DIST_SUPP_FOLLOWS_BURST)
				{
					next_supp_mark_within_range = true;
					
					// store the end time of suppression mark
					next_supp_end_time = temp_supp_mark->mark_end_time;		
					
					// delete the suppression mark
					temp_supp_mark->mark_start_time = -1;
					temp_supp_mark->mark_end_time = -1;
					temp_supp_mark->mark_event = -1;
				}

				// advance the suppression pointer
				temp_supp_mark = temp_supp_mark->next;

			}	// end for loop - traverse with the suppression pointer

			if ((prev_supp_mark_within_range == true) && (next_supp_mark_within_range == true))
			{
				temp_burst_mark->mark_start_time = prev_supp_start_time;
				temp_burst_mark->mark_end_time = next_supp_end_time;
				temp_burst_mark->mark_event = BURST_SUPPRESSION_MARK;
			}

			// advance the burst pointer
			temp_burst_mark = temp_burst_mark->next;

		}	// end traverse for 1 channel - with the burst pointer
	}	// end channel loop
}	// end of function




# if 0
void CEEG_marking_toolDlg::burst_followed_by_suppression_detection()
{
	int ch_no;
	int mark_count, i;
	struct data_mark_format* temp_data_mark;	// pointer to burst mark
	struct data_mark_format* temp_data_mark2;	// pointer to suppression mark
	bool overlapping_mark_found;

	// max distance between a burst and a suppression pattern (in either way) in seconds
	double DIST_SUPP_PRECEDS_BURST = 2;
	double DIST_SUPP_FOLLOWS_BURST = 3;

	// traverse through all the channels
	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		// initialize the pointer with the header node of the current channel
		temp_data_mark = marking_database[ch_no];

		// traverse through markings
		for (mark_count = 0; ((mark_count < mark_database_count[ch_no]) && (temp_data_mark->mark_end_time <= total_recording_duration_sec)); mark_count++)
		{
			overlapping_mark_found = false;

			// if current mark is not a burst mark or if
			// invalid start or end time of the mark - continue
			if ((temp_data_mark->mark_start_time == -1) || (temp_data_mark->mark_end_time == -1)
				|| (temp_data_mark->mark_event != BURST_MARK))	
			{
				temp_data_mark = temp_data_mark->next;	// advance the node pointer
				continue;	
			}

			// if current mark pointed by the 1st pointer is a burst mark
			// then initialize the 2nd pointer with the header node of the current channel
			temp_data_mark2 = marking_database[ch_no];

			for (i = 0; ((i < mark_database_count[ch_no]) && (temp_data_mark2->mark_end_time <= total_recording_duration_sec)); i++)
			{
				// if the mark pointed by second pointer is identical to the mark pointed by first pointer
				// or if the mark has invalid start or end time
				// or if the mark is not a suppression, then skip any comparison
				if ((i == mark_count) || 
					(temp_data_mark2->mark_event != SUPPRESSION_MARK) ||
					(temp_data_mark2->mark_start_time == -1) || 
					(temp_data_mark2->mark_end_time == -1))	
				{
					temp_data_mark2 = temp_data_mark2->next;	// advance the node pointer
					continue;
				}

				/////////////////////////////////////////
				// consider certain cases
				// case 1 - if one suppression end time just crosses another burst start time
				// but its start time is way before the burst start time
				// in that case, suppression end time = burst start time
				if ((temp_data_mark2->mark_end_time > temp_data_mark->mark_start_time) &&
					(temp_data_mark2->mark_end_time < temp_data_mark->mark_end_time) &&
					(temp_data_mark2->mark_start_time < temp_data_mark->mark_start_time))
				{
					temp_data_mark2->mark_end_time = temp_data_mark->mark_start_time;
				}

				// case 2 - if one suppression start time just preceds another burst end time 
				// but its end time is way after the burst end time
				// in that case, suppression start time = burst end time
				if ((temp_data_mark2->mark_start_time < temp_data_mark->mark_end_time) &&
					(temp_data_mark2->mark_start_time > temp_data_mark->mark_start_time) &&
					(temp_data_mark2->mark_end_time > temp_data_mark->mark_end_time))
				{
					temp_data_mark2->mark_start_time = temp_data_mark->mark_end_time;
				}

				/////////////////////////////////////////	

				// if current mark is a suppression mark and it preceds the ongoing burst mark
				// by marginal distance then mark them together 
				if (fabs(temp_data_mark->mark_start_time - temp_data_mark2->mark_end_time) <= DIST_SUPP_PRECEDS_BURST)
				{
					overlapping_mark_found = true;
					
					// update current burst mark start time with the start time of suppression mark
					temp_data_mark->mark_start_time = temp_data_mark2->mark_start_time;		
					
					// update the marking type
					temp_data_mark->mark_event = BURST_SUPPRESSION_MARK;

					// delete the suppression mark
					temp_data_mark2->mark_start_time = -1;
					temp_data_mark2->mark_end_time = -1;
					temp_data_mark2->mark_event = -1;
				}
				else if (fabs(temp_data_mark2->mark_start_time - temp_data_mark->mark_end_time) <= DIST_SUPP_FOLLOWS_BURST)
				{
					overlapping_mark_found = true;
					
					// update current burst mark end time with the end time of suppression mark
					temp_data_mark->mark_end_time = temp_data_mark2->mark_end_time;		
					
					// update the marking type
					temp_data_mark->mark_event = BURST_SUPPRESSION_MARK;

					// delete the suppression mark
					temp_data_mark2->mark_start_time = -1;
					temp_data_mark2->mark_end_time = -1;
					temp_data_mark2->mark_event = -1;
				}

				// advance the second node pointer
				temp_data_mark2 = temp_data_mark2->next;
			}	// end for loop - traverse with the second marking pointer

			// if such merging operation took place then we will again start from that mark
			// for future comparisons
			if (overlapping_mark_found)
				mark_count--;

			// advance the node pointer if previously there was no overlapping mark
			if (overlapping_mark_found == false)
				temp_data_mark = temp_data_mark->next;

		}	// end marking traverse for 1 channel
	}	// end channel loop

}	// end function
# endif // if 0