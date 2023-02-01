#pragma once
#include <Windows.h>

class Hook {
    void* m_toHook;
    char* m_oldOpcodes;
    int m_len;
public:
    Hook(void* to_hook, void* our_func, int len) : m_toHook(to_hook), m_oldOpcodes(nullptr), m_len(len) {
        // If less then 14 bytes, return.
        if (m_len < 14) {
            return;
        }

        DWORD curProtection;
        VirtualProtect(m_toHook, m_len, PAGE_EXECUTE_READWRITE, &curProtection);

        // Save old bytes
        m_oldOpcodes = (char*)malloc(m_len);
        if (m_oldOpcodes != nullptr) {
            for (int i = 0; i < m_len; ++i) {
                m_oldOpcodes[i] = ((char*)m_toHook)[i];
            }
        }

        memset(m_toHook, 0x90, m_len);
        unsigned char patch[] = {
            0x50,                                                           // push     rax
            0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // movabs   rax, 0x0
            0xFF, 0xE0,                                                     // jmp      rax
            0x58                                                            // pop      rax
        };
        *(uintptr_t*)&patch[3] = (uintptr_t)our_func;
        memcpy((void*)m_toHook, patch, sizeof(patch));

        VirtualProtect(m_toHook, m_len, curProtection, &curProtection);
    }

    ~Hook() {
        if (m_oldOpcodes != nullptr) {
            DWORD curProtection;
            VirtualProtect(m_toHook, m_len, PAGE_EXECUTE_READWRITE, &curProtection);
            // Restore our old bytes
            for (int i = 0; i < m_len; ++i) {
                ((char*)m_toHook)[i] = m_oldOpcodes[i];
            }
            VirtualProtect(m_toHook, m_len, curProtection, &curProtection);
            free(m_oldOpcodes);
        }
    }
};
