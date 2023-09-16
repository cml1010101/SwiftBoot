#ifndef SWIFTBOOTFS_HPP
#define SWIFTBOOTFS_HPP
#include <SwiftBootUtils.hpp>
namespace swiftboot
{
    class FileSystem;
    class File
    {
    public:
        void* handle;
        FileSystem* fileSystem;
        File() = default;
        inline File(void* handle, FileSystem* fileSystem)
        {
            this->handle = handle;
            this->fileSystem = fileSystem;
        }
        void readSome(void* buffer, size_t size);
        size_t getSize();
        inline void* read()
        {
            size_t size = getSize();
            void* buffer = AllocatePool(size);
            readSome(buffer, size);
            return buffer;
        }
        inline CHAR8* readASCII()
        {
            size_t size = getSize();
            CHAR8* buffer = (CHAR8*)AllocateZeroPool(size + 1);
            readSome(buffer, size);
            return buffer;
        }
        void close();
    };
    class FileSystem
    {
    public:
        inline File operator[](const CHAR16* path)
        {
            return openFile(path);
        }
        virtual File openFile(const CHAR16* path) = 0;
        virtual File openFile(File root, const CHAR16* path) = 0;
        virtual void readFile(File* file, void* buffer, size_t size) = 0;
        virtual size_t getFileSize(File* file) = 0;
        virtual void closeFile(File* file) = 0;
    };
    class EfiFileSystem : public FileSystem
    {
    private:
        EFI_FILE_HANDLE root;
    public:
        EfiFileSystem(EFI_HANDLE imageHandle);
        virtual File openFile(const CHAR16* path) override;
        virtual File openFile(File root, const CHAR16* path) override;
        virtual void readFile(File* file, void* buffer, size_t size) override;
        virtual size_t getFileSize(File* file) override;
        virtual void closeFile(File* file) override;
    };
}
#endif