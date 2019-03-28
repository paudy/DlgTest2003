/////////////////////
///CSmsVerifyCode 短信验证码处理 
///by zdleek 20180612
/////////////////////
#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include "CharsetCodingUtils.h"
#include "isms.h"

#pragma comment(lib , "ws2_32.lib")

#define BUFSIZE 4096 /*缓冲区大小*/
#define MAXLINE 4096
#define MAXSUB  2000
#define MAXPARAM 2048

//ihuyi.com短信接口
char *ihuyi_SmsHost = "106.ihuyi.com";
char *ihuyi_send_sms_uri = "/webservice/sms.php?method=Submit&format=json";

///wxchina.com短信接口http_post服务器ip
char *wxcn_RestApiHost = "116.31.71.146"; //电信
//char *wxcn_RestApiHost = "122.13.18.210"; //联通
//char *wxcn_RestApiHost = "183.232.76.34"; //移动
char *wxcn_rest_sms_uri = "/api/v1.0.0/message/mass/send";

static SOCKET m_Socket=NULL;
static SOCKET m_Socket2=NULL;

CSmsVerifyCode::CSmsVerifyCode()
{
	m_Socket = NULL;
	m_Socket2 = NULL;
	//初始化socket
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2 , 2) , &wsd) != 0) 
	{
		printf("Winsock 初始化失败!\n");
		return;
	}
	srand(GetTickCount());
}


CSmsVerifyCode::~CSmsVerifyCode()
{
	if(m_Socket)
	{
		closesocket(m_Socket);
		WSACleanup();    //清理
		m_Socket = NULL;
	}

	//清除验证码队列
	SMS_CODE_LIST::iterator itr = m_SmsCodeList.begin();
	for(; itr != m_SmsCodeList.end(); itr++)
	{
		delete itr->second;
		itr->second = NULL;	
	}
	m_SmsCodeList.clear();
}

//通过ihuyi.com SDK接口下发短信验证码
int CSmsVerifyCode::HttpPostSms_ihuyi(char *szMobile)
{
	//生成验证码
	int nCode = MakeSmsCode(szMobile); 
	//发送验证码
	return SendVerifyCode_ihuyi(szMobile, nCode);
}

//通过wxchina.com接口下发短信验证码
int CSmsVerifyCode::HttpPostSms_wxcn(char* szMobile)
{
	//生成验证码
	int nCode = MakeSmsCode(szMobile); 
	//发送验证码
	SendVerifyCode_wxcn(szMobile, nCode);
	return 0;
}

//构建 htttp_post 字符串
int MakeIhuyiHttpPostString(char *account, char *password, char *mobile, char *content, char* szBuffOut)
{
	char params[MAXPARAM + 1];
    char *poststr = params;
    sprintf(poststr,"account=%s&password=%s&mobile=%s&format=xml&content=%s", account, password, mobile, content);
	char *page = ihuyi_send_sms_uri;

	_snprintf(szBuffOut, MAXSUB,
        "POST %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "Content-type: application/x-www-form-urlencoded\r\n"
        "Content-length: %u\r\n\r\n"
        "%s", page, ihuyi_SmsHost, strlen(poststr), poststr);

	return 0;
}

