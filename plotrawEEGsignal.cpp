// plotrawEEGsignal.cpp : implementation file
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
	plotting raw EEG signal data
*/
void CEEG_marking_toolDlg::plot_raw_EEG_signal(int repaint_on)
{
	CPen *oldPen;
	CDC *pDC = m_Grid.GetDC();
	int startx, endx, starty, endy;
	CRect rect_temp = new CRect;

	SelectObject(hDC, &pen);
  FillRect(hDC, &rect, drawBrush);

  // plot x and y
  int x_start_index = 0;
  int x_end_index = (int)((sel_raw_eeg_end_time - sel_raw_eeg_start_time) * sampling_rate) - 1;
  int y_start_index = (int)(sel_raw_eeg_start_time * sampling_rate);
  int y_end_index = y_start_index + (x_end_index - x_start_index);

	int node_count;
	int start_node_offset, end_node_offset, start_node_num, end_node_num;

	struct channnel_data_list *temp1;
	struct channnel_data_list *temp2;

	int i, j, n;
	double x_axis_res_graph = ((x_end_index - x_start_index + 1) * 1.0) / (screen_x_size - SCREEN_X_OFFSET);

	// this modification with respect to sensitivity is done because the voltage is 2.5 times higher
	// increasing the resolution will help in correct display
	// check the MATLAB implementation
	double y_axis_res_graph = (sensitivity * RAW_EEG_DISPLAY_MULTIPLYING_FACTOR * 1.0) / PixelsPerMM;				

	double origin_pos_per_channel;

	bool mark_found_within_curr_window;
	struct data_mark_format* temp_data_mark;	// pointer to new allocated node

	CString time_label;

	start_node_offset = y_start_index % CHAN_DATA_BLOCK_SIZE;
	end_node_offset = y_end_index % CHAN_DATA_BLOCK_SIZE;
	start_node_num = y_start_index / CHAN_DATA_BLOCK_SIZE;
	end_node_num = y_end_index / CHAN_DATA_BLOCK_SIZE;

	// temporary ponter will point to the start of EEG data list
	temp1 = head;	//initialization

	// temp1 should reach to the starting node
	for (i = 0; i < start_node_num; i++)
	{
		temp1 = temp1->next;
	}

	//////////////////////////////
	// plot the grid lines	
	for (n = 0; n <= raw_eeg_per_page_duration; n++)
	{
		//move to x1, y1
		::MoveToEx(hDC, (int)((n * (screen_x_size - SCREEN_X_OFFSET) * 1.0) / raw_eeg_per_page_duration), 0, NULL);		

		//line to x2, y2
		LineTo(hDC, (int)((n * (screen_x_size - SCREEN_X_OFFSET) * 1.0 ) / raw_eeg_per_page_duration), (screen_y_size - SCREEN_Y_OFFSET));
	}
	//////////////////////////////
	// plot the graph
	// there is a sign reversal due to VC++ customization
	for (n = 0; n < no_of_channels; n++)
	{
		start_node_offset = y_start_index % CHAN_DATA_BLOCK_SIZE;

		// set channel origin for graph display
		origin_pos_per_channel = ((n + 0.5) * ((screen_y_size - SCREEN_Y_OFFSET) * 1.0) / no_of_channels);	

		// initialize the node pointer with the staring node
		temp2 = temp1;	//plot individual data

		i = 0;
		for (node_count = start_node_num; node_count <= end_node_num; node_count++) 
		{
			if (node_count == end_node_num)		// this is the last node - may or may not be completely processed
			{
				for (j = start_node_offset; j <= (end_node_offset - 1); j++, i++)	//start node offset is 0 or set value			
				{
					::MoveToEx(hDC, (int)((i - x_start_index) / x_axis_res_graph), (int)((temp2->channel_data[n][j] / y_axis_res_graph) /* * (-1.0) */ + origin_pos_per_channel), NULL);		
					LineTo(hDC, (int)((i + 1 - x_start_index) / x_axis_res_graph), (int)((temp2->channel_data[n][j+1] / y_axis_res_graph) /* * (-1.0) */ + origin_pos_per_channel));
				}
				if (end_node_offset == (CHAN_DATA_BLOCK_SIZE - 1))	/// this node is completely processed
					temp2 = temp2->next;					
			}
			else if (node_count == start_node_num)		// starting node but not end node
			{
				for (j = start_node_offset; j < (CHAN_DATA_BLOCK_SIZE - 1); j++, i++)
				{
					::MoveToEx(hDC, (int)((i - x_start_index) / x_axis_res_graph), (int)((temp2->channel_data[n][j] / y_axis_res_graph) /* * (-1.0) */ + origin_pos_per_channel), NULL);		
					LineTo(hDC, (int)((i + 1 - x_start_index) / x_axis_res_graph), (int)((temp2->channel_data[n][j+1] / y_axis_res_graph) /* * (-1.0) */ + origin_pos_per_channel));
				}					
				::MoveToEx(hDC, (int)((i - x_start_index) / x_axis_res_graph), (int)((temp2->channel_data[n][j] / y_axis_res_graph) /* * (-1.0) */ + origin_pos_per_channel), NULL);		
				temp2 = temp2->next;
				LineTo(hDC, (int)((i + 1 - x_start_index) / x_axis_res_graph), (int)((temp2->channel_data[n][0] / y_axis_res_graph) /* * (-1.0) */ + origin_pos_per_channel));
				i++;
			}
			else
			{
				for (j = start_node_offset; j < (CHAN_DATA_BLOCK_SIZE - 1); j++, i++)
				{
					::MoveToEx(hDC, (int)((i - x_start_index) / x_axis_res_graph), (int)((temp2->channel_data[n][j] / y_axis_res_graph) /* * (-1.0) */ + origin_pos_per_channel), NULL);		
					LineTo(hDC, (int)((i + 1 - x_start_index) / x_axis_res_graph), (int)((temp2->channel_data[n][j+1] / y_axis_res_graph) /* * (-1.0) */ + origin_pos_per_channel));
				}					
				::MoveToEx(hDC, (int)((i - x_start_index) / x_axis_res_graph), (int)((temp2->channel_data[n][j] / y_axis_res_graph) /* * (-1.0) */ + origin_pos_per_channel), NULL);		
				temp2 = temp2->next;
				LineTo(hDC, (int)((i + 1 - x_start_index) / x_axis_res_graph), (int)((temp2->channel_data[n][0] / y_axis_res_graph) /* * (-1.0) */ + origin_pos_per_channel));
				i++;
			}
			start_node_offset = 0;
		}

		// label to display the channels
		rect_temp.left = (screen_x_size - SCREEN_X_OFFSET + 1);
		rect_temp.top = (int)((n + 0.2) * ((screen_y_size - SCREEN_Y_OFFSET) * 1.0) / no_of_channels);
		rect_temp.right = screen_x_size;
		rect_temp.bottom = (int)((n + 0.8) * ((screen_y_size - SCREEN_Y_OFFSET) * 1.0) / no_of_channels);
		DrawText(hDC, channel_name[n], strlen(channel_name[n]), rect_temp, DT_CENTER);		
	}

	// insert labels of timing information
	for (i = 1; i <= 3; i++)	// i serves as a label counter
	{
		// n is the time offset from the start time (w.r,t current window)
		if (i == 1)
			n = 0;
		else if (i == 2)
			n = raw_eeg_per_page_duration / 2;
		else
			n = raw_eeg_per_page_duration;

		rect_temp.left = (int)((n * (screen_x_size - SCREEN_X_OFFSET) * 1.0) / raw_eeg_per_page_duration);	
		rect_temp.top = (screen_y_size - SCREEN_Y_OFFSET + 1);
		rect_temp.right = rect_temp.left + SCREEN_X_OFFSET;
		rect_temp.bottom = screen_y_size;
		time_label = convert_time_to_str((int)sel_raw_eeg_start_time + n);
		DrawText(hDC, time_label, strlen(time_label), rect_temp, DT_CENTER);		
	}
	//////////////////////////////
	// mark the marked portion
	if (marking_database != NULL)
	{
	for (n = 0; n < no_of_channels; n++)
	{
			if (marking_database[n] != NULL)
			{
		    // initialize the node pointer
		    temp_data_mark = marking_database[n];

		    for (i = 0; i < mark_database_count[n]; i++)
		    {
			    mark_found_within_curr_window = 0;	// reset the boolean variable

					// skip the mark where start time is invalid
					// or if there is a suppression mark
					if ((temp_data_mark->mark_start_time == -1) || 
						(temp_data_mark->mark_event == SUPPRESSION_MARK))
					{
						temp_data_mark = temp_data_mark->next;
						continue;
					}

			    /*
            4 cases
            case 1 - one mark exists completely within current window
            case 2 - one mark was started before current window but
            it ends within current window
            case 3 - one mark starts in current window but ends in next window   
            case 4 - one mark was started before current window and
            will end after current window
			    */

			    if ((temp_data_mark->mark_start_time >= sel_raw_eeg_start_time) && (temp_data_mark->mark_end_time <= sel_raw_eeg_end_time))	// case 1
			    {
				    mark_found_within_curr_window = 1;
						startx = (int)(((temp_data_mark->mark_start_time - sel_raw_eeg_start_time) * (screen_x_size - SCREEN_X_OFFSET)) / raw_eeg_per_page_duration);
						endx = (int)(((temp_data_mark->mark_end_time - sel_raw_eeg_start_time) * (screen_x_size - SCREEN_X_OFFSET)) / raw_eeg_per_page_duration);
    			}
			    else if ((temp_data_mark->mark_start_time < sel_raw_eeg_start_time) && (temp_data_mark->mark_end_time > sel_raw_eeg_start_time) && (temp_data_mark->mark_end_time <= sel_raw_eeg_end_time))   // case 2
			    {
				    mark_found_within_curr_window = 1;
				    startx = 0;
						endx = (int)(((temp_data_mark->mark_end_time - sel_raw_eeg_start_time) * (screen_x_size - SCREEN_X_OFFSET)) / raw_eeg_per_page_duration);
			    }
			    else if ((temp_data_mark->mark_start_time >= sel_raw_eeg_start_time) && (temp_data_mark->mark_start_time < sel_raw_eeg_end_time) && (temp_data_mark->mark_end_time > sel_raw_eeg_end_time))   // case 3
			    {
				    mark_found_within_curr_window = 1;
						startx = (int)(((temp_data_mark->mark_start_time - sel_raw_eeg_start_time) * (screen_x_size - SCREEN_X_OFFSET)) / raw_eeg_per_page_duration);
						endx = (int)(((sel_raw_eeg_end_time - sel_raw_eeg_start_time) * (screen_x_size - SCREEN_X_OFFSET)) / raw_eeg_per_page_duration);
			    }
			    else if ((temp_data_mark->mark_start_time < sel_raw_eeg_start_time) && (temp_data_mark->mark_end_time > sel_raw_eeg_end_time))   // case 4
			    {
				    mark_found_within_curr_window = 1;
				    startx = 0;
						endx = (int)(((sel_raw_eeg_end_time - sel_raw_eeg_start_time) * (screen_x_size - SCREEN_X_OFFSET)) / raw_eeg_per_page_duration);
			    }
			    if (mark_found_within_curr_window)
			    {
				    starty = (int)(((n + 0.2) * (screen_y_size - SCREEN_Y_OFFSET)) / no_of_channels);
				    endy = (int)(((n + 0.8) * (screen_y_size - SCREEN_Y_OFFSET)) / no_of_channels);

				    oldPen = pDC->SelectObject(&xpen[temp_data_mark->mark_event]);

				    pDC->MoveTo(startx, starty);		
				    pDC->LineTo(endx, starty);
				    pDC->MoveTo(endx, starty);		
				    pDC->LineTo(endx, endy);
				    pDC->MoveTo(endx, endy);		
				    pDC->LineTo(startx, endy);
				    pDC->MoveTo(startx, endy);		
				    pDC->LineTo(startx, starty);
				    pDC->SelectObject(oldPen);
			    }

			    // advance the node pointer	
			    temp_data_mark = temp_data_mark->next;

				}	// end mark loop
			}	// end if individual channel database ptr is not null
		}	// end channel loop
	}	// end if markign database structure is not NULL

	//////////////////////////////
	// plot the rectangle in the amplitude EEG graph
	// CDC *pDC_Amp = m_Grid_Amp.GetDC();

	if (temp_rect_Amp != NULL) 
		pDC_Amp->DrawFocusRect(temp_rect_Amp);	//calling twice this function erases the rectangle

	// sourya - based on selected raw EEG timing boundaries and the display format of aEEG, determine
	// the rectangle boundaries for displaying the selected region
	startx = (int)((sel_raw_eeg_start_time - sel_aeeg_start_time) * 1.0 * (screen_x_size_Amp - SCREEN_X_AMP_OFFSET)) / aeeg_per_page_duration;
	endx = (int)((sel_raw_eeg_end_time - sel_aeeg_start_time) * 1.0 * (screen_x_size_Amp - SCREEN_X_AMP_OFFSET)) / aeeg_per_page_duration;

	temp_rect_Amp.left = startx;
	temp_rect_Amp.right = endx;
	temp_rect_Amp.top = 0;
	temp_rect_Amp.bottom = (screen_y_size_Amp - SCREEN_Y_AMP_OFFSET);

	pDC_Amp->DrawFocusRect(temp_rect_Amp);

	//////////////////////////////
	// plot video data if exists
	if (repaint_on == 0)
	{
		if ((video_display) && (no_of_video_files > 0))
			plot_video_data();		//plot the video data 
	}

} //end function