// BrushSettingsDlg.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "afxdialogex.h"
#include "BrushSettingsDlg.h"


// CBrushSettingsDlg dialog

IMPLEMENT_DYNAMIC(CBrushSettingsDlg, CDialog)

CBrushSettingsDlg::CBrushSettingsDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_BRUSHSETTINGS, pParent)
{

}

CBrushSettingsDlg::~CBrushSettingsDlg()
{
}

void CBrushSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	float minyaw, maxyaw;
	float minpitch, maxpitch;
	float minroll, maxroll;
	float minsclx, maxsclx;
	float minscly, maxscly;
	float minsclz, maxsclz;
	int yaw, pitch, roll;
	int sclx, scly, sclz;
	int lockxy, lockyz;
	int yawdetent, pitchdetent, rolldetent;
	int yawdetentval, pitchdetentval, rolldetentval;
	int placemode;
	int snapx, snapy, snapz;
	float snapxval, snapyval, snapzval;
	

	if (!pDX->m_bSaveAndValidate)
	{
		yaw = theApp.m_Config->GetBool(_T("brush.yaw.apply"), true);
		minyaw = theApp.m_Config->GetFloat(_T("brush.yaw.min"), 0.0f);
		maxyaw = theApp.m_Config->GetFloat(_T("brush.yaw.max"), 360.0f);

		yawdetent = theApp.m_Config->GetBool(_T("brush.yaw.detents.apply"), false);
		yawdetentval = (int)theApp.m_Config->GetInt(_T("brush.yaw.detents.val"), 4);

		pitch = theApp.m_Config->GetBool(_T("brush.pitch.apply"), false);
		minpitch = theApp.m_Config->GetFloat(_T("brush.pitch.min"), 0.0f);
		maxpitch = theApp.m_Config->GetFloat(_T("brush.pitch.max"), 0.0f);

		pitchdetent = theApp.m_Config->GetBool(_T("brush.pitch.detents.apply"), false);
		pitchdetentval = (int)theApp.m_Config->GetInt(_T("brush.pitch.detents.val"), 4);

		roll = theApp.m_Config->GetBool(_T("brush.roll.apply"), false);
		minroll = theApp.m_Config->GetFloat(_T("brush.roll.min"), 0.0f);
		maxroll = theApp.m_Config->GetFloat(_T("brush.roll.max"), 0.0f);

		rolldetent = theApp.m_Config->GetBool(_T("brush.roll.detents.apply"), false);
		rolldetentval = (int)theApp.m_Config->GetInt(_T("brush.roll.detents.val"), 4);

		sclx = theApp.m_Config->GetBool(_T("brush.scale.x.apply"), true);
		minsclx = theApp.m_Config->GetFloat(_T("brush.scale.x.min"), 1.0f);
		maxsclx = theApp.m_Config->GetFloat(_T("brush.scale.x.max"), 1.0f);

		scly = theApp.m_Config->GetBool(_T("brush.scale.y.apply"), true);
		minscly = theApp.m_Config->GetFloat(_T("brush.scale.y.min"), 1.0f);
		maxscly = theApp.m_Config->GetFloat(_T("brush.scale.y.max"), 1.0f);

		sclz = theApp.m_Config->GetBool(_T("brush.scale.z.apply"), true);
		minsclz = theApp.m_Config->GetFloat(_T("brush.scale.z.min"), 1.0f);
		maxsclz = theApp.m_Config->GetFloat(_T("brush.scale.z.max"), 1.0f);

		lockxy = theApp.m_Config->GetBool(_T("brush.scale.lockxy"), true);
		lockyz = theApp.m_Config->GetBool(_T("brush.scale.lockyz"), false);

		placemode = (int)theApp.m_Config->GetInt(_T("brush.placemode"), PlacementMode::PLACEMENT_RAYCAST);

		snapx = theApp.m_Config->GetBool(_T("brush.snap.x.apply"), true);
		snapxval = theApp.m_Config->GetFloat(_T("brush.snap.x.val"), 1.0f);

		snapy = theApp.m_Config->GetBool(_T("brush.snap.y.apply"), true);
		snapyval = theApp.m_Config->GetFloat(_T("brush.snap.y.val"), 1.0f);

		snapz = theApp.m_Config->GetBool(_T("brush.snap.z.apply"), true);
		snapzval = theApp.m_Config->GetFloat(_T("brush.snap.z.val"), 1.0f);
	}

	DDX_Check(pDX, IDC_CHECK_YAW, yaw);
	DDX_Text(pDX, IDC_EDIT_MINYAW, minyaw);
	DDV_MinMaxFloat(pDX, minyaw, 0.0f, 360.0f);
	DDX_Text(pDX, IDC_EDIT_MAXYAW, maxyaw);
	DDV_MinMaxFloat(pDX, maxyaw, 0.0f, 360.0f);
	DDX_Check(pDX, IDC_CHECK_YAW_DETENTS, yawdetent);
	DDX_Text(pDX, IDC_EDIT_YAW_DETENTS, yawdetentval);
	DDV_MinMaxInt(pDX, yawdetentval, 1, 360);
	if (minyaw > maxyaw)
		std::swap(minyaw, maxyaw);

	DDX_Check(pDX, IDC_CHECK_PITCH, pitch);
	DDX_Text(pDX, IDC_EDIT_MINPITCH, minpitch);
	DDV_MinMaxFloat(pDX, minpitch, 0.0f, 360.0f);
	DDX_Text(pDX, IDC_EDIT_MAXPITCH, maxpitch);
	DDV_MinMaxFloat(pDX, maxpitch, 0.0f, 360.0f);
	DDX_Check(pDX, IDC_CHECK_PITCH_DETENTS, pitchdetent);
	DDX_Text(pDX, IDC_EDIT_PITCH_DETENTS, pitchdetentval);
	DDV_MinMaxInt(pDX, pitchdetentval, 1, 360);
	if (minpitch > maxpitch)
		std::swap(minpitch, maxpitch);

	DDX_Check(pDX, IDC_CHECK_ROLL, roll);
	DDX_Text(pDX, IDC_EDIT_MINROLL, minroll);
	DDV_MinMaxFloat(pDX, minroll, 0.0f, 360.0f);
	DDX_Text(pDX, IDC_EDIT_MAXROLL, maxroll);
	DDV_MinMaxFloat(pDX, maxroll, 0.0f, 360.0f);
	DDX_Check(pDX, IDC_CHECK_ROLL_DETENTS, rolldetent);
	DDX_Text(pDX, IDC_EDIT_ROLL_DETENTS, rolldetentval);
	DDV_MinMaxInt(pDX, rolldetentval, 1, 360);
	if (minroll > maxroll)
		std::swap(minroll, maxroll);

	DDX_Check(pDX, IDC_CHECK_SCLX, sclx);
	DDX_Text(pDX, IDC_EDIT_MINSCLX, minsclx);
	DDV_MinMaxFloat(pDX, minsclx, -100.0f, 100.0f);
	DDX_Text(pDX, IDC_EDIT_MAXSCLX, maxsclx);
	DDV_MinMaxFloat(pDX, maxsclx, -100.0f, 100.0f);

	DDX_Check(pDX, IDC_CHECK_SCLY, scly);
	DDX_Text(pDX, IDC_EDIT_MINSCLY, minscly);
	DDV_MinMaxFloat(pDX, minscly, -100.0f, 100.0f);
	DDX_Text(pDX, IDC_EDIT_MAXSCLY, maxscly);
	DDV_MinMaxFloat(pDX, maxscly, -100.0f, 100.0f);

	DDX_Check(pDX, IDC_CHECK_SCLZ, sclz);
	DDX_Text(pDX, IDC_EDIT_MINSCLZ, minsclz);
	DDV_MinMaxFloat(pDX, minsclz, -100.0f, 100.0f);
	DDX_Text(pDX, IDC_EDIT_MAXSCLZ, maxsclz);
	DDV_MinMaxFloat(pDX, maxsclz, -100.0f, 100.0f);

	DDX_Check(pDX, IDC_CHECK_LOCKXY, lockxy);
	DDX_Check(pDX, IDC_CHECK_LOCKYZ, lockyz);

	DDX_CBIndex(pDX, IDC_COMBO_PLACEMENTMODE, placemode);
	DDX_Check(pDX, IDC_CHECK_SNAPX, snapx);
	DDX_Text(pDX, IDC_EDIT_SNAPX, snapxval);
	DDV_MinMaxFloat(pDX, snapxval, 0.1f, 100.0f);

	DDX_Check(pDX, IDC_CHECK_SNAPY, snapy);
	DDX_Text(pDX, IDC_EDIT_SNAPY, snapyval);
	DDV_MinMaxFloat(pDX, snapyval, 0.1f, 100.0f);

	DDX_Check(pDX, IDC_CHECK_SNAPZ, snapz);
	DDX_Text(pDX, IDC_EDIT_SNAPZ, snapzval);
	DDV_MinMaxFloat(pDX, snapzval, 0.1f, 100.0f);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.m_Config->SetBool(_T("brush.yaw.apply"), yaw ? true : false);
		theApp.m_Config->SetFloat(_T("brush.yaw.min"), minyaw);
		theApp.m_Config->SetFloat(_T("brush.yaw.max"), maxyaw);
		theApp.m_Config->SetBool(_T("brush.yaw.detents.apply"), yawdetent ? true : false);
		theApp.m_Config->SetInt(_T("brush.yaw.detents.val"), yawdetentval);

		theApp.m_Config->SetBool(_T("brush.pitch.apply"), pitch ? true : false);
		theApp.m_Config->SetFloat(_T("brush.pitch.min"), minpitch);
		theApp.m_Config->SetFloat(_T("brush.pitch.max"), maxpitch);
		theApp.m_Config->SetBool(_T("brush.pitch.detents.apply"), pitchdetent ? true : false);
		theApp.m_Config->SetInt(_T("brush.pitch.detents.val"), pitchdetentval);

		theApp.m_Config->GetBool(_T("brush.roll.apply"), roll ? true : false);
		theApp.m_Config->SetFloat(_T("brush.roll.min"), minroll);
		theApp.m_Config->SetFloat(_T("brush.roll.max"), maxroll);
		theApp.m_Config->SetBool(_T("brush.roll.detents.apply"), rolldetent ? true : false);
		theApp.m_Config->SetInt(_T("brush.roll.detents.val"), rolldetentval);

		theApp.m_Config->SetBool(_T("brush.scale.x.apply"), sclx ? true : false);
		theApp.m_Config->SetFloat(_T("brush.scale.x.min"), minsclx);
		theApp.m_Config->SetFloat(_T("brush.scale.x.max"), maxsclx);

		theApp.m_Config->SetBool(_T("brush.scale.y.apply"), scly ? true : false);
		theApp.m_Config->SetFloat(_T("brush.scale.y.min"), minscly);
		theApp.m_Config->SetFloat(_T("brush.scale.y.max"), maxscly);

		theApp.m_Config->SetBool(_T("brush.scale.z.apply"), sclz ? true : false);
		theApp.m_Config->SetFloat(_T("brush.scale.z.min"), minsclz);
		theApp.m_Config->SetFloat(_T("brush.scale.z.max"), maxsclz);

		theApp.m_Config->SetBool(_T("brush.scale.lockxy"), lockxy ? true : false);
		theApp.m_Config->SetBool(_T("brush.scale.lockyz"), lockyz ? true : false);

		theApp.m_Config->SetInt(_T("brush.placemode"), placemode);

		theApp.m_Config->SetBool(_T("brush.snap.x.apply"), snapx ? true : false);
		theApp.m_Config->SetFloat(_T("brush.snap.x.val"), snapxval);

		theApp.m_Config->SetBool(_T("brush.snap.y.apply"), snapy ? true : false);
		theApp.m_Config->SetFloat(_T("brush.snap.y.val"), snapyval);

		theApp.m_Config->SetBool(_T("brush.snap.z.apply"), snapz ? true : false);
		theApp.m_Config->SetFloat(_T("brush.snap.z.val"), snapzval);
	}
}


