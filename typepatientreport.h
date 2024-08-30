#if !defined(AFX_TYPEPATIENTREPORT_H__CD7B0570_DBFE_4804_B758_16062B02BE02__INCLUDED_)
#define AFX_TYPEPATIENTREPORT_H__CD7B0570_DBFE_4804_B758_16062B02BE02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// typepatientreport.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// typepatientreport dialog

class typepatientreport : public CDialog
{
// Construction
public:
	typepatientreport(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(typepatientreport)
	enum { IDD = IDD_DIALOG_TYPEPATIENTREPORT };
	CComboBox	m_DOB_yr;
	CComboBox	m_DOB_month;
	CComboBox	m_DOB_date;
	CComboBox	m_Epileptic_Info;
	CComboBox	m_HrSinceLastMeal;
	CComboBox	m_patSex;
	CComboBox	m_GestationAge;
	CComboBox	m_Record_Dur_Second;
	CComboBox	m_Record_Dur_Min;
	CComboBox	m_Record_Dur_Hr;
	CEdit	m_PatCurrAge;
	CEdit	m_RecordingTechnician;
	CEdit	m_UnderDoctorName;
	CEdit	m_PatientAddress;
	CEdit	m_MedicationInfo;
	CEdit	m_HospName;
	CEdit	m_PatExamNotes;
	CEdit	m_PatName;
	CEdit	m_PatID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(typepatientreport)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation



	///////////////////////////
	// add - sourya
	// variables to be used in the patient report generation
	CString patient_ID;
	CString patient_name;	
	CString doctor_name;	
	CString technician_name;	
	CString hospital_name;	
	CString record_duration_hr;
	CString record_duration_min;
	CString record_duration_sec;
	CString patient_dob_yr;
	CString patient_dob_month;
	CString patient_dob_date;
	CString patient_gestational_age_week;
	CString patient_age;
	CString patient_sex;	// 1 - male  2 - female
	CString hr_since_last_meal;
	CString epileptic;	// 1 - yes, 0 - no
	CString patient_address;
	CString patient_medication;
	CString patient_exam_notes;
	///////////////////////////

protected:

	// Generated message map functions
	//{{AFX_MSG(typepatientreport)
	afx_msg void OnPatientReportSave();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TYPEPATIENTREPORT_H__CD7B0570_DBFE_4804_B758_16062B02BE02__INCLUDED_)
