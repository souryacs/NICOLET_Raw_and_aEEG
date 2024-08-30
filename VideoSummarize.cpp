// VideoSummarize.cpp : implementation file
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

// variables for initializing the bounded rectangle for patient region marking
CvRect box;
bool drawing_box = false;

// structures used in the paper implementation Zhang et.al.
bool **temporal_foreground;
bool **temporal_foreground_mask;
bool **spatial_foreground;
bool **final_detected_foreground;
double **temp_diff_image;
double **spat_diff_image;
double **temp_diff_image_plus1;

/*
	decimal to hex conversion 
*/
CString decimal_to_hex(int num)
{
	CString name("0000");
	int a[4] = {0,0,0,0};
	int i = 0;
	
	// convert the number and store it in temporary integer array
	while (num > 15)
	{
	   a[i] = num%16;
	   num = num/16;
	   i++;
	}
	a[i] = num;

	//now store it in the final string
	for (i = 0; i <= 3; i++)
	{
	   if (a[i]==10)
		name.SetAt((3-i), 'a');
	   else if (a[i]==11)
		name.SetAt((3-i), 'b');
	   else if (a[i]==12)
		name.SetAt((3-i), 'c');
	   else if (a[i]==13)
		name.SetAt((3-i), 'd');
	   else if (a[i]==14)
		name.SetAt((3-i), 'e');
	   else if (a[i]==15)
		name.SetAt((3-i), 'f');
	   else
		name.SetAt((3-i), ('0' + a[i]));
	}
	return name;
}	// end of decimal to hex conversion method

/*
	read motion information from a file (if already exists)
*/
void CEEG_marking_toolDlg::Read_Video_Motion_Information()
{
	CString video_motion_info_file;
	FILE *fid;
	int i;
	struct Motion_Information *temp_motion_info;
	struct Video_Information *temp_video_info;
	int day,mon,yr,hour,min,sec;

	// code within the #if reads the video and motion duration and finds the relative value
# if 0
	FILE *temp_fod;
	CString temp_motion_info_file = directory_name + "temp_video_motion.mtd";	
	double total_video_duration = 0;
	double total_motion_duration = 0;
	temp_fod = fopen(temp_motion_info_file, "w");
# endif

	// initialize the location of motion info containing file 
	video_motion_info_file = directory_name + video_motion_text_filename;	

	// open the file
	fid = fopen(video_motion_info_file, "r");

	// at each step, read information about one video file
	while (!feof(fid))
	{
		// allocate one node of list which will contain information of current video file
		temp_video_info = (struct Video_Information *)malloc(sizeof(struct Video_Information));
		if (temp_video_info == NULL)
		{
			AfxMessageBox("Insufficient memory - no new video node alloc");	
			return;
		}

		temp_video_info->next = NULL;
		temp_video_info->prev = NULL;
		if (head_video_info == NULL && tail_video_info == NULL)	//  this is the first node
		{
			head_video_info = tail_video_info = temp_video_info;		// assign pointer
		}
		else
		{
			temp_video_info->prev = tail_video_info;
			tail_video_info->next = temp_video_info;
			tail_video_info = temp_video_info;			// assign pointer
		}

		// read individual fields from the text file 
		fscanf(fid, "%d", &no_of_video_files);
		fscanf(fid, "%s", tail_video_info->video_filename);
		fscanf(fid, "%lf", &(tail_video_info->fps));
		fscanf(fid, "%lf", &(tail_video_info->duration));

# if 0
		fprintf(temp_fod, "\n video file : %d  duration : %lf ", no_of_video_files, tail_video_info->duration);
		total_video_duration = total_video_duration + tail_video_info->duration;
# endif


		fscanf(fid, "%lf", &(tail_video_info->no_of_frames));
		fscanf(fid, "%lf", &(tail_video_info->frame_height));
		fscanf(fid, "%lf", &(tail_video_info->frame_width));
		
		// set the recording time structure
		fscanf(fid, "%d %d %d %d %d %d", &day, &mon, &yr, &hour, &min, &sec);
		tail_video_info->video_file_create_time.SetDateTime(yr, mon, day, hour, min, sec);

		// video recording start and end time with respect to raw EEG recording start
		fscanf(fid, "%lf", &(tail_video_info->curr_video_file_start_time));
		fscanf(fid, "%lf", &(tail_video_info->curr_video_file_end_time));
	
		// motion information
		fscanf(fid, "%d", &(tail_video_info->motion_count));
		tail_video_info->head_motion_info = NULL;
		tail_video_info->tail_motion_info = NULL;
		if (tail_video_info->motion_count >= 1)
		{
			for (i = 1; i <= tail_video_info->motion_count; i++)
			{
				// there is motion information associated with this video file
				// allocate the nodes to store the info
				temp_motion_info = (struct Motion_Information *)malloc(sizeof(struct Motion_Information));
				if (temp_motion_info == NULL)
				{
					AfxMessageBox("Insufficient memory - no new motion node alloc");	
					return;
				}

				temp_motion_info->next = NULL;
				temp_motion_info->prev = NULL;
				if (tail_video_info->head_motion_info == NULL && tail_video_info->tail_motion_info == NULL)	//  this is the first node
				{
					tail_video_info->head_motion_info = tail_video_info->tail_motion_info = temp_motion_info;		// assign pointer
				}
				else
				{
					temp_motion_info->prev = tail_video_info->tail_motion_info;
					tail_video_info->tail_motion_info->next = temp_motion_info;
					tail_video_info->tail_motion_info = temp_motion_info;			// assign pointer
				}

				// now store the motion information
				fscanf(fid, "%lf %lf %d %d ", &(tail_video_info->tail_motion_info->motion_start_time), 
							&(tail_video_info->tail_motion_info->motion_end_time), 
							&(tail_video_info->tail_motion_info->motion_start_frame_count), 
							&(tail_video_info->tail_motion_info->motion_end_frame_count));


# if 0
				total_motion_duration = total_motion_duration + (tail_video_info->tail_motion_info->motion_end_time - tail_video_info->tail_motion_info->motion_start_time);
# endif

			}	//end motion info capture loop
		}
	}	// end while loop of file read

# if 0
	fprintf(temp_fod, "\n total video duration : %lf ", total_video_duration);
	fprintf(temp_fod, "\n total motion duration : %lf ", total_motion_duration);
	fclose(temp_fod);
# endif

	// close the file
	fclose(fid);
}

/*
	write motion information to a file
*/
void CEEG_marking_toolDlg::Write_Motion_Info(struct Video_Information *tail_video_info, int no_of_video_files)
{
	CString video_motion_info_file;
	FILE *fod;
	struct Motion_Information *temp_motion_info;
	int i;
	
	// initialize the location of motion info containing file 
	video_motion_info_file = directory_name + video_motion_text_filename;	

	if (no_of_video_files == 1)
	{
		// this is the first video file - open the file in write mode
		fod = fopen(video_motion_info_file, "w");
	}
	else
	{
		// open the file in append text mode
		fod = fopen(video_motion_info_file, "a");
    fprintf(fod, "\n");
	}

	// write the video related information
	fprintf(fod, "%d", no_of_video_files);
	fprintf(fod, "\n%s", tail_video_info->video_filename);
	fprintf(fod, "\n%lf", tail_video_info->fps);
	fprintf(fod, "\n%lf", tail_video_info->duration);
	fprintf(fod, "\n%lf", tail_video_info->no_of_frames);
	fprintf(fod, "\n%lf", tail_video_info->frame_height);
	fprintf(fod, "\n%lf", tail_video_info->frame_width);
	fprintf(fod, "\n%d %d %d %d %d %d", tail_video_info->video_file_create_time.GetDay(), 
				tail_video_info->video_file_create_time.GetMonth(),
				tail_video_info->video_file_create_time.GetYear(),
				tail_video_info->video_file_create_time.GetHour(),
				tail_video_info->video_file_create_time.GetMinute(),
				tail_video_info->video_file_create_time.GetSecond());
	fprintf(fod, "\n%lf", tail_video_info->curr_video_file_start_time);
	fprintf(fod, "\n%lf", tail_video_info->curr_video_file_end_time);
	fprintf(fod, "\n%d", tail_video_info->motion_count);

	// write the motion informaiton in details
	if (tail_video_info->motion_count > 0)
	{
		temp_motion_info = tail_video_info->head_motion_info;
		for (i = 1; i <= tail_video_info->motion_count; i++)
		{
			fprintf(fod, "\n%lf\t%lf\t%d\t%d", temp_motion_info->motion_start_time, 
				temp_motion_info->motion_end_time, temp_motion_info->motion_start_frame_count, 
				temp_motion_info->motion_end_frame_count);
			temp_motion_info = temp_motion_info->next;	// proceed to the next pointer
		}
	}

	//if (tail_video_info->motion_count >= 1)
	//	fprintf(fod, "\n");

	// insert a backspace character at the end
	//fprintf(fod, "%c", '\b');

	// close the text file
	fclose(fod);
}

