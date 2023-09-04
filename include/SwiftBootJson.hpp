#ifndef SWIFTBOOTJSON_HPP
#define SWIFTBOOTJSON_HPP
#ifdef JSON_CPP
#include <jsmn.h>
#endif
#include <SwiftBootUtils.hpp>
namespace swiftboot
{
#ifdef JSON_CPP
    inline UINTN getRecursiveSubTokens(jsmntok_t* token)
    {
        if (token->type == JSMN_PRIMITIVE || token->type == JSMN_STRING || token->type == JSMN_UNDEFINED)
        {
            return 1;
        }
        else if (token->type == JSMN_ARRAY)
        {
            UINTN j = 1;
            for (UINTN i = 0; i < token->size; i++)
            {
                j += getRecursiveSubTokens(&token[j]);
            }
            return j;
        }
        else
        {
            UINTN j = 1;
            for (UINTN i = 0; i < token->size; i++)
            {
                j += getRecursiveSubTokens(&token[j + 1]) +1;
            }
            return j;
        }
    }
    inline jsmntok_t* getJson(const CHAR8* str, jsmntok_t* token, const CHAR8* key)
    {
        size_t j = 1;
        for (size_t i = 0; i < token->size; i++)
        {
            if (strncmpa(&str[token[j].start], key, strlena(key)) == 0)
            {
                return &token[j + 1];
            }
            else
            {
                j += getRecursiveSubTokens(&token[j + 1]) + 1;
            }
        }
        return NULL;
    }
    inline jsmntok_t* getJson(const CHAR8* str, jsmntok_t* token, UINTN idx)
    {
        if (token->type == JSMN_ARRAY)
        {
            size_t j = 1;
            for (size_t i = 0; i < idx; i++)
            {
                j += getRecursiveSubTokens(&token[j]);
            }
        }
        else
        {
            size_t j = 1;
            for (size_t i = 0; i < idx; i++)
            {
                j += getRecursiveSubTokens(&token[j + 1]) + 1;
            }
        }
        return NULL;
    }
#endif
    struct DisplayOptions
    {
        UINTN width;
        UINTN height;
    };
    struct BootOption
    {
        const CHAR16* name;
        const CHAR16* partition;
        const CHAR16* kernelPath;
        const CHAR16* initrdPath;
        DisplayOptions options;
    };
    struct ConfigOptions
    {
        bool showMenu;
        const CHAR16* defaultOption;
        UINTN optionCount;
        BootOption* options;
    };
    extern EFI_STATUS getConfigOptions(const CHAR8* configContent, ConfigOptions* config);
#ifdef JSON_CPP
    inline CHAR16* getString(const CHAR8* content, jsmntok_t* token)
    {
        size_t size = token->end - token->start;
        CHAR16* string = (CHAR16*)AllocatePool((size + 1) * sizeof(CHAR16));
        string[size] = 0;
        for (UINTN i = 0; i < size; i++)
        {
            string[i] = content[token->start + i];
        }
        return string;
    }
    inline int getInt(const CHAR8* content, jsmntok_t* token)
    {
        return Atoi(getString(content, token));
    }
#endif
}
#endif