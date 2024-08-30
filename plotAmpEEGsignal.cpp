// plotAmpEEGsignal.cpp : implementation file
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
	plotting AMP EEG signal data
*/
void CEEG_marking_toolDlg::plot_amp_EEG_signal(void)
{

	//CDC *pDC_Amp = m_Grid_Amp.GetDC();
	struct channnel_data_list *temp1;
	struct channnel_data_list *temp2;
	int x_start_index, x_end_index, y_start_index, y_end_index;
	int node_count, start_node_offset, end_node_offset, start_node_num, end_node_num;
	int i, j, n;
	double x_axis_res_graph, y_axis_res_graph, origin_pos_per_channel, t;
	CRect rect_temp_amp = new CRect;
	int time_label_count, curr_time_label, time_display_round, reminder_time;
	CString time_label;

	// plot amplitude EEG signal
	if (aeeg_computation_done == 0)
	{
		// Dump Main DC contents to your DC if required
		dcMem.BitBlt(0, 0, rect_Amp.Width(), rect_Amp.Height(), pDC_Amp, rect_Amp.left, rect_Amp.top, SRCCOPY);
		pOldBitmap = dcMem.SelectObject(&bitmap);
		
		SelectObject(dcMem.m_hDC, &pen_Amp);
		FillRect(dcMem.m_hDC, &rect_Amp, drawBrush_Amp);

		// plot x and y
		x_start_index = 0;
		x_end_index = (int)((sel_aeeg_end_time - sel_aeeg_start_time) * sampling_rate) - 1;
		y_start_index = (int)(sel_aeeg_start_time * sampling_rate);
		y_end_index = y_start_index + (x_end_index - x_start_index);
		
		x_axis_res_graph = (aeeg_per_page_duration * sampling_rate) / (screen_x_size_Amp - SCREEN_X_AMP_OFFSET);

		// as in semi logarithmic scale, 1-10 is linear, 10-100 is non linear
		// so the computed original value from 0-100 maps in 0-20
		// for seperation, we take 20.5 as axis length for 1 channel
		y_axis_res_graph = ((20.5 * no_of_channels) / (screen_y_size_Amp - SCREEN_Y_AMP_OFFSET));	// per pixel y		

		// determine the displayed section in terms of node boundary
		start_node_offset = y_start_index % CHAN_DATA_BLOCK_SIZE;
		end_node_offset = y_end_index % CHAN_DATA_BLOCK_SIZE;
		start_node_num = y_start_index / CHAN_DATA_BLOCK_SIZE;
		end_node_num = y_end_index / CHAN_DATA_BLOCK_SIZE;

		// temporary ponter will point to the start of EEG data list
		temp1 = aeeg_head;	//initialization

		// temp1 should reach to the starting node
		for (i = 0; i < start_node_num; i++)
		{
			temp1 = temp1->next;
		}

		////////////////////////////////////////////
		// there is a sign reversal due to VC++ customization
		for (n = 0; n < no_of_channels; n++)
		{
			// set channel origin for graph display
			origin_pos_per_channel = ((n + 1) * 1.0 * (screen_y_size_Amp - SCREEN_Y_AMP_OFFSET)) / no_of_channels;	

			// initialize the node pointer with the staring node
			temp2 = temp1;	//plot individual data

			// start node offset for current channel
			// effective for start node num
			start_node_offset = y_start_index % CHAN_DATA_BLOCK_SIZE;

			i = 0;
			for (node_count = start_node_num; node_count <= end_node_num; node_count++) 
			{
				if (node_count == end_node_num)		// this is the last node - may or may not be completely processed
				{
					for (j = start_node_offset; j <= (end_node_offset - 1); j++, i++)	//start node offset is 0 or set value			
					{
						::MoveToEx(dcMem.m_hDC, (int)((i - x_start_index) / x_axis_res_graph), (int)((SemilogAdapt(temp2->channel_data[n][j]) * (-1.0) / y_axis_res_graph) + origin_pos_per_channel), NULL);		
						LineTo(dcMem.m_hDC, (int)((i + 1 - x_start_index) / x_axis_res_graph), (int)((SemilogAdapt(temp2->channel_data[n][j+1]) * (-1.0) / y_axis_res_graph) + origin_pos_per_channel));
					}
					if (end_node_offset == (CHAN_DATA_BLOCK_SIZE - 1))	/// this node is completely processed
						temp2 = temp2->next;					
				}
				else
				{
					for (j = start_node_offset; j < (CHAN_DATA_BLOCK_SIZE - 1); j++, i++)
					{
						::MoveToEx(dcMem.m_hDC, (int)((i - x_start_index) / x_axis_res_graph), (int)((SemilogAdapt(temp2->channel_data[n][j]) * (-1.0) / y_axis_res_graph) + origin_pos_per_channel), NULL);		
						LineTo(dcMem.m_hDC, (int)((i + 1 - x_start_index) / x_axis_res_graph), (int)((SemilogAdapt(temp2->channel_data[n][j+1]) * (-1.0) / y_axis_res_graph) + origin_pos_per_channel));
					}					
					::MoveToEx(dcMem.m_hDC, (int)((i - x_start_index) / x_axis_res_graph), (int)((SemilogAdapt(temp2->channel_data[n][j]) * (-1.0) / y_axis_res_graph) + origin_pos_per_channel), NULL);		
					temp2 = temp2->next;
					LineTo(dcMem.m_hDC, (int)((i + 1 - x_start_index) / x_axis_res_graph), (int)((SemilogAdapt(temp2->channel_data[n][0]) * (-1.0) / y_axis_res_graph) + origin_pos_per_channel));
					i++;
				}
				start_node_offset = 0;
			}

			// plot the grid lines signifying the semilogarithmic scale
			for (i = 0; i <= 5; i++)
			{
				::MoveToEx(dcMem.m_hDC, 0, (int)(origin_pos_per_channel - (i * 1.0 / y_axis_res_graph)), NULL);		
				LineTo(dcMem.m_hDC, (int)(screen_x_size_Amp - SCREEN_X_AMP_OFFSET), (int)(origin_pos_per_channel - (i * 1.0 / y_axis_res_graph)));
			}
			i = 10;
			::MoveToEx(dcMem.m_hDC, 0, (int)(origin_pos_per_channel - (i * 1.0 / y_axis_res_graph)), NULL);		
			LineTo(dcMem.m_hDC, (int)(screen_x_size_Amp - SCREEN_X_AMP_OFFSET), (int)(origin_pos_per_channel - (i * 1.0 / y_axis_res_graph)));

			t = 13.9794;
			::MoveToEx(dcMem.m_hDC, 0, (int)(origin_pos_per_channel - (t * 1.0 / y_axis_res_graph)), NULL);		
			LineTo(dcMem.m_hDC, (int)(screen_x_size_Amp - SCREEN_X_AMP_OFFSET), (int)(origin_pos_per_channel - (t * 1.0 / y_axis_res_graph)));
			
			t = 16.9897;
			::MoveToEx(dcMem.m_hDC, 0, (int)(origin_pos_per_channel - (t * 1.0 / y_axis_res_graph)), NULL);		
			LineTo(dcMem.m_hDC, (int)(screen_x_size_Amp - SCREEN_X_AMP_OFFSET), (int)(origin_pos_per_channel - (t * 1.0 / y_axis_res_graph)));

			i = 20;
			::MoveToEx(dcMem.m_hDC, 0, (int)(origin_pos_per_channel - (i * 1.0 / y_axis_res_graph)), NULL);		
			LineTo(dcMem.m_hDC, (int)(screen_x_size_Amp - SCREEN_X_AMP_OFFSET), (int)(origin_pos_per_channel - (i * 1.0 / y_axis_res_graph)));
			// end plot grid lines

			// label to display the channels
			rect_temp_amp.left = (screen_x_size_Amp - SCREEN_X_AMP_OFFSET + 1);
			rect_temp_amp.top = (int)((n + 0.2) * (screen_y_size_Amp - SCREEN_Y_AMP_OFFSET) / no_of_channels);
			rect_temp_amp.right = screen_x_size_Amp;
			rect_temp_amp.bottom = (int)((n + 0.8) * (screen_y_size_Amp - SCREEN_Y_AMP_OFFSET) / no_of_channels);
			DrawText(dcMem.m_hDC, channel_name[n], strlen(channel_name[n]), rect_temp_amp, DT_CENTER);		

		}	// end of channel loop

		///////////////////////////////////
		// label to display the time 
		// we have to display the rounded time (say after 30 min or 2 hr etc)
		time_label_count = 0;
		curr_time_label = 0;	// determines the current time whose label is given

		// if current aEEG display is on full scale
		// or if the time covers whe whole display boundary
		// then we insert five time label information
		if ((sel_aeeg_end_time - sel_aeeg_start_time) == aeeg_per_page_duration)
		{
			time_label_count = 5;
		}
		else
		{
			time_label_count = (int)(5 * ((sel_aeeg_end_time - sel_aeeg_start_time) / aeeg_per_page_duration) + 1);
		}

		// determination of the round time
		// if for instance, the time covered in current aeeg display is greater than 10 hrs then 
		// the time display will be rounded by 1 hr 
		// if it is less than 10 but greater than 3 hr then time display will be rounded by 30 min
		// otherwise the time display will be rounded by 10 min
		if ((sel_aeeg_end_time - sel_aeeg_start_time) >= 36000)		// 10 hr
			time_display_round = 3600;	// 1 hr
		else if (((sel_aeeg_end_time - sel_aeeg_start_time) >= 10800) && ((sel_aeeg_end_time - sel_aeeg_start_time) < 36000))		// 3 to 10 hr
			time_display_round = 1800;	// 30 min		
		else	// less than 3 hr
			time_display_round = 600;	// 10 min

		// now formulate the label timings
		for (i = 1; ((i <= time_label_count) && (curr_time_label <= sel_aeeg_end_time)); i++)
		{
			if (i == 1)
				curr_time_label = (int)sel_aeeg_start_time;
			else if (i == 5)
				curr_time_label = (int)sel_aeeg_end_time;
			else
			{
				curr_time_label = (int)(sel_aeeg_start_time + ((sel_aeeg_end_time - sel_aeeg_start_time) / time_label_count) * i);
				reminder_time = (curr_time_label % time_display_round);
				if ((reminder_time < (time_display_round / 2)) && ((curr_time_label - reminder_time) > sel_aeeg_start_time))
					curr_time_label = (curr_time_label - reminder_time);
				else if ((reminder_time > (time_display_round / 2)) && ((curr_time_label + time_display_round - reminder_time) < sel_aeeg_end_time))
					curr_time_label = (curr_time_label + time_display_round - reminder_time);
			}

			// now generate the time labels
			rect_temp_amp.left = (int)(((curr_time_label - sel_aeeg_start_time) * (screen_x_size_Amp - SCREEN_X_AMP_OFFSET)) / aeeg_per_page_duration);	
			rect_temp_amp.top = (int)(screen_y_size_Amp - SCREEN_Y_AMP_OFFSET + 1);
			rect_temp_amp.right = rect_temp_amp.left + SCREEN_X_AMP_OFFSET;
			rect_temp_amp.bottom = screen_y_size_Amp;
			time_label = convert_time_to_str(curr_time_label);
			DrawText(dcMem.m_hDC, time_label, strlen(time_label), rect_temp_amp, DT_CENTER);		
		}

		///////////////////////////////////

		aeeg_computation_done = 1;	// set to avoid further computation
	}	// end compute aEEG

    // put the drawn stuff back to Main DC
	pDC_Amp->BitBlt(rect_Amp.left, rect_Amp.top, rect_Amp.Width(), rect_Amp.Height(), &dcMem, 0, 0, SRCCOPY);
	//dcMem.SelectObject(pOldBitmap);
	pDC_Amp->SelectObject(pOldBitmap);

}