/* 
	drawing a box
*/
void draw_box( IplImage* img, CvRect rect ){
	cvRectangle( img, cvPoint(box.x, box.y), cvPoint(box.x+box.width,box.y+box.height),
				cvScalar(0xff,0x00,0x00) );
}

/*
	mouse handler to draw a box based on mouse movement
*/
void mouseHandler(int event, int x, int y, int flags, void *param)
{
    IplImage* image = (IplImage*) param;

	switch( event ){
		case CV_EVENT_MOUSEMOVE: 
			if( drawing_box ){
				box.width = x-box.x;
				box.height = y-box.y;
			}
			break;

		case CV_EVENT_LBUTTONDOWN:
			drawing_box = true;
			box = cvRect( x, y, 0, 0 );
			break;

		case CV_EVENT_LBUTTONUP:
			drawing_box = false;
			if( box.width < 0 ){
				box.x += box.width;
				box.width *= -1;
			}
			if( box.height < 0 ){
				box.y += box.height;
				box.height *= -1;
			}
			draw_box( image, box );
			break;
	}
}



/**************************************************/

/*
	motion detection for a video file
	old motion detection routine based on frame difference
	currently being commented
*/
# if 0
void CEEG_marking_toolDlg::video_motion_detect(struct Video_Information *tail_video_info, CString video_filename)
{
	CvCapture* cv_cap;

	// temporary pointer for motion information node
	struct Motion_Information *temp_motion_info;

	double CUTOFF, ALPHA, THRESHOLD_RATIO;
	int tot, eq, neq; 
	double matching_index;

	int i, j, k, state;
	int frame_count;
	IplImage* cap_frame = 0;
	IplImage* mod_frame = 0;
	IplImage* ref_frame = 0;
	IplImage* ext_frame = 0;

	// variables to store ongoing motion information
	double motion_start_time;
	double motion_end_time;
	int motion_start_frame_count;
	int motion_end_frame_count;

	// difference threshold between ongoing motion information and the previous one
	int VIDEO_MOTION_TIME_DIFF_THR = 3;	// 3 sec is the maximum time difference

  // design the motion detection constant thresholds
  CUTOFF = 85;	//30;    
  ALPHA = .1; //.05;
  THRESHOLD_RATIO = 0.03;		// 0.05	//.01;	//.002;  

	// capture the video file - global structure
	cv_cap = cvCaptureFromAVI(video_filename);

	// read the first frame
	cap_frame = cvQueryFrame(cv_cap);

	// resize frame structure
	//IplImage *resize = cvCreateImage(cvSize(cap_frame->width, cap_frame->height),8,3);

	mod_frame = cvCreateImage(cvSize(cap_frame->width,cap_frame->height),IPL_DEPTH_8U, 3);
	ref_frame = cvCreateImage(cvSize(cap_frame->width,cap_frame->height),IPL_DEPTH_8U, 1);
	ext_frame = cvCreateImage(cvSize(cap_frame->width,cap_frame->height),IPL_DEPTH_8U, 1);

	mod_frame->origin = cap_frame->origin;
	ref_frame->origin = cap_frame->origin;
	ext_frame->origin = cap_frame->origin;

	cvCopy(cap_frame, mod_frame, 0);

  // motion defining variables
  state = 0;  

  // DRAW RECTANGLE ON THE FIRST FRAME AND THEN PRESS ESCAPE TO START FURTHER PROCESSING
	// we draw the bounding box for the first video file only
	if (no_of_video_files == 0)
	{
		box = cvRect(-1,-1,0,0);
		IplImage* temp = cvCloneImage(cap_frame);
    
		// Create windows 
		cvNamedWindow("Video: Draw rectangle and then press ESC", CV_WINDOW_AUTOSIZE);
		
		cvSetMouseCallback( "Video: Draw rectangle and then press ESC", mouseHandler, (void*) cap_frame);
   
		 // Main loop
		while(1)
		{
			cvCopyImage(cap_frame, temp);
			if(drawing_box) 
				draw_box(temp, box);
			cvShowImage("Video: Draw rectangle and then press ESC", temp);

			if( cvWaitKey(15) == 27) //press ESC to exit the loop
				break;
		}
		
		//printf("\n box.x = %d, box.y = %d, box.x+box.width = %d, box.y+box.height = %d", box.x, box.y, box.x+box.width, box.y+box.height);
		init_x = box.x ;
		init_y = box.y ;
		final_x = box.x+box.width;
		final_y = box.y+box.height;

		if (init_x > final_x)
		{
			init_x = final_x;
			final_x = box.x;
		}
		if (init_y > final_y)
		{
			init_y = final_y;
			final_y = box.y;
		}
	}	// for the first video file only


  // we loop through the entire frames
  for(frame_count = 0; frame_count < (tail_video_info->no_of_frames - 2); frame_count += 5)	// there was originally <= operator
	{
		cvCopy(ext_frame, ref_frame, 0);

		// at first we have to seek to the target frame
		cvSetCaptureProperty(cv_cap, CV_CAP_PROP_POS_FRAMES, (frame_count+2));

		// now capture the target frame
		cap_frame = cvQueryFrame(cv_cap);

		// if there is a problem in current frame capture then continue with the next iteration of the loop
		if (!cap_frame)
			continue;

		// resize the frame
		//cvResize(cap_frame, resize,CV_INTER_NN);

		// compare the current frame vs the background reference frame pixel by pixel comparison 
    for(j = init_x /* 0 */; j < final_x /* cap_frame->width */; j++)
		{
			for(i = init_y /* 0 */; i < final_y /* cap_frame->height */; i++)
			{
				tot = 0;
        for(k = 1; k<3; k++)
				{
           tot = tot + abs((mod_frame->imageData + i*cap_frame->widthStep)[j*cap_frame->nChannels + k] 
							- (cap_frame->imageData + i*cap_frame->widthStep)[j*cap_frame->nChannels + k]);
                }
                // if the difference is greater than cutoff then we store the 
				// pixel as black in the corresponding stored binary image
				if ((tot/3) > CUTOFF)
				{
					((uchar *)(ext_frame->imageData + i*cap_frame->width))[j] = 255;
				}
        else
				{    
					// white pixel
					((uchar *)(ext_frame->imageData + i*cap_frame->width))[j] = 0;
					
					// update the background model
          for(k = 1; k < 3; k++)
					{
						(mod_frame->imageData + i*cap_frame->widthStep)[j*cap_frame->nChannels + k] = 
							(1-ALPHA) * (mod_frame->imageData + i*cap_frame->widthStep)[j*cap_frame->nChannels + k] 
							+ ALPHA * (cap_frame->imageData + i*cap_frame->widthStep)[j*cap_frame->nChannels + k];
					}
        } 
			}	// frame height loop
		}	// frame width loop

		// if current frame has significant difference corresponding to the background 
		// reference frame then update the state variable

		eq = 0; 
		neq = 0;
    for(j = init_x /* 0 */; j < final_x /* cap_frame->width */; j++)
		{
			for(i = init_y /* 0 */; i < final_y /* cap_frame->height */; i++)
			{	
				if ((ref_frame->imageData + i*cap_frame->width)[j] != (ext_frame->imageData + i*cap_frame->width)[j])
					eq++; 
        else
					neq++;
			}	
		}
		if (neq != 0)
			matching_index = (double)((double)eq /(double) neq);
		else
			matching_index = 0;
        
		if (matching_index > THRESHOLD_RATIO)	// there is a motion because pixel difference is greater than the threshold
		{
			if (state == 0)
			{
				// motion is started
				state = 1;

				// note down the motion start frame and the start time
				motion_start_frame_count = frame_count;
				motion_start_time = frame_count / tail_video_info->fps;
			}
      else if (state == 1)
			{
				// motion is continued
			}
		}
    else	// there is no motion
		{
			if (state == 1)		// if there was ongoing motion previously
			{
				// motion is stopped
				state = 0;

				// note down the motion end frame and the end time
				motion_end_frame_count = frame_count;
				motion_end_time = frame_count / tail_video_info->fps;

# if 0
				// if current motion start time is very close to the last motion end time (the difference
				// is less than a predefined time threshold) then merge the current motion information
				// to the last one
				if ((tail_video_info->motion_count > 0) && 
					((motion_start_time - tail_video_info->tail_motion_info->motion_end_time) <= VIDEO_MOTION_TIME_DIFF_THR))
				{
					// update the last stored motion information (end frame count and end time)
					tail_video_info->tail_motion_info->motion_end_frame_count = motion_end_frame_count;
					tail_video_info->tail_motion_info->motion_end_time = motion_end_time;
				}
				else
				{
# endif
					// there should be a new motion information entry
					// create a motion information node which will be associated with current video file
					temp_motion_info = (struct Motion_Information *)malloc(sizeof(struct Motion_Information));
					if (temp_motion_info == NULL)
					{
						AfxMessageBox("Insufficient memory - no new motion node alloc");	
						return;
					}

					temp_motion_info->next = NULL;
					temp_motion_info->prev = NULL;
					if (tail_video_info->motion_count == 0)	//(tail_video_info->head_motion_info == NULL && tail_video_info->tail_motion_info == NULL)	//  this is the first node
					{
						tail_video_info->head_motion_info = tail_video_info->tail_motion_info = temp_motion_info;		// assign pointer
					}
					else
					{
						temp_motion_info->prev = tail_video_info->tail_motion_info;
						tail_video_info->tail_motion_info->next = temp_motion_info;
						tail_video_info->tail_motion_info = temp_motion_info;			// assign pointer
					}

					// assign the fields onto this motion node structure
					tail_video_info->tail_motion_info->motion_start_frame_count = motion_start_frame_count;
					tail_video_info->tail_motion_info->motion_start_time = motion_start_time;
					tail_video_info->tail_motion_info->motion_end_frame_count = motion_end_frame_count;
					tail_video_info->tail_motion_info->motion_end_time = motion_end_time;

					// increment the motion node counter
					tail_video_info->motion_count++;
# if 0
				}
# endif

				// update the reference frames
        ref_frame = cvCreateImage(cvSize(cap_frame->width,cap_frame->height),IPL_DEPTH_8U, 1);
				ext_frame = cvCreateImage(cvSize(cap_frame->width,cap_frame->height),IPL_DEPTH_8U, 1);
				ref_frame->origin = cap_frame->origin;
				ext_frame->origin = cap_frame->origin;

				cvCopy(cap_frame, mod_frame, 0);
			}
		}
	} // end of frame by frame comparison loop

	// release the capture
	cvReleaseCapture(&cv_cap);

	// if there was ongoing motion then we note down the motion end mark
	// add - sourya
	if (state == 1)
	{
		// motion is stopped
		state = 0;

		// note down the motion end frame and the end time
		motion_end_frame_count = (int)(tail_video_info->no_of_frames - 2);
		motion_end_time = (tail_video_info->no_of_frames - 2) / tail_video_info->fps;

# if 0
		// if current motion start time is very close to the last motion end time (the difference
		// is less than a predefined time threshold) then merge the current motion information
		// to the last one
		if ((tail_video_info->motion_count > 0) && 
			((motion_start_time - tail_video_info->tail_motion_info->motion_end_time) <= VIDEO_MOTION_TIME_DIFF_THR))
		{
			// update the last stored motion information (end frame count and end time)
			tail_video_info->tail_motion_info->motion_end_frame_count = motion_end_frame_count;
			tail_video_info->tail_motion_info->motion_end_time = motion_end_time;
		}
		else
		{
# endif
			// there should be a new motion information entry
			// create a motion information node which will be associated with current video file
			temp_motion_info = (struct Motion_Information *)malloc(sizeof(struct Motion_Information));
			if (temp_motion_info == NULL)
			{
				AfxMessageBox("Insufficient memory - no new motion node alloc");	
				return;
			}

			temp_motion_info->next = NULL;
			temp_motion_info->prev = NULL;
			if (tail_video_info->motion_count == 0)	//(tail_video_info->head_motion_info == NULL && tail_video_info->tail_motion_info == NULL)	//  this is the first node
			{
				tail_video_info->head_motion_info = tail_video_info->tail_motion_info = temp_motion_info;		// assign pointer
			}
			else
			{
				temp_motion_info->prev = tail_video_info->tail_motion_info;
				tail_video_info->tail_motion_info->next = temp_motion_info;
				tail_video_info->tail_motion_info = temp_motion_info;			// assign pointer
			}

			// assign the fields onto this motion node structure
			tail_video_info->tail_motion_info->motion_start_frame_count = motion_start_frame_count;
			tail_video_info->tail_motion_info->motion_start_time = motion_start_time;
			tail_video_info->tail_motion_info->motion_end_frame_count = motion_end_frame_count;
			tail_video_info->tail_motion_info->motion_end_time = motion_end_time;

			// increment the motion node counter
			tail_video_info->motion_count++;
# if 0
		}
# endif
	}
}	// end of motion detection method
# endif
/**************************************************/

