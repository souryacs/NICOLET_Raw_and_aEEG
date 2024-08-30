// typepatientreport.cpp : implementation file
//

#include "stdafx.h"
#include "EEG_marking_tool.h"
#include "typepatientreport.h"
#include "EEG_marking_toolDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_LINE_LEN_REPORT 8192

/////////////////////////////////////////////////////////////////////////////
// typepatientreport dialog


typepatientreport::typepatientreport(CWnd* pParent /*=NULL*/)
	: CDialog(typepatientreport::IDD, pParent)
{
	//{{AFX_DATA_INIT(typepatientreport)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void typepatientreport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(typepatientreport)
	DDX_Control(pDX, IDC_DOB_YEAR_LIST, m_DOB_yr);
	DDX_Control(pDX, IDC_DOB_MONTH_LIST, m_DOB_month);
	DDX_Control(pDX, IDC_DOB_DATE_LIST, m_DOB_date);
	DDX_Control(pDX, IDC_EPILEPTIC, m_Epileptic_Info);
	DDX_Control(pDX, IDC_HOURS_SINCE_LAST_MEAL, m_HrSinceLastMeal);
	DDX_Control(pDX, IDC_PATIENT_SEX, m_patSex);
	DDX_Control(pDX, IDC_GESTATIONAL_AGE_LIST, m_GestationAge);
	DDX_Control(pDX, IDC_RECORDING_DURATION_SECONDS, m_Record_Dur_Second);
	DDX_Control(pDX, IDC_RECORDING_DURATION_MINUTES, m_Record_Dur_Min);
	DDX_Control(pDX, IDC_RECORDING_DURATION_HOUR, m_Record_Dur_Hr);
	DDX_Control(pDX, IDC_EDIT_PATIENT_AGE, m_PatCurrAge);
	DDX_Control(pDX, IDC_EDIT_RECORDING_TECHNICIAN, m_RecordingTechnician);
	DDX_Control(pDX, IDC_EDIT_UNDER_DOCTOR_NAME, m_UnderDoctorName);
	DDX_Control(pDX, IDC_EDIT_PATIENT_ADDRESS, m_PatientAddress);
	DDX_Control(pDX, IDC_EDIT_MEDICATION_INFO, m_MedicationInfo);
	DDX_Control(pDX, IDC_EDIT_HOSPITAL_NAME, m_HospName);
	DDX_Control(pDX, IDC_EDIT_EXAM_NOTES, m_PatExamNotes);
	DDX_Control(pDX, IDC_EDIT_PATIENT_NAME, m_PatName);
	DDX_Control(pDX, IDC_EDIT_PATIENT_ID, m_PatID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(typepatientreport, CDialog)
	//{{AFX_MSG_MAP(typepatientreport)
	ON_BN_CLICKED(IDC_PATIENT_REPORT_SAVE, OnPatientReportSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// typepatientreport message handlers

void typepatientreport::OnPatientReportSave() 
{
	// TODO: Add your control notification handler code here

	// all the edit box items are manipulated here
	m_PatID.GetWindowText(patient_ID);		
	m_PatName.GetWindowText(patient_name);
	m_UnderDoctorName.GetWindowText(doctor_name);
	m_RecordingTechnician.GetWindowText(technician_name);
	m_HospName.GetWindowText(hospital_name);
	m_PatCurrAge.GetWindowText(patient_age);
	m_PatientAddress.GetWindowText(patient_address);
	m_MedicationInfo.GetWindowText(patient_medication);
	m_PatExamNotes.GetWindowText(patient_exam_notes);

	// all the combo box items are manipulated here
	// recording duration
	m_Record_Dur_Hr.GetLBText(m_Record_Dur_Hr.GetCurSel(), record_duration_hr);
	m_Record_Dur_Min.GetLBText(m_Record_Dur_Min.GetCurSel(), record_duration_min);
	m_Record_Dur_Second.GetLBText(m_Record_Dur_Second.GetCurSel(), record_duration_sec);

	// date of birth
	m_DOB_date.GetLBText(m_DOB_date.GetCurSel(), patient_dob_yr);
	m_DOB_month.GetLBText(m_DOB_month.GetCurSel(), patient_dob_month);
	m_DOB_yr.GetLBText(m_DOB_yr.GetCurSel(), patient_dob_date);

	// gestational age
	m_GestationAge.GetLBText(m_GestationAge.GetCurSel(), patient_gestational_age_week);

	// patient sex
	m_patSex.GetLBText(m_patSex.GetCurSel(), patient_sex);

	// hours since last meal
	m_HrSinceLastMeal.GetLBText(m_HrSinceLastMeal.GetCurSel(), hr_since_last_meal);

	// epileptic information
	m_Epileptic_Info.GetLBText(m_Epileptic_Info.GetCurSel(), epileptic);


	// now save all the informations in a file 
	FILE *fod;
	//CString dir_name = CEEG_marking_toolDlg::directory_name;
	//CString out_report_filename = dir_name + "patient_report.rpt";
	CString out_report_filename = "patient_report.rpt";
	fod = fopen(out_report_filename, "w");
	if (fod == NULL)	//file open error
	{
		AfxMessageBox("Cannot Open the report generation file!");
	}

	// put all the informations in the output report file
	fprintf(fod, "%s\n", patient_ID);
	fprintf(fod, "%s\n", patient_name);
	fprintf(fod, "%s\n", doctor_name);
	fprintf(fod, "%s\n", technician_name);
	fprintf(fod, "%s\n", hospital_name);
	fprintf(fod, "%s\n", record_duration_hr);
	fprintf(fod, "%s\n", record_duration_min);
	fprintf(fod, "%s\n", record_duration_sec);
	fprintf(fod, "%s\n", patient_dob_yr);
	fprintf(fod, "%s\n", patient_dob_month);
	fprintf(fod, "%s\n", patient_dob_date);
	fprintf(fod, "%s\n", patient_gestational_age_week);
	fprintf(fod, "%s\n", patient_sex);
	fprintf(fod, "%s\n", hr_since_last_meal);
	fprintf(fod, "%s\n", patient_age);
	fprintf(fod, "%s\n", epileptic);
	fprintf(fod, "%s\n", patient_address);
	fprintf(fod, "%s\n", patient_medication);
	fprintf(fod, "%s", patient_exam_notes);

	fclose(fod);	//close the file

	// notification for report save
	AfxMessageBox("Patient Report is saved!");

}

BOOL typepatientreport::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	// here we manually set the different report generation item configurations
	int i;
	CString buf;

	// combo box recording duration
	for (i = 0; i <= 99; i++)
	{
		buf.Format(_T("%d"), i);
		m_Record_Dur_Hr.AddString(buf);
	}
	m_Record_Dur_Hr.SetCurSel(0);	//show the 1st item

	for (i = 0; i <= 59; i++)
	{
		buf.Format(_T("%d"), i);
		m_Record_Dur_Min.AddString(buf);
	}
	m_Record_Dur_Min.SetCurSel(0);	//show the 1st item

	for (i = 0; i <= 59; i++)
	{
		buf.Format(_T("%d"), i);
		m_Record_Dur_Second.AddString(buf);
	}
	m_Record_Dur_Second.SetCurSel(0);	//show the 1st item

	// combo box gestational age (weeks)
	for (i = 20; i <= 45; i++)
	{
		buf.Format(_T("%d"), i);
		m_GestationAge.AddString(buf);
	}
	m_GestationAge.SetCurSel(0);	//show the 1st item

	// combo box patient sex 
	CString temp = "Male";
	m_patSex.AddString(temp);
	temp = "Female";
	m_patSex.AddString(temp);
	m_patSex.SetCurSel(0);	//show the 1st item

	// combo box hours since last meal
	for (i = 0; i <= 23; i++)
	{
		buf.Format(_T("%d"), i);
		m_HrSinceLastMeal.AddString(buf);
	}
	m_HrSinceLastMeal.SetCurSel(0);	//show the 1st item

	// combo box - epileptic (boolean) information
	temp = "Yes";
	m_Epileptic_Info.AddString(temp);
	temp = "No";
	m_Epileptic_Info.AddString(temp);
	m_Epileptic_Info.SetCurSel(0);	//show the 1st item

	//combo boxes for mentioning the date of birth
	
	// date
	for (i = 1; i <= 31; i++)
	{
		buf.Format(_T("%d"), i);
		m_DOB_date.AddString(buf);
	}
	m_DOB_date.SetCurSel(0);	//show the 1st item
	
	// month
	m_DOB_month.AddString("January");
	m_DOB_month.AddString("February");
	m_DOB_month.AddString("March");
	m_DOB_month.AddString("April");
	m_DOB_month.AddString("May");
	m_DOB_month.AddString("June");
	m_DOB_month.AddString("July");
	m_DOB_month.AddString("August");
	m_DOB_month.AddString("September");
	m_DOB_month.AddString("October");
	m_DOB_month.AddString("November");
	m_DOB_month.AddString("December");
	m_DOB_month.SetCurSel(0);	//show the 1st item
	
	//year
	for (i = 1900; i <= 2100; i++)
	{
		buf.Format(_T("%d"), i);
		m_DOB_yr.AddString(buf);
	}	
	m_DOB_yr.SetCurSel(111);	//show the 1st item


	// here we search for existing report 
	// if we find then we insert the information in to current dialog box
	// for user visibility and editing (if any)
	FILE *fid;
	CString out_report_filename = "patient_report.rpt";
	CFileStatus status;
	char textLine[MAX_LINE_LEN_REPORT];
	char* token;

	if (CFile::GetStatus(out_report_filename, status))
	{
		fid = fopen(out_report_filename, "r");

		// scan individual fields and put those informations in the respective dialog controls		
		fgets(textLine, 20, fid);
		token = strtok(textLine, "\n");
		patient_ID = token;
		m_PatID.SetWindowText(patient_ID);

		fgets(textLine, 100, fid);
		token = strtok(textLine, "\n");
		patient_name = token;
		m_PatName.SetWindowText(patient_name);

		fgets(textLine, 100, fid);
		token = strtok(textLine, "\n");
		doctor_name = token;
		m_UnderDoctorName.SetWindowText(doctor_name);

		fgets(textLine, 100, fid);
		token = strtok(textLine, "\n");
		technician_name = token;
		m_RecordingTechnician.SetWindowText(technician_name);

		fgets(textLine, 100, fid);
		token = strtok(textLine, "\n");
		hospital_name = token;
		m_HospName.SetWindowText(hospital_name);

		fgets(textLine, 10, fid);
		token = strtok(textLine, "\n");
		record_duration_hr = token;
		m_Record_Dur_Hr.SetCurSel(m_Record_Dur_Hr.FindStringExact(0, record_duration_hr));

		fgets(textLine, 10, fid);
		token = strtok(textLine, "\n");
		record_duration_min = token;
		m_Record_Dur_Min.SetCurSel(m_Record_Dur_Min.FindStringExact(0, record_duration_min));

		fgets(textLine, 10, fid);
		token = strtok(textLine, "\n");
		record_duration_sec = token;
		m_Record_Dur_Second.SetCurSel(m_Record_Dur_Second.FindStringExact(0, record_duration_sec));

		fgets(textLine, 10, fid);
		token = strtok(textLine, "\n");
		patient_dob_date = token;
		m_DOB_date.SetCurSel(m_DOB_date.FindStringExact(0, patient_dob_date));

		fgets(textLine, 20, fid);
		token = strtok(textLine, "\n");
		patient_dob_month = token;
		m_DOB_month.SetCurSel(m_DOB_month.FindStringExact(0, patient_dob_month));

		fgets(textLine, 10, fid);
		token = strtok(textLine, "\n");
		patient_dob_yr = token;
		m_DOB_yr.SetCurSel(m_DOB_yr.FindStringExact(0, patient_dob_yr));

		fgets(textLine, 10, fid);
		token = strtok(textLine, "\n");
		patient_gestational_age_week = token;
		m_GestationAge.SetCurSel(m_GestationAge.FindStringExact(0, patient_gestational_age_week));

		fgets(textLine, 10, fid);
		token = strtok(textLine, "\n");
		patient_sex = token;
		m_patSex.SetCurSel(m_patSex.FindStringExact(0, patient_sex));

		fgets(textLine, 10, fid);
		token = strtok(textLine, "\n");
		hr_since_last_meal = token;
		m_HrSinceLastMeal.SetCurSel(m_HrSinceLastMeal.FindStringExact(0, hr_since_last_meal));

		fgets(textLine, 100, fid);
		token = strtok(textLine, "\n");
		patient_age = token;
		m_PatCurrAge.SetWindowText(patient_age);

		fgets(textLine, 10, fid);
		token = strtok(textLine, "\n");
		epileptic = token;
		m_Epileptic_Info.SetCurSel(m_Epileptic_Info.FindStringExact(0, epileptic));

		fgets(textLine, MAX_LINE_LEN_REPORT, fid);
		token = strtok(textLine, "\n");
		patient_address = token;
		m_PatientAddress.SetWindowText(patient_address);

		fgets(textLine, MAX_LINE_LEN_REPORT, fid);
		token = strtok(textLine, "\n");
		patient_medication = token;
		m_MedicationInfo.SetWindowText(patient_medication);

		fgets(textLine, MAX_LINE_LEN_REPORT, fid);
		token = strtok(textLine, "\n");
		patient_exam_notes = token;
		m_PatExamNotes.SetWindowText(patient_exam_notes);

		fclose(fid);
	}	// end check for file existence


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
