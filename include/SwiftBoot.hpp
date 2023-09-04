#ifndef SWIFTBOOT_HPP
#define SWIFTBOOT_HPP
#include <stdint.h>
#include <stddef.h>
namespace swiftboot
{
    struct GraphicsInfo
    {
        uintptr_t framebuffer;
        size_t framebufferSize;
        size_t width;
        size_t height;
        enum struct PixelType
        {
            PIXEL_TYPE_RGBA,
            PIXEL_TYPE_BGRA
        } pixelType;
        size_t pitch;
    };
    struct MemoryMapDescriptor
    {
        uint32_t type;
        uintptr_t virtualAddress;
        uintptr_t physicalAddress;
        size_t numberOfPages;
        uint64_t attributes;
    };
    struct MemoryMap
    {
        MemoryMapDescriptor* descriptors;
        size_t numDescriptors;
        uint64_t mapKey;
    };
    struct BootInfo
    {
        GraphicsInfo graphics;
        uintptr_t acpi;
        MemoryMap map;
    };
    typedef void(*KernelMain)(BootInfo* info);
}
#endif