/* 
	temporary function 
*/

int GetFileList(const char *searchkey, std::vector<std::string> &list)
{
    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(searchkey,&fd);

    if(h == INVALID_HANDLE_VALUE)
        return 0; // no files found

    while(1)
    {
        list.push_back(fd.cFileName);
        if(FindNextFile(h, &fd) == FALSE)
            break;
    }
    return list.size();
}

/*
	this function reads the associated .NPA file to decode the recording start time and date
*/
void CEEG_marking_toolDlg::SetRecordingStartTimeAndDate()
{
	std::vector<std::string> list;	
	FILE* fid_text;
	int i;
	char textLine[MAX_LINE_LEN];	// read text line from the .NPA file
	char* token;
	char date_textline[MAX_LINE_LEN];	// contains the date portion
	char time_textline[MAX_LINE_LEN];	// contains the time portion
	int record_hr, record_min, record_sec;
	int date_day, date_mon, date_yr;

	// search for the file with extension .NPA in the current recording directory
	CString tempPattern = directory_name + "*.NPA";
	int list_file_count = GetFileList(tempPattern, list);

	if (list_file_count == 0)	// no file found
		return;

	// search for the file with extension of .NPA
	CString filename_temp = list[0].c_str();
	fid_text = fopen(filename_temp, "r");     // file open

	// in the 65th line, there is reference of video recording start time
	for (i = 1; i <= 65; i++)
		fgets(textLine, MAX_LINE_LEN, fid_text);  //read the 1st line  - header information                 

	// now from the text line, extract the record date and time information
	// store the date and time part
	token = strtok(textLine, " ");
	strcpy(date_textline, token);
	token = strtok(NULL, " ");
	strcpy(time_textline, token);

	// extract the time part
	token = strtok(time_textline, ":");
	record_hr = atoi(token);
	token = strtok(NULL, ":");
	record_min = atoi(token);
	token = strtok(NULL, ":");
	record_sec = atoi(token);

	// extract the date part
	token = strtok(date_textline, "=");
	token = strtok(NULL, "=");
	strcpy(date_textline, token);
	token = strtok(date_textline, "/");
	date_mon = atoi(token);
	token = strtok(NULL, "/");
	date_day = atoi(token);
	token = strtok(NULL, "/");
	date_yr = atoi(token);

	// now construct the recording start time structure
	// it will contain the start time of the current recorded video sequence
	recording_start_time.SetDateTime(date_yr, date_mon, date_day, record_hr, record_min, record_sec);

	// close the file
	fclose(fid_text);
}

