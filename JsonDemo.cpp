// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

/* This executable is used for testing parser/writer using real JSON files.
 */

#include <stdafx.h>
#include "./json/json.h"
#include <algorithm> // sort
#include <stdio.h>

#include <fstream>

#if defined(_MSC_VER)  &&  _MSC_VER >= 1310
# pragma warning( disable: 4996 )     // disable fopen deprecation warning
#endif

#define GET_OFFSET(s,m) (size_t) &(((s*)0)->m)

int demo_main( int argc, const char *argv[] );

static std::string
readInputTestFile( const char *path )
{
   FILE *file = fopen( path, "rb" );
   if ( !file )
      return std::string("");
   fseek( file, 0, SEEK_END );
   long size = ftell( file );
   fseek( file, 0, SEEK_SET );
   std::string text;
   char *buffer = new char[size+1];
   buffer[size] = 0;
   if ( fread( buffer, 1, size, file ) == (unsigned long)size )
      text = buffer;
   fclose( file );
   delete[] buffer;
   return text;
}


static void
printValueTree( FILE *fout, Json::Value &value, const std::string &path = "." )
{
   switch ( value.type() )
   {
   case Json::nullValue:
      fprintf( fout, "%s=null\n", path.c_str() );
      break;
   case Json::intValue:
      fprintf( fout, "%s=%s\n", path.c_str(), Json::valueToString( value.asLargestInt() ).c_str() );
      break;
   case Json::uintValue:
      fprintf( fout, "%s=%s\n", path.c_str(), Json::valueToString( value.asLargestUInt() ).c_str() );
      break;
   case Json::realValue:
      fprintf( fout, "%s=%.16g\n", path.c_str(), value.asDouble() );
      break;
   case Json::stringValue:
      fprintf( fout, "%s=\"%s\"\n", path.c_str(), value.asString().c_str() );
      break;
   case Json::booleanValue:
      fprintf( fout, "%s=%s\n", path.c_str(), value.asBool() ? "true" : "false" );
      break;
   case Json::arrayValue:
      {
         fprintf( fout, "%s=[]\n", path.c_str() );
         int size = value.size();
         for ( int index =0; index < size; ++index )
         {
            static char buffer[16];
            sprintf( buffer, "[%d]", index );
            printValueTree( fout, value[index], path + buffer );
         }
      }
      break;
   case Json::objectValue:
      {
         fprintf( fout, "%s={}\n", path.c_str() );
         Json::Value::Members members( value.getMemberNames() );
         std::sort( members.begin(), members.end() );
         std::string suffix = *(path.end()-1) == '.' ? "" : ".";
         for ( Json::Value::Members::iterator it = members.begin(); 
               it != members.end(); 
               ++it )
         {
            const std::string &name = *it;
            printValueTree( fout, value[name], path + suffix + name );
         }
      }
      break;
   default:
      break;
   }
}


static int
parseAndSaveValueTree( const std::string &input, 
                       const std::string &actual,
                       const std::string &kind,
                       Json::Value &root,
                       const Json::Features &features,
                       bool parseOnly )
{
   Json::Reader reader( features );
   bool parsingSuccessful = reader.parse( input, root );
   if ( !parsingSuccessful )
   {
      printf( "Failed to parse %s file: \n%s\n", 
              kind.c_str(),
              reader.getFormattedErrorMessages().c_str() );
      return 1;
   }

   if ( !parseOnly )
   {
      FILE *factual = fopen( actual.c_str(), "wt" );
      if ( !factual )
      {
         printf( "Failed to create %s actual file.\n", kind.c_str() );
         return 2;
      }
      printValueTree( factual, root );
      fclose( factual );
   }
   return 0;
}


static int
rewriteValueTree( const std::string &rewritePath, 
                  const Json::Value &root, 
                  std::string &rewrite )
{
   //Json::FastWriter writer;
   //writer.enableYAMLCompatibility();
   Json::StyledWriter writer;
   rewrite = writer.write( root );
   FILE *fout = fopen( rewritePath.c_str(), "wt" );
   if ( !fout )
   {
      printf( "Failed to create rewrite file: %s\n", rewritePath.c_str() );
      return 2;
   }
   fprintf( fout, "%s\n", rewrite.c_str() );
   fclose( fout );
   return 0;
}


static std::string
removeSuffix( const std::string &path, 
              const std::string &extension )
{
   if ( extension.length() >= path.length() )
      return std::string("");
   std::string suffix = path.substr( path.length() - extension.length() );
   if ( suffix != extension )
      return std::string("");
   return path.substr( 0, path.length() - extension.length() );
}


