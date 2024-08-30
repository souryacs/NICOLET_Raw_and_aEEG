// FeatureCompute.cpp: implementation file
// this file contains functions for generating the features which will be used for burst, artifact detection

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
	this function computes Higuchi Fractal Dimension
*/
double CEEG_marking_toolDlg::ComputeHiguchiFD(double* input_signal, int inp_sig_len)
{
    int k, m, i;
	double temp_sum;
	double avg_len[5] = {0,0,0,0,0};
	double len[5][5] = {{0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}};

	double xpoint[5];
	double ypoint[5];
	double mean_xpoint, mean_ypoint;
	double sum_xypoint_dist = 0, sum_xxpoint_dist = 0;

	double HFD;

    for (k = 0; k < 5; k++)     // 5 is kmax
	{
        for (m = 0; m < k; m++)
		{
            temp_sum = 0;
            for (i = 1; i < ((inp_sig_len - m) / k); i++)
			{
                 temp_sum += fabs(input_signal[m + i*k] - input_signal[m + i*k - k]);   
            }
            len[m][k] = (temp_sum * (inp_sig_len - 1)) / (((inp_sig_len - m) / k) * k);
        }
        for (i = 0; i < k; i++)
		{
            avg_len[k] += len[i][k];
        }
        avg_len[k] /= (k + 1);
    }

	// now get the best line fit slope for curves ln(avg_len(k)) and ln(1/k)
	for (i = 0; i < 5; i++)
	{
		xpoint[i] = log(avg_len[i+1]);
		ypoint[i] = log(1.0 / (i+1));	
	}

	// mean of points
	mean_xpoint = mean(xpoint, 5);
	mean_ypoint = mean(ypoint, 5);

	// calculate the deviation of mean for both X and Y points
	// calculate the product of XY and XX
	for (i = 0; i < 5; i++)
	{
		sum_xypoint_dist += (mean_xpoint - xpoint[i]) * (mean_ypoint - ypoint[i]);
		sum_xxpoint_dist += (mean_xpoint - xpoint[i]) * (mean_xpoint - xpoint[i]);
	}

	// slope of the best fit line is the target HFD	
    HFD = (double) fabs(sum_xypoint_dist / sum_xxpoint_dist);      

	return HFD;
}

/*
	this function computes Kurtosis
*/
double CEEG_marking_toolDlg::ComputeKurt(double* input_signal, int inp_sig_len)
{
	double s, avg;
	int i;
	double kurt, var;

	var = ComputeVar(input_signal, inp_sig_len);

	s = 0;	kurt = 0;
	for (i = 0; i < inp_sig_len; i++)
		s += input_signal[i];
	
	avg = s / inp_sig_len;

	for (i = 0; i < inp_sig_len; i++)
		kurt += pow((input_signal[i] - avg), 4);
	
	if (var)
		kurt = (kurt / (inp_sig_len * pow(var, 2))) - 3;
	else
		kurt = 0;

	return kurt;
}

