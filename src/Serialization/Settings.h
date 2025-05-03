#pragma once

struct Settings
{
private:
	struct StringCmp
	{
		bool operator()(const std::string& a_lhs, const std::string& a_rhs) const
		{
			return _strcmpi(a_lhs.c_str(), a_rhs.c_str()) < 0;
		}
	};

public:
	static void Initialize();
	static void Save();

#define MCM_SETTING(STR, DEFAULT) static inline decltype(DEFAULT) STR{ DEFAULT };
#include "mcm.def"
#undef MCM_SETTING

	using SettingsVariants = std::variant<float*, std::string*, bool*, int*>;
	static inline std::map<std::string, SettingsVariants, StringCmp> table{
#define MCM_SETTING(STR, DEFAULT) { #STR##s, &STR },
#include "mcm.def"
#undef MCM_SETTING
	};

	// Quick Access Funcs
	static bool DoesPlayerAutoRecover() { return iKdFallbackTimer || fKdHealthThresh && RE::PlayerCharacter::GetSingleton()->GetActorValue(RE::ActorValue::kHealRate); }
};