BEGIN_MESSAGE_MAP(CBrushSettingsDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_YAW, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_PITCH, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_ROLL, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_SCLX, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_SCLY, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_SCLZ, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_LOCKXY, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_LOCKYZ, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_YAW_DETENTS, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_PITCH_DETENTS, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_ROLL_DETENTS, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_SNAPX, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_SNAPY, &CBrushSettingsDlg::OnClickedCheckbox)
	ON_BN_CLICKED(IDC_CHECK_SNAPZ, &CBrushSettingsDlg::OnClickedCheckbox)
END_MESSAGE_MAP()


// CBrushSettingsDlg message handlers

void CBrushSettingsDlg::UpdateEnabled(bool live)
{
	bool yaw = live ?
		((CButton *)GetDlgItem(IDC_CHECK_YAW))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.yaw.apply"), true);
	GetDlgItem(IDC_EDIT_MINYAW)->EnableWindow(yaw);
	GetDlgItem(IDC_EDIT_MAXYAW)->EnableWindow(yaw);

	bool yawdet = live ?
		((CButton *)GetDlgItem(IDC_CHECK_YAW_DETENTS))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.yaw.detents.apply"), false);
	GetDlgItem(IDC_EDIT_YAW_DETENTS)->EnableWindow(yawdet);

	bool pitch = live ?
		((CButton *)GetDlgItem(IDC_CHECK_PITCH))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.pitch.apply"), false);
	GetDlgItem(IDC_EDIT_MINPITCH)->EnableWindow(pitch);
	GetDlgItem(IDC_EDIT_MAXPITCH)->EnableWindow(pitch);

	bool pitchdet = live ?
		((CButton *)GetDlgItem(IDC_CHECK_PITCH_DETENTS))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.pitch.detents.apply"), false);
	GetDlgItem(IDC_EDIT_PITCH_DETENTS)->EnableWindow(pitchdet);

	bool roll = live ?
		((CButton *)GetDlgItem(IDC_CHECK_ROLL))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.roll.apply"), false);
	GetDlgItem(IDC_EDIT_MINROLL)->EnableWindow(roll);
	GetDlgItem(IDC_EDIT_MAXROLL)->EnableWindow(roll);

	bool rolldet = live ?
		((CButton *)GetDlgItem(IDC_CHECK_ROLL_DETENTS))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.roll.detents.apply"), false);
	GetDlgItem(IDC_EDIT_ROLL_DETENTS)->EnableWindow(rolldet);

	bool lockxy = live ?
		((CButton *)GetDlgItem(IDC_CHECK_LOCKXY))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.scale.lockxy"), true);

	bool lockyz = live ?
		((CButton *)GetDlgItem(IDC_CHECK_LOCKYZ))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.scale.lockyz"), false);

	bool sclx = live ?
		((CButton *)GetDlgItem(IDC_CHECK_SCLX))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.scale.x.apply"), true);
	GetDlgItem(IDC_EDIT_MINSCLX)->EnableWindow(sclx && !lockxy);
	GetDlgItem(IDC_EDIT_MAXSCLX)->EnableWindow(sclx && !lockxy);

	bool scly = live ?
		((CButton *)GetDlgItem(IDC_CHECK_SCLY))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.scale.y.apply"), true);
	GetDlgItem(IDC_EDIT_MINSCLY)->EnableWindow(scly && !lockyz);
	GetDlgItem(IDC_EDIT_MAXSCLY)->EnableWindow(scly && !lockyz);

	bool sclz = live ?
		((CButton *)GetDlgItem(IDC_CHECK_SCLZ))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.scale.z.apply"), true);
	GetDlgItem(IDC_EDIT_MINSCLZ)->EnableWindow(sclz);
	GetDlgItem(IDC_EDIT_MAXSCLZ)->EnableWindow(sclz);

	bool snapx = live ?
		((CButton *)GetDlgItem(IDC_CHECK_SNAPX))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.snap.x.apply"), false);
	GetDlgItem(IDC_EDIT_SNAPX)->EnableWindow(snapx);

	bool snapy = live ?
		((CButton *)GetDlgItem(IDC_CHECK_SNAPY))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.snap.y.apply"), false);
	GetDlgItem(IDC_EDIT_SNAPY)->EnableWindow(snapy);

	bool snapz = live ?
		((CButton *)GetDlgItem(IDC_CHECK_SNAPZ))->GetCheck() :
		theApp.m_Config->GetBool(_T("brush.snap.z.apply"), false);
	GetDlgItem(IDC_EDIT_SNAPZ)->EnableWindow(snapz);
}


BOOL CBrushSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateEnabled(false);

	return TRUE;
}


void CBrushSettingsDlg::OnClickedCheckbox()
{
	UpdateEnabled(true);
}
