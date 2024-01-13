#include "Papyrus/Config.h"

#include "Acheron/Resolution.h"

namespace Papyrus
{
	template <class T>
	inline T GetSetting(VM* a_vm, StackID a_stackID, const std::string& a_setting)
	{
		auto w = Settings::table.find(a_setting);
		if (w == Settings::table.end()) {
			a_vm->TraceStack(fmt::format("Unrecognized setting: {}", a_setting).c_str(), a_stackID);
			return 0;
		} else if (!std::holds_alternative<T>(w->second)) {
			a_vm->TraceStack(fmt::format("Setting {} is of invalid type, expected {}", a_setting, typeid(T).name()).c_str(), a_stackID);
			return 0;
		}
		return std::get<T>(w->second);
	}

	int GetSettingInt(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting)
	{
		auto ret = GetSetting<int*>(a_vm, a_stackID, a_setting);
		return ret ? *ret : 0;
	}

	float GetSettingFloat(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting)
	{
		auto ret = GetSetting<float*>(a_vm, a_stackID, a_setting);
		return ret ? *ret : 0.0f;
	}

	bool GetSettingBool(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting)
	{
		auto ret = GetSetting<bool*>(a_vm, a_stackID, a_setting);
		return ret ? *ret : false;
	}

	int GetSettingColor(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting)
	{
		auto ptr = GetSetting<std::string*>(a_vm, a_stackID, a_setting);
		if (!ptr) {
			return 0;
		}
		auto c = ptr->substr(1);
		try {
			return std::stoi(c.c_str(), nullptr, 16);
		} catch (const std::exception& e) {
			const auto err = fmt::format("Unable to retrieve color setting due to error, restore default. Error: {}", e.what());
			logger::error("{}", err);
			*ptr = "#FF0000";
			return 0xFF0000;
		}
	}


	void SetSettingInt(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting, int a_value)
	{
		auto s = GetSetting<int*>(a_vm, a_stackID, a_setting);
		if (!s)
			return;

		*s = a_value;
	}

	void SetSettingFloat(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting, float a_value)
	{
		auto s = GetSetting<float*>(a_vm, a_stackID, a_setting);
		if (!s)
			return;

		*s = a_value;
	}

	void SetSettingBool(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting, bool a_value)
	{
		auto s = GetSetting<bool*>(a_vm, a_stackID, a_setting);
		if (!s)
			return;

		*s = a_value;
	}

	void SetSettingColor(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting, int a_value)
	{
		auto s = GetSetting<std::string*>(a_vm, a_stackID, a_setting);
		if (!s)
			return;

		std::stringstream stream{};
		stream << '#' << std::setfill('0') << std::setw(6) << std::hex << a_value;
		*s = stream.str();
	}

	std::vector<std::string> GetEvents(VM* a_vm, StackID a_stackID, RE::TESQuest*, int a_type)
	{
		using Type = Acheron::Resolution::Type;
		if (a_type < 0 || a_type >= Type::Total) {
			a_vm->TraceStack(fmt::format("Invalid type {}", a_type).c_str(), a_stackID);
			return {};
		}

		const auto& events = Acheron::Resolution::GetEvents(Type(a_type));
		std::vector<std::string> ret{};
		ret.reserve(events.size());
		for (auto&& e : events) {
			ret.emplace_back(e.first);
		}
		return ret;
	}

	std::vector<int> GetEventWeights(VM* a_vm, StackID a_stackID, RE::TESQuest*, int a_type)
	{
		using Type = Acheron::Resolution::Type;
		if (a_type < 0 || a_type >= Type::Total) {
			a_vm->TraceStack(fmt::format("Invalid type {}", a_type).c_str(), a_stackID);
			return {};
		}

		const auto& events = Acheron::Resolution::GetEvents(Type(a_type));
		std::vector<int> ret{};
		ret.reserve(events.size());
		for (auto&& e : events) {
			ret.emplace_back(e.second);
		}
		return ret;
	}

	void SetEventWeight(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_event, int a_type, int a_newweight)
	{
		using Type = Acheron::Resolution::Type;
		if (a_type < 0 || a_type >= Type::Total) {
			a_vm->TraceStack(fmt::format("Invalid type {}", a_type).c_str(), a_stackID);
			return;
		}

		uint8_t weight = a_newweight > std::numeric_limits<uint8_t>::max() ? std::numeric_limits<uint8_t>::max() : static_cast<uint8_t>(a_newweight);
		return Acheron::Resolution::SetEventWeight(a_event.c_str(), Type(a_type), weight);
	}


	void UpdateKillmoveGlobal(VM*, StackID, RE::TESQuest*)
	{
		Acheron::GameForms::KillMove->value = Settings::bKillMove;
	}

}	 // namespace Papyrus
