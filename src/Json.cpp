#define JSON_CPP
#include <SwiftBootJson.hpp>
using namespace swiftboot;
EFI_STATUS swiftboot::getConfigOptions(const CHAR8* configContent, ConfigOptions* options)
{
    jsmn_parser parser;
    jsmn_init(&parser);
    jsmntok_t tokens[512];
    int numTokens = jsmn_parse(&parser, (const char*)configContent, strlena(configContent), tokens, 512);
    if (numTokens < 1)
    {
        return EFI_INVALID_LANGUAGE;
    }
    if (tokens[0].type != JSMN_OBJECT)
    {
        return EFI_INVALID_PARAMETER;
    }
    jsmntok_t* showMenuToken = getJson(configContent, tokens, (const CHAR8*)"showMenu");
    if (showMenuToken != nullptr)
    {
        options->showMenu = strncmpa((const CHAR8*)"true", &configContent[showMenuToken->start], 4) == 0;
    }
    else
    {
        options->showMenu = true;
    }
    jsmntok_t* defaultOptionToken = getJson(configContent, tokens, (const CHAR8*)"defaultOption");
    if (defaultOptionToken != nullptr)
    {
        options->defaultOption = getString(configContent, defaultOptionToken);
    }
    else
    {
        options->defaultOption = nullptr;
    }
    jsmntok_t* optionsToken = getJson(configContent, tokens, (const CHAR8*)"options");
    if (optionsToken == nullptr || optionsToken->size == 0)
    {
        return EFI_ABORTED;
    }
    else
    {
        options->optionCount = optionsToken->size;
        options->options = (BootOption*)AllocatePool(optionsToken->size * sizeof(BootOption));
        for (size_t i = 0; i < optionsToken->size; i++)
        {
            jsmntok_t* optionToken = getJson(configContent, optionsToken, i);
            jsmntok_t* optionNameToken = getJson(configContent, optionToken, (const CHAR8*)"name");
            if (optionNameToken == nullptr)
            {
                return EFI_INVALID_PARAMETER;
            }
            options->options[i].name = getString(configContent, optionNameToken);
            jsmntok_t* optionPartitionToken = getJson(configContent, optionToken, (const CHAR8*)"partition");
            if (optionPartitionToken == nullptr)
            {
                return EFI_INVALID_PARAMETER;
            }
            options->options[i].partition = getString(configContent, optionPartitionToken);
            jsmntok_t* kernelPathToken = getJson(configContent, optionToken, (const CHAR8*)"kernelPath");
            if (kernelPathToken == nullptr)
            {
                return EFI_INVALID_PARAMETER;
            }
            options->options[i].kernelPath = getString(configContent, kernelPathToken);
            jsmntok_t* initrdPathToken = getJson(configContent, optionToken, (const CHAR8*)"initrdPath");
            if (initrdPathToken == nullptr)
            {
                return EFI_INVALID_PARAMETER;
            }
            options->options[i].initrdPath = getString(configContent, initrdPathToken);
            jsmntok_t* displayOptionsToken = getJson(configContent, optionToken, (const CHAR8*)"displayOptions");
            if (displayOptionsToken == nullptr)
            {
                options->options[i].options.width = 0;
                options->options[i].options.height = 0;
            }
            else
            {
                jsmntok_t* widthToken = getJson(configContent, displayOptionsToken, (const CHAR8*)"width");
                if (widthToken == nullptr)
                {
                    options->options[i].options.width = 0;
                }
                else
                {
                    options->options[i].options.width = getInt(configContent, widthToken);
                }
                jsmntok_t* heightToken = getJson(configContent, displayOptionsToken, (const CHAR8*)"height");
                if (widthToken == nullptr)
                {
                    options->options[i].options.height = 0;
                }
                else
                {
                    options->options[i].options.height = getInt(configContent, heightToken);
                }
            }
        }
    }
    return EFI_SUCCESS;
}