int CSmsVerifyCode::SendVerifyCode_ihuyi(char *szMobile, int nCode)
{
	
	WSADATA wsd;
    //SOCKET sClient;
	char Buffer[BUFSIZE]={0};
    int ret;
    struct sockaddr_in server;
    unsigned short port = 80;
    struct hostent *host = NULL;
	
	//加载Winsock DLL
    if (WSAStartup(MAKEWORD(2 , 2) , &wsd) != 0) {
        printf("Winsock    初始化失败!\n");
        return 1;
    }

	//创建Socket
    m_Socket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET) {
        printf("socket() 失败: %d\n" , WSAGetLastError());
		////zdtestlog
		//TCHAR szInfo[260] = {0};
		//wsprintf(szInfo, TEXT("%s  创建socket() 失败: %d\n:  "),   AnsiToUnicode(__FUNCTION__), WSAGetLastError());
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);
        return 1;
    }

    //指定服务器地址
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ihuyi_SmsHost);

    if (server.sin_addr.s_addr == INADDR_NONE) {
        host = gethostbyname(ihuyi_SmsHost);    //输入的地址可能是域名等
        if (host == NULL) {
            printf("无法解析服务端地址: %s\n" , ihuyi_SmsHost);
            return 1;
        }
        CopyMemory(&server.sin_addr ,
                    host->h_addr_list[0] ,
                    host->h_length);
    }
    /*与服务器建立连接*/
    if (connect(m_Socket , (struct sockaddr*)&server ,
                    sizeof(server)) == SOCKET_ERROR) {
        printf("connect() 失败: %d\n" , WSAGetLastError());
        return 1;
    }
	
    //用户名是登录用户中心->验证码短信->产品总览->APIID
    char *account = "C02806127";

    //查看密码请登录用户中心->验证码短信->产品总览->APIKEY
    char *password = "f1f5a7639f25e9ba06b415bc57a5b7b0";

    //手机号
	char mobile[12] ={0};
	strncpy(mobile, szMobile, sizeof(mobile)); 

    //短信内容
	char *message = "您的验证码是：%06d。请不要把验证码泄露给其他人。";
	char content[256] = {0};
	sprintf(content, message, nCode);

	char* szBuffOut = Buffer;
	MakeIhuyiHttpPostString(account, password, mobile, content, szBuffOut);

	USES_CONVERSION;
	int errcode = 0;
    //发送、接收消息
    for (;;) 
	{

        //向服务器发送消息
        ret = send(m_Socket , Buffer , strlen(Buffer) , 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) {
            printf("send() 失败: %d\n" , WSAGetLastError());
			errcode = 2;
            break;
        }
        printf("Send %d bytes\n" , ret);
		
		////zdtestlog
		//TCHAR szInfo[560] = {0};
		//wsprintf(szInfo, TEXT("%s 发送HTTP POST, SMS验证码\n send %d bytes:\n\t%s\n  "),   AnsiToUnicode(__FUNCTION__), ret, A2W(Buffer));
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);

        //接收从服务器来的消息
        ret = recv(m_Socket , Buffer , BUFSIZE , 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) {
            printf("recv() 失败: %d\n" , WSAGetLastError());
			errcode = 3;
            break;
        }

        Buffer[ret] = '\0';
        //printf("Recv %d bytes:\n\t%s\n" , ret , Buffer);
		//zdtestlog
		////TCHAR szInfo[560] = {0};
		//wsprintf(szInfo, TEXT("%s 收到SMS接口返回消息\n Recv %d bytes:\n\t%s\n  "),   AnsiToUnicode(__FUNCTION__), ret, A2W(Buffer));
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);
		break;
    }

    //用完了，关闭socket句柄(文件描述符)
    closesocket(m_Socket);
    WSACleanup();    //清理
	m_Socket = NULL;
    return errcode;
}

char* getMassJsonContent(const char *mobile, const char *content, char* szBuffOut)
{
	const char *formatStr = "{\n" 
                "    \"batchName\": \"ZDTESTansi\",\n" 
                "    \"items\": [\n" 
                "        {\n" 
                "            \"to\": \"%s\",\n" 
                "            \"customMsgID\": \"\"\n" 
                "        }\n" 
                "    ],\n" 
                "    \"content\": \"%s\",\n" 
                "    \"msgType\": \"sms\",\n" 
                "    \"bizType\": \"100\"\n" 
                "}";
    
	sprintf(szBuffOut,formatStr, mobile, content);
	
	return szBuffOut;
		

 // char *formatStr  = "{\n" 
 //               "    \"batchName\": \"ZDTESTansi\",\n" 
 //               "    \"items\": [\n" 
 //               "        {\n" 
 //               "            \"to\": \"18850067319\",\n" 
 //               "            \"customMsgID\": \"\"\n" 
 //               "        }\n" 
 //               "    ],\n" 
 //               "    \"content\": \"短信验证码xxxxxxUTF-8编码\",\n" 
 //               "    \"msgType\": \"sms\",\n" 
 //               "    \"bizType\": \"100\"\n" 
 //               "}";

}

