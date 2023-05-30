#pragma once
#ifndef UTILITY_H
#define UTILITY_H
#include <vector>

/* 工具函数类 */
class Utility {
public:
	template<class T>
	static void copy(T* from, T* to, int count)    /* static void DWordCopy(int* src, int* dst, int count) */
	{
		while (count--)
		{
			*to++ = *from++;
		}
	}
	static void StringCopy(const char* src, char* dst)
	{
		while ((*dst++ = *src++) != 0);
	}
	static int strlen(char* pString)
	{
		int length = 0;
		char* pChar = pString;

		while (*pChar++)
		{
			length++;
		}

		/* 返回字符串长度 */
		return length;
	}
	/* 解析交互命令 */
	static vector<char*> parseCmd(char* s)
	{
		char* p = s, * q = s;
		vector<char*> result;
		while (*q != '\0')
		{
			if (*p == ' ')
			{
				p++;
				q++;
			} 
			else
			{
				while (*q != '\0' && *q != ' ') 
				{
					q++;
				}
				char* newString = new char[q - p + 1];
				for (int i = 0; i < q - p; i++) 
				{
					newString[i] = *(p + i);
				}
				newString[q - p] = '\0';
				result.push_back(newString);
				p = q;
			}
		}
		return result;
	}
};

#endif 




