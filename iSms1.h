#pragma once
#include <map>

//短信验证码处理

struct tagSmsCode
{
	int		nRandCode;			//随机验证码
	char	szMobileID[20];		//手机号
	time_t tmValidate;			//有效时限

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
	int		MakeSmsCode(const char *szMobile);	//生成随机验证码
	int		SendSmsCode(tagSmsCode *pSmsCode);						//通过第三方SDK发送短信验证码给用户手机
	int		VerifySmsCode(const char *szVerifyCode, const char *szMobile);					//匹配手机号与验证码
	int		ClearTimeOutSmsCode();				//清除已经失效的验证码
	int		RemoveVerifyCode(const char *szVerifyCode); //移除指定验证码

private:
	int InitSmsSdk();
	size_t send_sms(char *account, char *password, char *mobile, char *content);
	size_t http_post(char *page, char *poststr);

private:
	SMS_CODE_LIST m_SmsCodeList; //验证码队列
};