#ifndef SWIFTBOOTFS_HPP
#define SWIFTBOOTFS_HPP
#include <SwiftBootUtils.hpp>
namespace swiftboot
{
    extern EFI_STATUS getRoot(EFI_HANDLE imageHandle, EFI_FILE_HANDLE* file);
    extern EFI_STATUS getRoot(const CHAR16* partitionName, EFI_FILE_HANDLE* file);
    extern EFI_STATUS openFile(EFI_FILE_HANDLE root, const CHAR16* path, EFI_FILE_HANDLE* file);
    extern EFI_STATUS getFileSize(EFI_FILE_HANDLE file, UINTN* size);
    extern EFI_STATUS readFile(EFI_FILE_HANDLE file, UINTN size, void* buffer);
    extern EFI_STATUS closeFile(EFI_FILE_HANDLE file);
    extern EFI_STATUS openAndReadFile(EFI_FILE_HANDLE root, const CHAR16* path, void** buffer, UINTN* size);
    extern EFI_STATUS openAndReadASCII(EFI_FILE_HANDLE root, const CHAR16* path, CHAR8** buffer);
}
#endif