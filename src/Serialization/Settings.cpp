#include "Settings.h"

#include <SimpleIni.h>

void Settings::Initialize()
{
	const auto& path = CONFIGPATH("Settings.yaml");
	if (!fs::exists(path))
		return;

	try {
		const auto yaml = YAML::LoadFile(path);

		const auto ReadMCM = [&yaml]<typename T>(const char* a_key, T& a_out) {
			if (!yaml[a_key].IsDefined())
				return;
			const auto val = yaml[a_key].as<T>();
			a_out = val;
		};
#define MCM_SETTING(STR, DEFAULT) ReadMCM(#STR, STR);
#include "mcm.def"
#undef MCM_SETTING

		logger::info("Finished loading user settings");
	} catch (const std::exception& e) {
		logger::error("Unable to laod settings, error: {}", e.what());
	}
}

void Settings::Save()
{
	YAML::Node settings{};
#define MCM_SETTING(STR, DEFAULT) settings[#STR] = STR;
#include "mcm.def"
#undef MCM_SETTING

	std::ofstream fout(CONFIGPATH("Settings.yaml"));
	fout << settings;
	logger::info("Finished saving user settings");
}
