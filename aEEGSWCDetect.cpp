// aEEGSWCDetect.cpp : implementation file
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


void CEEG_marking_toolDlg::aEEG_SWC_determine(int ch_no)
{
	int SLEEP_MERGE_MAX_TIME_GAP = 120;	// 2 sec is for the nearby same sleep state marking merge 
	int feature_queue_aEEG_window_len = 300;	// 5 min feature storage	
	int aEEG_time_window_len = 10;	// 10 sec non-sliding window for aEEG bandwidth determination
	int queue_size = 30;

	double time_elapsed = 0;	// marks the time elapsed during data analysis 
	int start_index, end_index;		// location boundary of input signal segment
	int inp_sig_len = (int)(ceil(sampling_rate * aEEG_time_window_len));	// 1 sec signal length

	// allocate memory for the input signal data
	double *input_signal = (double *) calloc(inp_sig_len, sizeof(double));	

	double percentile_5, percentile_95, log_perc_5, log_perc95;
	int percentile_index;

	int start_hr, start_min, start_sec, end_hr, end_min, end_sec;

	// feature queues
	double *bandwidth_queue = (double *) calloc(queue_size, sizeof(double));	
	double *upper_margin_queue = (double *) calloc(queue_size, sizeof(double));	
	double *lower_margin_queue = (double *) calloc(queue_size, sizeof(double));	

	int queue_counter = 0;
	int i;

	// counters for indicating the status of the lower margin
	// lower margin is plotted in semilogarithmic scale
	int lower_margin_below3, lower_margin_3to5, lower_margin_5to7, lower_margin_7to10, lower_margin_above10;

	// counters for indicating the status of the upper margin
	// upper margin is plotted in semilogarithmic scale
	int upper_margin_below8, upper_margin_8to12, upper_margin_12to15, upper_margin_15to18, upper_margin_above18;

	// counters for indicating the bandwidth status
	// measured in semilogarithmic scale
	int bandwidth_below5, bandwidth_5to10, bandwidth_10to15, bandwidth_above15;

	// status of sleep states - QS and AS
	int QS_sleep_status = 0;
	int AS_sleep_status = 0;
	double QS_start_time, QS_end_time, AS_start_time, AS_end_time; 

	// pointer to node of the marking database
	struct data_mark_format* tail;	
	struct data_mark_format* last_sleep_info = NULL;	
	int sleep_count = 0;

	// debug - sourya 
	FILE *fod_aeeg;
	CString out_text_filename = directory_name + "sleep_state.txt";
	if (ch_no == 0)	
		fod_aeeg = fopen(out_text_filename, "w");
	else
		fod_aeeg = fopen(out_text_filename, "a");

	fprintf(fod_aeeg, "\n\n\n channel no : %d  channel name : %s ", ch_no, channel_name[ch_no]);


	// loop to read the data 
	// here first we read initial some segment to form the feature queue
	while((time_elapsed + TIME_WINDOW_LENGTH) < feature_queue_aEEG_window_len)
	{ 
		// calculate the segment boundary
		start_index = (int) floor(time_elapsed * sampling_rate);
		end_index = start_index + (int) floor(aEEG_time_window_len * sampling_rate) - 1;
		if (end_index >= channel_data_size)
			end_index = (channel_data_size - 1);

		// read current signal segment from the aEEG data
		Read_Signal_Segment(ch_no, start_index, end_index, input_signal, inp_sig_len, true);

		// sort the input signal
		insert_sort(input_signal, inp_sig_len);

		// calculate the percentile values
		percentile_index = (int)(inp_sig_len * 5.0 / 100.0);
		percentile_5 = input_signal[percentile_index];
		log_perc_5 = SemilogAdapt(percentile_5);

		percentile_index = (int)(inp_sig_len * 95.0 / 100.0);
		percentile_95 = input_signal[percentile_index];
		log_perc95 = SemilogAdapt(percentile_95);

		// insert the feature values in a queue
		// bandwidth and lower upper margins are calculated in semilogarithmic scale
		bandwidth_queue[queue_counter] = (log_perc95 - log_perc_5);
		upper_margin_queue[queue_counter] = log_perc95;
		lower_margin_queue[queue_counter] = log_perc_5;
		queue_counter = (queue_counter + 1) % queue_size;

        // update time counter by aEEG_time_window_len 
        time_elapsed += aEEG_time_window_len;  		
	}	// end of first loop

	queue_counter = 0;	// reset the queue counter

	// loop to read the data 
	while((time_elapsed + TIME_WINDOW_LENGTH) < total_recording_duration_sec)
	{
		// check the queues and update respective counters
		lower_margin_below3 = 0;
		lower_margin_3to5 = 0;
		lower_margin_5to7 = 0;
		lower_margin_7to10 = 0;
		lower_margin_above10 = 0;

		// counters for indicating the status of the upper margin
		// upper margin is plotted in semilogarithmic scale
		upper_margin_below8 = 0;
		upper_margin_8to12 = 0;	
		upper_margin_12to15 = 0;
		upper_margin_15to18 = 0;
		upper_margin_above18 = 0;

		// counters for indicating the bandwidth status
		// measured in semilogarithmic scale
		bandwidth_below5 = 0;
		bandwidth_5to10 = 0;
		bandwidth_10to15 = 0;
		bandwidth_above15 = 0;

		for (i = 0; i < queue_size; i++)
		{
			// check the lower margin queue and adjust the counters
			if (lower_margin_queue[i] < 3)
				lower_margin_below3++;
			else if ((lower_margin_queue[i] >= 3) && (lower_margin_queue[i] < 5))
				lower_margin_3to5++;
			else if ((lower_margin_queue[i] >= 5) && (lower_margin_queue[i] < 7))
				lower_margin_5to7++;	
			else if ((lower_margin_queue[i] >= 7) && (lower_margin_queue[i] < 10))
				lower_margin_7to10++;
			else	// if (lower_margin_queue[i] >= 10) 
				lower_margin_above10++;

			// check the upper margin queue and adjust the counters
			if (upper_margin_queue[i] < 8)
				upper_margin_below8++;
			else if ((upper_margin_queue[i] >= 8) && (upper_margin_queue[i] < 12))
				upper_margin_8to12++;
			else if ((upper_margin_queue[i] >= 12) && (upper_margin_queue[i] < 15))
				upper_margin_12to15++;
			else if ((upper_margin_queue[i] >= 15) && (upper_margin_queue[i] < 18))
				upper_margin_15to18++;
			else //if (upper_margin_queue[i] >= 18)
				upper_margin_above18++;

			// check the bandwidth queue and adjust the counters
			if (bandwidth_queue[i] < 5)	
				bandwidth_below5++;
			else if ((bandwidth_queue[i] >= 5) && (bandwidth_queue[i] < 10))
				bandwidth_5to10++;
			else if ((bandwidth_queue[i] >= 10) && (bandwidth_queue[i] < 15))
				bandwidth_10to15++;
			else //if (bandwidth_queue[i] >= 15)
				bandwidth_above15++;
		}	

		///////////////////////////////////
		// quiet sleep status check
		if (((double)(((lower_margin_below3 + lower_margin_3to5) * 1.0) / queue_size) /* <= */ >= 0.75) &&
			((double)(((bandwidth_below5 + bandwidth_5to10) * 1.0) / queue_size) <= 0.65))
		{
			if (QS_sleep_status == 0)	// start of quiet sleep
			{
				QS_sleep_status = 1;
				QS_start_time = time_elapsed - feature_queue_aEEG_window_len;
			}
		}
		else if (QS_sleep_status == 1)	// quiet sleep end
		{
			QS_sleep_status = 0;
			QS_end_time = time_elapsed - feature_queue_aEEG_window_len;

			// find the last node containing the QS information
			sleep_count = 0;
			tail = marking_database[ch_no];
			while (tail != NULL)
			{
				if (tail->mark_event == QUIET_SLEEP_MARK)
				{
					sleep_count++;
					last_sleep_info = tail;		// store the last QS pointer
				}
				tail = tail->next;
			}

			// if current QS interval overlaps with previous recorded QS
			// interval then update previous recorded QS information
			if ((sleep_count > 0) && 
				(QS_start_time > last_sleep_info->mark_start_time) && 
				(QS_start_time <= last_sleep_info->mark_end_time))
			{
				if (QS_end_time > last_sleep_info->mark_end_time)
				{
					last_sleep_info->mark_end_time = QS_end_time;
				}
			}
			// else if current QS just follows the last QS interval by a minimal distance then 
			// merge the entries
			else if ((sleep_count > 0) && 
				(QS_start_time > last_sleep_info->mark_end_time) && 
				((QS_start_time - last_sleep_info->mark_end_time) < SLEEP_MERGE_MAX_TIME_GAP))
			{
				last_sleep_info->mark_end_time = QS_end_time;
			}
			else	// insert the QS time in the marking database as a new entry
			{
				AddMarkData(ch_no, QS_start_time, QS_end_time, QUIET_SLEEP_MARK);
			}
		}
		///////////////////////////////////
		// active sleep status check
		if (((double)(((lower_margin_5to7 + lower_margin_7to10 + lower_margin_above10) * 1.0) / queue_size) >= 0.85) &&
			((double)(((bandwidth_below5 + bandwidth_5to10) * 1.0) / queue_size) >= 0.8))
		{
			if (AS_sleep_status == 0)	// start of active sleep
			{
				AS_sleep_status = 1;
				AS_start_time = time_elapsed - feature_queue_aEEG_window_len;
			}
		}
		else if (AS_sleep_status == 1)	// active sleep end
		{
			AS_sleep_status = 0;
			AS_end_time = time_elapsed - feature_queue_aEEG_window_len;

			// find the last node containing the AS information
			sleep_count = 0;
			tail = marking_database[ch_no];
			while (tail != NULL)
			{
				if (tail->mark_event == ACTIVE_SLEEP_MARK)
				{
					sleep_count++;
					last_sleep_info = tail;		// store the last AS pointer
				}
				tail = tail->next;
			}

			// if current AS interval overlaps with previous recorded AS
			// interval then update previous recorded AS information
			if ((sleep_count > 0) && 
				(AS_start_time > last_sleep_info->mark_start_time) && 
				(AS_start_time <= last_sleep_info->mark_end_time))
			{
				if (AS_end_time > last_sleep_info->mark_end_time)
				{
					last_sleep_info->mark_end_time = AS_end_time;
				}
			}
			// else if current AS just follows the last AS interval by a minimal distance then 
			// merge the entries
			else if ((sleep_count > 0) && 
				(AS_start_time > last_sleep_info->mark_end_time) && 
				((AS_start_time - last_sleep_info->mark_end_time) < SLEEP_MERGE_MAX_TIME_GAP))
			{
				last_sleep_info->mark_end_time = AS_end_time;
			}
			else	// insert the AS time in the marking database as a new entry
			{
				AddMarkData(ch_no, AS_start_time, AS_end_time, ACTIVE_SLEEP_MARK);
			}
		}
		///////////////////////////////////

		// calculate the segment boundary
		start_index = (int) floor(time_elapsed * sampling_rate);
		end_index = start_index + (int) floor(aEEG_time_window_len * sampling_rate) - 1;
		if (end_index >= channel_data_size)
			end_index = (channel_data_size - 1);

		// read current signal segment from the aEEG data
		Read_Signal_Segment(ch_no, start_index, end_index, input_signal, inp_sig_len, true);

		// sort the input signal
		insert_sort(input_signal, inp_sig_len);

		// calculate the percentile values
		percentile_index = (int)(inp_sig_len * 5.0 / 100.0);
		percentile_5 = input_signal[percentile_index];
		log_perc_5 = SemilogAdapt(percentile_5);

		percentile_index = (int)(inp_sig_len * 95.0 / 100.0);
		percentile_95 = input_signal[percentile_index];
		log_perc95 = SemilogAdapt(percentile_95);

		// insert the feature values in a queue
		// bandwidth and lower upper margins are calculated in semilogarithmic scale
		bandwidth_queue[queue_counter] = (log_perc95 - log_perc_5);
		upper_margin_queue[queue_counter] = log_perc95;
		lower_margin_queue[queue_counter] = log_perc_5;
		queue_counter = (queue_counter + 1) % queue_size;

        // update time counter by aEEG_time_window_len 
        time_elapsed = time_elapsed + aEEG_time_window_len;  		

	}	// end of 2nd for loop

	// now print the sleep information on the text file
	tail = marking_database[ch_no];
	while (tail != NULL)
	{
		if ((tail->mark_event == QUIET_SLEEP_MARK) || (tail->mark_event == ACTIVE_SLEEP_MARK))
		{
			// print the time
			start_hr = tail->mark_start_time / 3600;
			start_min = (tail->mark_start_time - start_hr * 3600) / 60;
			start_sec = (tail->mark_start_time - start_hr * 3600 - start_min * 60); 
			end_hr = tail->mark_end_time / 3600;
			end_min = (tail->mark_end_time - end_hr * 3600) / 60;
			end_sec = (tail->mark_end_time - end_hr * 3600 - end_min * 60); 
			
			if (tail->mark_event == QUIET_SLEEP_MARK)
				fprintf(fod_aeeg, "\n QS ---");
			else
				fprintf(fod_aeeg, "\n AS ---");

			fprintf(fod_aeeg, " start time : %d:%d:%d  end time : %d:%d:%d  \n", 
				start_hr, start_min, start_sec, end_hr, end_min, end_sec);			
		}
		tail = tail->next;
	}

	// close the text file
	fclose(fod_aeeg);

}	// end of function







