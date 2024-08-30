// ReadandProcessEEGData.cpp : implementation file
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

void CEEG_marking_toolDlg::Read_and_Process_EEG_Data(void)
{
	int ch_no;
	FILE *fid;
	int noi, nof;
	CvMat *mat_train_feat1;
	CvMat *mat_train_grp1;
	CvMat *mat_train_feat2;
	CvMat *mat_train_grp2;
	CvSVMParams svm_params;

	if (fileName.Find(".txt") != -1)
	{
		// filter the input raw EEG data			
		Filter_Amp_EEG_Data();		

		// rectify, smooth and logarithmic scale adaptation - it'll produce aEEG data format
		rectify_amp_eeg_data();		

    /********************************/
		// determine the sleep wave cycle based on bandwidth determination of the derived aEEG data
		// single channel based
# if 0		
		for (ch_no = 0; ch_no < no_of_channels; ch_no++)
			aEEG_SWC_determine(ch_no);
# endif
    /********************************/
	}
	//AfxMessageBox("Amp EEG Data Read + Filtering Done");

	// following code checks whether file containing burst detection result already exists or not
	// if not so, then we detect burst
	
	CString outfile = directory_name + output_text_filename; 	
	CFileStatus status;

	//if we read input text file and output marking file does not exist then only we proces the data
	if ((fileName.Find(".txt") != -1) && (!(CFile::GetStatus(outfile, status))))	
	{
		/////////////////////////////////////////////////////
		// burst detection

		// initialize the SVM parameters
		// for burst detection
		svm_params.svm_type = CvSVM::C_SVC;
		svm_params.kernel_type = CvSVM::RBF;
		//svm_params.degree = 3;
		//svm_params.coef0 = 1;
		svm_params.gamma = (0.2 * 0.000001);
		//svm_params.nu = 0.5;
		svm_params.C = 100.0;
		svm_params.term_crit.epsilon = 0.0001;
		svm_params.term_crit.max_iter = 15000;
		svm_params.term_crit.type = CV_TERMCRIT_ITER|CV_TERMCRIT_EPS;
		svm_params.class_weights = NULL;	
		
		// initialize SVM classifier
		svm_classifier_burst_detection = new CvSVM();

		// comment - sourya
		// train the classifier for burst detection
		// Classifier_Training(burst_detection_training_filename, no_of_feature_burst_detection, no_of_instance_burst_detection, svm_classifier_burst_detection);

		// add - sourya
		fid = fopen(burst_detection_training_filename, "r");
		if (fid == NULL)
			AfxMessageBox("cannot open burst training file");

		// training feature matrix
		mat_train_feat1 = cvCreateMat(no_of_instance_burst_detection, no_of_feature_burst_detection, CV_32FC1);

		// training label matrix
		mat_train_grp1 = cvCreateMat(no_of_instance_burst_detection, 1, CV_32SC1);

		// test feature matrix
		mat_test_feat_burst = cvCreateMat(1, no_of_feature_burst_detection, CV_32FC1);

		// now read the data from the data file (training data)
		for(noi = 0; noi < no_of_instance_burst_detection; noi++)
		{
			for(nof = 0; nof < no_of_feature_burst_detection; nof++)
			{
				// assign the feature values for a particular instance
				fscanf(fid, "%f", &(mat_train_feat1->data.fl[noi * no_of_feature_burst_detection + nof]));
			}
			// assign the label for that instance
			fscanf(fid, "%d", &(mat_train_grp1->data.fl[noi]));	
		}

		// close the input file
		if (fid != NULL)
			fclose(fid);

		// now declare the SVM classifier
		// initialize by learning
		svm_classifier_burst_detection->train(mat_train_feat1, mat_train_grp1, 0, 0, svm_params); 

		// call the ROI detection routine 
		for (ch_no = 0; ch_no < no_of_channels; ch_no++)
			ROI_Detection(ch_no);
			
		//AfxMessageBox("ROI detection Done");

		// delete the classifier 
		cvReleaseMat(&mat_train_feat1);
		cvReleaseMat(&mat_train_grp1);
		cvReleaseMat(&mat_test_feat_burst);

		//delete svm_classifier_burst_detection;

		/////////////////////////////////////////////////////
		// artifact detection
		
		// initialize the parameters
		svm_params.gamma = (3.0 * 0.000001);
		svm_params.C = 1.0;

		// initialize SVM classifier
		svm_classifier_artifact_detection = new CvSVM();

		// comment - sourya
		// train the classifier for artifact detection
		// Classifier_Training(artifact_detection_training_filename, no_of_feature_artifact_detection, no_of_instance_artifact_detection, svm_classifier_artifact_detection);

		// add - sourya
		fid = fopen(artifact_detection_training_filename, "r");
		if (fid == NULL)
			AfxMessageBox("cannot open artifact training file");

		// training feature matrix
		mat_train_feat2 = cvCreateMat(no_of_instance_artifact_detection, no_of_feature_artifact_detection, CV_32FC1);

		// training label matrix
		mat_train_grp2 = cvCreateMat(no_of_instance_artifact_detection, 1, CV_32SC1);

		// test feature matrix
		mat_test_feat_artifact = cvCreateMat(1, no_of_feature_artifact_detection, CV_32FC1);

		// now read the data from the data file (training data)
		for(noi = 0; noi < no_of_instance_artifact_detection; noi++)
		{
			for(nof = 0; nof < no_of_feature_artifact_detection; nof++)
			{
				// assign the feature values for a particular instance
				fscanf(fid, "%f", &(mat_train_feat2->data.fl[noi * no_of_feature_artifact_detection + nof]));
			}
			// assign the label for that instance
			fscanf(fid, "%d", &(mat_train_grp2->data.fl[noi]));	
		}

		// close the input file
		if (fid != NULL)
			fclose(fid);

		// now declare the SVM classifier
		// initialize by learning
		svm_classifier_artifact_detection->train(mat_train_feat2, mat_train_grp2, 0, 0, svm_params); 

		// from detected burst portions, seperate artifacts
		for (ch_no = 0; ch_no < no_of_channels; ch_no++)
			artifact_detection(ch_no);
		
		//AfxMessageBox("Artifact detection Done");

		// delete the classifier 
		cvReleaseMat(&mat_train_feat2);
		cvReleaseMat(&mat_train_grp2);
		cvReleaseMat(&mat_test_feat_artifact);

		//delete svm_classifier_artifact_detection;
		
		/***************************************/

		// now detect the suppression patterns
		for (ch_no = 0; ch_no < no_of_channels; ch_no++)
			Suppression_Detection(ch_no);

		// after the pattern detection techniques, delete markings which are completely overlapping
		// with another marking - that is the start and end time of one marking is completeky within
		// the timing boundary of other marking
		DelOverlappingMark();

		// now merge the similar markings
		// burst, artifact, seizure, suppression etc
		MergeSimilarMark();

		// discard the suppression marks which have duration less than 5 minutes
		DiscardSmallSuppMark();

		// finally consider burst followed by suppression patterns
		// suppression preceding burst patterns are also considered
		burst_followed_by_suppression_detection();
		
		//AfxMessageBox("Burst suppression detection Done");

		// reset the suppression markings	
		ResetSpecifiedMark();

    /***************************************/
	}

	// we dont save the file for the moment - we will save it once save button is clicked
	plot_amp_EEG_signal();	//plot amp eeg signal

	// we dont save the file for the moment - we will save it once save button is clicked
  plot_raw_EEG_signal(0);	// now plot the raw EEG signal

} //end function
