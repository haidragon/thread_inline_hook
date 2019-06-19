// 内联Hook线程安全.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <windows.h>


void InjectDll(HWND hWnd, const char* dllPath)
{
	//获取PID打开其进程
	DWORD dwPid = 0;
	GetWindowThreadProcessId(hWnd, &dwPid);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	if (!hProcess) {
		printf("打开进程失败\n");
		getchar();
		return;
	}

	//在被注入的进程中分配一块虚拟内存
	LPVOID lpAddr = VirtualAllocEx(hProcess,
		NULL, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
	if (!lpAddr) {
		printf("分配内存失败\n");
		getchar();
		return;
	}

	//把dll路径注入到目标进程中
	DWORD dwWrite = 0;
	WriteProcessMemory(hProcess, lpAddr,
		dllPath, strlen(dllPath) + 1, &dwWrite);
	if (strlen(dllPath) + 1 != dwWrite)
	{
		printf("dll路径写入失败\n");
		getchar();
		return;
	}

	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0,
		(LPTHREAD_START_ROUTINE)LoadLibraryA, lpAddr, 0, 0);
	if (!hThread)
	{
		printf("创建远程线程失败错误码:%d\n", GetLastError());
		getchar();
		return;
	}

	//等待线程结束
	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(hProcess, lpAddr, MAX_PATH, MEM_RESERVE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
}

// 提权操作
BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;

	// 打开令牌
	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&hToken))
	{
		printf("OpenProcessToken error: %u\n", GetLastError());
		return FALSE;
	}

	// 获取LUID
	if (!LookupPrivilegeValue(
		NULL,            // lookup privilege on local system
		lpszPrivilege,   // privilege to lookup 
		&luid))          // receives LUID of privilege
	{
		printf("LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// 修改权限
	if (!AdjustTokenPrivileges(hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		printf("AdjustTokenPrivileges error: %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		printf("The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}



// 普通的内联Hook
//#define DLL_NAME "D:\\C++练习3\\内联Hook线程安全\\Debug\\01_Inline Hook.dll"
// 原子操作内联Hook
//#define DLL_NAME "D:\\C++练习3\\内联Hook线程安全\\Debug\\02_Atom Hook.dll"
// 信号量操作Hook
//#define DLL_NAME "D:\\C++练习3\\内联Hook线程安全\\Debug\\03_Semaphore Hook.dll"
// 修改7字节内联Hook
#define DLL_NAME "D:\\C++练习3\\内联Hook线程安全\\Debug\\04_Hotfixes Hook.dll"

//#define DLL_NAME "C:\\Users\\15pb - win7\\Desktop\\1231"



int main()
{

	/*使用信号量Hook时才用到*/
	//CreateSemaphore(NULL, 1, 1, L"九阳道人");

	//获取目标进程句柄
	HWND hWnd = FindWindowA(NULL, "Z测试程序");

	//提升特权
	SetPrivilege(SE_DEBUG_NAME, TRUE);

	//注入函数
	InjectDll(hWnd, DLL_NAME);
	return 0;
}

