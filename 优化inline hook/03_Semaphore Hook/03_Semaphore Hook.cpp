// 03_Semaphore Hook.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"


// 原函数的地址数据
BYTE g_OldData[5];
// 替换后的数据
BYTE g_NewData[5] = { 0xE9 };

// 安装钩子
void InHook()
{
	// *获取到名为"九阳道人"的信号量
	HANDLE Semaphore = OpenSemaphore(
		SEMAPHORE_ALL_ACCESS, FALSE, L"九阳道人");

	// *把信号减为0，锁定当前线程
	WaitForSingleObject(Semaphore, -1);

	// 保存原函数地址
	memcpy(g_OldData, MessageBoxW, 5);
	// 计算出要跳转的偏移(自己的函数地址 - 原函数地址 - 5)	   
	DWORD Offset = (DWORD)MyMsg - (DWORD)MessageBoxW - 5;
	*(DWORD*)(g_NewData + 1) = Offset;
	// 修改内存属性
	DWORD Protect;
	VirtualProtect(MessageBoxW, 5, PAGE_EXECUTE_READWRITE, &Protect);
	// 替换函数地址		
	memcpy(MessageBoxW, g_NewData, 5);
	// 还原内存属性
	VirtualProtect(MessageBoxW, 5, Protect, &Protect);

	// *把信号加1
	LONG Count = 0;
	ReleaseSemaphore(Semaphore, 1, &Count);
}

// 卸载钩子
void UnHook()
{
	DWORD Protect;
	VirtualProtect(MessageBoxW, 5, PAGE_EXECUTE_READWRITE, &Protect);
	memcpy(MessageBoxW, g_OldData, 5);
	VirtualProtect(MessageBoxW, 5, Protect, &Protect);
}

// 自己的Hook函数
int
WINAPI
MyMsg(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCWSTR lpText,
	_In_opt_ LPCWSTR lpCaption,
	_In_ UINT uType)
{
	// 替换字符串
	lpText = L"九阳道人信号量线程同步注入成功！";
	lpCaption = L"九阳道人";

	// 先卸载钩子
	UnHook();
	// 在调用真正的MessageBoxW函数
	int nRet = MessageBoxW(hWnd, lpText, lpCaption, uType);
	// 最后在把钩子装上
	InHook();

	return nRet;
}
