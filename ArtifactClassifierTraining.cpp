// ArtifactClassifierTraining.cpp : implementation file
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
	classifier training routine
	this file is used as a general file for training of input data samples in SVM
	thus it is used for burst and artifact detection
	input is taken as the training filename, as well as the no of features and instance
	output is returned as the trained classifier which will be used for classifying new samples

	it returns the SVM classifier instance which will be used in respective burst or artifact detection
	module
*/

void CEEG_marking_toolDlg::Classifier_Training(CString training_filename, int no_of_feature, int no_of_instance, CvSVM *svm_classifier)
{
	FILE *fid;
	int noi, nof;
	CvMat *mat_train_feat;
	CvMat *mat_train_grp;
	CvSVMParams svm_params;


	///////////////////////
	// old code - not used 
# if 0

	// read all features of all training instances in excel file
	fid = fopen(training_filename, "r");
	if (fid == NULL)
		AfxMessageBox("cannot open training file");


	// training feature matrix
	mat_train_feat = cvCreateMat(no_of_instance, no_of_feature, CV_32FC1);

	// training label matrix
	mat_train_grp = cvCreateMat(no_of_instance, 1, CV_32SC1);

	// now read the data from the data file (training data)
	for(noi = 0; noi < no_of_instance; noi++)
	{
		for(nof = 0; nof < no_of_feature; nof++)
		{
			// assign the feature values for a particular instance
			fscanf(fid, "%f", &(mat_train_feat->data.fl[noi * no_of_feature + nof]));
		}
		// assign the label for that instance
		fscanf(fid, "%d", &(mat_train_grp->data.fl[noi]));	
	}

	// close the input file
	if (fid != NULL)
		fclose(fid);

# endif


	///////////////////////
	// temporary code for cross validation
	// check accuracy of the classifier

	// read all features of all training instances in excel file
	fid = fopen(training_filename, "r");
	if (fid == NULL)
		AfxMessageBox("cannot open training file");

	float temp1;
	int temp2;
	// sourya - train with half of the data set
	// training feature matrix
	mat_train_feat = cvCreateMat((no_of_instance / 2 + 1), no_of_feature, CV_32FC1);

	// training label matrix
	mat_train_grp = cvCreateMat((no_of_instance / 2 + 1), 1, CV_32SC1);

	// now read the data from the data file (training data)
	int x = 0;
	for(noi = 0; noi < no_of_instance; noi++)
	{
		if (noi % 2 == 1)
		{
			for(nof = 0; nof < no_of_feature; nof++)
			{
				// assign the feature values for a particular instance
				fscanf(fid, "%f", &(mat_train_feat->data.fl[x * no_of_feature + nof]));
			}
			// assign the label for that instance
			fscanf(fid, "%d", &(mat_train_grp->data.fl[x]));
			x++;
		}
		else
		{
			for(nof = 0; nof < no_of_feature; nof++)
				fscanf(fid, "%f", &temp1);
			fscanf(fid, "%d", &temp2);
		}
	}

	// close the input file
	if (fid != NULL)
		fclose(fid);

	////////////////////////
	// a new code for cross validation of the input training samples

	CString out_text_file = directory_name + "artifact_detect_classifier_RBF_kernel_test.txt";
	double sens, spec;

	//double gamma_arr[15] = {0.001, 0.01, 0.1, 0.2, 0.4, 0.6, 0.8, 1, 2, 3, 4, 5, 10, 20, 50};
	//double multiplier_arr[15] = {1, 0.1, 0.001, 0.0001, 5, 10, (1.0 / no_of_instance), (1.0 / (no_of_instance * 10)), (1.0 / (no_of_instance * 100)), (1.0 / (no_of_instance * 500)), (1.0 / (no_of_instance * 1000)), (1.0 / (no_of_instance * 5000)), (1.0 / (no_of_instance * 10000)), (1.0 / (no_of_instance * 50000)), (1.0 / (no_of_instance * 100000))};
	//double C_arr[12] = {1, 5, 10, 100, 1000, 10000, 0.1, 0.01, 0.001, 0.0001, 0.00001, (1.0 / no_of_instance)};

	double gamma_arr[20] = {pow(2, -15), pow(2, -14), pow(2, -13), pow(2, -12), pow(2, -11), pow(2, -10), pow(2, -9), pow(2, -8), pow(2, -7), pow(2, -6), pow(2, -5), pow(2, -4), pow(2, -3), pow(2, -2), pow(2, -1), 1, 2, 3, 4, 5};
	double C_arr[21] = {pow(2, -5), pow(2, -4), pow(2, -3), pow(2, -2), pow(2, -1), 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, pow(2, 13), pow(2, 14), pow(2, 15)};


	// sourya
	FILE *fp = fopen(out_text_file, "w");

	// add - sourya

	int i, j;
	for (i = 0; i < 20; i++)
	{		
		for (j = 0; j < 21; j++)
		{
			// initialize the SVM parameters
			svm_params.svm_type = CvSVM::C_SVC;
			svm_params.kernel_type = CvSVM::RBF;
			//svm_params.degree = 2;
			//svm_params.coef0 = 1;
			svm_params.gamma = gamma_arr[i];	//0.4;
			//svm_params.nu = 0.5;
			svm_params.C = C_arr[j];	//1;
			svm_params.term_crit.epsilon = 0.0001;
			svm_params.term_crit.max_iter = 15000;
			svm_params.term_crit.type = CV_TERMCRIT_ITER|CV_TERMCRIT_EPS;
			svm_params.class_weights = NULL;	//class_weights;

			//CvSVMParams svm_params(CvSVM::C_SVC, CvSVM::RBF, 0, 0.4, 0, 1, 0, 0, 0, cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 15000, FLT_EPSILON));

			// now declare the SVM classifier
			// initialize by learning
			svm_classifier->train(mat_train_feat, mat_train_grp, 0, 0, svm_params); 

			// temporary code for cross validation
			// check accuracy of the classifier

			// add - sourya
			// cross validation with half of the data
			fid = fopen(training_filename, "r");

			CvMat *mat_test_feat;
			int orig_label;
			int predicted_label;
			int match_count_orig0_got0 = 0, match_count_orig1_got1 = 0;
			int mismatch_count_orig0_got1 = 0, mismatch_count_orig1_got0 = 0;

			mat_test_feat = cvCreateMat(1, no_of_feature, CV_32FC1);

			for(noi = 0; noi < no_of_instance; noi++)
			{
				if (noi % 2 == 0)
				{
					for(nof = 0; nof < no_of_feature; nof++)
					{
						// assign the feature values for a particular instance
						fscanf(fid, "%f", &(mat_test_feat->data.fl[nof]));
					}
					// assign the label for that instance
					fscanf(fid, "%d", &orig_label);

					// test the classifier 
					predicted_label = svm_classifier->predict(mat_test_feat);

					if (predicted_label == orig_label)
					{
						if (orig_label == 0)
							match_count_orig0_got0++;
						else
							match_count_orig1_got1++;
					}
					else
					{
						if (orig_label == 0)
							mismatch_count_orig0_got1++;
						else
							mismatch_count_orig1_got0++;
					}
				}
				else
				{
					for(nof = 0; nof < no_of_feature; nof++)
						fscanf(fid, "%f", &temp1);
					fscanf(fid, "%d", &temp2);
				}
			}


			sens = (match_count_orig1_got1 * 1.0 ) / (match_count_orig1_got1 + mismatch_count_orig1_got0);
			spec = (match_count_orig0_got0 * 1.0 ) / (match_count_orig0_got0 + mismatch_count_orig0_got1);


			if (fid != NULL)
				fclose(fid);

			// write
			//fprintf(fp, "\n linear kernel - C val : %lf match_count_orig0_got0 : %d match_count_orig1_got1 : %d mismatch_count_orig0_got1 : %d mismatch_count_orig1_got0 : %d sens : %lf spec : %lf wss : %lf ", 
			//	C_arr[j], match_count_orig0_got0, match_count_orig1_got1, mismatch_count_orig0_got1, mismatch_count_orig1_got0, sens, spec, (0.5 * sens + 0.5 * spec));

			fprintf(fp, "\n RBF kernel - gamma val : %lf  C val : %lf match_count_orig0_got0 : %d match_count_orig1_got1 : %d mismatch_count_orig0_got1 : %d mismatch_count_orig1_got0 : %d sens : %lf spec : %lf wss : %lf ", 
				gamma_arr[i], C_arr[j], match_count_orig0_got0, match_count_orig1_got1, mismatch_count_orig0_got1, mismatch_count_orig1_got0, sens, spec, (0.5 * sens + 0.5 * spec));
			
			
		} //end j loop
	} // end i loop

	fclose(fp);
	// end debug - sourya
	/////////////////////// 

}



