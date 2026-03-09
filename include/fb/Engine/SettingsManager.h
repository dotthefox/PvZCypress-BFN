#pragma once

#define OFFSET_SETTINGSMANAGER_INSTANCE               CYPRESS_GW_SELECT(0, 0x142B508E0, 0x14421BFD0)
#define OFFSET_FUNC_SETTINGSMANAGER_GETCONTAINER      CYPRESS_GW_SELECT(0, 0x1401F2360, 0x14046F180)
#define OFFSET_FUNC_SETTINGSMANAGER_SET			      CYPRESS_GW_SELECT(0, 0x1401F01E0, 0x14046FA90)

namespace fb
{
	class SettingsManager
	{
	public:
		template <class T>
		T* getContainer(const char* identifier)
		{
			auto func = reinterpret_cast<T * (*)(SettingsManager*, const char*)>(OFFSET_FUNC_SETTINGSMANAGER_GETCONTAINER);
			return func(this, identifier);
		}

		bool set(const char* identifier, const char* value, void* typeInfo = nullptr)
		{
			auto func = reinterpret_cast<bool(*)(SettingsManager*, const char*, const char*, void*)>(OFFSET_FUNC_SETTINGSMANAGER_SET);
			return func(this, identifier, value, typeInfo);
		}

		void applySettings() {
			using tApplySettings = void(*)(SettingsManager*);
			auto _applySettings = reinterpret_cast<tApplySettings>(0x14046DC90);

			return _applySettings(this);
		}

		static SettingsManager* GetInstance()
		{
			return *(SettingsManager**)OFFSET_SETTINGSMANAGER_INSTANCE;
		}
	};
}