/*
	this is the stein's unbiased risk estimate based threshold calculation routine
*/
double CEEG_marking_toolDlg::calc_rigsure_thrs(double *inp_sig, int inp_sig_len)
{
	double curr_threshold;
	double t, x, drstdt, learning_rate, delta_threshold;
	int i;
	double *g;
	double *dgdt;
	double *d2gdt; 

	// estimate the initial threshold value by median filtering	
	for (i = 0; i < inp_sig_len; i++)
	{
		inp_sig[i] = (double)fabs(inp_sig[i]);
	}
	curr_threshold = sqrt(2 * (median(inp_sig, inp_sig_len)) * log(inp_sig_len) / 0.6745);

	// allocate the temporary memory
	g = (double *)calloc(inp_sig_len, sizeof(double));
	dgdt = (double *)calloc(inp_sig_len, sizeof(double));
	d2gdt = (double *)calloc(inp_sig_len, sizeof(double));
	
    // now loop through current coeffients to get the differential
    // values for threshold increment computation
	while (1)
	{
		t = curr_threshold;
        for (i = 0; i < inp_sig_len; i++)
		{
            x = inp_sig[i];
            if (x < -t)
			{
				g[i] = (6.0 * t / 7.0); 
				dgdt[i] = 6.0 / 7.0;
				d2gdt[i] = 0;
			}
            else if (x > t)
			{
				g[i] = (-6.0 * t / 7.0); 
				dgdt[i] = -6.0 / 7.0;
				d2gdt[i] = 0;
			}
			else
			{
				g[i] = (pow(x, 7) / (7.0 * pow(t, 6))) - x;
				dgdt[i] = ((-6.0 / 7.0) * pow((x / t), 7));
				d2gdt[i] = (-6.0 * pow(x, 6) / pow(t, 7));
			}
        }  // end partial array traverse

        // calculate risk differential
        drstdt = 0;
        for (i = 0; i < inp_sig_len; i++)
		{
            drstdt += (2 * g[i] * dgdt[i] + 2 * d2gdt[i]);
        }

        // initialize the learning rate (alpha)
        learning_rate = 1;

        // product of learnig rate and risk differential should be <= current threshold
        // unless the learning rate should be changed
        // threshold should be non negative
        while (((learning_rate * drstdt) > curr_threshold) && (curr_threshold > 0))
            learning_rate = learning_rate / 2;
        
        // finally the delta of threshold is computed
        delta_threshold = (learning_rate * drstdt);
            
        if (curr_threshold == 0)
            break;
        else if ((delta_threshold / curr_threshold) < pow(10, -6))
            break;
        else
            curr_threshold -= delta_threshold;
        
	}	// end of loop for threshold calculation

	// free the temporary alocated signal
	free(g);
	free(dgdt);	
	free(d2gdt);

	return curr_threshold;

} // end of STEIN based risk method

/////////////////////////////////////
// old version code - do not delete - may be required in future

# if 0

double CEEG_marking_toolDlg::calc_rigsure_thrs(double *inp_sig, int inp_sig_len)
{
	double thrs;
	int i, j;
	double hold;
	bool switched = 1;
	double* cumsum = (double *)calloc(inp_sig_len, sizeof(double));
	double* risks = (double *)calloc(inp_sig_len, sizeof(double));
	
	// make absolute version of the data
	for(i = 0; i < inp_sig_len; i++)
		inp_sig[i] = fabs(inp_sig[i]);

	// sort the input data
	for(i = 0; (i <(inp_sig_len - 1) && (switched == 1)); i++)
	{
		switched = 0;
		for(j = 0; j <(inp_sig_len - 1 - i); j++)
		{
			if(inp_sig[j] > inp_sig[j+1])
			{
				switched = 1;
				hold = inp_sig[j];
				inp_sig[j] = inp_sig[j+1];
				inp_sig[j+1] = hold;
			}
		}
	}

	// make each element square
	for(i = 0; i < inp_sig_len; i++)
	{
		inp_sig[i] = pow(inp_sig[i], 2);

		// calculate cumulative sum
		if (i == 0)
			cumsum[i] = inp_sig[i];
		else
			cumsum[i] = cumsum[i-1] + inp_sig[i];
	}


	// now manipulate the array
	j = (inp_sig_len - 1);
	for(i = 0; i < inp_sig_len; i++)
	{
		risks[i] = (j * inp_sig[i] + cumsum[i] + inp_sig_len - 2 * (i+1)) / inp_sig_len;  
		j--;
	}

	// find the minimum of the risk array
	double risk_min = risks[0];
	int best = 0;
	for(i = 1; i < inp_sig_len; i++)
	{
		if (risks[i] < risk_min)
		{	
			risk_min = risks[i];
			best = i;
		}
	}
	thrs = sqrt(inp_sig[best]);

	free(cumsum);
	free(risks);

	return thrs;
}

# endif // # if 0
/////////////////////////////////////