//构建 RestApi 字符串
int MakeWxcnRestApiString(char *account, char *password, char *mobile, char *content, char* szBuffOut)
{
	char params[MAXPARAM + 1]={0};
    char *poststr = params;
	//const char * fmtSmsJson = getMassJsonContent();
	//sprintf(poststr, fmtSmsJson, mobile, content);
	getMassJsonContent(mobile, content, poststr);

	//转换为UTF-8编码
	std::string strAnsi = poststr;
	std::wstring wPoststr = ANSIToUnicode(strAnsi);
	const wchar_t* pszUtf16 = wPoststr.c_str();
	uint32_t nSizeUnicode16 = wPoststr.length();
	char* pszUtf8 = params;
	uint32_t nSizeUtf8 = MAXPARAM;
	uint32_t uLen = Unicode16ToUTF8((const uint16_t*)pszUtf16, nSizeUnicode16, pszUtf8, nSizeUtf8); 
	poststr = pszUtf8;

	////生成验证密钥(暂未实现)
	//char szAuthKey[512]={0};
	//char *MD5Pwd = MD5Enc(password);
	//strcpy(szAuthKey, account);
	//strcat(szAuthKey,":");	
	//strcat(szAuthKey, MD5Pwd);
	//Base64Enc(szAuthKey);

	//**直接使用在线工具进行MD5和BASE64编码，得出结果char* Authorization
	const char* Authorization = "eG16eXdsQHhtenl3bDozNTIzOTA3QUNENTY4RDFGRkQyMUNGNDU5MDQ2QzZCMg=="; //密钥，Base64编码的用户名和密码组合，格式为Base64("用户名:MD5(密码)"）	
	_snprintf(szBuffOut, MAXSUB,
        "POST %s HTTP/1.0\n"
		"Host: %s\n"
        "Content-Type: application/json;charset=UTF-8\n"
        "Content-length: %u\n"
		"Accept: application/json\n"
		"Authorization: %s\n\n"
        "%s\n", 
		wxcn_rest_sms_uri, wxcn_RestApiHost, strlen(poststr),Authorization, poststr);

	return 0;
}

int CSmsVerifyCode::SendVerifyCode_wxcn(char *szMobile, int nCode)
{
	
	WSADATA wsd;
    //SOCKET sClient;
	char Buffer[BUFSIZE]={0};
    int ret;
    struct sockaddr_in server;
    unsigned short port = 80;
    struct hostent *host = NULL;
	
	//加载Winsock DLL
    if (WSAStartup(MAKEWORD(2 , 2) , &wsd) != 0) {
        printf("Winsock    初始化失败!\n");
        return 1;
    }

	//创建Socket
    m_Socket2 = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
    if (m_Socket2 == INVALID_SOCKET) {
        printf("socket() 失败: %d\n" , WSAGetLastError());
		////zdtestlog
		//TCHAR szInfo[260] = {0};
		//wsprintf(szInfo, TEXT("%s  创建socket() 失败: %d\n:  "),   AnsiToUnicode(__FUNCTION__), WSAGetLastError());
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);
        return 1;
    }

    //指定服务器地址
    server.sin_family = AF_INET;
    server.sin_port = htons(9051);
    server.sin_addr.s_addr = inet_addr(wxcn_RestApiHost);

    if (server.sin_addr.s_addr == INADDR_NONE) {
        host = gethostbyname(wxcn_RestApiHost);    //输入的地址可能是域名等
        if (host == NULL) {
            printf("无法解析服务端地址: %s\n" , wxcn_RestApiHost);
            return 1;
        }
        CopyMemory(&server.sin_addr ,
                    host->h_addr_list[0] ,
                    host->h_length);
    }
    /*与服务器建立连接*/
    if (connect(m_Socket2 , (struct sockaddr*)&server ,
                    sizeof(server)) == SOCKET_ERROR) {
        printf("connect() 失败: %d\n" , WSAGetLastError());
        return 1;
    }
	
	int nNetTimeout= 5000;
    setsockopt(m_Socket2,SOL_SOCKET,SO_SNDTIMEO,(char*)&nNetTimeout,sizeof(nNetTimeout)); 
	if (SOCKET_ERROR ==  setsockopt(m_Socket2,SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout,sizeof(int))) 
	{ 
			printf("Set Ser_RecTIMEO error !\r\n"); 
	} 

    //用户名
    char *account = "xmzywl@xmzywl";
    //密码(MD5)
    char *password = "3523907ACD568D1FFD21CF459046C6B2";

    //手机号
	char mobile[13] ={0};
	strncpy(mobile, szMobile, sizeof(mobile)); //"18850067319";

    //短信内容
	char *message = "您的验证码是：%06d。请不要把验证码泄露给其他人。";
	char content[256] = {0};
	sprintf(content, message, nCode);

	char* szBuffOut = Buffer;
	MakeWxcnRestApiString(account, password, mobile, content, szBuffOut);

    //发送、接收消息
    for (;;) 
	{

        //向服务器发送消息
        ret = send(m_Socket2 , Buffer , strlen(Buffer) , 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) {
            printf("send() 失败: %d\n" , WSAGetLastError());
            break;
        }
        //printf("Send %d bytes\n" , ret);
		
		////zdtestlog
		//TCHAR szInfo[560] = {0};
		//wsprintf(szInfo, TEXT("%s 发送HTTP POST, SMS验证码\n send %d bytes:\n\t%s\n  "),   AnsiToUnicode(__FUNCTION__), ret, Buffer);
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);

        //接收从服务器来的消息
        ret = recv(m_Socket2 , Buffer , BUFSIZE , 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) {
			char szErr[128] ={0};
            sprintf(szErr,"recv() 失败: %d\n" , WSAGetLastError());
            break;
        }

        Buffer[ret] = '\0';
        //printf("Recv %d bytes:\n\t%s\n" , ret , Buffer);

		//zdtestlog
		//TCHAR szInfo[560] = {0};
		//wsprintf(szInfo, TEXT("%s 收到SMS接口返回消息\n Recv %d bytes:\n\t%s\n  "),   AnsiToUnicode(__FUNCTION__), ret, Buffer);
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);
		break;
    }

    //用完了，关闭socket句柄(文件描述符)
    closesocket(m_Socket2);
    WSACleanup();    //清理
	m_Socket2 = NULL;
    return 0;
}


