#include "Stdafx.h"
#include "Resource.h"
#include "LogonServerDlg.h"
#include "isms.h"


//////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CLogonServerDlg, CDialog)
	ON_WM_QUERYENDSESSION()
	ON_BN_CLICKED(IDC_STOP_SERVICE, OnBnClickedStopService)
	ON_BN_CLICKED(IDC_START_SERVICE, OnBnClickedStartService)
	//ON_MESSAGE(WM_NETWORK_EVENT, OnNetworkEvent)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////
extern int zdTestParseJson();
extern int zdTestConstructJson();

//���캯��
CLogonServerDlg::CLogonServerDlg() : CDialog(IDD_DLG_LOGON_SERVER)
{
}

//��������
CLogonServerDlg::~CLogonServerDlg()
{
}

//�ؼ���
VOID CLogonServerDlg::DoDataExchange(CDataExchange * pDX)
{
	__super::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_TRACE_MESSAGE, m_TraceServiceControl);
}

//��ʼ������
BOOL CLogonServerDlg::OnInitDialog()
{
	__super::OnInitDialog();

	//���ñ���
	SetWindowText(TEXT("DlgTest2003 -- [ ֹͣ ]"));

	//����ͼ��
	HICON hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	SetIcon(hIcon,TRUE);
	SetIcon(hIcon,FALSE);

	////�������
	//m_ServiceUnits.SetServiceUnitsSink(this);
	//m_FlashAccredit.SetServiceParameter(843);
	//m_FlashAccredit.StartService();

	return TRUE;
}

//ȷ����Ϣ
VOID CLogonServerDlg::OnOK()
{
	return;
}

//ȡ������
VOID CLogonServerDlg::OnCancel()
{
	////�ر�ѯ��
	//if (m_ServiceUnits.GetServiceStatus()!=ServiceStatus_Stop)
	//{
		LPCTSTR pszQuestion=TEXT("DlgTest2003���������У���ȷʵҪ�˳�������");
		if (AfxMessageBox(pszQuestion,MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION)!=IDYES) return;
	//}

	//ֹͣ����
	//m_ServiceUnits.ConcludeService();

	__super::OnCancel();
}

//��Ϣ����
BOOL CLogonServerDlg::PreTranslateMessage(MSG * pMsg)
{
	//��������
	if ((pMsg->message==WM_KEYDOWN)&&(pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}

void CLogonServerDlg::zdTest1()
{
	USES_CONVERSION;
	TCHAR szWstr[4096] = { 0 };
	for (int i = 0; i < 4094; i++)
	{
		szWstr[i] = L'A';
	}
	int n1 = _tcslen(szWstr);
	char *pstr = W2A(szWstr);
	int n2 = strlen(pstr);
	TCHAR sztmp[100] = { 0 };
	_sntprintf(sztmp,sizeof(sztmp)/sizeof(TCHAR), TEXT("TCHARԭʼ�ַ�������Ϊ%d, W2Aת�����ַ�������Ϊ%d"), n1, n2);
	GetDlgItem(IDC_TRACE_MESSAGE)->SetWindowText(sztmp);
}

//��������
VOID CLogonServerDlg::OnBnClickedStartService()
{
	//zdTest1();
	
	CSmsVerifyCode sms_wrap;
	//sms_wrap.HttpPostSms_ihuyi("18850067319");
	//sms_wrap.HttpPostSms_wxcn("18850067319");

	//zdTestParseJson();

	zdTestConstructJson();

	return;
}

//ֹͣ����
VOID CLogonServerDlg::OnBnClickedStopService()
{
	//ֹͣ����
	//try
	//{
	//	m_ServiceUnits.ConcludeService();
	//}
	//catch (...)
	//{
	//	ASSERT(FALSE);
	//}

	return;
}


////�ر�ѯ��
//BOOL CLogonServerDlg::OnQueryEndSession()
//{
//	//��ʾ��Ϣ
//	if (m_ServiceUnits.GetServiceStatus()!=ServiceStatus_Stop)
//	{
//		CTraceService::TraceString(TEXT("�������������У�ϵͳҪ��ע���ػ�����ʧ��"),TraceLevel_Warning);
//		return FALSE;
//	}
//
//	return TRUE;
//}
//
////�����¼�
//LRESULT CLogonServerDlg::OnNetworkEvent(WPARAM wparam, LPARAM lparam)
//{
//	return 0;//m_NetworkEngine.OnNetworkEvent(wparam, lparam);
//}

//////////////////////////////////////////////////////////////////////////////////
