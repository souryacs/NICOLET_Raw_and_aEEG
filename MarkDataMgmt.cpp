// MarkDataMgmt.cpp : implementation file
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
	this function adds one node correponding to one event information
	in the marking database
*/

struct data_mark_format* CEEG_marking_toolDlg::AddMarkData(int ch_no, double event_start_time, double event_end_time, int mark_info)
{
	struct data_mark_format* temp_data_mark;	// pointer to new allocated node
	struct data_mark_format* tail;	// pointer to tail node

	// allocate one node of list which will contain channel data 
	temp_data_mark = (struct data_mark_format*)malloc(sizeof(struct data_mark_format));
	if (temp_data_mark == NULL)
	{
		AfxMessageBox("Insufficient memory - no new marking data node alloc");	
		return NULL;
	}

	temp_data_mark->next = NULL;		
	if (mark_database_count[ch_no] == 0)	//  this is the first event node of this channel
	{
		// initialize the header pointer on to this first node
		marking_database[ch_no] = temp_data_mark;		
	}
	else
	{
		// append the pointer to the tail node of the current channel event list
		tail = marking_database[ch_no];
		while (tail->next != NULL)
			tail = tail->next;
		tail->next = temp_data_mark;
	}
			
	// now fill the current node with the marking info
	temp_data_mark->mark_start_time = event_start_time;
	temp_data_mark->mark_end_time = event_end_time;
	temp_data_mark->mark_event = mark_info;
	
	// increment the event node counter for current channel
	mark_database_count[ch_no] = mark_database_count[ch_no] + 1;

	return temp_data_mark;
}
////////////////////////////////////////////
/*
	this function clears the cases for which one mark contains another mark completely within it
	this removes ambiguity between different markings
*/

void CEEG_marking_toolDlg::DelOverlappingMark()
{
	int ch_no;
	int mark_count, i;
	struct data_mark_format* temp_data_mark;	// pointer to new allocated node
	struct data_mark_format* temp_data_mark2;	// pointer to new allocated node
	bool overlapping_mark_found;

	// traverse through channels
	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		// initialize the pointer with the header node of the current channel
		temp_data_mark = marking_database[ch_no];

		// traverse through markings
		for (mark_count = 0; ((mark_count < mark_database_count[ch_no]) && (temp_data_mark->mark_end_time <= total_recording_duration_sec)); mark_count++)
		{	
			overlapping_mark_found = false;

			//invalid start or end time of the mark - continue
			if ((temp_data_mark->mark_start_time == -1) || (temp_data_mark->mark_end_time == -1))	
			{
				temp_data_mark = temp_data_mark->next;	// advance the node pointer
				continue;	
			}

			// otherwise
			if ((temp_data_mark->mark_event == SEIZURE_MARK) || 
				(temp_data_mark->mark_event == BURST_MARK) || 
				(temp_data_mark->mark_event == ARTIFACT_MARK) || 
				(temp_data_mark->mark_event == SLEEP_SPINDLE_MARK) || 
				(temp_data_mark->mark_event == BURST_SUPPRESSION_MARK) ||
				(temp_data_mark->mark_event == SUPPRESSION_MARK) ||
				(temp_data_mark->mark_event == NORMAL_MARK))
			{
				// now traverse through all markings
				// if any marking time is completely within current mark time
				// then discard both the marking from our future consideration

				// initialize the 2nd pointer with the header node of the current channel
				temp_data_mark2 = marking_database[ch_no];

				for (i = 0; ((i < mark_database_count[ch_no]) && (temp_data_mark2->mark_end_time <= total_recording_duration_sec)); i++)
				{
					// for current mark, or for an invalid mark, skip any comparison
					if ((i == mark_count) || (temp_data_mark2->mark_start_time == -1) || (temp_data_mark2->mark_end_time == -1))	
					{
						temp_data_mark2 = temp_data_mark2->next;	// advance the node pointer	
						continue;
					}

					if ((temp_data_mark2->mark_event != SEIZURE_MARK) && 
						(temp_data_mark2->mark_event != ARTIFACT_MARK) && 
						(temp_data_mark2->mark_event != BURST_MARK) && 
						(temp_data_mark2->mark_event != SLEEP_SPINDLE_MARK) && 
						(temp_data_mark2->mark_event != BURST_SUPPRESSION_MARK) &&
						(temp_data_mark2->mark_event != SUPPRESSION_MARK) &&
						(temp_data_mark2->mark_event != NORMAL_MARK))
					{
						temp_data_mark2 = temp_data_mark2->next;	// advance the node pointer	
						continue;
					}

					// if any mark starts within current mark interval
					if ((temp_data_mark2->mark_start_time >= temp_data_mark->mark_start_time) && 
						(temp_data_mark2->mark_end_time <= temp_data_mark->mark_end_time))
					{
						overlapping_mark_found = true;
						// destroy current completely overlapping mark
						temp_data_mark2->mark_start_time = -1;
						temp_data_mark2->mark_end_time = -1;
						temp_data_mark2->mark_event = -1;											
					}

					// advance the second node pointer
					temp_data_mark2 = temp_data_mark2->next;
				}

				if (overlapping_mark_found)
				{
					// destroy current mark
					temp_data_mark->mark_start_time = -1;
					temp_data_mark->mark_end_time = -1;
					temp_data_mark->mark_event = -1;											
				}
			}

			// advance the node pointer
			temp_data_mark = temp_data_mark->next;

		}	// end marking loop
	}	// end channel loop
}	// end function