/*
	this function reads the event data file generated from the event export mechanism
*/
void CEEG_marking_toolDlg::ReadEventExportData()
{
	std::vector<std::string> list;	
	FILE* fid_text;
	char textLine[MAX_LINE_LEN];	// read text line from the event*.txt file
	CString temp;
	char* token;
	char recording_pause_duration_textline[MAX_LINE_LEN];	
	char recording_pause_start_time_textline[MAX_LINE_LEN];	
	int record_hr, record_min, record_sec;

	record_pause_event_count = 0;

	// search for the file with extension .NPA in the current recording directory
	CString tempPattern = directory_name + "eventdata.txt";
	int list_file_count = GetFileList(tempPattern, list);

	if (list_file_count == 0)	// no file found
		return;

	// search for the file with extension of .NPA
	CString filename_temp = list[0].c_str();
	fid_text = fopen(filename_temp, "r");     // file open

	// read until the end of the file
	while ((fid_text != NULL) && (!feof(fid_text)))
	{
		fgets(textLine, MAX_LINE_LEN, fid_text);         
		
		// if the read line contains recording paused event then store the recording pause timings
		temp = textLine;

		// if the recording paused event is found from the event file
		if (temp.Find("Recording Paused -") != -1)	
		{
			token = strtok(textLine, "-");
			token = strtok(NULL, "\t");
			strcpy(recording_pause_duration_textline, token);
			token = strtok(NULL, "\t");
			strcpy(recording_pause_start_time_textline, token);

			// extract the time part for the recording pause duration
			token = strtok(recording_pause_duration_textline, ":");
			record_hr = atoi(token);
			token = strtok(NULL, ":");
			record_min = atoi(token);
			token = strtok(NULL, ":");
			record_sec = atoi(token);
			record_pause_duration[record_pause_event_count] = (record_hr * 3600 + record_min * 60 + record_sec);	

			// extract the time part for the recording pause start time
			token = strtok(recording_pause_start_time_textline, ":");
			record_hr = atoi(token);
			token = strtok(NULL, ":");
			record_min = atoi(token);
			token = strtok(NULL, ":");
			record_sec = atoi(token);
			record_pause_start_time[record_pause_event_count] = (record_hr * 3600 + record_min * 60 + record_sec);	

			// increment the event counter
			record_pause_event_count++;
		}
	}

	// close the file
	fclose(fid_text);
}

/*
 frees the allocated structure
 paper Zhang et. al.
*/
void FreeStruct(int no_of_pixels_x, int no_of_pixels_y)
{
	int i;
	for (i = 0; i < no_of_pixels_y; i++)
		free(temp_diff_image[i]);
	free(temp_diff_image);

	for (i = 0; i < no_of_pixels_y; i++)
		free(temp_diff_image_plus1[i]);
	free(temp_diff_image_plus1);

	for (i = 0; i < no_of_pixels_y; i++)
		free(spat_diff_image[i]);
	free(spat_diff_image);

	for (i = 0; i < no_of_pixels_y; i++)
		free(temporal_foreground[i]);
	free(temporal_foreground);

	for (i = 0; i < no_of_pixels_y; i++)
		free(temporal_foreground_mask[i]);
	free(temporal_foreground_mask);

	for (i = 0; i < no_of_pixels_y; i++)
		free(spatial_foreground[i]);
	free(spatial_foreground);

	for (i = 0; i < no_of_pixels_y; i++)
		free(final_detected_foreground[i]);
	free(final_detected_foreground);

	return;
}	// end function

/*
 write the allocation routine of the corresponding structures
 motion detection routine based on Zhang et. al.
*/
void AllocateStruct(int no_of_pixels_x, int no_of_pixels_y)
{
	int i;

	// allocate temporal difference image (middle and first frame)
	temp_diff_image = (double**)calloc(no_of_pixels_y, sizeof(double *));
	for (i = 0; i < no_of_pixels_y; i++)
		temp_diff_image[i] = (double *)calloc(no_of_pixels_x, sizeof(double));

	// allocate temporary difference image (last and middle frame)
	temp_diff_image_plus1 = (double**)calloc(no_of_pixels_y, sizeof(double *));
	for (i = 0; i < no_of_pixels_y; i++)
		temp_diff_image_plus1[i] = (double *)calloc(no_of_pixels_x, sizeof(double));

	// allocate spatial difference image
	spat_diff_image = (double**)calloc(no_of_pixels_y, sizeof(double *));
	for (i = 0; i < no_of_pixels_y; i++)
		spat_diff_image[i] = (double *)calloc(no_of_pixels_x, sizeof(double));

	// allocate temporal foreground structure
	temporal_foreground = (bool**)calloc(no_of_pixels_y, sizeof(bool *));
	for (i = 0; i < no_of_pixels_y; i++)
		temporal_foreground[i] = (bool *)calloc(no_of_pixels_x, sizeof(bool));

	// allocate temporal foreground mask structure
	temporal_foreground_mask = (bool**)calloc(no_of_pixels_y, sizeof(bool *));
	for (i = 0; i < no_of_pixels_y; i++)
		temporal_foreground_mask[i] = (bool *)calloc(no_of_pixels_x, sizeof(bool));

	// allocate spatial foreground
	spatial_foreground = (bool**)calloc(no_of_pixels_y, sizeof(bool *));
	for (i = 0; i < no_of_pixels_y; i++)
		spatial_foreground[i] = (bool *)calloc(no_of_pixels_x, sizeof(bool));

	// final detected foreground
	final_detected_foreground = (bool**)calloc(no_of_pixels_y, sizeof(bool *));
	for (i = 0; i < no_of_pixels_y; i++)
		final_detected_foreground[i] = (bool *)calloc(no_of_pixels_x, sizeof(bool));

	return;
}	// end function

/********************************/
// functions to compute the mean and standard deviation
// of an input image
// required during paper implementation of 
// paper Zhang et. al.
double calc_mean(double **img, int init_x, int final_x, int init_y, int final_y)
{
	int i, j;
	double mean;
	mean = 0;
	for (j = init_y; j <= final_y; j++)
	{
		for (i = init_x; i <= final_x; i++)
		{
			mean += img[j - init_y][i - init_x];
		}
	}
	mean /= (final_x - init_x + 1) * (final_y - init_y + 1);
	return mean;
}

double calc_std(double **img, int init_x, int final_x, int init_y, int final_y, double mean)
{
	int i, j;
	double std;
	std = 0;
	for (j = init_y; j <= final_y; j++)
	{
		for (i = init_x; i <= final_x; i++)
		{
			std += pow((img[j - init_y][i - init_x] - mean), 2);
		}
	}
	std = sqrt(std / ((final_x - init_x + 1) * (final_y - init_y + 1)));
	return std;
}
/********************************/

