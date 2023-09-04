#include <SwiftBootUtils.hpp>
void swiftboot::showErrorMessage(EFI_STATUS status, const char* file, int line)
{
    Print((CHAR16*)L"Error: %e at %a:%d\n", status, file, line);
}
EFI_STATUS swiftboot::disableWatchdog()
{
    EFI_STATUS status = BS->SetWatchdogTimer(0, 0, 0, nullptr);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::readString(EFI_SIMPLE_TEXT_IN_PROTOCOL* input, CHAR16 delim, CHAR16** output)
{
    *output = (CHAR16*)AllocateZeroPool(sizeof(CHAR16));
    while (true)
    {
        EFI_INPUT_KEY key;
        EFI_STATUS status = input->ReadKeyStroke(input, &key);
        if (!EFI_ERROR(status))
        {
            if (key.UnicodeChar == delim)
            {
                break;
            }
            CHAR16 miniStr[2];
            miniStr[0] = key.UnicodeChar;
            miniStr[1] = 0;
            StrCat(*output, miniStr);
        }
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::findACPI(void** acpi)
{
    EFI_GUID acpiGuid = ACPI_20_TABLE_GUID;
    for (UINTN i = 0; i < ST->NumberOfTableEntries; i++)
    {
        if (CompareGuid(&acpiGuid, &ST->ConfigurationTable[i].VendorGuid) == 0)
        {
            *acpi = ST->ConfigurationTable[i].VendorTable;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}
EFI_STATUS swiftboot::getMemoryMap(swiftboot::MemoryMap* map)
{
    EFI_MEMORY_DESCRIPTOR* memoryMap = nullptr;
    UINTN memoryMapSize;
    UINTN mapKey;
    UINTN descriptorSize;
    UINT32 descriptorVersion;
    EFI_STATUS status = BS->GetMemoryMap(&memoryMapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);
    if (status != EFI_BUFFER_TOO_SMALL)
    {
        if (status == EFI_SUCCESS)
        {
            status = EFI_OUT_OF_RESOURCES;
        }
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    memoryMap = (EFI_MEMORY_DESCRIPTOR*)AllocatePool(memoryMapSize);
    status = BS->GetMemoryMap(&memoryMapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    map->numDescriptors = memoryMapSize / descriptorSize;
    map->mapKey = mapKey;
    map->descriptors = (MemoryMapDescriptor*)AllocatePool(map->numDescriptors * sizeof(MemoryMapDescriptor));
    for (size_t i = 0; i < map->numDescriptors; i++)
    {
        map->descriptors[i].type = memoryMap[i].Type;
        map->descriptors[i].virtualAddress = memoryMap[i].VirtualStart;
        map->descriptors[i].physicalAddress = memoryMap[i].PhysicalStart;
        map->descriptors[i].numberOfPages = memoryMap[i].NumberOfPages;
        map->descriptors[i].attributes = memoryMap[i].Attribute;
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::allocatePage(UINT64 virtualAddress)
{
    EFI_MEMORY_DESCRIPTOR* memoryMap = nullptr;
    UINTN memoryMapSize;
    UINTN mapKey;
    UINTN descriptorSize;
    UINT32 descriptorVersion;
    EFI_STATUS status = BS->GetMemoryMap(&memoryMapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);
    if (status != EFI_BUFFER_TOO_SMALL)
    {
        if (status == EFI_SUCCESS)
        {
            status = EFI_OUT_OF_RESOURCES;
        }
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    memoryMap = (EFI_MEMORY_DESCRIPTOR*)AllocatePool(memoryMapSize);
    status = BS->GetMemoryMap(&memoryMapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    UINTN numDescriptors = memoryMapSize / descriptorSize;
    bool found = false;
    for (UINTN i = 0; i < numDescriptors; i++)
    {
        if (memoryMap[i].Type == EfiConventionalMemory)
        {
            if (memoryMap[i].NumberOfPages == 1)
            {
                memoryMap[i].Type = EfiLoaderCode;
                memoryMap[i].VirtualStart = virtualAddress;
            }
            else if (i == (numDescriptors - 1))
            {
                memoryMap = (EFI_MEMORY_DESCRIPTOR*)ReallocatePool(memoryMap, memoryMapSize, memoryMapSize + sizeof(EFI_MEMORY_DESCRIPTOR));
                memoryMap[i + 1].Attribute = memoryMap[i].Attribute;
                memoryMap[i + 1].NumberOfPages = memoryMap[i].NumberOfPages - 1;
                memoryMap[i + 1].PhysicalStart = memoryMap[i].PhysicalStart + 0x1000;
                memoryMap[i + 1].VirtualStart = memoryMap[i].VirtualStart + 0x1000;
                memoryMap[i + 1].Pad = 0;
                memoryMap[i + 1].Type = EfiConventionalMemory;
                memoryMap[i].Type = EfiLoaderCode;
                memoryMap[i].VirtualStart = virtualAddress;
            }
            else
            {
                memoryMap = (EFI_MEMORY_DESCRIPTOR*)ReallocatePool(memoryMap, memoryMapSize, memoryMapSize + sizeof(EFI_MEMORY_DESCRIPTOR));
                EFI_MEMORY_DESCRIPTOR* tmp = (EFI_MEMORY_DESCRIPTOR*)AllocatePool((numDescriptors - (i + 1)) * sizeof(EFI_MEMORY_DESCRIPTOR));
                CopyMem(tmp, &memoryMap[i], (numDescriptors - (i + 1)) * sizeof(EFI_MEMORY_DESCRIPTOR));
                CopyMem(&memoryMap[i + 1], tmp, (numDescriptors - (i + 1)) * sizeof(EFI_MEMORY_DESCRIPTOR));
                memoryMap[i + 1].Attribute = memoryMap[i].Attribute;
                memoryMap[i + 1].NumberOfPages = memoryMap[i].NumberOfPages - 1;
                memoryMap[i + 1].PhysicalStart = memoryMap[i].PhysicalStart + 0x1000;
                memoryMap[i + 1].VirtualStart = memoryMap[i].VirtualStart + 0x1000;
                memoryMap[i + 1].Pad = 0;
                memoryMap[i + 1].Type = EfiConventionalMemory;
                memoryMap[i].Type = EfiLoaderCode;
                memoryMap[i].VirtualStart = virtualAddress;
            }
            found = true;
            break;
        }
    }
    if (!found)
    {
        return EFI_OUT_OF_RESOURCES;
    }
    memoryMapSize += sizeof(EFI_MEMORY_DESCRIPTOR);
    status = ST->RuntimeServices->SetVirtualAddressMap(memoryMapSize, descriptorSize, descriptorVersion, memoryMap);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::exitBootServices(EFI_HANDLE imageHandle, UINTN mapKey)
{
    EFI_STATUS status = BS->ExitBootServices(imageHandle, mapKey);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    return EFI_SUCCESS;
}