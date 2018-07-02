#pragma once
#include <map>

//������֤�봦��

struct tagSmsCode
{
	int		nRandCode;			//�����֤��
	char	szMobileID[15];		//�ֻ���
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
	int HttpPostSms_ihuyi(char *szMobile); //ͨ��ihuyi.com SDK�ӿ��·�������֤��
	int HttpPostSms_wxcn(char* szMobile); //ͨ��wxchina.com�ӿ��·�������֤��
	int	VerifySmsCode(const char *szVerifyCode, const char *szMobile);		//ƥ���ֻ�������֤��

private:
	int		MakeSmsCode(const char *szMobile);	//���������֤��

	int		ClearTimeOutSmsCode();				//����Ѿ�ʧЧ����֤��
	int		RemoveVerifyCode(const char *szVerifyCode); //�Ƴ�ָ����֤��

private:
	int		SendVerifyCode_ihuyi(char *szMobile, int nCode); //�·���֤��
	int		SendVerifyCode_wxcn(char *szMobile, int nCode); //�·���֤��

private:
	SMS_CODE_LIST m_SmsCodeList; //��֤�����
};