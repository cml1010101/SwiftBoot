#include <SwiftBootFS.hpp>
EFI_STATUS swiftboot::getRoot(EFI_HANDLE imageHandle, EFI_FILE_HANDLE* handle)
{
    EFI_LOADED_IMAGE* loadedImage;
    EFI_STATUS status = BS->HandleProtocol(imageHandle, &gEfiLoadedImageProtocolGuid, (void**)&loadedImage);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    EFI_FILE_IO_INTERFACE* ioInterface;
    status = BS->HandleProtocol(loadedImage->DeviceHandle, &gEfiFileSystemInfoGuid, (void**)&ioInterface);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    status = ioInterface->OpenVolume(ioInterface, handle);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::openFile(EFI_FILE_HANDLE root, const CHAR16* path, EFI_FILE_HANDLE* file)
{
    CHAR16* fileName = (CHAR16*)AllocateZeroPool(sizeof(CHAR16));
    UINTN i = 0;
    for (; path[i] && path[i] != '/'; i++)
    {
        CHAR16 tmp[2];
        tmp[0] = path[i];
        tmp[1] = 0;
        StrCat(fileName, tmp);
    }
    EFI_FILE_HANDLE handle;
    EFI_STATUS status = root->Open(root, &handle, fileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    if (path[i] == 0)
    {
        *file = handle;
    }
    else
    {
        EFI_STATUS status = openFile(handle, &path[i + 1], file);
        if (status)
        {
            SHOW_ERROR_MESSAGE(status);
            return status;
        }
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::getFileSize(EFI_FILE_HANDLE file, UINTN* size)
{
    EFI_FILE_INFO* info = LibFileInfo(file);
    *size = info->Size;
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::readFile(EFI_FILE_HANDLE file, UINTN size, void* buffer)
{
    EFI_STATUS status = file->Read(file, &size, buffer);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::closeFile(EFI_FILE_HANDLE file)
{
    EFI_STATUS status = file->Close(file);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::openAndReadFile(EFI_FILE_HANDLE root, const CHAR16* path, void** buffer, UINTN* size)
{
    EFI_FILE_HANDLE configFile;
    EFI_STATUS status = swiftboot::openFile(root, path, &configFile);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    status = swiftboot::getFileSize(configFile, size);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    *buffer = AllocatePool(*size);
    status = swiftboot::readFile(configFile, *size, *buffer);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    status = swiftboot::closeFile(configFile);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::openAndReadASCII(EFI_FILE_HANDLE root, const CHAR16* path, CHAR8** buffer)
{
    EFI_FILE_HANDLE configFile;
    EFI_STATUS status = swiftboot::openFile(root, path, &configFile);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    UINTN size;
    status = swiftboot::getFileSize(configFile, &size);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    *buffer = (CHAR8*)AllocatePool(size + 1);
    *buffer[size] = 0;
    status = swiftboot::readFile(configFile, size, *buffer);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    status = swiftboot::closeFile(configFile);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    return EFI_SUCCESS;
}
EFI_STATUS swiftboot::getRoot(const CHAR16* partitionName, EFI_FILE_HANDLE* file)
{
    UINTN handleCount;
    EFI_HANDLE* handles;
    EFI_STATUS status = BS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &handleCount, &handles);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return status;
    }
    for (UINTN i = 0; i < handleCount; i++)
    {
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* protocol;
        status = BS->OpenProtocol(handles[i], &gEfiSimpleFileSystemProtocolGuid, (void**)&protocol, NULL, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if (status)
        {
            SHOW_ERROR_MESSAGE(status);
            return status;
        }
        status = protocol->OpenVolume(protocol, file);
        if (status)
        {
            SHOW_ERROR_MESSAGE(status);
            return status;
        }
        EFI_FILE_INFO* info = LibFileInfo(*file);
        if (StrCmp(partitionName, info->FileName) == 0)
        {
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}