// FilterEEGData.cpp : implementation file
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

// filtering function
void CEEG_marking_toolDlg::Filter_Amp_EEG_Data()
{
	// calculate kaiser window parameters
	int kaiserWindowLength;
	double beta;
	double ripple = 0.001;	//0.0001;		//0.001;	// ripple
	double transWidth = 0.5;	//0.1;	//0.5;		// pass band step
	double dw = 2 * M_PI * transWidth / sampling_rate;	// Calculate delta w
	double a = -20.0 * log10(ripple);	// Calculate ripple dB
	int m;				// Calculated filter order
	
	if (a > 21) 
	{
		m = (int)ceil((a - 7.95) / (2.285 * dw));
	}
	else 
	{
		m = (int)ceil(5.79 / dw);
	}
	kaiserWindowLength = m + 1;
	if (a <= 21) 
	{
		beta = 0.0;
	}
	else if (a <= 50)
	{
		beta = 0.5842 * pow(a - 21, 0.4) + 0.07886 * (a - 21);
	}
	else 
	{
		beta = 0.1102 * (a - 8.7);
	}

	// calculate sinc window
	double *bpf = (double *) malloc(kaiserWindowLength * sizeof(double));
	double ft1 = filter_low_freq_pass / sampling_rate;
	double ft2 = filter_high_freq_pass / sampling_rate;
	double m_2 = 0.5 * (kaiserWindowLength - 1);
	int halfLength = kaiserWindowLength / 2;
	int n;
	double val, val1, val2, denom;


	// Set centre tap, if present
	// This avoids a divide by zero
	if (2 * halfLength != kaiserWindowLength) 
	{
		val = 2.0 * (ft2 - ft1);
		bpf[halfLength] = val;
	}

	// Calculate taps
	// Due to symmetry, only need to calculate half the window
	for (n = 0; n < halfLength; n++) 
	{
		val1 = sin(2.0 * M_PI * ft1 * (n - m_2)) / (M_PI * (n - m_2));
		val2 = sin(2.0 * M_PI * ft2 * (n - m_2)) / (M_PI * (n - m_2));

		bpf[n] = val2 - val1;
		bpf[kaiserWindowLength - n - 1] = val2 - val1;
	}


	double *bpf_kaiser = (double *)malloc(kaiserWindowLength * sizeof(double));
	m_2 = 0.5 * (kaiserWindowLength - 1);
	denom = modZeroBessel(beta);					// Denominator of Kaiser function

	for (n = 0; n < kaiserWindowLength; n++)
	{
		val = ((n) - m_2) / m_2;
		val = 1 - (val * val);
		bpf_kaiser[n] = modZeroBessel(beta * sqrt(val)) / denom;
	}

	for (n = 0; n < kaiserWindowLength; n++) 
	{ 
		bpf_kaiser[n] *= bpf[n];
	}

	// now apply the kaiser window onto the input signal
	int nx = (int)sampling_rate * TIME_INTERVAL;		// TIME_INTERVAL sec data
	int ny = nx + kaiserWindowLength - 1;
	int filt_offset = (kaiserWindowLength / 2);	

	///////////////////////////
	int ch_no, time_elapsed;
	long start_index, end_index;
	int i, j;
	struct channnel_data_list *read_temp;	// temporary pointer to the new node 
	struct channnel_data_list *write_temp;	// temporary pointer to the new node 
	struct channnel_data_list *aeeg_write_temp;	// temporary pointer to the new node for amp eeg data
	int start_node_num, end_node_num;
	int node_count;
	int start_node_offset, end_node_offset;

	int L;
	L = ComputeNFFT(nx, kaiserWindowLength); 

	// initialize for filtering (fftfilt)
	
	//output data store - of length ny = nx + kaiserWindowLength - 1
	double* y = (double *) calloc(ny, sizeof(double));	
	
	// input data store - it is of size nx 
	double* x = (double *) calloc(nx, sizeof(double));	
	
	// data storing the product of filter and the input data (freq domain multipication)
	// it is of size 2*NFFT because even indices store real data and odd indices store imaginary data
	double* y_mult = (double *) calloc((2 * NFFT), sizeof(double));	

	// data store for storing the FFT results of the input data
	// it is of size 2*NFFT because even indices store real data and odd indices store imaginary data
	double* x_fft = (double *) calloc((2 * NFFT), sizeof(double));	

	// data store for storing the FFT results of the filtered data
	// it is of size 2*NFFT because even indices store real data and odd indices store imaginary data
	double* filter_fft = (double *) calloc((2 * NFFT), sizeof(double));	
	
	// store the filter coefficients in the filter array
	// even indices will store the real data
	// odd indices will store zero
	for (i = 0; i < kaiserWindowLength; i++)
	{
		filter_fft[2 * i] = bpf_kaiser[i];
		filter_fft[2 * i + 1] = 0;
	}
	for (i = kaiserWindowLength; i < NFFT; i++)
	{
		filter_fft[2 * i] = 0;
		filter_fft[2 * i + 1] = 0;
	}

	// perform the FFT of filter
	FFT_C(filter_fft, NFFT, 1);	

	///////////////////////////

	// calculate filtered data
	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		// for each channel, point the temporary pointer to the head of the channel data
		read_temp = head;
		write_temp = head;
		aeeg_write_temp = aeeg_head;

		// we traverse input data by 1 sec intervals
		for (time_elapsed = 0; time_elapsed < total_recording_duration_sec; (time_elapsed = time_elapsed + TIME_INTERVAL))
		{
			start_index = (int)(time_elapsed * sampling_rate);
			end_index = (int)((time_elapsed + TIME_INTERVAL) * sampling_rate - 1);
			if (end_index >= channel_data_size)
			{
				end_index = channel_data_size - 1;
			}
			nx = end_index - start_index + 1;
			ny = nx + kaiserWindowLength - 1;

			// start offset of the current node from which data will be read
			start_node_offset = start_index % CHAN_DATA_BLOCK_SIZE;
			end_node_offset = end_index % CHAN_DATA_BLOCK_SIZE;
			start_node_num = start_index / CHAN_DATA_BLOCK_SIZE;
			end_node_num = end_index / CHAN_DATA_BLOCK_SIZE;

			// for storing input data
			i = 0;
			for (node_count = start_node_num; node_count <= end_node_num; node_count++) 
			{
				if (node_count == end_node_num)		// this is the last node - may or may not be completely processed
				{
					for (j = start_node_offset; j <= end_node_offset; j++, i++)	//start node offset is 0 or set value
						x[i] = read_temp->channel_data[ch_no][j];

					if (end_node_offset == (CHAN_DATA_BLOCK_SIZE - 1))	/// this node is completely processed
						read_temp = read_temp->next;					
				}
				else if (node_count == start_node_num)		// starting node but not end node
				{
					for (j = start_node_offset; j < CHAN_DATA_BLOCK_SIZE; j++, i++)
						x[i] = read_temp->channel_data[ch_no][j];

					read_temp = read_temp->next;
				}
				else
				{
					for (j = start_node_offset; j < CHAN_DATA_BLOCK_SIZE; j++, i++)
						x[i] = read_temp->channel_data[ch_no][j];

					read_temp = read_temp->next;
				}
				start_node_offset = 0;
			}

			// reset output data
			for (i = 0; i < ny; i++)
				y[i] = 0;

			//////////////////////////
			// fftfilt function implemented in C
			fftfilt_C_impl(filter_fft, x, y, nx, ny, kaiserWindowLength, x_fft, y_mult, L);		
			//////////////////////////

			// now filtered data will be placed back to the storage space allocated for filtered data
			start_node_offset = start_index % CHAN_DATA_BLOCK_SIZE;

			i = 0;
			for (node_count = start_node_num; node_count <= end_node_num; node_count++) 
			{
				if (node_count == end_node_num)		// this is the last node - may or may not be completely processed
				{
					for (j = start_node_offset; j <= end_node_offset; j++, i++)	//start node offset is 0 or set value
					{
						// raw EEG
						// filtered data is scaled by 4 - check the MATLAB implementation
						write_temp->channel_data[ch_no][j] = y[i + filt_offset] * FILTER_DATA_MULTIPLYING_FACTOR;		// filtered data is scaled by 4 - check the MATLAB implementation

						// amplitude EEG - we do the full wave rectification as well
						aeeg_write_temp->channel_data[ch_no][j] = fabs(y[i + filt_offset]);	// / 3.0;	// * 0.25;
					}

					if (end_node_offset == (CHAN_DATA_BLOCK_SIZE - 1))	/// this node is completely processed
					{
						write_temp = write_temp->next;	
						aeeg_write_temp = aeeg_write_temp->next;					
					}
				}
				else if (node_count == start_node_num)		// starting node but not end node
				{
					for (j = start_node_offset; j < CHAN_DATA_BLOCK_SIZE; j++, i++)
					{
						// filtered data is scaled by 4 - check the MATLAB implementation
						// raw EEG
						write_temp->channel_data[ch_no][j] = y[i + filt_offset] * FILTER_DATA_MULTIPLYING_FACTOR;		// filtered data is scaled by 4 - check the MATLAB implementation

						// amplitude EEG - we do the full wave rectification as well
						aeeg_write_temp->channel_data[ch_no][j] = fabs(y[i + filt_offset]);	// / 3.0;	// * 0.25;
					}
					write_temp = write_temp->next;
					aeeg_write_temp = aeeg_write_temp->next;
				}
				else
				{
					for (j = start_node_offset; j < CHAN_DATA_BLOCK_SIZE; j++, i++)
					{
						// filtered data is scaled by 4 - check the MATLAB implementation
						// raw EEG
						write_temp->channel_data[ch_no][j] = y[i + filt_offset] * FILTER_DATA_MULTIPLYING_FACTOR;		// filtered data is scaled by 4 - check the MATLAB implementation
						
						// amplitude EEG - we do the full wave rectification as well
						aeeg_write_temp->channel_data[ch_no][j] = fabs(y[i + filt_offset]);	// / 3.0;	// * 0.25;
					}
					write_temp = write_temp->next;
					aeeg_write_temp = aeeg_write_temp->next;
				}
				start_node_offset = 0;
			}
			// end of filtered data store

		}	// end time elapsed loop	
	} // end all channel filtering loop

	// free the memory
	free(bpf);
	free(bpf_kaiser);
	free(y);
	free(x);
	free(y_mult);
	free(x_fft);
	free(filter_fft);
} //end function


