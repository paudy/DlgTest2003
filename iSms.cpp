/////////////////////
///CSmsVerifyCode ������֤�봦�� 
///by zdleek 20180612
/////////////////////
#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include "CharsetCodingUtils.h"
#include "isms.h"

#pragma comment(lib , "ws2_32.lib")

#define BUFSIZE 4096 /*��������С*/
#define MAXLINE 4096
#define MAXSUB  2000
#define MAXPARAM 2048

//ihuyi.com���Žӿ�
char *ihuyi_SmsHost = "106.ihuyi.com";
char *ihuyi_send_sms_uri = "/webservice/sms.php?method=Submit&format=json";

///wxchina.com���Žӿ�http_post������ip
char *wxcn_RestApiHost = "116.31.71.146"; //����
//char *wxcn_RestApiHost = "122.13.18.210"; //��ͨ
//char *wxcn_RestApiHost = "183.232.76.34"; //�ƶ�
char *wxcn_rest_sms_uri = "/api/v1.0.0/message/mass/send";

static SOCKET m_Socket=NULL;
static SOCKET m_Socket2=NULL;

CSmsVerifyCode::CSmsVerifyCode()
{
	m_Socket = NULL;
	m_Socket2 = NULL;
	//��ʼ��socket
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2 , 2) , &wsd) != 0) 
	{
		printf("Winsock ��ʼ��ʧ��!\n");
		return;
	}
	srand(GetTickCount());
}


CSmsVerifyCode::~CSmsVerifyCode()
{
	if(m_Socket)
	{
		closesocket(m_Socket);
		WSACleanup();    //����
		m_Socket = NULL;
	}

	//�����֤�����
	SMS_CODE_LIST::iterator itr = m_SmsCodeList.begin();
	for(; itr != m_SmsCodeList.end(); itr++)
	{
		delete itr->second;
		itr->second = NULL;	
	}
	m_SmsCodeList.clear();
}

//ͨ��ihuyi.com SDK�ӿ��·�������֤��
int CSmsVerifyCode::HttpPostSms_ihuyi(char *szMobile)
{
	//������֤��
	int nCode = MakeSmsCode(szMobile); 
	//������֤��
	return SendVerifyCode_ihuyi(szMobile, nCode);
}

//ͨ��wxchina.com�ӿ��·�������֤��
int CSmsVerifyCode::HttpPostSms_wxcn(char* szMobile)
{
	//������֤��
	int nCode = MakeSmsCode(szMobile); 
	//������֤��
	SendVerifyCode_wxcn(szMobile, nCode);
	return 0;
}

//���� htttp_post �ַ���
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
	
	//����Winsock DLL
    if (WSAStartup(MAKEWORD(2 , 2) , &wsd) != 0) {
        printf("Winsock    ��ʼ��ʧ��!\n");
        return 1;
    }

	//����Socket
    m_Socket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET) {
        printf("socket() ʧ��: %d\n" , WSAGetLastError());
		////zdtestlog
		//TCHAR szInfo[260] = {0};
		//wsprintf(szInfo, TEXT("%s  ����socket() ʧ��: %d\n:  "),   AnsiToUnicode(__FUNCTION__), WSAGetLastError());
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);
        return 1;
    }

    //ָ����������ַ
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ihuyi_SmsHost);

    if (server.sin_addr.s_addr == INADDR_NONE) {
        host = gethostbyname(ihuyi_SmsHost);    //����ĵ�ַ������������
        if (host == NULL) {
            printf("�޷���������˵�ַ: %s\n" , ihuyi_SmsHost);
            return 1;
        }
        CopyMemory(&server.sin_addr ,
                    host->h_addr_list[0] ,
                    host->h_length);
    }
    /*���������������*/
    if (connect(m_Socket , (struct sockaddr*)&server ,
                    sizeof(server)) == SOCKET_ERROR) {
        printf("connect() ʧ��: %d\n" , WSAGetLastError());
        return 1;
    }
	
    //�û����ǵ�¼�û�����->��֤�����->��Ʒ����->APIID
    char *account = "C02806127";

    //�鿴�������¼�û�����->��֤�����->��Ʒ����->APIKEY
    char *password = "f1f5a7639f25e9ba06b415bc57a5b7b0";

    //�ֻ���
	char mobile[12] ={0};
	strncpy(mobile, szMobile, sizeof(mobile)); 

    //��������
	char *message = "������֤���ǣ�%06d���벻Ҫ����֤��й¶�������ˡ�";
	char content[256] = {0};
	sprintf(content, message, nCode);

	char* szBuffOut = Buffer;
	MakeIhuyiHttpPostString(account, password, mobile, content, szBuffOut);

	USES_CONVERSION;
	int errcode = 0;
    //���͡�������Ϣ
    for (;;) 
	{

        //�������������Ϣ
        ret = send(m_Socket , Buffer , strlen(Buffer) , 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) {
            printf("send() ʧ��: %d\n" , WSAGetLastError());
			errcode = 2;
            break;
        }
        printf("Send %d bytes\n" , ret);
		
		////zdtestlog
		//TCHAR szInfo[560] = {0};
		//wsprintf(szInfo, TEXT("%s ����HTTP POST, SMS��֤��\n send %d bytes:\n\t%s\n  "),   AnsiToUnicode(__FUNCTION__), ret, A2W(Buffer));
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);

        //���մӷ�����������Ϣ
        ret = recv(m_Socket , Buffer , BUFSIZE , 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) {
            printf("recv() ʧ��: %d\n" , WSAGetLastError());
			errcode = 3;
            break;
        }

        Buffer[ret] = '\0';
        //printf("Recv %d bytes:\n\t%s\n" , ret , Buffer);
		//zdtestlog
		////TCHAR szInfo[560] = {0};
		//wsprintf(szInfo, TEXT("%s �յ�SMS�ӿڷ�����Ϣ\n Recv %d bytes:\n\t%s\n  "),   AnsiToUnicode(__FUNCTION__), ret, A2W(Buffer));
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);
		break;
    }

    //�����ˣ��ر�socket���(�ļ�������)
    closesocket(m_Socket);
    WSACleanup();    //����
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
 //               "    \"content\": \"������֤��xxxxxxUTF-8����\",\n" 
 //               "    \"msgType\": \"sms\",\n" 
 //               "    \"bizType\": \"100\"\n" 
 //               "}";

}