//生成随机验证码
int	CSmsVerifyCode::MakeSmsCode(const char *szMobile)				
{
	if(strlen(szMobile) > 18) return 0;

	SMS_CODE_LIST::iterator itr = m_SmsCodeList.end();
	int nCode = 0;
	int nCount = 0;
	
	do
	{
		//随机6位验证码
		for(int i = 0; i < 6; i++)
		{
			nCode *= 10;
			nCode += (rand() % 9 + 1);
		}
		
		itr = m_SmsCodeList.find(nCode);
		if(itr == m_SmsCodeList.end())
		{
			//验证码可用,加入队列
			tagSmsCode *pSmsCode = new tagSmsCode();
			pSmsCode->nRandCode = nCode;
			pSmsCode->tmValidate = time(NULL);
			strncpy(pSmsCode->szMobileID, szMobile, sizeof(pSmsCode->szMobileID));

			m_SmsCodeList[nCode] = pSmsCode;
			
			if(m_SmsCodeList.size()>1000)
			{
				//如果队列元素太多，清理过期的验证码
				ClearTimeOutSmsCode();
			}
		}
		else
		{
			nCode = 0; //重新生成验证码
		}
	
	}while(nCode == 0 && (nCount++) < 10000 );
	
	return nCode;
}



//匹配手机号与验证码,返回0表示成功
int	CSmsVerifyCode::VerifySmsCode(const char *szVerifyCode, const char *szMobile)			
{
	int nCode = atoi(szVerifyCode);
	SMS_CODE_LIST::iterator itr;
	itr = m_SmsCodeList.find(nCode);
	if(itr != m_SmsCodeList.end())
	{
		if(itr->second && strnicmp(szMobile, itr->second->szMobileID, sizeof(itr->second->szMobileID)-1) == 0)
		{
			time_t tNow = time(NULL);
			if((tNow - itr->second->tmValidate) < 360) //6分钟以内
			{
				return 0;
			}
		}		
	}

	return -1;
}

//清除已经失效的验证码
int	CSmsVerifyCode::ClearTimeOutSmsCode()		
{
	int n = 0;
	time_t tNow = time(NULL);

	SMS_CODE_LIST::iterator itr = m_SmsCodeList.begin();
	for(; itr != m_SmsCodeList.end(); itr++)
	{
		if((tNow - itr->second->tmValidate) > 300)
		{
			delete itr->second;
			itr = m_SmsCodeList.erase(itr);
			n++;
			continue;
		}
	}

	return n;
}

//移除指定验证码
int	CSmsVerifyCode::RemoveVerifyCode(const char *szVerifyCode) 
{
	int nCode = atoi(szVerifyCode);
	SMS_CODE_LIST::iterator itr;
	itr = m_SmsCodeList.find(nCode);
	if(itr != m_SmsCodeList.end())
	{
		m_SmsCodeList.erase(itr);
		return 1;
	}

	return 0;
}