//************************************************

double CEEG_marking_toolDlg::modZeroBessel(double x)
{
	int i;

	double x_2 = x/2;
	double num = 1;
	double fact = 1;
	double result = 1;

	for (i=1 ; i<20 ; i++) {
		num *= x_2 * x_2;
		fact *= i;
		result += num / (fact * fact);
	}
	return result;
}

//************************************************
/*
	fftfilt function in MATLAB is implemented in C
*/	

void CEEG_marking_toolDlg::fftfilt_C_impl(double* filter_fft, double* inp_sig, double* out_sig, int in_sig_len, int out_sig_len, int filt_sig_len, double* x_fft, double* y_mult, int L)
{
	//int L = NFFT - filt_sig_len + 1;	// fft level
	int istart, iend, yend;
	int i;

	istart = 0;
	while (istart < in_sig_len)
	{		
		iend = ((istart + L - 1) >= in_sig_len) ? (in_sig_len - 1) : (istart + L - 1);	// min operator

		// construct the input data array and perform its FFT for freq domain multiplication
		if (iend == istart)
		{
			// fill the x_real and x_im portions - NFFT no of 1's
			for (i = 0; i < NFFT; i++)
			{
				x_fft[2 * i] = 1; 
				x_fft[2 * i + 1] = 0;
			}
		}
		else
		{
			for (i = istart; i <= iend; i++)
			{
				x_fft[2 * (i - istart)] = inp_sig[i]; 
				x_fft[2 * (i - istart) + 1] = 0;
			}
			for (i = iend + 1; i <= (istart + NFFT - 1); i++)
			{
				x_fft[2 * (i - istart)] = 0; 
				x_fft[2 * (i - istart) + 1] = 0;
			}
			FFT_C(x_fft, NFFT, 1);
		}

		// multiplication of two real and imaginary component arrays and storing it into 3rd array
		// we have freq domain conversions of both input data part and the filter part
		for (i = 0; i < NFFT; i++)
		{
			y_mult[2 * i] = x_fft[2 * i] * filter_fft[2 * i] - x_fft[2 * i + 1] * filter_fft[2 * i + 1];
			y_mult[2 * i + 1] = x_fft[2 * i] * filter_fft[2 * i + 1] + x_fft[2 * i + 1] * filter_fft[2 * i];
		}

		// inverse FFT operation on the freq domain product sum
		FFT_C(y_mult, NFFT, -1);

		// copy the real part of the inverse FFT output to the output Y array	
		//yend = ((istart + NFFT - 1) >= in_sig_len) ? (in_sig_len - 1) : (istart + NFFT - 1);	// min operator
		yend = ((istart + NFFT - 1) >= out_sig_len) ? (out_sig_len - 1) : (istart + NFFT - 1);	// min operator
		
		for (i = istart; i <= yend; i++)
		{
			// taking only real part
			out_sig[i] += y_mult[2 * (i - istart)];		

			// considering both real and imaginary parts - however, the sign is always positive
			// out_sig[i] += sqrt(pow(y_mult[2 * (i - istart)], 2) + pow(y_mult[2 * (i - istart) + 1], 2));	
		}

		istart = istart + L;
	}
}	// end of fftfilt function

