////////////////////////////////////
//Filename: zdJsonWrap.h
//Created by zdleek on 2018.06 base on JsonCpp r0.62
//Description: the json wrapper
///////////////////////////////////
#pragma once

#include "json/json.h"

//***宏定义***
//获取数据结构成员的偏移位置
#define GET_OFFSET(stt,memb) (size_t) &(((stt*)0)->memb)

static const char *const ZD_KEY = "KeyName";
static const char *const ZD_TYPE = "TypeName";
static const char *const ZD_LEN = "TypeLen";
static const char *const ZD_OFFSET = "Offset";

bool JsonParse(std::string strInput, Json::Value &JsonOut);

bool JsonParse(TCHAR *tcsInput, size_t tSize, Json::Value &JsonOut);

bool JsonParse(char *szInput, size_t tSize, Json::Value &JsonOut);

int JsonRewriteValueTree( const Json::Value &root, std::string &strOut, const std::string &rewriteFile = "");

int Struct2Json(const char *pStructIn, const Json::Value &stJsonDesc, std::string &strJsonOut);

int zdTestStruct2Json();