/*
  a new motion detection routine based on temporal difference and 
  dynamic spatial and temporal foreground difference calculation
*/
void CEEG_marking_toolDlg::HybridMotionDetect(struct Video_Information *tail_video_info, 
                        IplImage* first_frame, IplImage* middle_frame, 
                        IplImage* last_frame, IplImage* backgrnd_frame, 
                        int frame_count, IplImage* det_for_img)
{
	// temporary pointer for motion information node
	struct Motion_Information *temp_motion_info;

	// variables to store ongoing motion information
	static double motion_start_time;
	static double motion_end_time;
	static int motion_start_frame_count;
	static int motion_end_frame_count;

	int no_of_pixels_x, no_of_pixels_y;
	int i, j, k;

	double temp_diff_img_mean, temp_diff_img_std;
	double temp_diff_image_plus1_mean, temp_diff_image_plus1_std;
	double spat_diff_img_mean, spat_diff_img_std;

	bool r_t, r_tplus1;
	double tot1, tot2, tot3;

	static int motion_ongoing;

	// these variables are used for defining the temporal foreground mask
	int minx, miny, maxx, maxy;

	// allocate the structures used for calculating temporal difference
	// and the motion detection
	no_of_pixels_x = (final_x - init_x + 1);
	no_of_pixels_y = (final_y - init_y + 1);

	// allocate the dynamic memory
	AllocateStruct(no_of_pixels_x, no_of_pixels_y);

	// define the temporal foreground mask  boundaries (initial)
	minx = (final_x - init_x);
	maxx = 0;
	miny = (final_y - init_y);
	maxy = 0;

  // for the first frame in a new video file
  // that is, when the frame count is 10
  // the motion indicator variable needs to be reset
  if (frame_count == 10)
     motion_ongoing = 0;


# if 0
	double white_pix = 0, black_pix = 0;
	for (i = init_y; i <= final_y; i++)
	{
		for (j = init_x; j <= final_x; j++)
		{
			if ((det_for_img->imageData + i*det_for_img->widthStep)[j] == 0)
				black_pix++;
			else
				white_pix++;
		}
	}
	printf("\n\n  ---- iteration -- white pix : %lf  black pix : %lf ", white_pix, black_pix);
# endif

	// now fill the values of temporal difference images
	for (i = init_y; i <= final_y; i++)
	{
		for (j = init_x; j <= final_x; j++)
		{
			// accumulate the intensity difference
			tot1 = 0;	tot2 = 0;	tot3 = 0;
			for (k = 1; k < 3; k++)
			{
				tot1 += (first_frame->imageData + i*first_frame->widthStep)[j*first_frame->nChannels + k];
				tot2 += (middle_frame->imageData + i*middle_frame->widthStep)[j*middle_frame->nChannels + k];
				tot3 += (last_frame->imageData + i*last_frame->widthStep)[j*last_frame->nChannels + k];
			}
			// fill the temporary difference images
			temp_diff_image[i - init_y][j - init_x] = fabs(tot2 - tot1);
			temp_diff_image_plus1[i - init_y][j - init_x] = fabs(tot3 - tot2);
		}
	}

	// calculate the mean and standard deviation of the difference images
	temp_diff_img_mean = calc_mean(temp_diff_image, init_x, final_x, init_y, final_y);
	temp_diff_img_std = calc_std(temp_diff_image, init_x, final_x, init_y, final_y, temp_diff_img_mean);

	temp_diff_image_plus1_mean = calc_mean(temp_diff_image_plus1, init_x, final_x, init_y, final_y);
	temp_diff_image_plus1_std = calc_std(temp_diff_image_plus1, init_x, final_x, init_y, final_y, temp_diff_image_plus1_mean);

	// now calculate the temporal foreground
	for (i = init_y; i <= final_y; i++)
	{
		for (j = init_x; j <= final_x; j++)
		{
			r_t = (fabs(temp_diff_image[i - init_y][j - init_x] - temp_diff_img_mean) > (TH_K * temp_diff_img_std)) ? 1 : 0;
			r_tplus1 = (fabs(temp_diff_image_plus1[i - init_y][j - init_x] - temp_diff_image_plus1_mean) > (TH_K * temp_diff_image_plus1_std)) ? 1 : 0;

			// fill the value according to the AND operation
			temporal_foreground[i - init_y][j - init_x] = (r_t && r_tplus1);
			if (temporal_foreground[i - init_y][j - init_x] == 1)
			{
				// update the minimum and maximum x and y boundaries
				// it'll be needed later to define the temporal foreground mask
				minx = (minx > (j - init_x)) ? (j - init_x) : minx;
				maxx = (maxx < (j - init_x)) ? (j - init_x) : maxx;
				miny = (miny > (i - init_y)) ? (i - init_y) : miny;
				maxy = (maxy < (i - init_y)) ? (i - init_y) : maxy;
			}
		}
	}

	// create the temporal foreground mask
	for (i = init_y; i <= final_y; i++)
	{
		for (j = init_x; j <= final_x; j++)
		{
			if (((j - init_x) >= minx) && ((j - init_x) <= maxx) && ((i - init_y) >= miny) && ((i - init_y) <= maxy))
				temporal_foreground_mask[i - init_y][j - init_x] = 1;
			else
				temporal_foreground_mask[i - init_y][j - init_x] = 0;
		}
	}

	// now fill the values of spatial foreground
	for (i = init_y; i <= final_y; i++)
	{
		for (j = init_x; j <= final_x; j++)
		{
			// accumulate the intensity difference
			tot1 = 0;	tot2 = 0;
			for (k = 1; k < 3; k++)
			{
				tot1 += (middle_frame->imageData + i*middle_frame->widthStep)[j*middle_frame->nChannels + k];
				tot2 += (backgrnd_frame->imageData + i*backgrnd_frame->widthStep)[j*backgrnd_frame->nChannels + k];
			}
			spat_diff_image[i - init_y][j - init_x] = fabs(tot2 - tot1);
		}
	}

	// calculate the mean and standard deviation of the spatial difference image
	spat_diff_img_mean = calc_mean(spat_diff_image, init_x, final_x, init_y, final_y);
	spat_diff_img_std = calc_std(spat_diff_image, init_x, final_x, init_y, final_y, spat_diff_img_mean);

	// now fill the values of spatial foreground image
	for (i = init_y; i <= final_y; i++)
	{
		for (j = init_x; j <= final_x; j++)
		{
			spatial_foreground[i - init_y][j - init_x] = (fabs(spat_diff_image[i - init_y][j - init_x] - spat_diff_img_mean) > (TH_K * spat_diff_img_std)) ? 1 : 0;
		}
	}

	// now fill the final detected foreground
	double foreground_pixel_ratio = 0;
	for (i = init_y; i <= final_y; i++)
	{
		for (j = init_x; j <= final_x; j++)
		{
			final_detected_foreground[i - init_y][j - init_x] = temporal_foreground_mask[i - init_y][j - init_x] && (temporal_foreground[i - init_y][j - init_x] || spatial_foreground[i - init_y][j - init_x]);
			if (final_detected_foreground[i - init_y][j - init_x] == 1)
				foreground_pixel_ratio++;
		}
	}
	foreground_pixel_ratio /= (no_of_pixels_x * no_of_pixels_y);

	// calculate the detected foreground and also the change of this foreground with respect to the previous frame
	double change_detected_foreground = 0;
	double white_to_black = 0;
	double black_to_white = 0;

	for (i = init_y; i <= final_y; i++)
	{
		for (j = init_x; j <= final_x; j++)
		{
			if (final_detected_foreground[i - init_y][j - init_x] == 1)
			{
				if ((det_for_img->imageData + i*det_for_img->widthStep)[j] == 0)
				{
					change_detected_foreground++;
					black_to_white++;
				}
				(det_for_img->imageData + i*det_for_img->widthStep)[j] = 255;
			}
			else
			{
				if ((det_for_img->imageData + i*det_for_img->widthStep)[j] != 0)
				{
					change_detected_foreground++;
					white_to_black++;
				}
				(det_for_img->imageData + i*det_for_img->widthStep)[j] = 0;
			}
		}
	}
	change_detected_foreground /= (no_of_pixels_x * no_of_pixels_y);

# if 0
	int sec = (frame_count / 10);	// fps should be replaced
	int min = (sec / 60);
	sec = sec - (min * 60);
	//printf("\n frame : %d  time %d:%d ---  current foreground : %lf change foreground : %lf   BTW : %lf   WTB : %lf    BTW / WTB : %lf ", frame_count, min, sec, foreground_pixel_ratio, change_detected_foreground, black_to_white, white_to_black, (black_to_white / white_to_black));
# endif

# if 0
	// show the original image
	cvNamedWindow("Video: original image", CV_WINDOW_AUTOSIZE);
	cvShowImage("Video: original image", middle_frame);
	cvWaitKey(10); //wait for 10 ms for user to hit some key in the window

	// show the detected foreground
	cvNamedWindow("Video: detected foreground", CV_WINDOW_AUTOSIZE);
	cvShowImage("Video: detected foreground", det_for_img);
	cvWaitKey(10); //wait for 10 ms for user to hit some key in the window
# endif

	// motion detection decision
	if ((white_to_black != 0) && (motion_ongoing == 0) &&
			((change_detected_foreground >= 0.05) ||
			 ((black_to_white / white_to_black) >= 1.4) ||
			 	 ((change_detected_foreground >= 0.03) && ((black_to_white / white_to_black) >= 1.1))))
	{
		motion_ongoing = 1;
		//printf("\n --- motion start --- frame : %d  time %d:%d ", frame_count, min, sec);
		//printf("\n\t %d:%d ", min, sec);

		// note down the motion start frame and the start time
		motion_start_frame_count = frame_count;
		motion_start_time = frame_count / tail_video_info->fps;
	}
	else if ((white_to_black != 0) && (motion_ongoing == 1) &&
			(change_detected_foreground < 0.04) && ((change_detected_foreground + (black_to_white / white_to_black)) <= 0.95))
	{
		motion_ongoing = 0;
		//printf(" --- end --- frame : %d  time %d:%d ", frame_count, min, sec);
		//printf("\t %d:%d ", min, sec);

		// note down the motion end frame and the end time
		motion_end_frame_count = frame_count;
		motion_end_time = frame_count / tail_video_info->fps;

		// there should be a new motion information entry
		// create a motion information node which will be associated with current video file
		temp_motion_info = (struct Motion_Information *)malloc(sizeof(struct Motion_Information));
		if (temp_motion_info == NULL)
		{
			AfxMessageBox("Insufficient memory - no new motion node alloc");	
			return;
		}

		temp_motion_info->next = NULL;
		temp_motion_info->prev = NULL;
		if (tail_video_info->motion_count == 0)	//(tail_video_info->head_motion_info == NULL && tail_video_info->tail_motion_info == NULL)	//  this is the first node
		{
			tail_video_info->head_motion_info = tail_video_info->tail_motion_info = temp_motion_info;		// assign pointer
		}
		else
		{
			temp_motion_info->prev = tail_video_info->tail_motion_info;
			tail_video_info->tail_motion_info->next = temp_motion_info;
			tail_video_info->tail_motion_info = temp_motion_info;			// assign pointer
		}

		// assign the fields onto this motion node structure
		tail_video_info->tail_motion_info->motion_start_frame_count = motion_start_frame_count;
		tail_video_info->tail_motion_info->motion_start_time = motion_start_time;
		tail_video_info->tail_motion_info->motion_end_frame_count = motion_end_frame_count;
		tail_video_info->tail_motion_info->motion_end_time = motion_end_time;

		// increment the motion node counter
		tail_video_info->motion_count++;
	}

	// free the allocated structure
	FreeStruct(no_of_pixels_x, no_of_pixels_y);

} // end motion detection routine