//************************************************

/* 
	FFT implementation in C code 

  // data -> double array that represent the array of complex samples
  // number_of_complex_samples -> number of samples (2^N order number) 
  // isign -> 1 to calculate FFT and -1 to calculate Reverse FFT
*/

void CEEG_marking_toolDlg::FFT_C(double *data, int number_of_complex_samples, int isign)	
{
	unsigned long n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta, tempr, tempi;

    // the complex array is real+complex so the array as a size n = 2* number of complex samples
    // real part is the data[index] and the complex part is the data[index+1]
	n = number_of_complex_samples * 2; 

    // binary inversion (note that the indexes start from 0 witch means that the real part of the 
	// complex is on the even-indexes and the complex part is on the odd-indexes
    j=0;
    for (i=0; i < (n/2); i+=2) 
	{
        if (j > i) 
		{
            //swap the real part
            SWAP(data[j], data[i]);

            //swap the complex part
            SWAP(data[j+1], data[i+1]);

            // checks if the changes occurs in the first half and use the mirrored effect on the second half
            if((j/2)<(n/4))
			{
                //swap the real part
                SWAP(data[(n-(i+2))], data[(n-(j+2))]);

                //swap the complex part
                SWAP(data[(n-(i+2))+1], data[(n-(j+2))+1]);
            }
        }
        m=n/2;
        while (m >= 2 && j >= m) 
		{
            j -= m;
            m = m/2;
        }
        j += m;
    }


    //Danielson-Lanzcos routine 
    mmax = 2;

    //external loop
    while (n > mmax)
    {
        istep = mmax << 1;
        theta = isign * (2 * M_PI / (mmax * 1.0));
        wtemp = sin(0.5 * theta);
        wpr = -2.0 * wtemp * wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;

        //internal loops
        for (m = 1; m < mmax; m += 2) 
		{
            for (i = m; i <= n; i += istep) 
			{
                j = i + mmax;
                tempr = wr * data[j-1] - wi * data[j];
                tempi = wr * data[j] + wi * data[j-1];
                data[j-1] = data[i-1] - tempr;
                data[j] = data[i] - tempi;
                data[i-1] += tempr;
                data[i] += tempi;
            }
            wr = (wtemp = wr) * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }
        mmax = istep;
    }

	// add - sourya

	// Scaling for forward transform 
	if (isign == -1) 
	{
      for (i = 0; i < number_of_complex_samples /* n */; i++) 
	  {
         data[2 * i] /= number_of_complex_samples /* n */;
         data[2 * i + 1] /= number_of_complex_samples /* n */;
      }
	}

	// end add - sourya

}	// end of FFT routine

