// 04_Hotfixes Hook.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

// 旧地址
BYTE g_OldData[5];
// 新地址
BYTE g_NewData[5] = { 0xE9 };

// 声明hook的函数类型
typedef int (WINAPI*FnMsg)(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCWSTR lpText,
	_In_opt_ LPCWSTR lpCaption,
	_In_ UINT uType);

void InHook()
{
	// 保存旧地址的前两个字节
	memcpy(g_OldData, MessageBoxW, 2);
	//memcpy(g_OldData, (const void*)((DWORD)MessageBoxW - 5), 7);

	// 跳转到5个字节之前的位置的字节码
	BYTE pByte[2] = { 0xEB,0xF9 };

	// 优化程序
	if (*(BYTE*)MessageBoxW == 0xEB)
		return;

	// 计算偏移
	DWORD Offset = (DWORD)MyMsg - (DWORD)MessageBoxW;
	*(DWORD*)(g_NewData + 1) = Offset;

	// 修改内存属性
	DWORD Protect;
	VirtualProtect((LPVOID)((DWORD)MessageBoxW - 5), 7, PAGE_EXECUTE_READWRITE, &Protect);

	// 修改函数地址数据
	memcpy((LPVOID)((DWORD)MessageBoxW - 5), g_NewData, 5);
	memcpy(MessageBoxW, pByte, 2);

	// 还原内存属性
	VirtualProtect((LPVOID)((DWORD)MessageBoxW - 5), 7, Protect, &Protect);
}

void UnHook()
{
	DWORD Protect;
	VirtualProtect(MessageBoxW, 2, PAGE_EXECUTE_READWRITE, &Protect);
	memcpy(MessageBoxW, g_OldData, 2);
	VirtualProtect(MessageBoxW, 2, Protect, &Protect);
}


int
WINAPI
MyMsg(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCWSTR lpText,
	_In_opt_ LPCWSTR lpCaption,
	_In_ UINT uType)
{
	lpText = L"九阳道人修改7字节内联HOOK注入成功";
	lpCaption = L"九阳道人";

	//FARPROC pFunc = GetProcAddress(GetModuleHandleA("user32.dll"), "MessageBoxW");
	FARPROC pFunc = (FARPROC)((DWORD)MessageBoxW + 2);
	int han = ((FnMsg)pFunc)(hWnd, lpText, lpCaption, uType);

	return han;
}
