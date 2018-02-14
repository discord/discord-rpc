#include <windows.h>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#endif

BOOL WINAPI DllMain(HMODULE, DWORD, LPVOID)
{
    return TRUE;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