# if 0
void CEEG_marking_toolDlg::Classifier_Training(CString training_filename, float **training_feature, int *training_group, int no_of_feature, int no_of_instance, CvSVM *svm_classifier)
{
	FILE *fid;
	int noi, nof;
	CvMat mat_train_feat, mat_train_grp;

	// read all features of all gtraining instances in excel file
	fid = fopen(training_filename, "r");
	if (fid == NULL)
		AfxMessageBox("cannot open training file");

	// read the instance and the features
	for (noi = 0; noi < no_of_instance; noi++)
	{
		for (nof = 0; nof < no_of_feature; nof++) 
		{
			fscanf(fid, "%f", &training_feature[noi][nof]);
		}
		fscanf(fid, "%d", &training_group[noi]);	
	}

	if (fid != NULL)
		fclose(fid);
	// end of data read

	// initialize the matrices for storage of training data
	// these matrices will be used for svm_train function 
	// training feature matrix
	cvInitMatHeader(&mat_train_feat, no_of_instance, no_of_feature, CV_32FC1, training_feature);
	// training label matrix
	cvInitMatHeader(&mat_train_grp, no_of_instance, 1, CV_32SC1, training_group);

	// now call the SVM train function
	// initialize the SVM parameters
	// gamma is taken as 0.4; RBF kernel is used
	CvSVMParams svm_params(CvSVM::C_SVC, CvSVM::RBF, 0, 0.4, 0, 1, 0, 0, 0,
		cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000, FLT_EPSILON));

	// now declare the SVM classifier
	// initialize by learning
	svm_classifier->train(&mat_train_feat, &mat_train_grp, 0, 0, svm_params); 

}	// end of classifier training routine
# endif