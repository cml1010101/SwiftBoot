#ifndef SWIFTBOOTUTILS_HPP
#define SWIFTBOOTUTILS_HPP
#include <efi.h>
#include <efilib.h>
#include <stddef.h>
#include <SwiftBoot.hpp>
#define SHOW_ERROR_MESSAGE(status) swiftboot::showErrorMessage(status, __FILE__, __LINE__)
namespace swiftboot
{
    extern void showErrorMessage(EFI_STATUS status, const char* file, int line);
    extern EFI_STATUS disableWatchdog();
    extern EFI_STATUS readString(EFI_SIMPLE_TEXT_IN_PROTOCOL* input, CHAR16 delim, CHAR16** output);
    extern EFI_STATUS findACPI(void** acpi);
    extern EFI_STATUS getMemoryMap(MemoryMap* map);
    extern EFI_STATUS allocatePage(UINT64 virtualAddress);
    inline EFI_STATUS allocatePages(UINT64 virtualAddress, UINTN pages)
    {
        for (UINTN i = 0; i < pages; i++)
        {
            EFI_STATUS status = allocatePage(virtualAddress + i * 0x1000);
            if (status)
            {
                SHOW_ERROR_MESSAGE(status);
                return status;
            }
        }
        return EFI_SUCCESS;
    }
    extern EFI_STATUS exitBootServices(EFI_HANDLE imageHandle, UINTN mapKey);
}
#endif