/*
	video summarization routine
*/
void CEEG_marking_toolDlg::Video_Summarize()
{
	//int len, index;
	CString tempstr, tempstr1, subdir_name, video_filename;
	//CFileStatus status;
	CvCapture *capture;
	CString video_motion_info_file;

	// variables to contain the .NPA file structure 
	// it is needed to store the start of recording time
	std::vector<std::string> list;	
	char textLine[MAX_LINE_LEN];	
	char date_textline[MAX_LINE_LEN];	// contains the date portion
	char time_textline[MAX_LINE_LEN];	// contains the time portion
	char* token;
	int record_hr, record_min, record_sec;
	int date_day, date_mon, date_yr;
	struct stat attrib;	// create a file attribute structure

	double record_pause_time_elapsed;

	// temporary pointers for video information nodes
	// used for maintaining the linked list structure
	struct Video_Information *temp_video_info;

	// stores the frames for computing the temporal difference
	IplImage* first_frame;
	IplImage* middle_frame;
	IplImage* last_frame;

	// computes the background reference frame
	IplImage* background_frame;

	// detected foreground image used to generate the final foreground picture
	IplImage* det_for_img;

	// current captured frame
	IplImage* cap_frame = 0;

	int i, j, k;
	int frame_count = 0;
	
	// initialize the location of motion info containing file 
	video_motion_info_file = directory_name + video_motion_text_filename;	

	// initialize the video global parameters
	no_of_video_files = 0;
	head_video_info = NULL;
	tail_video_info = NULL;
	
	// procedure to extract the subdirectory within the current directory
	// which contains the video files
# if 0
	// find out the directory containing the video files (if any)
	// old code - is not robust
	len = directory_name.GetLength();
	tempstr = directory_name;
	tempstr.TrimRight('\\');	//remove last '\' character
	index = tempstr.ReverseFind('\\');
	tempstr1 = tempstr.Mid((index+1), (tempstr.GetLength() - index));
	index = tempstr1.ReverseFind('_');
	subdir_name = tempstr1.Left(index);
	video_files_dir_name = directory_name + subdir_name + "\\";
# else
	// here we search for a folder within the current directory
	// if such a folder exists

  WIN32_FIND_DATA fd;
	CString pattern = directory_name + "*";
  HANDLE h = FindFirstFile(pattern, &fd);
	bool dir_found = false;

  if(h == INVALID_HANDLE_VALUE)	// no files or directories found
      return; 

  while(1)
  {
		// there are total 3 directories
		// the first two are the links of current directory and the parent directory
		if ((fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) && 
			(strcmp(fd.cFileName, ".") != 0) && (strcmp(fd.cFileName, "..") != 0))
		{
			dir_found = true;
			subdir_name = fd.cFileName;
			video_files_dir_name = directory_name + subdir_name + "\\";
		}
    if(FindNextFile(h, &fd) == FALSE)
        break;
  }

	if (dir_found == false)	// there is no directory containing the video files
		return;

# endif

	// set the recording start time and corresponding date from the .NPA file information
	SetRecordingStartTimeAndDate();

	// read the recording pause events
	ReadEventExportData();

	// check whether video motion information already exists
	if (_access (video_motion_info_file, 0) == 0)
	{
		Read_Video_Motion_Information();
	}
	else
	{
		while(1)
		{
			tempstr1 = "nrva" + decimal_to_hex(no_of_video_files) + ".avi";
			video_filename = video_files_dir_name + tempstr1;
					
			// check whether video motion detection file name exists
			if (_access (video_filename, 0) == 0)
			{
				// if the file exists then capture its header by opencv standard function
				capture = cvCaptureFromAVI(video_filename);

				if(!capture) 
				{
					// file read error or possibly there is no such video file - break from loop
					break;
				}
				else
				{
# if 1
	        // read the first frame
	        cap_frame = cvQueryFrame(capture);
	        frame_count++;
# endif

					// there is a valid video file
					// so append one node which will contain current video file's information
					
					// allocate one node of list which will contain information of current video file
					temp_video_info = (struct Video_Information *)malloc(sizeof(struct Video_Information));
					if (temp_video_info == NULL)
					{
						AfxMessageBox("Insufficient memory - no new video node alloc");	
						return;
					}

					temp_video_info->next = NULL;
					temp_video_info->prev = NULL;
					if (head_video_info == NULL && tail_video_info == NULL)	//  this is the first node
					{
						head_video_info = tail_video_info = temp_video_info;		// assign pointer
					}
					else
					{
						temp_video_info->prev = tail_video_info;
						tail_video_info->next = temp_video_info;
						tail_video_info = temp_video_info;			// assign pointer
					}

					// fill the parameters for current video node
					tail_video_info->fps = cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);	// fps
					tail_video_info->no_of_frames = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);	// no of frames
					if (tail_video_info->fps != 0)
						tail_video_info->duration = (tail_video_info->no_of_frames / tail_video_info->fps);	//duration
					else
						tail_video_info->duration = 0;
					tail_video_info->frame_height = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
					tail_video_info->frame_width = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
					tail_video_info->motion_count = 0;	//initialization
					tail_video_info->head_motion_info = NULL;
					tail_video_info->tail_motion_info = NULL;
					//strcpy(tail_video_info->video_filename, tempstr1);	// current video file name
          strcpy(tail_video_info->video_filename, "");
          strcat(tail_video_info->video_filename, tempstr1);


# if 0
          // release the capture
					cvReleaseCapture(&capture);
# endif

					// now get the video attributes from the input .avi file
					// and store it in a corresponding video structures
					stat(video_filename, &attrib);		// get the attributes of the video file
					
					// extract the date and time information from the file statistics
					strftime(textLine, MAX_LINE_LEN, "%d/%m/%Y %H:%M:%S", localtime(&attrib.st_mtime));

					// string token based field isolation
					token = strtok(textLine, " ");
					strcpy(date_textline, token);
					token = strtok(NULL, " ");
					strcpy(time_textline, token);

					// extract the time part
					token = strtok(time_textline, ":");
					record_hr = atoi(token);
					token = strtok(NULL, ":");
					record_min = atoi(token);
					token = strtok(NULL, ":");
					record_sec = atoi(token);

					// extract the date part
					token = strtok(date_textline, "/");
					date_day = atoi(token);
					token = strtok(NULL, "/");
					date_mon = atoi(token);
					token = strtok(NULL, "/");
					date_yr = atoi(token);
					
					// now construct the recording start time structure
					// it will contain the start time of the current recorded video sequence
					tail_video_info->video_file_create_time.SetDateTime(date_yr, date_mon, date_day, record_hr, record_min, record_sec);

					// set the video file start and end time with respect to the raw EEG recording start time

					// we find the total time where the recording was paused, within the current raw EEG selection time
					record_pause_time_elapsed = 0;

					for (j = 0; j < record_pause_event_count; j++)
					{
						// we traverse through the record pause events and note down the total duration of pause upto current 
						// raw EEG selection interval
						if (record_pause_start_time[j] <= sel_raw_eeg_start_time)
							record_pause_time_elapsed += record_pause_duration[j];
						else
							break;
					}

					// video file start time with respect to the start time of the recording is calculated by
					// video file creation time - video file duration - record start time - total record pause time
					tail_video_info->curr_video_file_start_time = (tail_video_info->video_file_create_time - recording_start_time).GetTotalSeconds() - tail_video_info->duration - record_pause_time_elapsed;
					tail_video_info->curr_video_file_end_time = tail_video_info->curr_video_file_start_time + tail_video_info->duration;

					/*******************************/
					/* this code is old video motion detection routine
					based on the frame difference (pixel wise)
					and threshold based comparison
					it is, for the moment, commented */
						
# if 0
					// call the video summarization routine now
					video_motion_detect(tail_video_info, video_filename);
# endif
					/*******************************/
					// a new motion detection routine
					// and associated initializations
# if 1
					// DRAW RECTANGLE ON THE FIRST FRAME AND THEN PRESS ESCAPE TO START FURTHER PROCESSING
					// we draw the bounding box for the first video file only
					if (no_of_video_files == 0)
					{
						box = cvRect(-1,-1,0,0);
						IplImage* temp = cvCloneImage(cap_frame);
    
						// Create windows 
						cvNamedWindow("Video: Draw rectangle and then press c", CV_WINDOW_AUTOSIZE);
						cvSetMouseCallback( "Video: Draw rectangle and then press c", mouseHandler, (void*) cap_frame);
   
						// Main loop
						while(1)
						{
							cvCopyImage(cap_frame, temp);
							if(drawing_box) 
								draw_box(temp, box);
							cvShowImage("Video: Draw rectangle and then press c", temp);
							char tt;
							tt = cvWaitKey(15);
							if(tt == 'c' || tt == 'C') //press 'c' character to exit the loop	//27 is the ascii value
								break;
						}
						
						//printf("\n box.x = %d, box.y = %d, box.x+box.width = %d, box.y+box.height = %d", box.x, box.y, box.x+box.width, box.y+box.height);
						init_x = box.x ;
						init_y = box.y ;
						final_x = box.x+box.width;
						final_y = box.y+box.height;

						if (init_x > final_x)
						{
							init_x = final_x;
							final_x = box.x;
						}
						if (init_y > final_y)
						{
							init_y = final_y;
							final_y = box.y;
						}
						cvDestroyWindow("Video: Draw rectangle and then press c");

						// allocate the image structures required to contain the frames
						first_frame = cvCreateImage(cvSize(cap_frame->width, cap_frame->height),IPL_DEPTH_8U, 3);
						middle_frame = cvCreateImage(cvSize(cap_frame->width, cap_frame->height),IPL_DEPTH_8U, 3);
						last_frame = cvCreateImage(cvSize(cap_frame->width, cap_frame->height),IPL_DEPTH_8U, 3);

						first_frame->origin = cap_frame->origin;
						middle_frame->origin = cap_frame->origin;
						last_frame->origin = cap_frame->origin;

						// allocate the background reference frame
						background_frame = cvCreateImage(cvSize(cap_frame->width, cap_frame->height),IPL_DEPTH_8U, 3);
						background_frame->origin = cap_frame->origin;

						// allocate the detected foreground frame
						det_for_img = cvCreateImage(cvSize(cap_frame->width, cap_frame->height),IPL_DEPTH_8U, 1);
						for (i = 0; i < cap_frame->height; i++)	// fill all the values with 0
						{
							for (j = 0; j < cap_frame->width; j++)
							{
								(det_for_img->imageData + i*det_for_img->widthStep)[j] = 0;
							}
						}
						det_for_img->origin = cap_frame->origin;
					}	// for the first video file only

					// this section will process frames (half of the frame rate)
					for(frame_count = 0; frame_count < (tail_video_info->no_of_frames - 2); frame_count += 5)
					{
						// at first we have to seek to the target frame
						cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, (frame_count + 2));

						// now capture the target frame
						cap_frame = cvQueryFrame(capture);

						// if there is a problem in current frame capture then continue with the next iteration of the loop
						if (!cap_frame)
							continue;

						if (frame_count == 0)	// copy to first frame
							cvCopy(cap_frame, first_frame, 0);
						else if (frame_count == 5)
							cvCopy(cap_frame, middle_frame, 0);
						else if (frame_count == 10)
							cvCopy(cap_frame, last_frame, 0);
						else
						{
							cvCopy(middle_frame, first_frame, 0);
							cvCopy(last_frame, middle_frame, 0);
							cvCopy(cap_frame, last_frame, 0);
						}

						if (frame_count >= 10)
						{
							/*
							 * call the motion detection routine
							 * the routine is based on the paper
							 * A Novel Hybrid Motion Detection Algorithm Based On Dynamic Thresholding Segmentation
							 * by Peng Zhang, Tie-Yong Cao, Tao Zhu
							 */
							HybridMotionDetect(tail_video_info, first_frame, middle_frame, 
                last_frame, background_frame, frame_count, det_for_img);
						}

						/*
						 * the background reference frame also needs to be updated
						 * but for the current iteration (at time t) the previous background ref frame (at time t-1) needs to be used
						 * so the reference frame updation is to be done after the main function call
						 */
						if (frame_count == 0)	// first frame is the initial background reference
						{
							cvCopy(cap_frame, background_frame, 0);
						}
						else
						{
							for (i = 0; i < cap_frame->height; i++)
							{
								for (j = 0; j < cap_frame->width; j++)
								{
									for (k = 1; k < 3; k++)
									{
										(background_frame->imageData + i*background_frame->widthStep)[j*background_frame->nChannels + k] =
												(1 - TH_ALPHA) * (background_frame->imageData + i*background_frame->widthStep)[j*background_frame->nChannels + k] +
													TH_ALPHA * (cap_frame->imageData + i*cap_frame->widthStep)[j*cap_frame->nChannels + k];
									}
								}
							}
						}
					}	// end frame loop
					
					// release the capture for the current frame
					cvReleaseCapture(&capture);

# endif          
          /*******************************/
					// increment the counter
					no_of_video_files++;

					/*******************************/
					// write the motion information to a text file
					Write_Motion_Info(tail_video_info, no_of_video_files);
					/*******************************/
				}
			}
			else
				break;
		} // end while
	}	// end motion file existence check

  // used with the latest video motion detection method
