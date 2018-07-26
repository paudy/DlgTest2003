////////////////////////////////////
//Filename: zdJsonWrap.cpp
//Created by zdleek on 2018.06 base on JsonCpp r0.62
//Description: the json wrapper
///////////////////////////////////
#include "stdafx.h"
#include "zdJsonWrap.h"
#include <fstream>


//*** 函数定义***
//json格式字符串分析，生成Json::Value对象
bool JsonParse(std::string strInput, Json::Value &JsonOut)
{
	Json::Features features;
	Json::Reader reader( features );
	bool parsingSuccessful = reader.parse( strInput, JsonOut );
	if ( !parsingSuccessful )
	{
		char szLog[4096];
		sprintf(szLog, "Failed to parse json: %s \n  %s\n", 
			strInput.c_str(),
			reader.getFormattedErrorMessages().c_str() );
		CA2CT tsLog(szLog);
		CTraceService::TraceString(tsLog,TraceLevel_Normal);
	}

	return parsingSuccessful;
}

//json格式字符串分析，生成Json::Value对象
bool JsonParse(TCHAR *tcsInput, size_t tSize, Json::Value &JsonOut)
{
	if(tcsInput[tSize-1] != 0) tcsInput[tSize]=0;

	CT2CA caInput(tcsInput);
	std::string strInput = (LPSTR)caInput;
	return JsonParse(strInput, JsonOut);
}

//json格式字符串分析，生成Json::Value对象
bool JsonParse(char *szInput, size_t tSize, Json::Value &JsonOut)
{
	if(szInput[tSize-1] != 0) szInput[tSize]=0;

	std::string strInput = szInput;
	return JsonParse(strInput, JsonOut);
}

//json::value值输出为格式化的json字符串
int JsonRewriteValueTree( const Json::Value &root, std::string &strOut, const std::string &rewriteFile /*= ""*/)
{
   //Json::FastWriter writer;
   //writer.enableYAMLCompatibility();

   Json::StyledWriter writer;
   strOut = writer.write( root );

   if(rewriteFile.length() > 3)
   {
		FILE *fout = fopen( rewriteFile.c_str(), "wt" );
		if ( !fout )
		{
			printf( "Failed to create rewrite file: %s\n", rewriteFile.c_str() );
			return 2;
		}
		fprintf( fout, "%s\n", strOut.c_str() );
		fclose( fout );
   }

   return 0;
}

//数据结构转换为JSON描述
int zdConstructValue(const char *pStructIn, const Json::Value &jValue, Json::Value &jvResult);

int Struct2Json(const char *pStructIn, const Json::Value &stJsonRoot, std::string &strJsonOut)
{
	//Json::Value jvRoot;
	Json::Value jvResult;
	
	if(stJsonRoot.empty()) return -1;

	//根据json描述，获取数据结构成员
	if(stJsonRoot.isArray())
	{
		int nSize = stJsonRoot.size();

		for (int index = 0;index < nSize; index++)
		{
			const Json::Value &childValue = stJsonRoot[index];	
			zdConstructValue(pStructIn, childValue, jvResult);
		}
	}
	else if(stJsonRoot.type()==Json::objectValue)
	{
		Json::Value::Members membs = stJsonRoot.getMemberNames();
		Json::Value::Members::iterator itr =  membs.begin();
		for(; itr != membs.end(); itr++)
		{
			std::string strName = *itr;
			const Json::Value &jValue = stJsonRoot[strName];
			zdConstructValue(pStructIn, jValue, jvResult);
		}//end of for
	}
	else
	{
		return -2;
	}

	//数据结构转换为Json描述输出
	JsonRewriteValueTree(jvResult, strJsonOut); 
	//JsonRewriteValueTree(jvResult, strJsonOut, "d:\\test_struct2json.json");
	return 0;
}

