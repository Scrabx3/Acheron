#include "Settings.h"

#include <SimpleIni.h>

void Settings::Initialize()
{
	const auto& path = CONFIGPATH("Settings.yaml");
	if (!fs::exists(path))
		return;

	try {
		const auto yaml = YAML::LoadFile(path);
		for (auto&& node : yaml) {
			auto keyname = node.first.as<std::string>();
			auto w = table.find(keyname);
			if (w == table.end()) {
				logger::error("Unrecognized setting: {}", keyname);
				continue;
			}

			switch (VariantType(w->second.index())) {
			case VariantType::INT:
				{
					auto s = std::get<int*>(w->second);
					*s = node.second.as<int>();
				}
				break;
			case VariantType::FLOAT:
				{
					auto s = std::get<float*>(w->second);
					*s = node.second.as<float>();
				}
				break;
			case VariantType::BOOL:
				{
					auto s = std::get<bool*>(w->second);
					*s = node.second.as<bool>();
				}
				break;
			case VariantType::STRING:
				{
					auto s = std::get<std::string*>(w->second);
					*s = node.second.as<std::string>();
				}
				break;
			default:
				logger::error("Unreocnigzed setting type for setting {}", keyname);
				break;
			}
		}
		logger::info("Finished loading user settings");
	} catch (const std::exception& e) {
		logger::error("Unable to laod settings, error: {}", e.what());
	}
}

void Settings::Save()
{
	YAML::Node settings{};
	for (auto&& s : table) {
		switch (VariantType(s.second.index())) {
		case VariantType::INT:
			{
				settings[s.first] = *std::get<int*>(s.second);
			}
			break;
		case VariantType::FLOAT:
			{
				settings[s.first] = *std::get<float*>(s.second);
			}
			break;
		case VariantType::BOOL:
			{
				settings[s.first] = *std::get<bool*>(s.second);
			}
			break;
		case VariantType::STRING:
			{
				settings[s.first] = *std::get<std::string*>(s.second);
			}
			break;
		default:
			logger::error("Unreocnigzed setting type for setting {}", s.first);
			break;
		}
	}
	std::ofstream fout(CONFIGPATH("Settings.yaml"));
	fout << settings;
	logger::info("Finished saving user settings");
}
