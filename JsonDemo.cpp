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
	char szContent[]={"this is test string for jsoncpp by zdleek"};
	Json::Value root;
	root["nID"] = 10;
	root["nLen"] = strlen(szContent);
	root["szDesc"] = szContent;
	
	Json::StyledWriter swriter;
	std::ofstream ofs;

	std::string str = swriter.write(root);
	ofs.open("d:\\example_styled_writer.json");
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

