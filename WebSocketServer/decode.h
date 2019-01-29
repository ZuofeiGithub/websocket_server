// GB2312, BIG5, UTF8之间的转换通过Unicode充当切换的桥梁
// 例如: GB2312<->Unicode<->BIG5
#ifdef _WIN32
#include <Windows.h>
#define CODEPAGE_GB2312		936
#define CODEPAGE_BIG5		950 
#define CODEPAGE_UTF8		65001

// 转换成Unicode
wchar_t* ToWideChar(const char* pStr, int nCodePage)
{
	int nStrLen = MultiByteToWideChar(nCodePage, 0, pStr, -1, NULL, 0);
	wchar_t* pWStr = new wchar_t[nStrLen + 1];
	memset(pWStr, 0, nStrLen + 1);
	MultiByteToWideChar(nCodePage, 0, pStr, -1, pWStr, nStrLen);
	return pWStr;
}

// 转换成多字节
char* ToMultiByte(const wchar_t* pWStr, int nCodePage)
{
	int nStrLen = WideCharToMultiByte(nCodePage, 0, pWStr, -1, NULL, 0, NULL, NULL);
	char* pStr = new char[nStrLen + 1];
	memset(pStr, 0, nStrLen + 1);
	WideCharToMultiByte(nCodePage, 0, pWStr, -1, pStr, nStrLen, NULL, NULL);
	return pStr;
}

char* CPTransform(const char* pStrSrc, int nCPSrc, int nCPTar)
{
	wchar_t* pWStr = ToWideChar(pStrSrc, nCPSrc);
	char* pStr = ToMultiByte(pWStr, nCPTar);
	delete[] pWStr;
	return pStr;
}

// GB2312转UTF8
char* GB2312ToUTF8(const char* pStrGB2312)
{
	return CPTransform(pStrGB2312, CODEPAGE_GB2312, CODEPAGE_UTF8);
}

// UTF8转GB2312
char* UTF8ToGB2312(const char* pStrUTF8)
{
	return CPTransform(pStrUTF8, CODEPAGE_UTF8, CODEPAGE_GB2312);
}

// GB2312转BIG5
char* GB2312ToBIG5(const char* pStrBIG5)
{
	return CPTransform(pStrBIG5, CODEPAGE_GB2312, CODEPAGE_BIG5);
}

// BIG5转GB2312
char* BIG5ToGB2312(const char* pStrBIG5)
{
	return CPTransform(pStrBIG5, CODEPAGE_BIG5, CODEPAGE_GB2312);
}

// UTF8转BIG5
char* UTF8ToBIG5(const char* pStrUTF8)
{
	return CPTransform(pStrUTF8, CODEPAGE_UTF8, CODEPAGE_BIG5);
}

// BIG5转UTF8
char* BIG5ToUTF8(const char* pStrBIG5)
{
	return CPTransform(pStrBIG5, CODEPAGE_BIG5, CODEPAGE_UTF8);
}
#endif