# if 1
	// free the allocated image structures
	cvReleaseImage(&first_frame);
	cvReleaseImage(&middle_frame);
	cvReleaseImage(&last_frame);
	cvReleaseImage(&background_frame);
	cvReleaseImage(&det_for_img);
# endif

}	// end of Video_Summarize method 


/*
	video plot routine in correspondence to the raw EEG data
*/
void CEEG_marking_toolDlg::plot_video_data()
{
	double start_video_time, end_video_time;
	struct Video_Information *temp_start_video_ptr;
	struct Video_Information *temp_end_video_ptr;
	//double curr_video_file_start_time, curr_video_file_end_time;
	int i, start_video_file_no, end_video_file_no;
    CString vlc_executable, vlc_command_line_option, batch_filename;
	char t1[20], t2[20];
	FILE *fp;
	CString space = " ";
	CString quote = "\"";
	CString video_filename;

	// vlc executable 
	vlc_executable = "C:\\Program Files\\VideoLAN\\VLC\\vlc.exe";

	// select the video file which contains the start time video
	temp_start_video_ptr = head_video_info;	// point to the head of the video linked list
	start_video_file_no = -1;

	for(i = 0; i < no_of_video_files; i++)
	{
		if ( (sel_raw_eeg_start_time >= temp_start_video_ptr->curr_video_file_start_time) && 
			(sel_raw_eeg_start_time <= temp_start_video_ptr->curr_video_file_end_time) )
		{
			start_video_file_no = i;
			start_video_time = floor(sel_raw_eeg_start_time - temp_start_video_ptr->curr_video_file_start_time);
			break;
		}
		temp_start_video_ptr = temp_start_video_ptr->next;
	}


	// select the video file which contains the end time video - can be
    // different from the video containing the start time
	temp_end_video_ptr = head_video_info;	// point to the head of the video linked list
	end_video_file_no = -1;
	for(i = 0; i < no_of_video_files; i++)
	{
		if ( (sel_raw_eeg_end_time >= temp_end_video_ptr->curr_video_file_start_time) && 
			(sel_raw_eeg_end_time <= temp_end_video_ptr->curr_video_file_end_time) )
		{
			end_video_file_no = i;
			end_video_time = floor(sel_raw_eeg_end_time - temp_end_video_ptr->curr_video_file_start_time);
			break;
		}
		temp_end_video_ptr = temp_end_video_ptr->next;
	}

	// sometimes due to discontinuous video recording, it might happen that current EEG segment has partial 
	// video information
	// in that case, may be start video time is bounded within some video segment file but end video time 
	// is not bound within a proper video segment
	// in that case, we bind the start or end video time
	if ((start_video_file_no != -1) && (end_video_file_no == -1))
	{
		end_video_file_no = start_video_file_no;
		end_video_time = temp_start_video_ptr->duration;
	}
	else if ((start_video_file_no == -1) && (end_video_file_no != -1))
	{
		start_video_file_no = end_video_file_no;
		start_video_time = 0;	
	}

    // start and end time containing video files are valid
    if ( (start_video_file_no != -1) && (end_video_file_no != -1) )
	{
		// open the batch file for containing the VLC execution command
        batch_filename = directory_name + "sample.bat";
        fp = fopen(batch_filename, "w");

        //if start and end video time is contained within the same file
        if ( start_video_file_no == end_video_file_no )
		{
			video_filename = video_files_dir_name + "nrva" + decimal_to_hex(start_video_file_no) + ".avi";
			itoa((int)start_video_time, t1, 10);
			itoa((int)end_video_time, t2, 10);
            vlc_command_line_option = quote + vlc_executable + quote + space + "--play-and-exit" + space + quote + video_filename + quote + space + "--start-time=" + t1 + space + "--stop-time=" + t2 + space + "--video-on-top";    
			fprintf(fp, "%s\n", vlc_command_line_option);
			fclose(fp);
		}
        else
		{
            for (i = start_video_file_no; i <= end_video_file_no; i++)
			{
                if (i == start_video_file_no)
				{
					video_filename = video_files_dir_name + "nrva" + decimal_to_hex(i) + ".avi";
					itoa((int)start_video_time, t1, 10);
					//itoa((int)(video_file_duration[i]), t2, 10);
					//vlc_command_line_option = quote + vlc_executable + quote + space + "--play-and-exit" + space + quote + video_filename + quote + space + "--start-time=" + t1 + space + "--stop-time=" + t2 + space + "--video-on-top";    
					vlc_command_line_option = quote + vlc_executable + quote + space + "--play-and-exit" + space + quote + video_filename + quote + space + "--start-time=" + t1 + space + "--video-on-top";    
				}
                else if (i == end_video_file_no)
				{
					video_filename = video_files_dir_name + "nrva" + decimal_to_hex(i) + ".avi";
					//itoa(0, t1, 10);
					itoa((int)end_video_time, t2, 10);
					//vlc_command_line_option = quote + vlc_executable + quote + space + "--play-and-exit" + space + quote + video_filename + quote + space + "--start-time=" + t1 + space + "--stop-time=" + t2 + space + "--video-on-top";    
					vlc_command_line_option = quote + vlc_executable + quote + space + "--play-and-exit" + space + quote + video_filename + quote + space + "--stop-time=" + t2 + space + "--video-on-top";    
				}
                else
				{
					video_filename = video_files_dir_name + "nrva" + decimal_to_hex(i) + ".avi";
					//itoa(0, t1, 10);
					//itoa((int)(video_file_duration[i]), t2, 10);
					//vlc_command_line_option = quote + vlc_executable + quote + space + "--play-and-exit" + space + quote + video_filename + quote + space + "--start-time=" + t1 + space + "--stop-time=" + t2 + space + "--video-on-top";    
					vlc_command_line_option = quote + vlc_executable + quote + space + "--play-and-exit" + space + quote + video_filename + quote + space + "--video-on-top";    
				}
                fprintf(fp, "%s\n", vlc_command_line_option);
            }
            fclose(fp);
        }
		//system(batch_filename); 
		WinExec(batch_filename, 0);
    }
}