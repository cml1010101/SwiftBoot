#ifndef SWIFTBOOTGRAPHICS_HPP
#define SWIFTBOOTGRAPHICS_HPP
#include <SwiftBootUtils.hpp>
namespace swiftboot
{
    extern EFI_STATUS getGraphicsOutputProtocol(EFI_GRAPHICS_OUTPUT_PROTOCOL** protocol);
    extern EFI_STATUS getGraphicsMode(EFI_GRAPHICS_OUTPUT_PROTOCOL* protocol, UINTN width, UINTN height, UINTN* mode);
    extern EFI_STATUS setGraphicsMode(EFI_GRAPHICS_OUTPUT_PROTOCOL* protocol, UINTN mode);
}
#endif