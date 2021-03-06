#pragma once
#include <map>

//短信验证码处理

struct tagSmsCode
{
	int		nRandCode;			//随机验证码
	char	szMobileID[15];		//手机号
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
	int HttpPostSms_ihuyi(char *szMobile); //通过ihuyi.com SDK接口下发短信验证码
	int HttpPostSms_wxcn(char* szMobile); //通过wxchina.com接口下发短信验证码
	int	VerifySmsCode(const char *szVerifyCode, const char *szMobile);		//匹配手机号与验证码

private:
	int		MakeSmsCode(const char *szMobile);	//生成随机验证码

	int		ClearTimeOutSmsCode();				//清除已经失效的验证码
	int		RemoveVerifyCode(const char *szVerifyCode); //移除指定验证码

private:
	int		SendVerifyCode_ihuyi(char *szMobile, int nCode); //下发验证码
	int		SendVerifyCode_wxcn(char *szMobile, int nCode); //下发验证码

private:
	SMS_CODE_LIST m_SmsCodeList; //验证码队列
};