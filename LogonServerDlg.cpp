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

//构造函数
CLogonServerDlg::CLogonServerDlg() : CDialog(IDD_DLG_LOGON_SERVER)
{
}

//析构函数
CLogonServerDlg::~CLogonServerDlg()
{
}

//控件绑定
VOID CLogonServerDlg::DoDataExchange(CDataExchange * pDX)
{
	__super::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_TRACE_MESSAGE, m_TraceServiceControl);
}

//初始化函数
BOOL CLogonServerDlg::OnInitDialog()
{
	__super::OnInitDialog();

	//设置标题
	SetWindowText(TEXT("DlgTest2003 -- [ 停止 ]"));

	//设置图标
	HICON hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	SetIcon(hIcon,TRUE);
	SetIcon(hIcon,FALSE);

	////设置组件
	//m_ServiceUnits.SetServiceUnitsSink(this);
	//m_FlashAccredit.SetServiceParameter(843);
	//m_FlashAccredit.StartService();

	return TRUE;
}

//确定消息
VOID CLogonServerDlg::OnOK()
{
	return;
}

//取消函数
VOID CLogonServerDlg::OnCancel()
{
	////关闭询问
	//if (m_ServiceUnits.GetServiceStatus()!=ServiceStatus_Stop)
	//{
		LPCTSTR pszQuestion=TEXT("DlgTest2003正在运行中，您确实要退出程序吗？");
		if (AfxMessageBox(pszQuestion,MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION)!=IDYES) return;
	//}

	//停止服务
	//m_ServiceUnits.ConcludeService();

	__super::OnCancel();
}

//消息解释
BOOL CLogonServerDlg::PreTranslateMessage(MSG * pMsg)
{
	//按键过滤
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
	_sntprintf(sztmp,sizeof(sztmp)/sizeof(TCHAR), TEXT("TCHAR原始字符串长度为%d, W2A转换后字符串长度为%d"), n1, n2);
	GetDlgItem(IDC_TRACE_MESSAGE)->SetWindowText(sztmp);
}

//启动服务
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

//停止服务
VOID CLogonServerDlg::OnBnClickedStopService()
{
	//停止服务
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


////关闭询问
//BOOL CLogonServerDlg::OnQueryEndSession()
//{
//	//提示消息
//	if (m_ServiceUnits.GetServiceStatus()!=ServiceStatus_Stop)
//	{
//		CTraceService::TraceString(TEXT("服务正在运行中，系统要求注销回话请求失败"),TraceLevel_Warning);
//		return FALSE;
//	}
//
//	return TRUE;
//}
//
////网络事件
//LRESULT CLogonServerDlg::OnNetworkEvent(WPARAM wparam, LPARAM lparam)
//{
//	return 0;//m_NetworkEngine.OnNetworkEvent(wparam, lparam);
//}

//////////////////////////////////////////////////////////////////////////////////