static void
printConfig()
{
   // Print the configuration used to compile JsonCpp
#if defined(JSON_NO_INT64)
   printf( "JSON_NO_INT64=1\n" );
#else
   printf( "JSON_NO_INT64=0\n" );
#endif
}


static int 
printUsage( const char *argv[] )
{
   printf( "Usage: %s [--strict] input-json-file", argv[0] );
   return 3;
}


int
parseCommandLine( int argc, const char *argv[], 
                  Json::Features &features, std::string &path,
                  bool &parseOnly )
{
   parseOnly = false;
   if ( argc < 2 )
   {
      return printUsage( argv );
   }

   int index = 1;
   if ( std::string(argv[1]) == "--json-checker" )
   {
      features = Json::Features::strictMode();
      parseOnly = true;
      ++index;
   }

   if ( std::string(argv[1]) == "--json-config" )
   {
      printConfig();
      return 3;
   }

   if ( index == argc  ||  index + 1 < argc )
   {
      return printUsage( argv );
   }

   path = argv[index];
   return 0;
}

///zd test functions////////////////////////////////////////////////////////
//json格式字符串分析，生成Json::Value对象
static bool JsonParse(std::string strInput, Json::Value &JsonOut)
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
		//CA2CT tsLog(szLog);
		//CTraceService::TraceString(tsLog,TraceLevel_Normal);
	}

	return parsingSuccessful;
}

static bool JsonParse(TCHAR *tcsInput, size_t tSize, Json::Value &JsonOut)
{
	if(tcsInput[tSize-1] != 0) tcsInput[tSize]=0;

	CT2CA caInput(tcsInput);
	std::string strInput = (LPSTR)caInput;
	return JsonParse(strInput, JsonOut);
}

