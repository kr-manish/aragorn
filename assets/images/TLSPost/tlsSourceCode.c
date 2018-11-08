#include "stdafx.h"
#include <Windows.h>
#include <cstdio>
__declspec(thread) int a;
void NTAPI on_tls_callback(void* dll, DWORD reason, void* reserved)
{
    a = 543;
    MessageBox(nullptr, L"In TLS", L"In TLS", MB_OK);
}

#pragma comment (linker, "/INCLUDE:__tls_used")
#pragma comment (linker, "/INCLUDE:__xl_b")
#pragma data_seg(".CRT$XLB")
EXTERN_C

PIMAGE_TLS_CALLBACK _xl_b = on_tls_callback;
#pragma data_seg()

int main()
{
    printf("%d\n", a);
    MessageBox(nullptr, L"In Main", L"In Main", MB_OK);
    return 0;
}