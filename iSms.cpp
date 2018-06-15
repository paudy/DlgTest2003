/////////////////////
///CSmsVerifyCode 短信验证码处理 
///by zdleek 20180612
/////////////////////
#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include "isms.h"

#pragma comment(lib , "ws2_32.lib")

#define BUFSIZE 4096 /*缓冲区大小*/
#define MAXLINE 4096
#define MAXSUB  2000
#define MAXPARAM 2048

char *szSmsHost = "106.ihuyi.com";
char *send_sms_uri = "/webservice/sms.php?method=Submit&format=json";
SOCKET m_Socket=NULL;

CSmsVerifyCode::CSmsVerifyCode()
{
	m_Socket = NULL;
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

int CSmsVerifyCode::HttpPostSms(char *szMobile)
{
	//生成验证码
	int nCode = MakeSmsCode(szMobile); 
	//发送验证码
	SendVerifyCodeBySmsSdk(szMobile, nCode);
	return 0;
}

////* 发http post请求
//size_t CSmsVerifyCode::http_post(char *page, char *poststr)
//{
//    char sendline[MAXLINE + 1]/*, recvline[MAXLINE + 1]*/;
//    size_t n=0;
//    _snprintf(sendline, MAXSUB,
//        "POST %s HTTP/1.0\r\n"
//        "Host: %s\r\n"
//        "Content-type: application/x-www-form-urlencoded\r\n"
//        "Content-length: %zu\r\n\r\n"
//        "%s", page, szSmsHost, strlen(poststr), poststr);
//
//    //write(basefd, sendline, strlen(sendline));
//    //while ((n = read(basefd, recvline, MAXLINE)) > 0) {
//    //    recvline[n] = '\0';
//    //    printf("%s", recvline);
//    //}
//    return n;
//}
//
////* 发送短信
//size_t CSmsVerifyCode::send_sms(char *account, char *password, char *mobile, char *content)
//{
//    char params[MAXPARAM + 1];
//    char *cp = params;
//    sprintf(cp,"account=%s&password=%s&mobile=%s&content=%s", account, password, mobile, content);
//    return http_post(send_sms_uri, cp);
//}

//构建 htttp_post 字符串
int MakeHttpPostString(char *account, char *password, char *mobile, char *content, char* szBuffOut)
{
	char params[MAXPARAM + 1];
    char *poststr = params;
    sprintf(poststr,"account=%s&password=%s&mobile=%s&format=xml&content=%s", account, password, mobile, content);
	char *page = send_sms_uri;

	_snprintf(szBuffOut, MAXSUB,
        "POST %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "Content-type: application/x-www-form-urlencoded\r\n"
        "Content-length: %u\r\n\r\n"
        "%s", page, szSmsHost, strlen(poststr), poststr);

	return 0;
}

int CSmsVerifyCode::SendVerifyCodeBySmsSdk(char *szMobile, int nCode)
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
    server.sin_addr.s_addr = inet_addr(szSmsHost);

    if (server.sin_addr.s_addr == INADDR_NONE) {
        host = gethostbyname(szSmsHost);    //输入的地址可能是域名等
        if (host == NULL) {
            printf("无法解析服务端地址: %s\n" , szSmsHost);
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
    char *account = "C44034602";

    //查看密码请登录用户中心->验证码短信->产品总览->APIKEY
    char *password = "972ae1c34bda81ca1f0564026e12cdc4";

    //手机号
	char mobile[12] ={0};
	strncpy(mobile, szMobile, sizeof(mobile)); //"18850067319";

    //短信内容
	char *message = "您的验证码是：%06d。请不要把验证码泄露给其他人。";
	char content[256] = {0};
	sprintf(content, message, nCode);

	char* szBuffOut = Buffer;
	MakeHttpPostString(account, password, mobile, content, szBuffOut);

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
            break;
        }
        printf("Send %d bytes\n" , ret);
		
		////zdtestlog
		//TCHAR szInfo[560] = {0};
		//wsprintf(szInfo, TEXT("%s 发送HTTP POST, SMS验证码\n send %d bytes:\n\t%s\n  "),   AnsiToUnicode(__FUNCTION__), ret, Buffer);
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);

        //接收从服务器来的消息
        ret = recv(m_Socket , Buffer , BUFSIZE , 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) {
            printf("recv() 失败: %d\n" , WSAGetLastError());
            break;
        }

        Buffer[ret] = '\0';
        printf("Recv %d bytes:\n\t%s\n" , ret , Buffer);

		////zdtestlog
		////TCHAR szInfo[560] = {0};
		//wsprintf(szInfo, TEXT("%s 收到SMS接口返回消息\n Recv %d bytes:\n\t%s\n  "),   AnsiToUnicode(__FUNCTION__), ret, Buffer);
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);
		break;
    }

    //用完了，关闭socket句柄(文件描述符)
    closesocket(m_Socket);
    WSACleanup();    //清理
	m_Socket = NULL;
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

//通过第三方SDK发送短信验证码给用户手机
int	CSmsVerifyCode::SendSmsCode(tagSmsCode *pSmsCode)			
{
	char szCode[20]={0};
	itoa(pSmsCode->nRandCode, szCode, 10);
	//http_post(szCode);

	return 0;
}

//匹配手机号与验证码,返回1表示成功
int	CSmsVerifyCode::VerifySmsCode(const char *szVerifyCode, const char *szMobile)			
{
	int nCode = atoi(szVerifyCode);
	SMS_CODE_LIST::iterator itr;
	itr = m_SmsCodeList.find(nCode);
	if(itr != m_SmsCodeList.end())
	{
		if(strnicmp(szMobile, itr->second->szMobileID, sizeof(itr->second->szMobileID)) == 0)
		{
			time_t tNow = time(NULL);
			if((tNow - itr->second->tmValidate) < 300)
			{
				return 1;
			}
		}		
	}

	return 0;
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
