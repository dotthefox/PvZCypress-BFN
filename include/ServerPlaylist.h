#pragma once
#include <vector>
#include <string.h>
#include <fstream>
#include <json.hpp>
#include <random>

struct PlaylistLevelSetup
{
    std::string LevelName;
    std::string GameMode;
    std::string StartPoint;
    std::string SettingsToApply;
    std::string Loadscreen_GamemodeName;
    std::string Loadscreen_LevelName;
    std::string Loadscreen_LevelDescription;
};

struct MixedModeConfig
{
    std::vector<std::pair<std::string, std::string>> AvailableModes;
    std::unordered_map<std::string, std::vector<std::string>> AvailableLevelsForModes;
};

class ServerPlaylist
{
public:

    bool LoadFromFile(const char* path)
    {
        std::ifstream playlistFile(path);
        if (playlistFile.is_open())
        {
            nlohmann::json playlistJson = nlohmann::json::parse(playlistFile);

            m_setups.clear();

            m_mixedEnabled = playlistJson["IsMixed"].get<bool>();
            m_roundsPerSetup = playlistJson["RoundsPerSetup"].get<unsigned int>();

            if(playlistJson.contains("Loadscreen_GamemodeNameOverride"))
                m_loadscreenGamemodeName = playlistJson["Loadscreen_GamemodeNameOverride"].get<std::string>();

            if (playlistJson.contains("Loadscreen_LevelNameOverride"))
                m_loadscreenLevelName = playlistJson["Loadscreen_LevelNameOverride"].get<std::string>();

            if(playlistJson.contains("Loadscreen_LevelDescriptionOverride"))
                m_loadscreenLevelDescription = playlistJson["Loadscreen_LevelDescriptionOverride"].get<std::string>();

            // init rng engine
            m_mtRNG = std::mt19937(m_rd());

            if (m_mixedEnabled)
            {
                for (const auto& [startPoint, mode] : playlistJson["AvailableModes"].items())
                {
                    m_mixedConfig.AvailableModes.emplace_back(startPoint, mode.get<std::string>());
                }

                for (const auto& [mode, levels] : playlistJson["AvailableLevelsForModes"].items())
                {
                    m_mixedConfig.AvailableLevelsForModes[mode] = levels.get<std::vector<std::string>>();
                }
            }
            else
            {
                for (const auto& setup : playlistJson.at("PlaylistRotation"))
                {
                    PlaylistLevelSetup levelSetup;
                    levelSetup.LevelName = setup["LevelName"].get<std::string>();
                    levelSetup.GameMode = setup["GameMode"].get<std::string>();
                    levelSetup.StartPoint = setup["StartPoint"].get<std::string>();

                    if (setup.contains("SettingsToApply"))
                        levelSetup.SettingsToApply = setup["SettingsToApply"].get<std::string>();

                    setup.contains("Loadscreen_LevelName")
                        ? levelSetup.Loadscreen_LevelName = setup["Loadscreen_LevelName"].get<std::string>()
                        : levelSetup.Loadscreen_LevelName = m_loadscreenLevelName;

                    setup.contains("Loadscreen_GamemodeName")
                        ? levelSetup.Loadscreen_GamemodeName = setup["Loadscreen_GamemodeName"].get<std::string>()
                        : levelSetup.Loadscreen_GamemodeName = m_loadscreenGamemodeName;

                    setup.contains("Loadscreen_LevelDescription")
                        ? levelSetup.Loadscreen_LevelDescription = setup["Loadscreen_LevelDescription"].get<std::string>()
                        : levelSetup.Loadscreen_LevelDescription = m_loadscreenLevelDescription;


                    m_setups.push_back(levelSetup);
                }
            }

            return true;
        }

        return false;
    }

    bool IsMixedMode() { return m_mixedEnabled; }

    bool AllRoundsCompletedForSetup() { return m_currentRoundsOnSetup == m_roundsPerSetup; }

    void ResetRoundCount() { m_currentRoundsOnSetup = 0; }

    void RoundCompleted()
    {
        m_currentRoundsOnSetup++;
        if (AllRoundsCompletedForSetup())
        {
            ResetRoundCount();
            m_shouldGetNewSetup = true;
        }
        else
            m_shouldGetNewSetup = false;
    }

    const PlaylistLevelSetup* GetMixedLevelSetup()
    {
        std::uniform_int_distribution<> modeDist(0, m_mixedConfig.AvailableModes.size() - 1);
pick_mode:
        const std::pair<std::string, std::string>& randomMode = m_mixedConfig.AvailableModes[modeDist(m_mtRNG)];
        // Check if the random pick is the same mode 
        // Need to check without the last character because of alt modes for alt coop maps, such as Domination0 and Domination1
        if (CompareStrWithoutLastCharacter(m_currentSetup.StartPoint, randomMode.first))
        {
            goto pick_mode;
        }

        m_currentSetup.GameMode = randomMode.second;
        
        int numLevelsForMode = m_mixedConfig.AvailableLevelsForModes[randomMode.first.c_str()].size();

        std::uniform_int_distribution<> levelDist(0, numLevelsForMode - 1);
pick_level:
        const std::string& randomLevel = m_mixedConfig.AvailableLevelsForModes[randomMode.first][levelDist(m_mtRNG)];
        if (m_currentSetup.LevelName == randomLevel)
        {
            goto pick_level;
        }

        m_currentSetup.LevelName = randomLevel;
        m_currentSetup.StartPoint = randomMode.first;

        if(!m_loadscreenLevelName.empty())
            m_currentSetup.Loadscreen_LevelName = m_loadscreenLevelName;

        if(!m_loadscreenLevelDescription.empty())
            m_currentSetup.Loadscreen_LevelDescription = m_loadscreenLevelDescription;

        if(!m_loadscreenGamemodeName.empty())
            m_currentSetup.Loadscreen_GamemodeName = m_loadscreenGamemodeName;

        return &m_currentSetup;
    }

    const PlaylistLevelSetup* GetNextSetup()
    {
        RoundCompleted();

        if (!m_shouldGetNewSetup)
            return &m_currentSetup;

        if (IsMixedMode())
            return GetMixedLevelSetup();

        m_currentSetupIndex++;

        if (m_currentSetupIndex >= m_setups.size())
            m_currentSetupIndex = 0;

        m_currentSetup = m_setups[m_currentSetupIndex];
        return &m_currentSetup;
    }

    const PlaylistLevelSetup* GetSetup(int index) 
    { 
        return &m_setups[index]; 
    }

    void SetCurrentSetup(int index) {
        m_currentSetup = m_setups[index];
    }

    const PlaylistLevelSetup* GetCurrentSetup() { return &m_currentSetup; }

private:
    bool CompareStrWithoutLastCharacter(const std::string& str1, const std::string& str2)
    {
        if (str1.length() < 1 || str2.length() < 1)
        {
            return false;
        }

        return str1.substr(0, str1.length() - 1) == str2.substr(0, str2.length() - 1);
    }

    MixedModeConfig m_mixedConfig;
    std::vector<PlaylistLevelSetup> m_setups;
    PlaylistLevelSetup m_currentSetup;
    std::string m_loadscreenGamemodeName;
    std::string m_loadscreenLevelName;
    std::string m_loadscreenLevelDescription;
    unsigned int m_currentRoundsOnSetup = 0;
    unsigned int m_roundsPerSetup = 1;
    unsigned int m_currentSetupIndex = 0;
    bool m_mixedEnabled;
    bool m_shouldGetNewSetup = true;
    std::random_device m_rd;
    std::mt19937 m_mtRNG;
};