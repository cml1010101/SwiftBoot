#include <SwiftBootGraphics.hpp>
EFI_STATUS swiftboot::getGraphicsOutputProtocol(EFI_GRAPHICS_OUTPUT_PROTOCOL** protocol)
{
    EFI_STATUS status = BS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void**)&protocol);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::getGraphicsMode(EFI_GRAPHICS_OUTPUT_PROTOCOL* protocol, UINTN width, UINTN height, UINTN* mode)
{
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* bestInfo = nullptr;
    UINTN infoSize;
    int bestMode = -1;
    for (UINTN i = 0; i < protocol->Mode->MaxMode; i++)
    {
        EFI_STATUS status = protocol->QueryMode(protocol, i, &infoSize, &info);
        if (status)
        {
            SHOW_ERROR_MESSAGE(status);
            return status;
        }
        if (info->HorizontalResolution == width && info->VerticalResolution == height)
        {
            *mode = i;
            return EFI_SUCCESS;
        }
        else if (bestInfo == nullptr)
        {
            bestInfo = info;
            bestMode = i;
        }
        else if (info->HorizontalResolution == width && info->VerticalResolution > bestInfo->VerticalResolution && height == 0)
        {
            bestInfo = info;
            bestMode = i;
        }
        else if (info->HorizontalResolution > bestInfo->HorizontalResolution && info->VerticalResolution == height && width == 0)
        {
            bestInfo = info;
            bestMode = i;
        }
        else if (info->HorizontalResolution > bestInfo->HorizontalResolution && info->VerticalResolution > bestInfo->VerticalResolution
            && width == 0 && height == 0)
        {
            bestInfo = info;
            bestMode = i;
        }
    }
    if (bestMode == -1)
    {
        return EFI_NOT_FOUND;
    }
    else
    {
        *mode = bestMode;
        return EFI_SUCCESS;
    }
}
EFI_STATUS swiftboot::setGraphicsMode(EFI_GRAPHICS_OUTPUT_PROTOCOL* protocol, UINTN mode)
{
    EFI_STATUS status = protocol->SetMode(protocol, mode);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    return EFI_SUCCESS;
}