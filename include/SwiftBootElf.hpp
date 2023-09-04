#ifndef SWIFTBOOTELF_HPP
#define SWIFTBOOTELF_HPP
#include <SwiftBootUtils.hpp>
#include <elf.h>
namespace swiftboot
{
    class Elf
    {
    private:
        Elf64_Ehdr* programHeader;
        void* data;
        Elf64_Shdr* getSectionHeader(UINTN idx);
    public:
        Elf(void* buffer);
        bool checkHeader();
        EFI_STATUS loadProgramHeaders();
        EFI_STATUS getSymbolValue(const char* symbolName, uintptr_t* value);
        uintptr_t getEntry();
    };
}
#endif