/*
	this is the wavelet decomposition algorithm 
	here Coiflet3 (coif3) algorithm is used
*/
void CEEG_marking_toolDlg::wtr_fwd_coif3(double *a, int n, int *len_coeff)
{
	int i, j, k;
	int half;
	int l;
	double h[18];
	double g[18];
	double *tmp_arr;

	// allocate memory
	tmp_arr = (double *) calloc(n, sizeof(double));

	// coiflet 3 wavelet coefficients
	// smoothing coefficients
	h[0] = -0.0000345998;
	h[1] = -0.0000709833;
	h[2] =  0.0004662170;
	h[3] =  0.0011175188;
	h[4] = -0.0025745177;
	h[5] = -0.0090079761;
	h[6] =  0.0158805449;
	h[7] =  0.0345550276;
	h[8] = -0.0823019271;
	h[9] = -0.0717998216;
	h[10] =  0.4284834764;
	h[11] =  0.7937772226;
	h[12] =  0.4051769024;
	h[13] = -0.0611233900;
	h[14] = -0.0657719113;
	h[15] =  0.0234526961;
	h[16] =  0.0077825964;
	h[17] = -0.0037935129;

	// detail coefficients
	g[0] =  0.0037935129;
	g[1] =  0.0077825964;
	g[2] = -0.0234526961;
	g[3] = -0.0657719113;
	g[4] =  0.0611233900;
	g[5] =  0.4051769024;
	g[6] = -0.7937772226;
	g[7] =  0.4284834764;
	g[8] =  0.0717998216;
	g[9] = -0.0823019271;
	g[10] = -0.0345550276;
	g[11] =  0.0158805449;
	g[12] =  0.0090079761;
	g[13] = -0.0025745177;
	g[14] = -0.0011175188;
	g[15] =  0.0004662170;
	g[16] =  0.0000709833;
	g[17] = -0.0000345998;


	for (l = 1; l <= LEVEL_OF_DECOMPOSITION; l++) 
	{
		if (n >= 4) 
		{
			 half = n >> 1;

			 i = 0;
			 for (j = 0; j < n; j += 2)		// traverse all coefficients
			 {
				// smoothing coefficients
				tmp_arr[i] = 0;
				for (k = 0; k < 18; k++)
				{
					if ((j + k) < n)
					{
						tmp_arr[i] += a[j + k] * h[k];	// weighted coefficients
					}
					else
					{
						tmp_arr[i] += a[j + k - n] * h[k];	// wrap around
					}
				}

				// detail coefficients
				tmp_arr[i + half] = 0;
				for (k = 0; k < 18; k++)
				{
					if ((j + k) < n)
					{
						tmp_arr[i + half] += a[j + k] * g[k];	// weighted coefficients
					}
					else
					{
						tmp_arr[i + half] += a[j + k - n] * g[k];	// wrap around
					}
				}
				// increment the counter for coefficient generation
				i++;
			 }

			 // now store the smooth and detail coefficients back to the original array
			 // for the next level of decomposition
			 for (i = 0; i < n; i++) 
			 {
				a[i] = tmp_arr[i];
			 }
		}

		n /= 2;	// half of prev no of elements

		// store the detail coefficient length, acccording to level
		len_coeff[LEVEL_OF_DECOMPOSITION - l + 1] = n;

	}	// end level wise decomposition loop 

	// store the approximate coeff len
	len_coeff[0] = n;
	
	// free temporary allocated memory
	free(tmp_arr);
}


# if 0

/*
	this is the wavelet decomposition algorithm 
	here Daubechies4 (Db4) algorithm is used
*/
void CEEG_marking_toolDlg::wtr_fwd_db4(double *a, int n, int *len_coeff)
{
	int i, j;
	int half;
	int l;
	double h0, h1, h2, h3, g0, g1, g2, g3;
	double *tmp_arr;

	// allocate memory
	tmp_arr = (double *) calloc(n, sizeof(double));

	// daubetchies 4 wavelet coefficients
	h0 = (1 + sqrt(3)) / (4 * sqrt(2));
	h1 = (3 + sqrt(3)) / (4 * sqrt(2));
	h2 = (3 - sqrt(3)) / (4 * sqrt(2));
	h3 = (1 - sqrt(3)) / (4 * sqrt(2));
	g0 = h3;
	g1 = (-1) * h2;
	g2 = h1;
	g3 = (-1) * h0;
		

	for (l = 1; l <= LEVEL_OF_DECOMPOSITION; l++) 
	{
		if (n >= 4) 
		{
			 half = n >> 1;

			 i = 0;
			 for (j = 0; j < n-3; j += 2) 
			 {
				tmp_arr[i]      = a[j] * h0 + a[j+1] * h1 + a[j+2] * h2 + a[j+3] * h3;
				tmp_arr[i + half] = a[j] * g0 + a[j+1] * g1 + a[j+2] * g2 + a[j+3] * g3;
				i++;
			 }

			 tmp_arr[i]      = a[n-2] * h0 + a[n-1] * h1 + a[0] * h2 + a[1] * h3;
			 tmp_arr[i + half] = a[n-2] * g0 + a[n-1] * g1 + a[0] * g2 + a[1] * g3;

			 for (i = 0; i < n; i++) 
			 {
				a[i] = tmp_arr[i];
			 }
		}

		n /= 2;	// half of prev no of elements

		// store the detail coefficient length, acccording to level
		len_coeff[LEVEL_OF_DECOMPOSITION - l + 1] = n;

	}	// end level wise decomposition loop 

	// store the approximate coeff len
	len_coeff[0] = n;
	
	// free temporary allocated memory
	free(tmp_arr);
}