static int JsonRewriteValueTree( const Json::Value &root, std::string &strOut, const std::string &rewriteFile = "")
{
   //Json::FastWriter fwriter;
   //fwriter.enableYAMLCompatibility();
   //strOut = fwriter.write(root);

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

int zdConstructValue(const char *pStructIn, const Json::Value &jValue, Json::Value &jvResult);
static int Struct2Json(char *pStructIn, std::string strStructJsonDesc, std::string &strJsonOut)
{
	Json::Value jvRoot;
	Json::Value jvResult;

	//解析json
	JsonParse(strStructJsonDesc, jvRoot);
	
	if(jvRoot.empty()) return -1;

	//根据json描述，获取数据结构成员
	//if(jvRoot.type()==Json::ValueType::arrayValue)
	if(jvRoot.isArray())
	{
		int nSize = jvRoot.size();

		for (int index = 0;index < nSize; index++)
		{
			const Json::Value &childValue = jvRoot[index];	
			zdConstructValue(pStructIn, childValue, jvResult);
		}
	

	}
	else if(jvRoot.type()==Json::objectValue)
	{
		Json::Value::Members membs = jvRoot.getMemberNames();
		Json::Value::Members::iterator itr =  membs.begin();
		for(; itr != membs.end(); itr++)
		{
			std::string strName = *itr;
			const Json::Value &jValue = jvRoot[strName];
			zdConstructValue(pStructIn, jValue, jvResult);
		}//end of for
	}

	//数据结构转换为Json描述输出
	JsonRewriteValueTree(jvResult, strJsonOut, "d:\\test_struct2json.json");

	return 0;
}

int zdConstructValue(const char *pStructIn, const Json::Value &jValue, Json::Value &jvResult)
{


	if(jValue.isMember("KeyName") && jValue.isMember("TypeName"))
	{

		std::string strKeyName = jValue["KeyName"].asString();
		std::string strType = jValue["TypeName"].asString();
		int nTypeLen = jValue["TypeLen"].asInt();
		int nOffset = jValue["Offset"].asInt();

		//union unNumericType
		//{
		//	int nInt;
		//	unsigned int uInt;
		//	__int64 nInt64;
		//	long long lInt64;
		//	short nInt16;
		//	float fVal32;
		//	double fDouble64;
		//	WORD uInt16;
		//	DWORD uInt32;
		//	char pBuf[8];
		//}unVal;

		char szBuf[1024]={0};		
		if( stricmp(strType.c_str(), "int") == 0 || stricmp(strType.c_str(), "long") == 0)
		{
			union unInt
			{
				int nInt;
				char pInt[4];
			}unVal;
			memcpy(unVal.pInt, pStructIn + nOffset, sizeof(unVal.pInt));	
			//_itoa(unVal.nInt, szBuf, 10);

			jvResult[strKeyName] = unVal.nInt;
		}
		else if(stricmp(strType.c_str(), "dword") == 0 || stricmp(strType.c_str(), "uint") == 0)
		{
			union unDWord
			{
				unsigned int  uInt32;
				char pInt32[4];
			}unVal;

			memcpy(unVal.pInt32, pStructIn + nOffset, sizeof(unVal.pInt32));
			//_itoa(unVal.uInt32, szBuf, 10);

			jvResult[strKeyName] = unVal.uInt32;
		}
		else if(stricmp(strType.c_str(), "int64") == 0 || stricmp(strType.c_str(), "longlong") == 0)
		{
			union unInt64
			{
				long long nInt64;
				char pInt64[8];
			}unVal;

			memcpy(unVal.pInt64, pStructIn + nOffset, sizeof(unVal.pInt64));
			//_i64toa(unVal.nInt64, szBuf, 10);

			jvResult[strKeyName] = unVal.nInt64;
		}
		else if(stricmp(strType.c_str(), "word") == 0 || stricmp(strType.c_str(), "uint16") == 0)
		{
			union unWord
			{
				unsigned short uInt16;
				char pInt16[2];
			}unVal;

			memcpy(unVal.pInt16, pStructIn + nOffset, sizeof(unVal.pInt16));
			//_itoa(unVal.uInt16, szBuf, 10);

			jvResult[strKeyName] = unVal.uInt16;
		}
		else if(stricmp(strType.c_str(), "short") == 0 || stricmp(strType.c_str(), "int16") == 0)
		{
			union unWord
			{
				short nInt16;
				char pInt16[2];
			}unVal;

			memcpy(unVal.pInt16, pStructIn + nOffset, sizeof(unVal.pInt16));
			//_itoa(unVal.nInt16, szBuf, 10);

			jvResult[strKeyName] = unVal.nInt16;
		}
		else if(stricmp(strType.c_str(), "float") == 0)
		{
			union unFloat
			{
				float  fFloat;
				char pFloat[sizeof(float)];
			}unVal;

			memcpy(unVal.pFloat, pStructIn + nOffset, sizeof(unVal.pFloat));
			//sprintf(szBuf,"%f",unVal.fFloat);

			jvResult[strKeyName] = unVal.fFloat;
		}
		else if(stricmp(strType.c_str(), "double") == 0)
		{
			union unDouble
			{
				double  fDouble;
				char pFloat64[sizeof(double)];
			}unVal;

			memcpy(unVal.pFloat64, pStructIn + nOffset, sizeof(unVal.pFloat64));
			//sprintf(szBuf,"%f",unVal.fDouble);

			jvResult[strKeyName] = unVal.fDouble;
		}
		else if(stricmp(strType.c_str(), "char") == 0)
		{
			union unCharStr
			{
				char *pstr;
			}unVal;
			unVal.pstr = szBuf;
			if(nTypeLen < sizeof(szBuf))
			{
				memcpy(szBuf, pStructIn + nOffset, nTypeLen);
			}
			else //buf size is not enough
			{				
				memcpy(szBuf, pStructIn + nOffset, sizeof(szBuf)-1);
			}

			jvResult[strKeyName] = unVal.pstr;
		}
		else if(stricmp(strType.c_str(), "tchar") == 0 || stricmp(strType.c_str(), "wchar") == 0)
		{
			union unWCharStr
			{
				wchar_t tcbuf[1024];
				wchar_t pstr[1];
			}unVal;
			memset(unVal.tcbuf, 0, sizeof(unVal.tcbuf));
			_tcsncpy(unVal.tcbuf, (wchar_t*)(pStructIn + nOffset), sizeof(unVal.tcbuf)/sizeof(wchar_t) - 1);
			CT2CA csTmp(unVal.tcbuf);
			//strcpy(szBuf, csTmp.m_psz);

			jvResult[strKeyName] = csTmp.m_psz;
		}
		else //unknown datatype
		{
			szBuf[0] = 0;
			//if(nTypeLen < sizeof(szBuf))
			//{
			//	memcpy(szBuf, pStructIn + nOffset, nTypeLen);
			//}
			//else //buf size is not enough
			//{				
			//	memcpy(szBuf, pStructIn + nOffset, sizeof(szBuf)-1);
			//}

			jvResult[strKeyName] = szBuf;
		}

		//cur_pos = nOffset;
		return 0;
	}
	
	return -1;
}

////zd test functions//////////////////////////////////////

int zdTestConstructJson();
int zdTestStruct2Json();

int zdTestParseJson()
{

   std::string strJsonIn = "";
   strJsonIn = "{\
      \"cmake_variants\" : [\
         {\"name\": \"generator\",\
         \"generators\": [\
            {\"generator\": [\
               \"Visual Studio 7 .NET 2003\",\
               \"Visual Studio 9 2008\",\
               \"Visual Studio 9 2008 Win64\",\
               \"Visual Studio 10\",\
               \"Visual Studio 10 Win64\",\
               \"Visual Studio 11\",\
               \"Visual Studio 11 Win64\"\
               ]\
            },\
            {\"generator\": [\"MinGW Makefiles\"],\
            \"env_prepend\": [{\"path\": \"c:/wut/prg/MinGW/bin\"}]\
            }\
         ]\
         },\
         {\"name\": \"shared_dll\",\
         \"variables\": [\
            [\"BUILD_SHARED_LIBS=true\"],\
            [\"BUILD_SHARED_LIBS=false\"]\
         ]\
         },\
         {\"name\": \"build_type\",\
         \"build_types\": [\
            \"debug\",\
            \"release\"\
            ]\
         }\
      ]\
   }";

    std::string actualPath = "d:\\zdtest2.json";
	std::string rewritePath = "d:\\rewrite.txt";
    std::string rewriteActualPath = "d:\\actual-rewrite.txt";
	Json::Features features;
	bool parseOnly = false;
    Json::Value root;

    parseAndSaveValueTree( strJsonIn, actualPath, "input", root, features, parseOnly ); 
	
	//构建json并输出到文件
	zdTestStruct2Json();

	return 0;
}

int zdTestStruct2Json()
{
	struct STTest
	{
		int		nID;
		TCHAR	szName[33];
		float	fHeight;
		double	fWeigth;
		WORD	wAge;
		DWORD	dwProcess;
	};
	STTest st1;
	Json::Value stDesc;
	Json::Value vMemb;

	st1.nID = 1;
	_tcscpy(st1.szName, TEXT("TEST NAME"));
	st1.fHeight = 1.82;
	st1.fWeigth = 70.5;
	st1.wAge = 26;
	st1.dwProcess = 223123;

	const char *KEY = "KeyName";
	const char *TYPE = "TypeName";
	const char *LEN = "TypeLen";
	const char *OFFSET = "Offset";

	int n=0;
	vMemb[KEY] = "nid";
	vMemb[TYPE] = "int";
	vMemb[LEN] = sizeof(int);
	vMemb[OFFSET] = GET_OFFSET(STTest, nID);
	//stDesc["Memb01"] = vMemb;
	stDesc.append(vMemb);

	vMemb[KEY] = "szName";
	vMemb[TYPE] = "tchar";
	vMemb[LEN] = sizeof(st1.szName);
	vMemb[OFFSET] = GET_OFFSET(STTest, szName);
	//stDesc["Memb02"] = vMemb;
	stDesc.append(vMemb);

	vMemb[KEY] = "Height";
	vMemb[TYPE] = "float";
	vMemb[LEN] = sizeof(float);
	vMemb[OFFSET] = GET_OFFSET(STTest, fHeight);
	//stDesc["Memb03"] = vMemb;
	stDesc.append(vMemb);

	vMemb[KEY] = "Weigth";
	vMemb[TYPE] = "double";
	vMemb[LEN] = sizeof(double);
	vMemb[OFFSET] = GET_OFFSET(STTest, fWeigth);
	//stDesc["Memb04"] = vMemb;
	stDesc.append(vMemb);

	vMemb[KEY] = "Age";
	vMemb[TYPE] = "word";
	vMemb[LEN] = sizeof(WORD);
	vMemb[OFFSET] = GET_OFFSET(STTest, wAge);
	//stDesc["Memb05"] = vMemb;
	stDesc.append(vMemb);

	vMemb[KEY] = "Process";
	vMemb[TYPE] = "dword";
	vMemb[LEN] = sizeof(DWORD);
	vMemb[OFFSET] = GET_OFFSET(STTest, dwProcess);
	//stDesc["Memb06"] = vMemb;
	stDesc.append(vMemb);

	const std::string rewritePath = ""; 
    std::string strStDesc; 
	
	JsonRewriteValueTree(stDesc, strStDesc);

	std::string strJsonOut;
	Struct2Json((char *)(&st1), strStDesc, strJsonOut);

	
	Json::StyledWriter swriter;
	std::ofstream ofs;

	std::string str = swriter.write(strStDesc);
	ofs.open("d:\\struct2json_styled.json");
	ofs << str;
	ofs.close();

	return 0;
}

int zdTestConstructJson()
{
	Json::Value root;
	Json::Value vTodayRate1;
	Json::Value vTodayRate2;
	Json::Value vTodayRate;

	wchar_t szRate[10][20]={{L"0.1"},{L"0.2"},{L"0.3"},{L"0.4"},{L"0.5"}, {L"0.6"}, {L"0.7"}, {L"0.8"}, {L"0.9"}, {L"1.0"}};

	//构建json数组成员
	for(int i = 0; i < 10; i++)
	{
		CT2CA caTodayRate(szRate[i]);
		vTodayRate1["TodayRate"]= caTodayRate.m_psz;
		root["TodayOpenRate"].append(vTodayRate1);
	}
	
	vTodayRate["TodayRate01"] = 0.1;
	vTodayRate["TodayRate02"] = 0.2;
	vTodayRate["TodayRate03"] = 0.3;
	vTodayRate["TodayRate04"] = 0.4;
	vTodayRate["TodayRate05"] = 0.5;
	vTodayRate["TodayRate06"] = 0.6;
	vTodayRate["TodayRate07"] = 0.7;
	vTodayRate["TodayRate08"] = 0.8;
	vTodayRate["TodayRate09"] = 0.9;
	vTodayRate["TodayRate10"] = 1.0;
	root["TodayOpenRate2"] = vTodayRate;

	/////拆分【双,单,大,小】四种比率
	std::string str = "0.11,0.12,0.13,0.14";
	size_t nPosBegin = 0;
	size_t nPos =0;
	std::string str2;
	const char *szKeys[4] = {"RateDouble", "RateSingle", "RateBig", "RateSmall"};
	for(int n = 0; n < 4; n++)
	{
		nPos = str.find(',', nPosBegin);
		str2.clear();
		if(nPos !=  std::string::npos)
		{
			str2 = str.substr( nPosBegin, nPos - nPosBegin);
			nPosBegin = nPos+1;
			vTodayRate2[szKeys[n]] = str2;
		}
		else 
		{
			str2 = str.substr(nPosBegin);
			vTodayRate2[szKeys[n]] = str2;
			break;
		}				
	}
	root["TodayOpenRate3"].append(vTodayRate2);
	root["TodayOpenRate3"].append(vTodayRate2);
	root["TodayOpenRate3"].append(vTodayRate2);
	////////////////

	std::string strJson = root.toStyledString();
	
	std::ofstream ofs;
	//std::string str = swriter.write(root);
	ofs.open("d:\\json_styled_string.json");
	ofs << strJson;
	ofs.close();

	return 0;
}

int demo_main( int argc, const char *argv[] )
{
   std::string path;
   Json::Features features;
   bool parseOnly;
   int exitCode = parseCommandLine( argc, argv, features, path, parseOnly );
   ///del by zdleek
   //if ( exitCode != 0 )
   //{
   //   return exitCode;
   //}
	
   path =  "d:\\agent_vmw7.json";
   try
   {
      std::string input = readInputTestFile( path.c_str() );
      if ( input.empty() )
      {
         printf( "Failed to read input or empty input: %s\n", path.c_str() );
         return 3;
      }

      std::string basePath = removeSuffix( argv[1], ".json" );
	  basePath = "d:\\";
      if ( !parseOnly  &&  basePath.empty() )
      {
         printf( "Bad input path. Path does not end with '.expected':\n%s\n", path.c_str() );
         return 3;
      }

      std::string actualPath = basePath + ".actual";
      std::string rewritePath = basePath + ".rewrite";
      std::string rewriteActualPath = basePath + ".actual-rewrite";

      Json::Value root;
      exitCode = parseAndSaveValueTree( input, actualPath, "input", root, features, parseOnly );
      if ( exitCode == 0  &&  !parseOnly )
      {
         std::string rewrite;
         exitCode = rewriteValueTree( rewritePath, root, rewrite );
         if ( exitCode == 0 )
         {
            Json::Value rewriteRoot;
            exitCode = parseAndSaveValueTree( rewrite, rewriteActualPath, 
               "rewrite", rewriteRoot, features, parseOnly );
         }
      }
   }
   catch ( const std::exception &e )
   {
      printf( "Unhandled exception:\n%s\n", e.what() );
      exitCode = 1;
   }

   return exitCode;
}