//根据数据结构的json描述,构造json::value对象
int zdConstructValue(const char *pStructIn, const Json::Value &jValue, Json::Value &jvResult)
{

	if(jValue.isMember(ZD_KEY) && jValue.isMember(ZD_TYPE) &&  jValue.isMember(ZD_OFFSET))
	{
		std::string strKeyName = jValue[ZD_KEY].asString();
		std::string strType = jValue[ZD_TYPE].asString();
		int nTypeLen = jValue[ZD_LEN].asInt();
		size_t nOffset = jValue[ZD_OFFSET].asUInt();
		const char * pBeginPos = pStructIn + nOffset;

		//zdtestlog
		TCHAR tcsValLog[512] = {0};//for test log only

		if( stricmp(strType.c_str(), "int") == 0 || stricmp(strType.c_str(), "long") == 0)
		{
			union unInt
			{
				int nInt;
				char pInt[4];
			}unVal;
			memcpy(unVal.pInt, pBeginPos, sizeof(unVal.pInt));	
	
			jvResult[strKeyName] = unVal.nInt;

			//zd test log
			wsprintf(tcsValLog, TEXT("Val=%d"), unVal.nInt);
		}
		else if(stricmp(strType.c_str(), "DWORD") == 0 || stricmp(strType.c_str(), "UINT") == 0)
		{
			union unDWord
			{
				unsigned int  uInt32;
				char pInt32[4];
			}unVal;

			memcpy(unVal.pInt32, pBeginPos, sizeof(unVal.pInt32));
			
			jvResult[strKeyName] = unVal.uInt32;

			//zd test log
			wsprintf(tcsValLog, TEXT("Val=%u"), unVal.uInt32);
			
		}
		else if(stricmp(strType.c_str(), "INT64") == 0 || stricmp(strType.c_str(), "LONGLONG") == 0)
		{
			union unInt64
			{
				long long nInt64;
				char pInt64[8];
			}unVal;

			memcpy(unVal.pInt64, pBeginPos, sizeof(unVal.pInt64));

			jvResult[strKeyName] = unVal.nInt64;

			//zd test log
			wsprintf(tcsValLog, TEXT("Val=%I64d"), unVal.nInt64);

		}
		else if(stricmp(strType.c_str(), "WORD") == 0 || stricmp(strType.c_str(), "UINT16") == 0)
		{
			union unWord
			{
				unsigned short uInt16;
				char pInt16[2];
			}unVal;

			memcpy(unVal.pInt16, pBeginPos, sizeof(unVal.pInt16));

			jvResult[strKeyName] = unVal.uInt16;

			//zd test log
			wsprintf(tcsValLog, TEXT("Val=%u"), unVal.uInt16);
		}
		else if(stricmp(strType.c_str(), "SHORT") == 0 || stricmp(strType.c_str(), "INT16") == 0)
		{
			union unWord
			{
				short nInt16;
				char pInt16[2];
			}unVal;

			memcpy(unVal.pInt16, pBeginPos, sizeof(unVal.pInt16));

			jvResult[strKeyName] = unVal.nInt16;
			
			//zd test log
			wsprintf(tcsValLog, TEXT("Val=%d"), unVal.nInt16);
		}
		else if(stricmp(strType.c_str(), "FLOAT") == 0)
		{
			union unFloat
			{
				float  fFloat;
				char pFloat[sizeof(float)];
			}unVal;

			memcpy(unVal.pFloat, pBeginPos, sizeof(unVal.pFloat));

			jvResult[strKeyName] = unVal.fFloat;

			//zd test log
			wsprintf(tcsValLog, TEXT("Val=%f"), unVal.fFloat);
		}
		else if(stricmp(strType.c_str(), "DOUBLE") == 0)
		{
			union unDouble
			{
				double  fDouble;
				char pFloat64[sizeof(double)];
			}unVal;

			memcpy(unVal.pFloat64, pBeginPos, sizeof(unVal.pFloat64));
			//sprintf(szBuf,"%f",unVal.fDouble);

			jvResult[strKeyName] = unVal.fDouble;
			//zd test log
			wsprintf(tcsValLog, TEXT("Val=%f"), unVal.fDouble);
		}
		else if(stricmp(strType.c_str(), "CHAR") == 0)
		{
			union unCharStr
			{	
				char szBuf[1280];
				char pstr[1];
			}unVal;
			memset(unVal.szBuf, 0, sizeof(unVal.szBuf));
			
			if(nTypeLen < sizeof(unVal.szBuf))
			{
				memcpy(unVal.szBuf, pBeginPos, nTypeLen);
			}
			else //buf size is not enough
			{				
				memcpy(unVal.szBuf, pBeginPos, sizeof(unVal.szBuf)-1);
			}

			jvResult[strKeyName] = unVal.pstr;

			//zd test log
			CA2CT ctVal(unVal.pstr);
			wsprintf(tcsValLog, TEXT("Val=%s"), ctVal.m_psz);
		}
		else if(stricmp(strType.c_str(), "TCHAR") == 0 || stricmp(strType.c_str(), "WCHAR") == 0)
		{
			union unWCharStr
			{
				wchar_t tcbuf[1024];
				wchar_t pstr[1];
			}unVal;
			memset(unVal.tcbuf, 0, sizeof(unVal.tcbuf));
			_tcsncpy(unVal.tcbuf, (wchar_t*)(pBeginPos), sizeof(unVal.tcbuf)/sizeof(wchar_t) - 1);
			CT2CA csTmp(unVal.tcbuf); //unicode转换为ansi
			//strcpy(szBuf, csTmp.m_psz);

			jvResult[strKeyName] = csTmp.m_psz;

			//zd test log
			_tcsncpy(tcsValLog, unVal.tcbuf, sizeof(tcsValLog)/sizeof(wchar_t) - 1);

		}
		else if(stricmp(strType.c_str(), "BYTE") == 0)
		{
			union unByteStr
			{	
				char szBuf[1280];
				char pstr[1];
			}unVal;
			memset(unVal.szBuf, 0, sizeof(unVal.szBuf));

			if(nTypeLen < sizeof(unVal.szBuf))
			{
				memcpy(unVal.szBuf, pStructIn + nOffset, nTypeLen);
				if(nTypeLen == 1)
				{
					jvResult[strKeyName] = (WORD)(unVal.pstr[0]);
				}
				else
				{
					jvResult[strKeyName] = (BYTE *)unVal.pstr;
				}
			}
			else //buf size is not enough
			{				
				memcpy(unVal.szBuf, pBeginPos, sizeof(unVal.szBuf)-1);
				jvResult[strKeyName] = unVal.pstr;
			}
			
			//zd test log
			wsprintf(tcsValLog, TEXT("Val=%d"), (BYTE)(unVal.pstr[0]));
		}
		else //unknown datatype
		{
			//union unCharStr
			//{	
			//	char szBuf[1280];
			//	char pstr[1];
			//}unVal;
			//unVal.szBuf[0] = 0;
			//if(nTypeLen < sizeof(unVal.szBuf))
			//{
			//	memcpy(unVal.szBuf, pStructIn + nOffset, nTypeLen);
			//}
			//else //buf size is not enough
			//{				
			//	memcpy(unVal.szBuf, pStructIn + nOffset, sizeof(szBuf)-1);
			//}

			jvResult[strKeyName] = "";
			
			//zd test log
			wsprintf(tcsValLog, TEXT("Val=%s"), TEXT("Unknown DATA_TYPE"));
		}

		//cur_pos = nOffset;
		
		////zd test log
		//TCHAR szInfo[1260] = {0};
		//std::string str1 = strKeyName + "," + strType;		
		//CA2CT tsLog((char*)str1.c_str());
		//wsprintf(szInfo, TEXT("%s : %s, TypeLen=%d, Offset=%d, %s\r\n"),   AnsiToUnicode(__FUNCTION__), tsLog.m_psz, nTypeLen, nOffset, tcsValLog);
		//CTraceService::TraceString(szInfo, TraceLevel_Debug);

		return 0;
	}
	
	return -1;
}