# endif // #if 0

/*
	custom function to read the signal data from a signal segment
	the segment start and end duration are specified
*/
void CEEG_marking_toolDlg::Read_Signal_Segment(int ch_no, int start_index, int end_index, double* input_signal, int inp_sig_len, bool aEEG_data_read)
{
	struct channnel_data_list *read_temp;	// channel data - list pointer
	int start_node_offset, end_node_offset, start_node_num, end_node_num, node_count;	// channel data block traverse
	int i, j;
        
	// start offset of the current node from which data will be read
	start_node_offset = start_index % CHAN_DATA_BLOCK_SIZE;
	end_node_offset = end_index % CHAN_DATA_BLOCK_SIZE;
	start_node_num = start_index / CHAN_DATA_BLOCK_SIZE;
	end_node_num = end_index / CHAN_DATA_BLOCK_SIZE;

	// reading pointer should reach to the starting node
	// the pointer can point to the start of the aEEG data or the raw EEG data, depending upon the final boolean input
	if (aEEG_data_read == true)
	{
		read_temp = aeeg_head;
	}
	else
	{
		read_temp = head;	
	}

	for (i = 0; i < start_node_num; i++)
	{
		read_temp = read_temp->next;
	}		
		
	// for storing input data
	i = 0;
	for (node_count = start_node_num; node_count <= end_node_num; node_count++) 
	{
		if (node_count == end_node_num)		// this is the last node - may or may not be completely processed
		{
			for (j = start_node_offset; j <= end_node_offset; j++, i++)	//start node offset is 0 or set value
				input_signal[i] = read_temp->channel_data[ch_no][j];

			if (end_node_offset == (CHAN_DATA_BLOCK_SIZE - 1))	/// this node is completely processed
				read_temp = read_temp->next;					
		}
		else if (node_count == start_node_num)		// starting node but not end node
		{
			for (j = start_node_offset; j < CHAN_DATA_BLOCK_SIZE; j++, i++)
				input_signal[i] = read_temp->channel_data[ch_no][j];

			read_temp = read_temp->next;
		}
		else
		{
			for (j = start_node_offset; j < CHAN_DATA_BLOCK_SIZE; j++, i++)
				input_signal[i] = read_temp->channel_data[ch_no][j];

			read_temp = read_temp->next;
		}
		start_node_offset = 0;
	}

	// fill the remaining data portion with 0's
	for (; i < inp_sig_len; i++)
		input_signal[i] = 0;

}	// end of file segment read function



/*
	median finding code
*/

double CEEG_marking_toolDlg::median(double* x, int n)
{
	int i, j;
	double temp;
	for (i = 0; i < n-1; i++)
	{
		for (j = i+1; j < n; j++)
		{
			if (x[j] < x[i])
			{
				temp = x[j];
				x[j] = x[i];
				x[i] = temp;
			}
		}
	}
	if (n % 2 == 0)
		return ((x[n/2] + x[n/2 - 1]) / 2.0);
	else
		return (x[n/2]);
}

/*
	mean finding code
*/
double CEEG_marking_toolDlg::mean(double* x, int n)
{
	int i;
	double mean_val = 0;
	for (i = 0; i < n; i++)
	{	
		mean_val += x[i];
	}
	mean_val = (mean_val / n);
	return mean_val;
}


/*
	insertion sort algorithm
*/
void CEEG_marking_toolDlg::insert_sort(double* x, int n)
{
	int i, j;
	double key;
	for (j = 1; j < n; j++)
	{
		key = x[j];
		// insert x[j] in the sorted sequence x[0] to x[j-1]
		i = j - 1;
		while ((i >= 0) && (x[i] > key))
		{
			x[i+1] = x[i];
			i--;
		}
		x[i+1] = key;
	}
}	// end function

