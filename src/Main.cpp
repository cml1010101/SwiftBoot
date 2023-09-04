#include <SwiftBootUtils.hpp>
#include <SwiftBootFS.hpp>
#include <SwiftBootJson.hpp>
#include <SwiftBootGraphics.hpp>
#include <SwiftBoot.hpp>
#include <SwiftBootElf.hpp>
extern "C" EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
    InitializeLib(imageHandle, systemTable);
    EFI_STATUS status = swiftboot::disableWatchdog();
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    EFI_FILE_HANDLE efiRoot;
    status = swiftboot::getRoot(imageHandle, &efiRoot);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    CHAR8* configContent;
    status = swiftboot::openAndReadASCII(efiRoot, (const CHAR16*)L"swift/config.json", &configContent);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    swiftboot::ConfigOptions config;
    status = swiftboot::getConfigOptions(configContent, &config);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    if (!config.showMenu && !config.defaultOption)
    {
        return EFI_ABORTED;
    }
    UINTN choice;
    if (config.showMenu)
    {
        Print((const CHAR16*)L"Options:\n");
        for (UINTN i = 0; i < config.optionCount; i++)
        {
            Print((const CHAR16*)"\t[%d]: %s\n", i + 1, config.options[i].name);
        }
        Print((const CHAR16*)L"Please enter your choice: ");
        CHAR16* choiceString;
        status = swiftboot::readString(ST->ConIn, '\n', &choiceString);
        if (status)
        {
            SHOW_ERROR_MESSAGE(status);
            return status;
        }
        choice = Atoi(choiceString);
    }
    else
    {
        for (UINTN i = 0; i < config.optionCount; i++)
        {
            if (StrCmp(config.defaultOption, config.options[i].name) == 0)
            {
                choice = i;
                break;
            }
        }
    }
    swiftboot::BootOption option = config.options[choice];
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    status = swiftboot::getGraphicsOutputProtocol(&gop);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    UINTN mode;
    status = swiftboot::getGraphicsMode(gop, option.options.width, option.options.height, &mode);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    status = swiftboot::setGraphicsMode(gop, mode);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    void* acpi;
    status = swiftboot::findACPI(&acpi);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    swiftboot::BootInfo* bootInfo = (swiftboot::BootInfo*)AllocatePool(sizeof(swiftboot::BootInfo));
    bootInfo->graphics.framebuffer = gop->Mode->FrameBufferBase;
    bootInfo->graphics.framebufferSize = gop->Mode->FrameBufferSize;
    bootInfo->graphics.width = gop->Mode->Info->HorizontalResolution;
    bootInfo->graphics.height = gop->Mode->Info->VerticalResolution;
    bootInfo->graphics.pixelType = (gop->Mode->Info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor) ?
        swiftboot::GraphicsInfo::PixelType::PIXEL_TYPE_RGBA : swiftboot::GraphicsInfo::PixelType::PIXEL_TYPE_BGRA;
    bootInfo->graphics.pitch = gop->Mode->Info->PixelsPerScanLine * sizeof(uint32_t);
    bootInfo->acpi = (uintptr_t)acpi;
    EFI_FILE_HANDLE partitionRoot;
    status = swiftboot::getRoot(option.partition, &partitionRoot);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    void* kernelBuffer;
    UINTN kernelSize;
    status = swiftboot::openAndReadFile(partitionRoot, option.kernelPath, &kernelBuffer, &kernelSize);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    swiftboot::Elf elf(kernelBuffer);
    if (!elf.checkHeader())
    {
        SHOW_ERROR_MESSAGE(EFI_INVALID_LANGUAGE);
        return EFI_INVALID_LANGUAGE;
    }
    status = elf.loadProgramHeaders();
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    swiftboot::KernelMain kernelMain = (swiftboot::KernelMain)elf.getEntry();
    uintptr_t initrdVirtualAddress;
    status = elf.getSymbolValue("initrd_start", &initrdVirtualAddress);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    FreePool(kernelBuffer);
    void* initrdBuffer;
    UINTN initrdSize;
    status = swiftboot::openAndReadFile(partitionRoot, option.initrdPath, &initrdBuffer, &initrdSize);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    status = swiftboot::allocatePages(initrdVirtualAddress, EFI_SIZE_TO_PAGES(initrdSize));
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    CopyMem((void*)initrdVirtualAddress, initrdBuffer, initrdSize);
    FreePool(initrdBuffer);
    status = swiftboot::getMemoryMap(&bootInfo->map);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    status = swiftboot::exitBootServices(imageHandle, bootInfo->map.mapKey);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    kernelMain(bootInfo);
    return EFI_ABORTED;
}