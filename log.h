//使用说明
//LogText:只输出字符串    自动换行
//LogData 使用可变参数输出字符串或数字,使用方法和printf相同,  自动换行
//example
		//DefineVar();//须定义为全局
		//wchar_t buffer[] = L"tom";
		//OpenFile();
		//LogText(L"*********");
		//LogData(L"the age of %s is %d,%x",buffer,24,24);
		//LogText(L"********");
		//CloseFile();  

#ifndef __LOG_H__
#define __LOG_H__

#include <windows.h>
#include<strsafe.h>
#pragma  warning(disable : 4995) 

#ifndef __LOG
//#define __LOG     //ON/OFF LOG
#endif

#ifndef __DBGVIEW
//#define __DBGVIEW //for DbgView  
#endif

#ifndef __LOGFILE
//#define __LOGFILE //for Log file
#endif


//定义一次输出字符时临时缓冲的长度
//如果字符过长 会截去多余部分 不会溢出
#define LOG_BUFFER_SIZE 512          
#define LOGFILEPATH L"c:\\soundfilter_log.txt" //定义文件名 

#ifdef __LOG

	#if defined(__DBGVIEW) && !defined(__LOGFILE)
		extern	wchar_t  g_LogBuffer[LOG_BUFFER_SIZE];
		extern	wchar_t  g_LogBuffer2[LOG_BUFFER_SIZE];

		#define DefineVar()\
		wchar_t g_LogBuffer[LOG_BUFFER_SIZE];\
		wchar_t g_LogBuffer2[LOG_BUFFER_SIZE];

		#define OpenFile()
		#define CloseFile()
		#define FlushData()

		#define LogData(Format,...)\
		{\
			StringCchPrintf(g_LogBuffer,LOG_BUFFER_SIZE, Format, __VA_ARGS__);\
			StringCchPrintf(g_LogBuffer2,LOG_BUFFER_SIZE, L"%s\r\n", g_LogBuffer);\
			OutputDebugString(g_LogBuffer2);\
		}
	    #define LogText(Str)\
		{\
			LogData(L"%s",Str);\
		}

	#elif defined(__LOGFILE) && !defined(__DBGVIEW)
		extern	FILE* g_pLogFile;
		extern	wchar_t  g_LogBuffer[LOG_BUFFER_SIZE];
		extern	wchar_t  g_LogBuffer2[LOG_BUFFER_SIZE];

		#define DefineVar()\
			FILE* g_pLogFile;\
		    wchar_t g_LogBuffer[LOG_BUFFER_SIZE];\
		    wchar_t g_LogBuffer2[LOG_BUFFER_SIZE];
			

		#define OpenFile()\
		{\
			SYSTEMTIME st;\
			GetLocalTime(&st);\
			StringCchPrintf(g_LogBuffer, LOG_BUFFER_SIZE,LOGFILEPATH);\
			_wfopen_s(&g_pLogFile,g_LogBuffer,L"wb");\
			unsigned int head = 0xfeff;\
			fwrite(&head,sizeof(unsigned int),1,g_pLogFile);\
			StringCchPrintf(g_LogBuffer,LOG_BUFFER_SIZE, L"-- Open at %02u:%02u:%02u --\r\n\r\n", st.wHour, st.wMinute, st.wSecond); \
			fwrite(g_LogBuffer, 2,(wcslen(g_LogBuffer)),g_pLogFile);\
			fflush(g_pLogFile);\
		}

		#define CloseFile()\
		{\
			SYSTEMTIME st;\
			GetLocalTime(&st);\
			StringCchPrintf(g_LogBuffer,LOG_BUFFER_SIZE, L"\r\n\r\n-- Close at %02u:%02u:%02u --", st.wHour, st.wMinute, st.wSecond); \
			fwrite(g_LogBuffer, 2,(wcslen(g_LogBuffer)),g_pLogFile);\
			fflush(g_pLogFile);\
			fclose(g_pLogFile);\
		}

		#define LogData(Format,...)\
		{\
			StringCchPrintf(g_LogBuffer,LOG_BUFFER_SIZE, Format, __VA_ARGS__);\
			StringCchPrintf(g_LogBuffer2,LOG_BUFFER_SIZE, L"%s\r\n", g_LogBuffer);\
			fwrite(g_LogBuffer2, 2,(wcslen(g_LogBuffer2)),g_pLogFile);\
			fflush(g_pLogFile);\
		}
		#define LogText(Str)\
		{\
			LogData(L"%s",Str);\
		}
		#define FlushData()\
		{\
			fflush(g_pLogFile);\
		}	
	#elif defined(__LOGFILE) && defined(__DBGVIEW)
		extern	FILE* g_pLogFile;
		extern	wchar_t  g_LogBuffer[LOG_BUFFER_SIZE];
		extern	wchar_t  g_LogBuffer2[LOG_BUFFER_SIZE];

		#define DefineVar()\
			FILE* g_pLogFile;\
		    wchar_t g_LogBuffer[LOG_BUFFER_SIZE];\
		    wchar_t g_LogBuffer2[LOG_BUFFER_SIZE];
			

		#define OpenFile()\
		{\
			SYSTEMTIME st;\
			GetLocalTime(&st);\
			StringCchPrintf(g_LogBuffer, LOG_BUFFER_SIZE,LOGFILEPATH);\
			_wfopen_s(&g_pLogFile,g_LogBuffer,L"wb");\
			unsigned int head = 0xfeff;\
			fwrite(&head,sizeof(unsigned int),1,g_pLogFile);\
			StringCchPrintf(g_LogBuffer,LOG_BUFFER_SIZE, L"-- Open at %02u:%02u:%02u --\r\n\r\n", st.wHour, st.wMinute, st.wSecond); \
			fwrite(g_LogBuffer, 2,(wcslen(g_LogBuffer)),g_pLogFile);\
			fflush(g_pLogFile);\
		}

		#define CloseFile()\
		{\
			SYSTEMTIME st;\
			GetLocalTime(&st);\
			StringCchPrintf(g_LogBuffer,LOG_BUFFER_SIZE, L"\r\n\r\n-- Close at %02u:%02u:%02u --", st.wHour, st.wMinute, st.wSecond); \
			fwrite(g_LogBuffer, 2,(wcslen(g_LogBuffer)),g_pLogFile);\
			fflush(g_pLogFile);\
			fclose(g_pLogFile);\
		}

		#define LogData(Format,...)\
		{\
			StringCchPrintf(g_LogBuffer,LOG_BUFFER_SIZE, Format, __VA_ARGS__);\
			StringCchPrintf(g_LogBuffer2,LOG_BUFFER_SIZE, L"%s\r\n", g_LogBuffer);\
			fwrite(g_LogBuffer2, 2,(wcslen(g_LogBuffer2)),g_pLogFile);\
			fflush(g_pLogFile);\
			OutputDebugString(g_LogBuffer2);\
		}
		#define LogText(Str)\
		{\
			LogData(L"%s",Str);\
		}
		#define FlushData()\
		{\
			fflush(g_pLogFile);\
		}	

	#else
		#define OpenFile()
		#define CloseFile()
		#define LogData(Format,...)
		#define LogText(Str)
		#define FlushData()
		#define DefineVar()	
	#endif
#else
	#define OpenFile()
	#define CloseFile()
	#define LogData(Format,...)
	#define LogText(Str)
	#define FlushData()
	#define DefineVar()	
#endif //__LOG

#endif//__LOG_H__
