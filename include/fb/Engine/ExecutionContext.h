#pragma once
#include <ctype.h>
#include <EASTL/vector.h>
#include <fb/Engine/String.h>

#define OFFSET_EXECUTIONCONTEXT_GETOPTIONVALUE CYPRESS_GW_SELECT(0x1403A97D0, 0x1401C51E0, 0x1404681C0)
#define OFFSET_EXECUTIONCONTEXT_ADDOPTIONS 0x1403A7A80 // gw1
#define OFFSET_ECDATA_START CYPRESS_GW_SELECT(0, 0x142B7EA80, 0x14421A390)

namespace fb
{
	class ExecutionContext
	{
	public:
		static const char* getOptionValue(const char* optionName, const char* defaultValue = nullptr, int* token = nullptr)
		{
			auto ExecutionContext__getOptionValue =
				reinterpret_cast<char* (*)(const char* optionName, const char* defaultValue, int* token)>(OFFSET_EXECUTIONCONTEXT_GETOPTIONVALUE);
			return ExecutionContext__getOptionValue(optionName, defaultValue, token);
		}

		static int argc()
		{
            eastl::vector<fb::String>* ecDataOptions = (eastl::vector<fb::String>*)OFFSET_ECDATA_START;
            return ecDataOptions->size();
		}

		static void addOptions(bool replace, const char* text)
		{
            // inlined in gw2 of course, impl based off of bf3's ExecutionContextData::addOptions
            eastl::vector<fb::String>* ecDataOptions = (eastl::vector<fb::String>*)OFFSET_ECDATA_START;
            eastl::vector<fb::String>& options = *ecDataOptions;

            if (replace)
            {
                options.empty() ? options.push_back("<no argv[0]>") : options.resize(1);
            }

            const char* optStrStart = text;

            while (*optStrStart)
            {
                while (*optStrStart && isspace(*optStrStart))
                    optStrStart++;

                if (*optStrStart == 0)
                    break;

                const char* optStrEnd = optStrStart;
                if (*optStrStart == '"')
                {
                    optStrStart++;
                    optStrEnd = optStrStart;

                    while (*optStrEnd && *optStrEnd != '"')
                        optStrEnd++;

                    if (*optStrEnd == '"')
                    {
                        fb::String newOption(optStrStart, eastl_size_t(optStrEnd - optStrStart));
                        options.push_back(newOption);
                        optStrStart = optStrEnd + 1;
                    }
                }
                else
                {
                    while (*optStrEnd && !isspace(*optStrEnd))
                        optStrEnd++;

                    fb::String newOption(optStrStart, eastl_size_t(optStrEnd - optStrStart));
                    options.push_back(newOption);
                    optStrStart = optStrEnd;
                }
            }
		}
	};
}