////////////////////////////////////////////

/*
	this function merges similar markings if they are maintained within very small distance
	for suppression mark, the proximity threshold between adjacent markings is 0.5 sec
	for other marks, the threshold is 2 sec
*/

void CEEG_marking_toolDlg::MergeSimilarMark()
{
	int ch_no;
	int mark_count, i;
	struct data_mark_format* temp_data_mark;	// node pointer for one mark
	struct data_mark_format* temp_data_mark2;	// node pointer for comparison with current marking
	bool overlapping_mark_found;
	double time_gap;

  /*
	// for the suppression interval, maximum 0.5 sec displacement is allowed for merging two close marks
	double SUPPRESSION_MARK_MERGE_MAX_TIME_GAP = 0.5;

	// for other marks such as burst, artifact etc. same marks within relative distance of 2 seconds can 
	// be merged
	double SAME_MARK_MERGE_MAX_TIME_GAP = 0.5;	//2;
  */

	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		// initialize the pointer with the header node of the current channel
		temp_data_mark = marking_database[ch_no];

		// traverse through markings
		for (mark_count = 0; ((mark_count < mark_database_count[ch_no]) && (temp_data_mark->mark_end_time <= total_recording_duration_sec)); mark_count++)
		{
			overlapping_mark_found = false;

			//invalid start or end time of the mark - continue
			if ((temp_data_mark->mark_start_time == -1) || (temp_data_mark->mark_end_time == -1))	
			{
				temp_data_mark = temp_data_mark->next;	// advance the node pointer
				continue;	
			}

			// current mark cases are considered
			if ((temp_data_mark->mark_event == SEIZURE_MARK) || 
				(temp_data_mark->mark_event == BURST_MARK) || 
				(temp_data_mark->mark_event == ARTIFACT_MARK) || 
				(temp_data_mark->mark_event == SLEEP_SPINDLE_MARK) || 
				(temp_data_mark->mark_event == BURST_SUPPRESSION_MARK) ||
				(temp_data_mark->mark_event == SUPPRESSION_MARK) ||
				(temp_data_mark->mark_event == NORMAL_MARK))
			{	
				// according to the mark, determine the time gap for proximity and merging
				if (temp_data_mark->mark_event == SUPPRESSION_MARK)
					time_gap = SUPPRESSION_MARK_MERGE_MAX_TIME_GAP;
				else
					time_gap = SAME_MARK_MERGE_MAX_TIME_GAP;

				// initialize the 2nd pointer with the header node of the current channel
				temp_data_mark2 = marking_database[ch_no];

				for (i = 0; ((i < mark_database_count[ch_no]) && (temp_data_mark2->mark_end_time <= total_recording_duration_sec)); i++)
				{
					// for same mark, or for invalid start or end time, skip any comparison
					if ((i == mark_count) || 
						(temp_data_mark2->mark_start_time == -1) || 
						(temp_data_mark2->mark_end_time == -1))	
					{
						temp_data_mark2 = temp_data_mark2->next;	// advance the node pointer
						continue;
					}

					// check for similar markings
					if (temp_data_mark2->mark_event == temp_data_mark->mark_event)
					{
						// for close similar markings - mark them together 
						// case 1 - here the second mark lags first mark very closely
						if ((fabs(temp_data_mark->mark_start_time - temp_data_mark2->mark_end_time)) < time_gap)
						{
							overlapping_mark_found = true;
							
							// update first mark start time with the start time of second mark
							temp_data_mark->mark_start_time = temp_data_mark2->mark_start_time;		

							// delete the second mark
							temp_data_mark2->mark_start_time = -1;
							temp_data_mark2->mark_end_time = -1;
							temp_data_mark2->mark_event = -1;
						}
						else if ((fabs(temp_data_mark2->mark_start_time - temp_data_mark->mark_end_time)) < time_gap)
						{
							// case 2 - here the second mark leads the first mark very closely
							overlapping_mark_found = true;

							// update current mark end time with the end time of second mark
							temp_data_mark->mark_end_time = temp_data_mark2->mark_end_time;		

							// delete the suppression mark
							temp_data_mark2->mark_start_time = -1;
							temp_data_mark2->mark_end_time = -1;
							temp_data_mark2->mark_event = -1;
						}
					}

					// advance the second node pointer
					temp_data_mark2 = temp_data_mark2->next;
				}

				// if such merging operation took place then we will again start from that mark
				// for future comparisons
				if (overlapping_mark_found)
					mark_count--;

			}	// end check data mark type

			// advance the node pointer if previously there was no overlapping mark
			if (overlapping_mark_found == false)
				temp_data_mark = temp_data_mark->next;

		}	// end marking traverse for 1 channel
	}	// end channel loop
}	// end function
////////////////////////////////////////////////////
/*
	this function deletes the defined marks (for example, suppression marks) from the marking database
*/

