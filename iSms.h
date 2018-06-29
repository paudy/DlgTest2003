#pragma once
#include <map>

//������֤�봦��

struct tagSmsCode
{
	int		nRandCode;			//�����֤��
	char	szMobileID[12];		//�ֻ���
	time_t tmValidate;			//��Чʱ��

	tagSmsCode(){memset(this, 0, sizeof(tagSmsCode));}
};

typedef std::map<int, tagSmsCode*> SMS_CODE_LIST;


class CSmsVerifyCode
{
public:
	CSmsVerifyCode();
	~CSmsVerifyCode();

public:
	int HttpPostSms(char *szMobile); //ͨ��ihuyi.com SDK�ӿ��·�������֤��
	int RestApiSendSms(char* szMobile); //ͨ��wxchina.com�ӿ��·�������֤��
	int	VerifySmsCode(const char *szVerifyCode, const char *szMobile);					//ƥ���ֻ�������֤��

private:
	int		MakeSmsCode(const char *szMobile);	//���������֤��
	int		SendSmsCode(tagSmsCode *pSmsCode);						//ͨ��������SDK���Ͷ�����֤����û��ֻ�

	int		ClearTimeOutSmsCode();				//����Ѿ�ʧЧ����֤��
	int		RemoveVerifyCode(const char *szVerifyCode); //�Ƴ�ָ����֤��

private:
	int		SendVerifyCodeBySmsSdk(char *szMobile, int nCode); //�·���֤��
	int		SendVerifyCodeByRestAPI(char *szMobile, int nCode); //�·���֤��
	size_t send_sms(char *account, char *password, char *mobile, char *content);
	size_t http_post(char *page, char *poststr);

private:
	SMS_CODE_LIST m_SmsCodeList; //��֤�����
};