//���� RestApi �ַ���
int MakeWxcnRestApiString(char *account, char *password, char *mobile, char *content, char* szBuffOut)
{
	char params[MAXPARAM + 1]={0};
    char *poststr = params;
	//const char * fmtSmsJson = getMassJsonContent();
	//sprintf(poststr, fmtSmsJson, mobile, content);
	getMassJsonContent(mobile, content, poststr);

	//ת��ΪUTF-8����
	std::string strAnsi = poststr;
	std::wstring wPoststr = ANSIToUnicode(strAnsi);
	const wchar_t* pszUtf16 = wPoststr.c_str();
	uint32_t nSizeUnicode16 = wPoststr.length();
	char* pszUtf8 = params;
	uint32_t nSizeUtf8 = MAXPARAM;
	uint32_t uLen = Unicode16ToUTF8((const uint16_t*)pszUtf16, nSizeUnicode16, pszUtf8, nSizeUtf8); 
	poststr = pszUtf8;

	////������֤��Կ(��δʵ��)
	//char szAuthKey[512]={0};
	//char *MD5Pwd = MD5Enc(password);
	//strcpy(szAuthKey, account);
	//strcat(szAuthKey,":");	
	//strcat(szAuthKey, MD5Pwd);
	//Base64Enc(szAuthKey);

	//**ֱ��ʹ�����߹��߽���MD5��BASE64���룬�ó����char* Authorization
	const char* Authorization = "eG16eXdsQHhtenl3bDozNTIzOTA3QUNENTY4RDFGRkQyMUNGNDU5MDQ2QzZCMg=="; //��Կ��Base64������û�����������ϣ���ʽΪBase64("�û���:MD5(����)"��	
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
	
	//����Winsock DLL
    if (WSAStartup(MAKEWORD(2 , 2) , &wsd) != 0) {
        printf("Winsock    ��ʼ��ʧ��!\n");
        return 1;
    }

	//����Socket
    m_Socket2 = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
    if (m_Socket2 == INVALID_SOCKET) {
        printf("socket() ʧ��: %d\n" , WSAGetLastError());
		////zdtestlog
		//TCHAR szInfo[260] = {0};
		//wsprintf(szInfo, TEXT("%s  ����socket() ʧ��: %d\n:  "),   AnsiToUnicode(__FUNCTION__), WSAGetLastError());
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);
        return 1;
    }

    //ָ����������ַ
    server.sin_family = AF_INET;
    server.sin_port = htons(9051);
    server.sin_addr.s_addr = inet_addr(wxcn_RestApiHost);

    if (server.sin_addr.s_addr == INADDR_NONE) {
        host = gethostbyname(wxcn_RestApiHost);    //����ĵ�ַ������������
        if (host == NULL) {
            printf("�޷���������˵�ַ: %s\n" , wxcn_RestApiHost);
            return 1;
        }
        CopyMemory(&server.sin_addr ,
                    host->h_addr_list[0] ,
                    host->h_length);
    }
    /*���������������*/
    if (connect(m_Socket2 , (struct sockaddr*)&server ,
                    sizeof(server)) == SOCKET_ERROR) {
        printf("connect() ʧ��: %d\n" , WSAGetLastError());
        return 1;
    }
	
	int nNetTimeout= 5000;
    setsockopt(m_Socket2,SOL_SOCKET,SO_SNDTIMEO,(char*)&nNetTimeout,sizeof(nNetTimeout)); 
	if (SOCKET_ERROR ==  setsockopt(m_Socket2,SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout,sizeof(int))) 
	{ 
			printf("Set Ser_RecTIMEO error !\r\n"); 
	} 

    //�û���
    char *account = "xmzywl@xmzywl";
    //����(MD5)
    char *password = "3523907ACD568D1FFD21CF459046C6B2";

    //�ֻ���
	char mobile[13] ={0};
	strncpy(mobile, szMobile, sizeof(mobile)); //"18850067319";

    //��������
	char *message = "������֤���ǣ�%06d���벻Ҫ����֤��й¶�������ˡ�";
	char content[256] = {0};
	sprintf(content, message, nCode);

	char* szBuffOut = Buffer;
	MakeWxcnRestApiString(account, password, mobile, content, szBuffOut);

    //���͡�������Ϣ
    for (;;) 
	{

        //�������������Ϣ
        ret = send(m_Socket2 , Buffer , strlen(Buffer) , 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) {
            printf("send() ʧ��: %d\n" , WSAGetLastError());
            break;
        }
        //printf("Send %d bytes\n" , ret);
		
		////zdtestlog
		//TCHAR szInfo[560] = {0};
		//wsprintf(szInfo, TEXT("%s ����HTTP POST, SMS��֤��\n send %d bytes:\n\t%s\n  "),   AnsiToUnicode(__FUNCTION__), ret, Buffer);
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);

        //���մӷ�����������Ϣ
        ret = recv(m_Socket2 , Buffer , BUFSIZE , 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) {
			char szErr[128] ={0};
            sprintf(szErr,"recv() ʧ��: %d\n" , WSAGetLastError());
            break;
        }

        Buffer[ret] = '\0';
        //printf("Recv %d bytes:\n\t%s\n" , ret , Buffer);

		//zdtestlog
		//TCHAR szInfo[560] = {0};
		//wsprintf(szInfo, TEXT("%s �յ�SMS�ӿڷ�����Ϣ\n Recv %d bytes:\n\t%s\n  "),   AnsiToUnicode(__FUNCTION__), ret, Buffer);
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);
		break;
    }

    //�����ˣ��ر�socket���(�ļ�������)
    closesocket(m_Socket2);
    WSACleanup();    //����
	m_Socket2 = NULL;
    return 0;
}