//************************************************

int CEEG_marking_toolDlg::ComputeNFFT(int inp_sig_len, int filt_sig_len)
{
	int L;

	if (filt_sig_len >= inp_sig_len)     // take a single FFT in this case
	{
        NFFT = (int)(pow(2, (int)ceil(log(inp_sig_len + filt_sig_len - 1) / log(2))));
        L = inp_sig_len;
	}
	else
	{
		double fftflops[20] = {18, 59, 138, 303, 660, 1441, 3150, 6875, 14952, 32373, 69762, 149647, 
								319644, 680105, 1441974, 3047619, 6422736, 13500637, 28311786, 59244791};

		double n[20];
		int i, validset;
		double min_val, temp_val;
		int min_index;

		for (i = 0; i < 20; i++)
			n[i] = pow(2, (i+1));

		for (i = 0; i < 20; i++)
		{
			if (n[i] > (filt_sig_len - 1))
			{
				validset = i;
				break;
			}
		}

		min_val = (ceil(inp_sig_len / (n[validset] - filt_sig_len + 1))) * fftflops[validset];
		min_index = validset;

		for (i = (validset + 1); i < 20; i++)
		{
			temp_val = (ceil(inp_sig_len / (n[i] - filt_sig_len + 1))) * fftflops[i];
			if (temp_val < min_val)
			{
				min_val = temp_val;
				min_index = i;
			}
		}

		NFFT = (int)n[min_index];
		L = (int)n[min_index] - filt_sig_len + 1;
	}

	return L;
}
