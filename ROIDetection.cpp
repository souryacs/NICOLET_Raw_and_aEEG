// ROIDetection.cpp : implementation file
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
 call the burst detection routine with the output file name as the argument
 this is our custom implementation of burst suppression pattern detection.
*/
void CEEG_marking_toolDlg::ROI_Detection(int ch_no)
{

# if 0
	// ratio threshold values - static threshold
	// thresholds are maintained according to detect as many as possible
  int ENRGY_MULTIPLE_THRESHOLD = 3;
  int VARIANCE_MULTIPLE_THRESHOLD = 3;
  int PSD_MULTIPLE_THRESHOLD = 3;	
	// average burst detection voltage values
	int BURST_LOWER_VOLTAGE_THRESHOLD = 75;	//120;	
	int BURST_UPPER_VOLTAGE_THRESHOLD = 250;  
# endif
	
	double time_elapsed = 0;	// marks the time elapsed during data analysis 
	
	int start_index, end_index;		// location boundary of input signal segment

	int inp_sig_len = (int)(ceil(sampling_rate * TIME_WINDOW_LENGTH));	// 1 sec signal length
	int i;	

	// burst signifying variables
	int burst_status = 0;
	double burst_start_time, burst_end_time;

	int nfft = (int)(pow(2, (int)ceil(log(sampling_rate) / log(2))) * TIME_WINDOW_LENGTH);	

	// allocate memory for the input signal data
	double *input_signal = (double *) calloc(inp_sig_len, sizeof(double));	

	// auxiliary storage for calculation of fourier transform
	double *x = (double *) calloc((2 * nfft), sizeof(double));

	// queues for storing various feature data (computation of ratio based features)
	double *non_linear_energy_queue = (double *) calloc(QUEUE_SIZE, sizeof(double));	
	double *variance_queue = (double *) calloc(QUEUE_SIZE, sizeof(double));	
	double *psd_queue = (double *) calloc(QUEUE_SIZE, sizeof(double));	
	double *mean_abs_volt_queue = (double *) calloc(QUEUE_SIZE, sizeof(double));	

	// queue counters
	int elem_index1 = 0, elem_index2 = 0, elem_index3 = 0, elem_index4 = 0;

	// variables to store current features
	//double power_3_hz;
	double mnle = 0, abs_volt_sum = 0, total_psd = 0, var = 0, power_10_hz;
	double d = nfft * nfft * 1.0;	// divider in PSD estimate

	// the minimum time where feature is to be extracted is set here
	// it is used because burst queue initialization is dependent upon median values of 
	// respective features 
	double min_queue_feat_extract_time = 120;	// 2 min	
	if (min_queue_feat_extract_time > total_recording_duration_sec)
	{	
		min_queue_feat_extract_time = total_recording_duration_sec;
	}

	//allocate different storage arrays for different features
	double *temp_nle = (double *) calloc((int)(min_queue_feat_extract_time / SLIDING_WINDOW_OFFSET + 1), sizeof(double));	
	double *temp_var = (double *) calloc((int)(min_queue_feat_extract_time / SLIDING_WINDOW_OFFSET + 1), sizeof(double));	
	double *temp_psd = (double *) calloc((int)(min_queue_feat_extract_time / SLIDING_WINDOW_OFFSET + 1), sizeof(double));	
	double *temp_volt = (double *) calloc((int)(min_queue_feat_extract_time / SLIDING_WINDOW_OFFSET + 1), sizeof(double));	
	double *temp_power_10hz = (double *) calloc((int)(min_queue_feat_extract_time / SLIDING_WINDOW_OFFSET + 1), sizeof(double));	
	int temp_feat_count = 0;

	// mean of several feature queues
	double mean_nle_queue, mean_var_queue, mean_psd_queue, mean_mean_abs_volt_queue;

	// ratio based feature value
	double ratio_nle, ratio_abs_volt, norm_psd, ratio_var, ratio_psd;	

	// defined in header file
	/*
# if 1
	// test data creation
	CvMat *mat_test_feat;		
# endif
	*/

	// pointer to node of the marking database
	// struct data_mark_format* tail;	
	struct data_mark_format* last_burst_info = NULL;	
  int burst_count = 0;  // counts the no of burst patterns occurred so far

	// test outcome for SVM classification
	int test_label = 0;

	// sourya - debug
# if 0
	FILE* temp_out;
	temp_out = fopen("sample_feature_out.xls", "a");
	fprintf(temp_out, "\n\n\n\n channel no : %d name : %s ", ch_no, channel_name[ch_no]);
	fprintf(temp_out, "%s \t %s \t %s \t %s \t %s \t %s \t %s \t %s", "time", "2", "4", "5", "6", "9", "10", "11");
# endif


	// loop 
	// here first we read initial some segment to decide the background feature segments
	// we calculate segment features and store them in temporary storage
	while((time_elapsed + TIME_WINDOW_LENGTH) < min_queue_feat_extract_time)
	{ 
		// calculate the segment boundary
		start_index = (int) floor(time_elapsed * sampling_rate);
		end_index = start_index + (int) floor(TIME_WINDOW_LENGTH * sampling_rate) - 1;
		if (end_index >= channel_data_size)
			end_index = (channel_data_size - 1);

		// read current signal segment
		Read_Signal_Segment(ch_no, start_index, end_index, input_signal, inp_sig_len, false);

		// fill the auxiliary array structures
		for (i = 0; i < nfft; i++)
		{
			// fill realpart of the signal with input signal data
			if (i < inp_sig_len)
				x[2 * i] = input_signal[i];		
			else
				x[2 * i] = 0;					// fill with 0

			// fill imaginary part of the signal
			x[2 * i + 1] = 0;
		}
		//////////////////////////////////
		// now extract different features 

		// 1. mean non linear energy
		mnle = ComputeMNLE(input_signal, inp_sig_len);

		// 2. variance
		var = ComputeVar(input_signal, inp_sig_len);
	
		// 3. absolute voltage sum
		abs_volt_sum = ComputeMeanAbsVolt(input_signal, inp_sig_len);

		// 4. power spectral density
		DeriveFFT(x, nfft);
		total_psd = ComputePSD(x, nfft, d);

		// 5. power at 10 Hz
		power_10_hz = Compute_X_Hz_power(x, nfft, d, 10);

		// store the features in the respective storage
		temp_nle[temp_feat_count] = mnle;
		temp_var[temp_feat_count] = var;
		temp_psd[temp_feat_count] = total_psd;
		temp_volt[temp_feat_count] = abs_volt_sum;
		temp_power_10hz[temp_feat_count] = power_10_hz;
		temp_feat_count++;

		//////////////////////////////////////
    // update time counter by SLIDING_WINDOW_OFFSET 
    time_elapsed = time_elapsed + SLIDING_WINDOW_OFFSET;  		

	} // end of loop for computing features on initial segments

	// we have stored the features for approximately 2 minutes
	// we find the median of the feature values and store the output in the respective feature queues
	// mean can also be used 
	mnle = mean(temp_nle, temp_feat_count);	// median(temp_nle, temp_feat_count);
	var = mean(temp_var, temp_feat_count);	// median(temp_var, temp_feat_count);
	total_psd = mean(temp_psd, temp_feat_count);	// median(temp_psd, temp_feat_count);
	abs_volt_sum = mean(temp_volt, temp_feat_count);	// median(temp_volt, temp_feat_count);

	// store the median values in respective queues
	for (i = 0; i < QUEUE_SIZE; i++)
	{
		non_linear_energy_queue[i] = mnle;
		variance_queue[i] = var;
		psd_queue[i] = total_psd;
		mean_abs_volt_queue[i] = abs_volt_sum;
	}

	// reset the time counter
	time_elapsed = 0;
	temp_feat_count = 0;

	// loop to process the data 	
	// this time we traverse whole data 
	while((time_elapsed + TIME_WINDOW_LENGTH) <= total_recording_duration_sec)
	{
		if ((time_elapsed + TIME_WINDOW_LENGTH) >= min_queue_feat_extract_time)
		{
			// calculate the segment boundary
			start_index = (int) floor(time_elapsed * sampling_rate);
			end_index = start_index + (int) floor(TIME_WINDOW_LENGTH * sampling_rate) - 1;
			if (end_index >= channel_data_size)
				end_index = (channel_data_size - 1);

			// read current signal segment
			Read_Signal_Segment(ch_no, start_index, end_index, input_signal, inp_sig_len, false);

			// fill the auxiliary array structures
			for (i = 0; i < nfft; i++)
			{
				// fill realpart of the signal with input signal data
				if (i < inp_sig_len)
					x[2 * i] = input_signal[i];		
				else
					x[2 * i] = 0;					// fill with 0

				// fill imaginary part of the signal
				x[2 * i + 1] = 0;
			}

			//////////////////////////////////
			// now extract different features 

			// 1. mean non linear energy
			mnle = ComputeMNLE(input_signal, inp_sig_len);

			// 2. variance
			var = ComputeVar(input_signal, inp_sig_len);
		
			// 3. absolute voltage sum
			abs_volt_sum = ComputeMeanAbsVolt(input_signal, inp_sig_len);

			// 4. power spectral density
			DeriveFFT(x, nfft);
			total_psd = ComputePSD(x, nfft, d);

			// 5. power at 10 Hz
			power_10_hz = Compute_X_Hz_power(x, nfft, d, 10);
		}
		else	// use already computed feature values
		{
			// we have already calculated and stored the feature
			// so we extract those values
			mnle = temp_nle[temp_feat_count];
			var = temp_var[temp_feat_count];
			total_psd = temp_psd[temp_feat_count];			
			abs_volt_sum = temp_volt[temp_feat_count];
			power_10_hz = temp_power_10hz[temp_feat_count];			
			temp_feat_count++;
		}

		// obtain mean values of queues so far
		mean_nle_queue = mean(non_linear_energy_queue, QUEUE_SIZE);
		mean_var_queue = mean(variance_queue, QUEUE_SIZE);
		mean_psd_queue = mean(psd_queue, QUEUE_SIZE);
		mean_mean_abs_volt_queue = mean(mean_abs_volt_queue, QUEUE_SIZE);

		////////////////////////////////////////////
		// feature set preperation
		// feature 2
		ratio_nle = mnle / mean_nle_queue;	
		// feature 4
		ratio_abs_volt = abs_volt_sum / mean_mean_abs_volt_queue;	
		// feature 6
		norm_psd = total_psd / inp_sig_len;		
		// ratio of psd is the feature 7
		ratio_psd = total_psd / mean_psd_queue;
		// power_10_hz is feature 9
		// var is feature 10
		// feature 11
		ratio_var = var / mean_var_queue;	

		////////////////////////////////////////////

		// debug - sourya - write down the feature values
# if 0
		fprintf(temp_out, "\n %lf - %lf", time_elapsed, (time_elapsed + TIME_WINDOW_LENGTH));		// time interval
		fprintf(temp_out, "\t %lf", ratio_nle);	// feature 2
		fprintf(temp_out, "\t %lf", ratio_abs_volt);	// feature 4
		fprintf(temp_out, "\t %lf", mnle);	// feature 5
		fprintf(temp_out, "\t %lf", norm_psd);	// feature 6
		fprintf(temp_out, "\t %lf", power_10_hz);	// feature 9
		fprintf(temp_out, "\t %lf", var);	// feature 10
		fprintf(temp_out, "\t %lf", ratio_var);	// feature 11
# endif
		////////////////////////////////////////////
# if 1
		// now we have to call the SVM function for classifying between normal EEG segments and 
		// burst EEG segments
		
		// create the test sample
		// allocated already in calling function
		//mat_test_feat = cvCreateMat(1, no_of_feature_burst_detection, CV_32FC1);


		mat_test_feat_burst->data.fl[0] = ratio_nle;
		mat_test_feat_burst->data.fl[1] = ratio_abs_volt;
		mat_test_feat_burst->data.fl[2] = norm_psd;		
		mat_test_feat_burst->data.fl[3] = ratio_psd;
		mat_test_feat_burst->data.fl[4] = power_10_hz;
		mat_test_feat_burst->data.fl[5] = var;
		mat_test_feat_burst->data.fl[6] = ratio_var;

		// predict the sample
		test_label = (int) svm_classifier_burst_detection->predict(mat_test_feat_burst);

# endif

# if 0
		// burst detection conditions based on static threshold
		if ( (ratio_nle >= ENRGY_MULTIPLE_THRESHOLD) && 
			(abs_volt_sum >= BURST_LOWER_VOLTAGE_THRESHOLD) && 
			(abs_volt_sum <= BURST_UPPER_VOLTAGE_THRESHOLD) && 
			((ratio_var >= VARIANCE_MULTIPLE_THRESHOLD) || 
			(total_psd >= (PSD_MULTIPLE_THRESHOLD * mean_psd_queue)) )  )   
		{
			test_label = 1;
		}
		else 
			test_label = 0;
# endif
		////////////////////////////////////////////
		
		if (test_label == 1)	
		{
			if (burst_status == 0)
			{
				// burst has started
				burst_status = 1;
				burst_start_time = time_elapsed;
			}
		}
		else if (burst_status == 1)
		{
			// end of the burst
			burst_status = 0;
			burst_end_time = time_elapsed + TIME_WINDOW_LENGTH - SLIDING_WINDOW_OFFSET;   
			if (burst_end_time > total_recording_duration_sec)
			{
			   burst_end_time = total_recording_duration_sec; 
			}

			// insert the burst time in the marking database
			if ((burst_count > 0) && 
				(burst_start_time > last_burst_info->mark_start_time) && 
				(burst_start_time <= last_burst_info->mark_end_time))
			{
				// current burst interval overlaps with previous recorded
				// burst interval - so update previous recorded burst
				if (burst_end_time > last_burst_info->mark_end_time)
				{
					last_burst_info->mark_end_time = burst_end_time;
				}
			}
			else
			{
				// new burst interval or non overlapping burst - add one more burst entry
				last_burst_info = AddMarkData(ch_no, burst_start_time, burst_end_time, BURST_MARK);
				burst_count++;
			}
		}

		////////////////////////////////////////////

		// update the feature queues with suppression or background
		// samples (i.e. when there is no burst interval that is ongoing)
		if (burst_status == 0)            
		{
			if (mnle < (2 * mean_nle_queue))
			{
				non_linear_energy_queue[elem_index1] = mnle;   // update the nean non linear energy queue
				elem_index1 = (elem_index1 + 1) % QUEUE_SIZE;  // circular queue
			}
			if (var < (2 * mean_var_queue))
			{
				variance_queue[elem_index2] = var;   // update the variance queue
				elem_index2 = (elem_index2 + 1) % QUEUE_SIZE;	//circular queue
			}
			if (total_psd < (2 * mean_psd_queue))
			{
				psd_queue[elem_index3] = total_psd;   // update the PSD queue
				elem_index3 = (elem_index3 + 1) % QUEUE_SIZE;	//circular queue
			}
			if (abs_volt_sum < (2 * mean_mean_abs_volt_queue))
			{
				mean_abs_volt_queue[elem_index4] = abs_volt_sum;	// update the absolute voltage queue
				elem_index4 = (elem_index4 + 1) % QUEUE_SIZE;	//circular queue
			}
		}
    // update time counter by SLIDING_WINDOW_OFFSET 
    time_elapsed = time_elapsed + SLIDING_WINDOW_OFFSET;  		
	}

	// free the allocated memory
	free(input_signal);
	free(x);
	free(non_linear_energy_queue);
	free(variance_queue);
	free(psd_queue);
	free(mean_abs_volt_queue);
	free(temp_nle);
	free(temp_var);
	free(temp_psd);
	free(temp_volt);
	free(temp_power_10hz);

	// sourya - debug
# if 0
	fclose(temp_out);
# endif

}	// end of ROI detection function