//���������֤��
int	CSmsVerifyCode::MakeSmsCode(const char *szMobile)				
{
	if(strlen(szMobile) > 18) return 0;

	SMS_CODE_LIST::iterator itr = m_SmsCodeList.end();
	int nCode = 0;
	int nCount = 0;
	
	do
	{
		//���6λ��֤��
		for(int i = 0; i < 6; i++)
		{
			nCode *= 10;
			nCode += (rand() % 9 + 1);
		}
		
		itr = m_SmsCodeList.find(nCode);
		if(itr == m_SmsCodeList.end())
		{
			//��֤�����,�������
			tagSmsCode *pSmsCode = new tagSmsCode();
			pSmsCode->nRandCode = nCode;
			pSmsCode->tmValidate = time(NULL);
			strncpy(pSmsCode->szMobileID, szMobile, sizeof(pSmsCode->szMobileID));

			m_SmsCodeList[nCode] = pSmsCode;
			
			if(m_SmsCodeList.size()>1000)
			{
				//�������Ԫ��̫�࣬������ڵ���֤��
				ClearTimeOutSmsCode();
			}
		}
		else
		{
			nCode = 0; //����������֤��
		}
	
	}while(nCode == 0 && (nCount++) < 10000 );
	
	return nCode;
}



//ƥ���ֻ�������֤��,����0��ʾ�ɹ�
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
			if((tNow - itr->second->tmValidate) < 360) //6��������
			{
				return 0;
			}
		}		
	}

	return -1;
}

//����Ѿ�ʧЧ����֤��
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

//�Ƴ�ָ����֤��
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


