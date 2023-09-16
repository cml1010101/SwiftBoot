#include <SwiftBootFS.hpp>
swiftboot::EfiFileSystem::EfiFileSystem(EFI_HANDLE imageHandle)
{
    EFI_LOADED_IMAGE* loadedImage;
    EFI_STATUS status = uefi_call_wrapper((void*)BS->HandleProtocol, 3, imageHandle, &gEfiLoadedImageProtocolGuid, (void**)&loadedImage);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return;
    }
    root = LibOpenRoot(loadedImage->DeviceHandle);
}
swiftboot::File swiftboot::EfiFileSystem::openFile(const CHAR16* path)
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
    EFI_STATUS status = uefi_call_wrapper((void*)root->Open, 5, root, &handle, fileName, EFI_FILE_MODE_READ,
        EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return File();
    }
    File file = File(handle, this);
    if (path[i] == 0)
    {
        return File(handle, this);
    }
    else
    {
        return openFile(file, &path[i + 1]);
    }
    SHOW_ERROR_MESSAGE(EFI_NOT_FOUND);
    return File();
}
swiftboot::File swiftboot::EfiFileSystem::openFile(File parentFile, const CHAR16* path)
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
    EFI_FILE_HANDLE parent = (EFI_FILE_HANDLE)parentFile.handle;
    EFI_STATUS status = uefi_call_wrapper((void*)parent->Open, 5, parent, &handle, fileName, EFI_FILE_MODE_READ,
        EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
        return File();
    }
    File file = File(handle, this);
    if (path[i] == 0)
    {
        return File(handle, this);
    }
    else
    {
        return openFile(file, &path[i + 1]);
    }
    SHOW_ERROR_MESSAGE(EFI_NOT_FOUND);
    return File();
}
size_t swiftboot::EfiFileSystem::getFileSize(File* file)
{
    EFI_FILE_INFO* info = LibFileInfo((EFI_FILE_HANDLE)file->handle);
    return info->Size;
}
void swiftboot::EfiFileSystem::readFile(File* file, void* buffer, size_t size)
{
    EFI_FILE_HANDLE handle = (EFI_FILE_HANDLE)file->handle;
    EFI_STATUS status = uefi_call_wrapper((void*)handle->Read, 3, handle, &size, buffer);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
    }
}
void swiftboot::EfiFileSystem::closeFile(File* file)
{
    EFI_FILE_HANDLE handle = (EFI_FILE_HANDLE)file->handle;
    EFI_STATUS status = uefi_call_wrapper((void*)handle->Close, 1, handle);
    if (status)
    {
        SHOW_ERROR_MESSAGE(status);
    }
}
void swiftboot::File::close()
{
    fileSystem->closeFile(this);
}
void swiftboot::File::readSome(void* buffer, size_t size)
{
    fileSystem->readFile(this, buffer, size);
}
size_t swiftboot::File::getSize()
{
    return fileSystem->getFileSize(this);
}