void CEEG_marking_toolDlg::DiscardSmallSuppMark()
{
	int ch_no;
	int mark_count;
	struct data_mark_format* temp_data_mark;	// pointer to new allocated node
	double MIN_SUPP_DURATION = 5;	// 5 sec
	
		
	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		// initialize the pointer with the header node of the current channel
		temp_data_mark = marking_database[ch_no];

		// traverse through markings
		for (mark_count = 0; ((mark_count < mark_database_count[ch_no]) && (temp_data_mark->mark_end_time <= total_recording_duration_sec)); mark_count++)
		{
			// delete the suppression marks which have duration less than minimum 5 sec (after merging the 
			// close suppressions)
			if ((temp_data_mark->mark_event == SUPPRESSION_MARK) && 
				((temp_data_mark->mark_end_time - temp_data_mark->mark_start_time) < MIN_SUPP_DURATION))
			{
				temp_data_mark->mark_start_time = -1;
				temp_data_mark->mark_end_time = -1;
				temp_data_mark->mark_event = -1;											
			}
			temp_data_mark = temp_data_mark->next;	// advance the node pointer
		}	
	} // end channel loop
}	// end function

////////////////////////////////////////////////////
/*
	this function deletes the defined marks (for example, suppression marks) from the marking database
*/

void CEEG_marking_toolDlg::ResetSpecifiedMark()
{
	int ch_no;
	int mark_count;
	struct data_mark_format* temp_data_mark;	// pointer to new allocated node

	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		// initialize the pointer with the header node of the current channel
		temp_data_mark = marking_database[ch_no];

		// traverse through markings
		for (mark_count = 0; ((mark_count < mark_database_count[ch_no]) && (temp_data_mark->mark_end_time <= total_recording_duration_sec)); mark_count++)
		{
			// reset / delete the suppression marks
			if (temp_data_mark->mark_event == SUPPRESSION_MARK)
			{
				temp_data_mark->mark_start_time = -1;
				temp_data_mark->mark_end_time = -1;
				temp_data_mark->mark_event = -1;											
			}
			temp_data_mark = temp_data_mark->next;	// advance the node pointer
		}	
	} // end channel loop
}	// end function