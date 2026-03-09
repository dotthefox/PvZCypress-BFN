#pragma once
#include <fb/Engine/String.h>
#include <fb/Engine/Array.h>

#define OFFSET_LEVELSETUP_SETINCLUSIONOPTION CYPRESS_GW_SELECT(0x1404FA7F0, 0x14038D580, 0x140F9DAC0)
#define OFFSET_LEVELSETUP_SETINCLUSIONOPTIONS CYPRESS_GW_SELECT(0x1404FA9F0, 0x14038DBB0, 0x140F9DD60)
#define OFFSET_LEVELSETUP_GETINCLUSIONOPTION CYPRESS_GW_SELECT(0x1404ECC30, 0x14038DDB0, 0x140F9C1B0)

namespace fb
{
    struct LevelSetupOption {
        String m_criterion;
        String m_value;
    };

    struct LevelSetup 
    {
        LevelSetup() = default;

        ~LevelSetup()
        {
            m_name.freeRep();
            m_levelManagerInitialLevel.freeRep();
            m_levelManagerStartPoint.freeRep();

            m_loadScreen_LevelName.freeRep();
            m_loadScreen_LevelDescription.freeRep();
            m_loadScreen_LevelAsset.freeRep();
            m_loadScreen_GameMode.freeRep();

            m_startPoint.freeRep();
            m_playlist.freeRep();

            for (auto& inclusion : m_inclusionOptions)
            {
                inclusion.m_criterion.freeRep();
                inclusion.m_value.freeRep();
            }

            m_inclusionOptions.clear();
        }

        const char* getInclusionOption(const char* criterion)
        {
            auto LevelSetup__getInclusionOption = reinterpret_cast<const char* (*)(void* thisPtr, const char* criterion)>(OFFSET_LEVELSETUP_GETINCLUSIONOPTION);
            return LevelSetup__getInclusionOption(this, criterion);
        }

        void setInlusionOption(const char* criterion, const char* value)
        {
            auto LevelSetup__setInclusionOption = reinterpret_cast<void (*)(void* thisPtr, const char* criterion, const char* value)>(OFFSET_LEVELSETUP_SETINCLUSIONOPTION);
            LevelSetup__setInclusionOption(this, criterion, value);
        }

        void setInclusionOptions(const char* inclusionCriteria)
        {
            auto LevelSetup__setInclusionOptions = reinterpret_cast<void (*)(void* thisPtr, const char* inclusionCriteria)>(OFFSET_LEVELSETUP_SETINCLUSIONOPTIONS);
            LevelSetup__setInclusionOptions(this, inclusionCriteria);
        }

        Array<LevelSetupOption> m_inclusionOptions;
        unsigned int m_difficultyIndex = 0;
        fb::String m_loadScreen_GameMode;
        fb::String m_startPoint;
        fb::String m_levelManagerInitialLevel;
        fb::String m_loadScreen_LevelName;
        fb::String m_name;
        fb::String m_levelManagerStartPoint;
        fb::String m_loadScreen_LevelAsset;
        fb::String m_playlist;
        fb::String m_loadScreen_LevelDescription;
        int m_loadScreen_LevelCategory = 0;
        bool m_hasPersistentSave = false;
        bool m_forceReloadResources = false;
        bool m_isSaveGame = false;
    };

    static_assert(sizeof(fb::LevelSetup) == 96);
}