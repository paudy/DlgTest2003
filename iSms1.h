#pragma once
#include <map>

//������֤�봦��

struct tagSmsCode
{
	int		nRandCode;			//�����֤��
	char	szMobileID[20];		//�ֻ���
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
	void TestHttpPostSms();

private:
	int		MakeSmsCode(const char *szMobile);	//���������֤��
	int		SendSmsCode(tagSmsCode *pSmsCode);						//ͨ��������SDK���Ͷ�����֤����û��ֻ�
	int		VerifySmsCode(const char *szVerifyCode, const char *szMobile);					//ƥ���ֻ�������֤��
	int		ClearTimeOutSmsCode();				//����Ѿ�ʧЧ����֤��
	int		RemoveVerifyCode(const char *szVerifyCode); //�Ƴ�ָ����֤��

private:
	int InitSmsSdk();
	size_t send_sms(char *account, char *password, char *mobile, char *content);
	size_t http_post(char *page, char *poststr);

private:
	SMS_CODE_LIST m_SmsCodeList; //��֤�����
};