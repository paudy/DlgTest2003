#pragma once
#include <string>

//Unicode16转ANSI
static std::string UnicodeToANSI(const wchar_t* str)
{
    char*  pElementText;
    int    iTextLen;
    // wide char to multi char
    iTextLen = WideCharToMultiByte( CP_ACP,
        0,
        str,
        -1,
        NULL,
        0,
        NULL,
        NULL );
    pElementText = new char[iTextLen + 1];
    memset( ( void* )pElementText, 0, sizeof( char ) * ( iTextLen + 1 ) );
    ::WideCharToMultiByte( CP_ACP,
        0,
        str,
        -1,
        pElementText,
        iTextLen,
        NULL,
        NULL );
    std::string strText;
    strText = pElementText;
    delete[] pElementText;

	return strText;
}

//ANSI转Unicode16:
static std::wstring ANSIToUnicode(const std::string& str)
{
    int  len = 0;
    len = str.length();
    int  unicodeLen = ::MultiByteToWideChar( CP_ACP,
        0,
        str.c_str(),
        -1,
        NULL,
        0 ); 
    wchar_t *  pUnicode; 
    pUnicode = new  wchar_t[unicodeLen+1]; 
    memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t)); 
    ::MultiByteToWideChar( CP_ACP,
        0,
        str.c_str(),
        -1,
        (LPWSTR)pUnicode,
        unicodeLen ); 
    std::wstring  rt; 
    rt = ( wchar_t* )pUnicode;
    delete  pUnicode;

    return  rt; 
}


//Unicode16转UTF-8 :
typedef unsigned __int32 uint32_t;
typedef unsigned short  uint16_t;

static uint32_t Unicode16ToUTF8(IN const uint16_t* pszUtf16, IN uint32_t nSizeUnicode16, IN char* pszUtf8, IN uint32_t nSizeUtf8)  
{  
    if (0 == nSizeUtf8)
    {
        return (nSizeUnicode16 * 3 + 1);
    }

    uint32_t i = 0, count = 0;       
    uint16_t wch;  
    for(i=0; i < nSizeUnicode16; i++) 
    {
        wch = *(uint16_t*)&pszUtf16[i];       
        if( wch < 0x80)  
        {
            if ((count+1) >= nSizeUtf8)
            {
                break;
            }
            pszUtf8[count] = wch & 0x7f;  
            count++;  
        }  
        else if( wch >= 0x80 && wch < 0x07ff)  
        {
            if ((count+2) >= nSizeUtf8)
            {
                break;
            }
            //tmp1 = wch >> 6;  
            pszUtf8[count] = 0xC0 | (0x1F & wch>>6);  
            pszUtf8[count+1] = 0x80 | (0x3F & wch);  
            count += 2;  
        }  
        else if( wch>=0x0800 )  
        {
            if ((count+3) >= nSizeUtf8)
            {
                break;
            }
            //tmp1 = wch >> 12;  
            pszUtf8[count] = 0xE0 | (0x0F & wch>>12);  
            pszUtf8[count+1] = 0x80 | ((0x0FC0 & wch)>>6);  
            pszUtf8[count+2] = 0x80 | (0x003F & wch);  
             
            count += 3;  
        }     
        else  
        {  
            printf("error/n");  
        }  
    }

    if (count < nSizeUtf8)
    {
        pszUtf8[count] = 0;
        return (count + 1);
    }
    else if (nSizeUtf8 > 0)
    {
        pszUtf8[nSizeUtf8-1] = 0;
        return nSizeUtf8;
    }

    return 0;  
}


//UTF-8转Unicode16:
static uint32_t UTF8ToUnicode16(IN const unsigned char* pszUtf8, IN uint32_t nSizeUtf8, OUT uint16_t* pszUtf16, IN uint32_t nSizeUnicode16)  
{  
    if (0 == nSizeUnicode16)
    {
        return (nSizeUtf8 + 1);
    }

    uint32_t count = 0, i = 0;  
    uint16_t wch;  
    uint16_t *p;  
    for(i = 0; count < nSizeUtf8 && i < nSizeUnicode16; i++) 
    {
        p = (uint16_t*)&pszUtf16[i];  
         
        if( pszUtf8[count] < 0x80)  
        {  
            wch = pszUtf8[count];             
            count++;  
        }  
        else if( (pszUtf8[count] < 0xDF) && (pszUtf8[count] >= 0x80))  
        {  
            wch = pszUtf8[count] & 0x1F;  
            wch = wch << 6;  
            wch += pszUtf8[count+1] & 0x3F;  
            count += 2;  
        }  
        else if( (pszUtf8[count] <= 0xEF) && (pszUtf8[count] >= 0xDF))  
        {  
            wch = pszUtf8[count] & 0x0F;  
            wch = wch << 6;  
            wch += pszUtf8[count+1] & 0x3F;  
            wch = wch << 6;  
            wch += pszUtf8[count+2] & 0x3F;  
            count += 3;  
        }  
        else  
        {  
            printf("error!/n");  
        }  
        *p = wch;  
    }  

    if (i < nSizeUnicode16)
    {
        pszUtf16[i] = 0;
        return (i + 1);
    }
    else if (nSizeUnicode16 > 0)
    {
        pszUtf16[nSizeUnicode16-1] = 0;
        return nSizeUnicode16;
    }
 
    return 0;  
}  