/*
	compute mean non linear energy for a signal segment
*/
double CEEG_marking_toolDlg::ComputeMNLE(double* input_signal, int inp_sig_len)
{
	int i;
	double mnle = 0;
	for (i = 1; i < (inp_sig_len - 1); i++)
	{
		mnle += (input_signal[i] * input_signal[i] - input_signal[i-1] * input_signal[i+1]);
	}
    //mnle /= inp_sig_len;	// mean of NLE
	return mnle;
}

/*
	compute mean absolute voltage for a signal segment
*/
double CEEG_marking_toolDlg::ComputeMeanAbsVolt(double* input_signal, int inp_sig_len)
{
	int i;
	double mean_abs_volt = 0;
	for (i = 0; i < inp_sig_len; i++)
	{
		mean_abs_volt += (double) fabs(input_signal[i]);	//absolute of double
	}
    mean_abs_volt /= inp_sig_len;	// mean of absolute voltage sum
	return mean_abs_volt;
}

/*
	compute the FFT and return the transform array
	for further spectrum related operations
*/
void CEEG_marking_toolDlg::DeriveFFT(double *input_signal, int nfft)
{
	//calculate the real FFT with FFT block length equal to nfft
	FFT_C(input_signal, nfft, 1);	
}

/*
	compute power spectral density for a signal segment
*/
double CEEG_marking_toolDlg::ComputePSD(double* input_signal, int nfft, double d)
{
	double total_psd = 0;
	int i;

	for (i = 0; i <= (nfft/2); i++)
	{
		if ((i == 0) || (i == (nfft/2)))
			total_psd += (pow(input_signal[2 * i], 2) + pow(input_signal[2 * i + 1], 2)) / d;
		else
			total_psd += (pow(input_signal[2 * i], 2) + pow(input_signal[2 * (nfft - i)], 2) + pow(input_signal[2 * i + 1], 2) + pow(input_signal[2 * (nfft - i) + 1], 2)) / d;
	}
	return total_psd;	
}

/*
	compute 3 Hz or 10 Hz power from power spectrum
	x is the input of the numeric power (Hz) value
*/
double CEEG_marking_toolDlg::Compute_X_Hz_power(double* input_signal, int nfft, double d, double x)
{
	double total_power = 0;
	double lower_limit_freq = x - 0.5;
	double upper_limit_freq = x + 0.5;
	int i;

	for (i = 0; i <= (nfft/2); i++)	
	{
		if ((i >= lower_limit_freq) && (i <= upper_limit_freq))
		{
			if ((i == 0) || (i == (nfft/2)))
				total_power += (pow(input_signal[2*i], 2) + pow(input_signal[2*i + 1], 2)) / d;
			else
				total_power += (pow(input_signal[2 * i], 2) + pow(input_signal[2 * (nfft - i)], 2) + pow(input_signal[2 * i + 1], 2) + pow(input_signal[2 * (nfft - i) + 1], 2)) / d;
		}
	}
	return total_power;
}

/*
	compute spectral edge frequency (SEF95) 
*/

double CEEG_marking_toolDlg::ComputeSEF95(double* input_signal, double sampling_rate, int nfft, double d, double total_psd)
{
	double cum_psd = 0;
	int i;
	double sef;

	for (i = 0; i <= (nfft/2); i++)	
	{
		if ((i == 0) || (i == (nfft/2)))
			cum_psd += (pow(input_signal[2*i], 2) + pow(input_signal[2*i + 1], 2)) / d;
		else
			cum_psd += (pow(input_signal[2 * i], 2) + pow(input_signal[2 * (nfft - i)], 2) + pow(input_signal[2 * i + 1], 2) + pow(input_signal[2 * (nfft - i) + 1], 2)) / d;

		if (cum_psd >= 0.95 * total_psd)		//95% of total power
		{
			sef = (double)(i * 1.0 * sampling_rate / nfft);
			break;	
		}
	}
	return sef;
}


/*
	compute the variance of the signal segment
*/
double CEEG_marking_toolDlg::ComputeVar(double* input_signal, int inp_sig_len)
{
	double sum1 = 0, var, sum2 = 0; 
    int i;
    
	for (i = 0; i < inp_sig_len; i++)
	{
        sum1 += input_signal[i];
    }
	sum1 = sum1 / inp_sig_len;

    for (i = 0; i < inp_sig_len; i++)
	{
        sum2 += (sum1 - input_signal[i]) * (sum1 - input_signal[i]);
    }
	var = (sum2 / (inp_sig_len - 1));		

	return var;
}