# if 0
void CEEG_marking_toolDlg::aEEG_SWC_determine(int ch_no)
{
	int feature_queue_aEEG_window_len = 300;	// 5 min feature storage	
	int aEEG_time_window_len = 10;	// 10 sec non-sliding window for aEEG bandwidth determination
	int queue_size = 30;
	
	double time_elapsed = 0;	// marks the time elapsed during data analysis 
	int start_index, end_index;		// location boundary of input signal segment
	int inp_sig_len = (int)(ceil(sampling_rate * aEEG_time_window_len));	// 1 sec signal length

	// allocate memory for the input signal data
	double *input_signal = (double *) calloc(inp_sig_len, sizeof(double));	

	double percentile_5;
	double percentile_95;
	double log_perc_5, log_perc95;
	
	int percentile_index;

	int start_hr, start_min, start_sec;
	int end_hr, end_min, end_sec;

	// feature queues
	double *bandwidth_queue = (double *) calloc(queue_size, sizeof(double));	
	double *upper_margin_queue = (double *) calloc(queue_size, sizeof(double));	
	double *lower_margin_queue = (double *) calloc(queue_size, sizeof(double));	

	int queue_counter = 0;
	int i;
	
	// counters for indicating the status of the lower margin
	// lower margin is plotted in semilogarithmic scale
	int lower_margin_below3, lower_margin_3to5, lower_margin_5to7, lower_margin_7to10, lower_margin_above10;

	// counters for indicating the status of the upper margin
	// upper margin is plotted in semilogarithmic scale
	int upper_margin_below8, upper_margin_8to12, upper_margin_12to15, upper_margin_15to18, upper_margin_above18;

	// counters for indicating the bandwidth status
	// measured in semilogarithmic scale
	int bandwidth_below5, bandwidth_5to10, bandwidth_10to15, bandwidth_above15;

	// debug - sourya 
	FILE *fod_aeeg;
	CString out_text_filename = directory_name + "aEEG_bandwidth_final.txt";
	if (ch_no == 0)	
		fod_aeeg = fopen(out_text_filename, "w");
	else
		fod_aeeg = fopen(out_text_filename, "a");

	fprintf(fod_aeeg, "\n\n\n channel no : %d  channel name : %s ", ch_no, channel_name[ch_no]);

	// loop to read the data 
	// here first we read initial some segment to form the feature queue
	while((time_elapsed + TIME_WINDOW_LENGTH) < feature_queue_aEEG_window_len)
	{ 
		// calculate the segment boundary
		start_index = (int) floor(time_elapsed * sampling_rate);
		end_index = start_index + (int) floor(aEEG_time_window_len * sampling_rate) - 1;
		if (end_index >= channel_data_size)
			end_index = (channel_data_size - 1);

		// read current signal segment from the aEEG data
		Read_Signal_Segment(ch_no, start_index, end_index, input_signal, inp_sig_len, true);

		// sort the input signal
		insert_sort(input_signal, inp_sig_len);

		// calculate the percentile values
		percentile_index = (int)(inp_sig_len * 5.0 / 100.0);
		percentile_5 = input_signal[percentile_index];
		log_perc_5 = SemilogAdapt(percentile_5);

		percentile_index = (int)(inp_sig_len * 95.0 / 100.0);
		percentile_95 = input_signal[percentile_index];
		log_perc95 = SemilogAdapt(percentile_95);

		// insert the feature values in a queue
		// bandwidth and lower upper margins are calculated in semilogarithmic scale
		bandwidth_queue[queue_counter] = (log_perc95 - log_perc_5);
		upper_margin_queue[queue_counter] = log_perc95;
		lower_margin_queue[queue_counter] = log_perc_5;
		queue_counter = (queue_counter + 1) % queue_size;

        // update time counter by aEEG_time_window_len 
        time_elapsed = time_elapsed + aEEG_time_window_len;  		
	}	// end of first loop

	queue_counter = 0;	// reset the queue counter

	// loop to read the data 
	while((time_elapsed + TIME_WINDOW_LENGTH) < total_recording_duration_sec)
	{
		// at first print the current status
		// time interval
		start_hr = time_elapsed / 3600;
		start_min = (time_elapsed - start_hr * 3600) / 60;
		start_sec = (time_elapsed - start_hr * 3600 - start_min * 60); 
		end_hr = (time_elapsed + aEEG_time_window_len) / 3600;
		end_min = ((time_elapsed + aEEG_time_window_len) - end_hr * 3600) / 60;
		end_sec = ((time_elapsed + aEEG_time_window_len) - end_hr * 3600 - end_min * 60); 

		fprintf(fod_aeeg, "\n start time : %d:%d:%d  end time : %d:%d:%d  \n", 
			start_hr, start_min, start_sec, end_hr, end_min, end_sec);

		// check the queues and update respective counters
		lower_margin_below3 = 0;
		lower_margin_3to5 = 0;
		lower_margin_5to7 = 0;
		lower_margin_7to10 = 0;
		lower_margin_above10 = 0;

		// counters for indicating the status of the upper margin
		// upper margin is plotted in semilogarithmic scale
		upper_margin_below8 = 0;
		upper_margin_8to12 = 0;	
		upper_margin_12to15 = 0;
		upper_margin_15to18 = 0;
		upper_margin_above18 = 0;

		// counters for indicating the bandwidth status
		// measured in semilogarithmic scale
		bandwidth_below5 = 0;
		bandwidth_5to10 = 0;
		bandwidth_10to15 = 0;
		bandwidth_above15 = 0;

		for (i = 0; i < queue_size; i++)
		{
			// check the lower margin queue and adjust the counters
			if (lower_margin_queue[i] < 3)
				lower_margin_below3++;
			else if ((lower_margin_queue[i] >= 3) && (lower_margin_queue[i] < 5))
				lower_margin_3to5++;
			else if ((lower_margin_queue[i] >= 5) && (lower_margin_queue[i] < 7))
				lower_margin_5to7++;	
			else if ((lower_margin_queue[i] >= 7) && (lower_margin_queue[i] < 10))
				lower_margin_7to10++;
			else	// if (lower_margin_queue[i] >= 10) 
				lower_margin_above10++;

			// check the upper margin queue and adjust the counters
			if (upper_margin_queue[i] < 8)
				upper_margin_below8++;
			else if ((upper_margin_queue[i] >= 8) && (upper_margin_queue[i] < 12))
				upper_margin_8to12++;
			else if ((upper_margin_queue[i] >= 12) && (upper_margin_queue[i] < 15))
				upper_margin_12to15++;
			else if ((upper_margin_queue[i] >= 15) && (upper_margin_queue[i] < 18))
				upper_margin_15to18++;
			else //if (upper_margin_queue[i] >= 18)
				upper_margin_above18++;

			// check the bandwidth queue and adjust the counters
			if (bandwidth_queue[i] < 5)	
				bandwidth_below5++;
			else if ((bandwidth_queue[i] >= 5) && (bandwidth_queue[i] < 10))
				bandwidth_5to10++;
			else if ((bandwidth_queue[i] >= 10) && (bandwidth_queue[i] < 15))
				bandwidth_10to15++;
			else //if (bandwidth_queue[i] >= 15)
				bandwidth_above15++;
		}	

		fprintf(fod_aeeg, "\n lower margin -- below3 : %lf 3to5 : %lf 5to7 : %lf 7to10 : %lf above10 : %lf ",
			(double)((lower_margin_below3 * 1.0) / queue_size), (double)((lower_margin_3to5 * 1.0) / queue_size),
			(double)((lower_margin_5to7 * 1.0) / queue_size), (double)((lower_margin_7to10 * 1.0) / queue_size),
			(double)((lower_margin_above10 * 1.0) / queue_size));

		fprintf(fod_aeeg, "\n upper margin -- below8 : %lf 8to12 : %lf 12to15 : %lf 15to18 : %lf above18 : %lf \n",
			(double)((upper_margin_below8 * 1.0) / queue_size), (double)((upper_margin_8to12 * 1.0) / queue_size),
			(double)((upper_margin_12to15 * 1.0) / queue_size), (double)((upper_margin_15to18 * 1.0) / queue_size),
			(double)((upper_margin_above18 * 1.0) / queue_size));

		fprintf(fod_aeeg, "\n bandwidth -- below5 : %lf 5to10 : %lf 10to15 : %lf above15 : %lf ",
			(double)((bandwidth_below5 * 1.0) / queue_size), (double)((bandwidth_5to10 * 1.0) / queue_size),
			(double)((bandwidth_10to15 * 1.0) / queue_size), (double)((bandwidth_above15 * 1.0) / queue_size));


		// calculate the segment boundary
		start_index = (int) floor(time_elapsed * sampling_rate);
		end_index = start_index + (int) floor(aEEG_time_window_len * sampling_rate) - 1;
		if (end_index >= channel_data_size)
			end_index = (channel_data_size - 1);

		// read current signal segment from the aEEG data
		Read_Signal_Segment(ch_no, start_index, end_index, input_signal, inp_sig_len, true);

		// sort the input signal
		insert_sort(input_signal, inp_sig_len);

		// calculate the percentile values
		percentile_index = (int)(inp_sig_len * 5.0 / 100.0);
		percentile_5 = input_signal[percentile_index];
		log_perc_5 = SemilogAdapt(percentile_5);

		percentile_index = (int)(inp_sig_len * 95.0 / 100.0);
		percentile_95 = input_signal[percentile_index];
		log_perc95 = SemilogAdapt(percentile_95);


		//fprintf(fod_aeeg, "\n start time : %d:%d:%d  end time : %d:%d:%d  percentile_5 : %lf percentile_95 : %lf bandwidth : %lf  --- log scale - percentile_5 : %lf percentile_95 : %lf bandwidth : %lf ", 
		//	start_hr, start_min, start_sec, end_hr, end_min, end_sec, percentile_5, percentile_95, (percentile_95 - percentile_5), log_perc_5, log_perc95, (log_perc95 - log_perc_5));

		// insert the feature values in a queue
		// bandwidth and lower upper margins are calculated in semilogarithmic scale
		bandwidth_queue[queue_counter] = (log_perc95 - log_perc_5);
		upper_margin_queue[queue_counter] = log_perc95;
		lower_margin_queue[queue_counter] = log_perc_5;
		queue_counter = (queue_counter + 1) % queue_size;

        // update time counter by aEEG_time_window_len 
        time_elapsed = time_elapsed + aEEG_time_window_len;  		

	}	// end of channel data read loop


	// close the text file
	fclose(fod_aeeg);

}	// end of function
# endif