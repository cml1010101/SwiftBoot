cmake_minimum_required(VERSION 3.10)
project(SwiftBoot VERSION 1.0.0 LANGUAGES CXX)
find_path(GNUEFI_INCLUDE_DIR NAMES "efi.h" "efilib.h" PATH_SUFFIXES "efi" REQUIRED)
find_library(GNUEFI_LIBEFI "libefi.a" REQUIRED)
find_library(GNUEFI_LIBGNUEFI "libgnuefi.a" REQUIRED)
find_program(CMAKE_OBJCOPY objcopy REQUIRED)
include_directories(include)
function(add_efi_executable ARCH TARGET_NAME)
    message("Adding EFI executable for ${ARCH} called ${TARGET_NAME} built by ${ARGN}")
    find_file(GNUEFI_CRT0 NAMES "crt0-efi-${ARCH}.o" HINTS /usr/lib /usr/local/lib REQUIRED)
    find_file(GNUEFI_LINKER_SCRIPT NAMES "elf_${ARCH}_efi.lds" HINTS /usr/lib /usr/local/lib REQUIRED)
    include_directories(${GNUEFI_INCLUDE_DIR} ${GNUEFI_INCLUDE_DIR}/${ARCH} ${GNUEFI_INCLUDE_DIR}/protocol)
    add_library(${TARGET_NAME}_shared SHARED ${ARGN} ${GNUEFI_CRT0})
    target_compile_options(${TARGET_NAME}_shared PRIVATE -fpic -fno-stack-protector -fshort-wchar -mno-red-zone)
    target_compile_definitions(${TARGET_NAME}_shared PRIVATE EFI_FUNCTION_WRAPPER)
    target_link_libraries(${TARGET_NAME}_shared PRIVATE ${GNUEFI_LIBEFI} ${GNUEFI_LIBGNUEFI})
    target_link_options(${TARGET_NAME}_shared PRIVATE -Wl,-shared -Wl,-Bsymbolic -Wl,-T${GNUEFI_LINKER_SCRIPT} -nostdlib -Wl,-znocombreloc)
    add_custom_command(
        OUTPUT ${TARGET_NAME}.efi
        COMMAND ${CMAKE_OBJCOPY} -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc
            --target=efi-app-x86_64 --subsystem=10 lib${TARGET_NAME}_shared.so ${TARGET_NAME}.efi
        DEPENDS ${TARGET_NAME}_shared
        COMMENT "Converting ELF to EFI executable"
    )
    add_custom_target(${TARGET_NAME} ALL
        DEPENDS ${TARGET_NAME}.efi
    )
endfunction()
add_efi_executable(x86_64 SwiftBoot_x86_64 "src/Main.cpp" "src/FS.cpp" "src/Utils.cpp" "src/Graphics.cpp" "src/Json.cpp" "src/Elf.cpp")
function(create_efi_partition TARGET_NAME ARCH PARTITION_TARGET PARTITION_SIZE CONFIG_JSON)
    message("Copying ${TARGET_NAME} and ${CONFIG_JSON} to ${PARTITION_TARGET}")
    add_custom_command(
        OUTPUT ${PARTITION_TARGET}.img
        COMMAND dd if=/dev/zero of=${PARTITION_TARGET}.img bs=512 count=${PARTITION_SIZE} conv=notrunc
        COMMAND mformat -i ${PARTITION_TARGET}.img -h 32 -t 32 -n 64 -c 1
        COMMAND mmd -i ${PARTITION_TARGET}.img ::/EFI
        COMMAND mmd -i ${PARTITION_TARGET}.img ::/EFI/BOOT
        COMMAND mcopy -i ${PARTITION_TARGET}.img ${TARGET_NAME} ::/EFI/BOOT/BOOT${ARCH}.efi
        COMMAND mmd -i ${PARTITION_TARGET}.img ::/swift
        COMMAND mcopy -i ${PARTITION_TARGET}.img ${CONFIG_JSON} ::/swift/config.json
        DEPENDS ${TARGET_NAME} ${CONFIG_JSON}
        COMMENT "Creating efi partition"
    )
    add_custom_target(${PARTITION_TARGET} ALL
        DEPENDS ${PARTITION_TARGET}.img
    )
endfunction()