int Json2Struct(void *pStructIn, char *szStructJsonDesc, std::string &strJsonOut)
{
	//暂未实现
	return 0;
}

//次函数为测试用例
int zdTestStruct2Json()
{
	//测试用的数据结构
	struct STTest
	{
		WORD	nID;
		TCHAR	szName[33];
		float	fHeight;
		double	fWeigth;
		WORD	wAge;
		DWORD	dwProcess;
		BYTE	cbAgent;
	};
	STTest st1;
	Json::Value stDesc;
	Json::Value vMemb;

	st1.nID = 3;
	_tcscpy(st1.szName, TEXT("TEST NAME"));
	st1.fHeight = 1.82;
	st1.fWeigth = 70.5;
	st1.wAge = 26;
	st1.dwProcess = 223123;
	st1.cbAgent =2;

	//const char *ZD_KEY = "KeyName";
	//const char *ZD_TYPE = "TypeName";
	//const char *ZD_LEN = "TypeLen";
	//const char *ZD_OFFSET = "Offset";

	//构造数据结构的json描述
	//int n=0;
	vMemb[ZD_KEY] = "ID";
	vMemb[ZD_TYPE] = "WORD";
	vMemb[ZD_LEN] = sizeof(WORD);
	vMemb[ZD_OFFSET] = GET_OFFSET(STTest, nID);
	//stDesc["Memb01"] = vMemb;
	stDesc.append(vMemb);

	vMemb[ZD_KEY] = "Name";
	vMemb[ZD_TYPE] = "tchar";
	vMemb[ZD_LEN] = sizeof(st1.szName);
	vMemb[ZD_OFFSET] = GET_OFFSET(STTest, szName);
	//stDesc["Memb02"] = vMemb;
	stDesc.append(vMemb);

	vMemb[ZD_KEY] = "Height";
	vMemb[ZD_TYPE] = "float";
	vMemb[ZD_LEN] = sizeof(float);
	vMemb[ZD_OFFSET] = GET_OFFSET(STTest, fHeight);
	//stDesc["Memb03"] = vMemb;
	stDesc.append(vMemb);

	vMemb[ZD_KEY] = "Weigth";
	vMemb[ZD_TYPE] = "double";
	vMemb[ZD_LEN] = sizeof(double);
	vMemb[ZD_OFFSET] = GET_OFFSET(STTest, fWeigth);
	//stDesc["Memb04"] = vMemb;
	stDesc.append(vMemb);

	vMemb[ZD_KEY] = "Age";
	vMemb[ZD_TYPE] = "word";
	vMemb[ZD_LEN] = sizeof(WORD);
	vMemb[ZD_OFFSET] = GET_OFFSET(STTest, wAge);
	//stDesc["Memb05"] = vMemb;
	stDesc.append(vMemb);

	vMemb[ZD_KEY] = "cbAgent";
	vMemb[ZD_TYPE] = "BYTE";
	vMemb[ZD_LEN] = sizeof(BYTE);
	vMemb[ZD_OFFSET] = GET_OFFSET(STTest, cbAgent);
	//stDesc["Memb06"] = vMemb;
	stDesc.append(vMemb);

	vMemb[ZD_KEY] = "Process";
	vMemb[ZD_TYPE] = "dword";
	vMemb[ZD_LEN] = sizeof(DWORD);
	vMemb[ZD_OFFSET] = GET_OFFSET(STTest, dwProcess);
	//stDesc["Memb07"] = vMemb;
	stDesc.append(vMemb);

	const std::string rewritePath = ""; 
    std::string strStDesc; 
	
	//JsonRewriteValueTree(stDesc, strStDesc);

	std::string strJsonOut;
	Struct2Json((char *)(&st1), stDesc, strJsonOut);

	//输出到磁盘文件
	Json::StyledWriter swriter;
	std::ofstream ofs;

	std::string str = swriter.write(strJsonOut);
	ofs.open("d:\\zdTestStruct2Json_styled.json");
	ofs << str;
	ofs.close();

	return 0;
}

////end of json functions/////////////////////////////////////