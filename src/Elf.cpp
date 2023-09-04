#include <SwiftBootElf.hpp>
Elf64_Shdr* swiftboot::Elf::getSectionHeader(UINTN idx)
{
    Elf64_Shdr* sectionHeaders = (Elf64_Shdr*)((CHAR8*)data + programHeader->e_shoff);
    return &sectionHeaders[idx];
}
swiftboot::Elf::Elf(void* buffer) : data(buffer)
{
    programHeader = (Elf64_Ehdr*)buffer;
}
bool swiftboot::Elf::checkHeader()
{
    return programHeader->e_ident[EI_MAG0] == ELFMAG0 && programHeader->e_ident[EI_MAG1] == ELFMAG1
        && programHeader->e_ident[EI_MAG2] == ELFMAG2 && programHeader->e_ident[EI_MAG3] == ELFMAG3;
}
EFI_STATUS swiftboot::Elf::loadProgramHeaders()
{
    Elf64_Phdr* headers = (Elf64_Phdr*)((CHAR8*)data + programHeader->e_phoff);
    for (UINTN i = 0; i < programHeader->e_phnum; i++)
    {
        if (headers[i].p_type == PT_LOAD)
        {
            EFI_STATUS status = swiftboot::allocatePages(headers[i].p_vaddr, EFI_SIZE_TO_PAGES(headers[i].p_memsz));
            if (status)
            {
                SHOW_ERROR_MESSAGE(status);
                return status;
            }
            CopyMem((void*)headers[i].p_vaddr, (const void*)((CHAR8*)data + headers[i].p_offset), headers[i].p_filesz);
            ZeroMem((void*)(headers[i].p_vaddr + headers[i].p_filesz), headers[i].p_filesz - headers[i].p_memsz);
        }
    }
    return EFI_SUCCESS;
}
uintptr_t swiftboot::Elf::getEntry()
{
    return programHeader->e_entry;
}
EFI_STATUS swiftboot::Elf::getSymbolValue(const char* symbolName, uintptr_t* value)
{
    for (UINTN i = 0; i < programHeader->e_shnum; i++)
    {
        Elf64_Shdr* sectionHeader = getSectionHeader(i);
        if (sectionHeader->sh_type == SHT_SYMTAB)
        {
            Elf64_Shdr* stringHeader = getSectionHeader(sectionHeader->sh_link);
            Elf64_Sym* symbols = (Elf64_Sym*)((CHAR8*)data + sectionHeader->sh_offset);
            UINTN symbolCount = sectionHeader->sh_size / sectionHeader->sh_entsize;
            CHAR8* stringValues = (CHAR8*)data + stringHeader->sh_offset;
            for (UINTN j = 0; j < symbolCount; j++)
            {
                if (strcmpa(&stringValues[symbols[j].st_name], (const CHAR8*)symbolName) == 0)
                {
                    *value = symbols[j].st_value;
                    return EFI_SUCCESS;
                }
            }
        }
    }
    return EFI_NOT_FOUND;
}