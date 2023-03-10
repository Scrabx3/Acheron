#include "Acheron/Resolution.h"

#include "Acheron/Animation/Animation.h"
#include "Acheron/Validation.h"

namespace Acheron
{
	EventData::EventData(const std::string& a_filepath)
	{
		if (!a_filepath.ends_with(".yaml") && !a_filepath.ends_with(".yml"))
			throw ParseException("Invalid file extension");

		const auto root{ YAML::LoadFile(a_filepath) };
		const auto v = root["Version"].IsDefined() ? root["Version"].as<int>() : 0;
		switch (v) {
		case 1:
			{
				if (const auto req = root["Requirements"]; req.IsDefined()) {
					const auto plugins = req.as<std::vector<std::string>>();
					for (auto&& plugin : plugins) {
						if (RE::TESDataHandler::GetSingleton()->LookupModByName(plugin) == nullptr)
							throw ParseException("Expected requirement not loaded: {}", plugin);
					}
				}

				quest = FormFromString<RE::TESQuest*>(root["Quest"].as<std::string>());
				if (!quest) {
					throw ParseException("Unable to find quest: {}", root["Quest"].as<std::string>());
				}

				const auto setattribute = [this, &root]<class T>(const char* a_setting, T& a_attribute) {
					if (!root[a_setting].IsDefined())
						return;

					a_attribute = root[a_setting].as<T>();
				};
				setattribute("Name", name);
				setattribute("Cooldown", cooldown);
				setattribute("Priority", priority);

				const auto setflag = [this, &root](const char* a_setting, Flags a_flag) {
					if (!root[a_setting].IsDefined())
						return;

					if (root[a_setting].as<bool>()) {
						flags.set(a_flag);
					} else {
						flags.reset(a_flag);
					}
				};
				setflag("Teleport", Flags::Teleport);
				setflag("InCombat", Flags::InCombat);
				setflag("Hidden", Flags::Hidden);

				if (const auto cons = root["Conditions"]; cons.IsDefined()) {
					using Type = CONDITION_DATA::ConditionType;
					const auto makecondition = [&cons](std::vector<CONDITION_DATA>& v, const char* a_attribute, Type a_type) -> void {
						if (!cons[a_attribute].IsDefined())
							return;

						v.emplace_back(a_type, cons[a_attribute].as<std::string>());
					};
					auto& c1 = conditions[ConditionTarget::Victoire];
					makecondition(c1, "RaceType", Type::Race);
					makecondition(c1, "Faction", Type::Faction);
					auto& c2 = conditions[ConditionTarget::Victim];
					makecondition(c2, "VictimFaction", Type::Faction);
				}
			}
			break;
		default:
			throw ParseException("Invalid version: {}", v);
		}
	}

	bool EventData::CheckConditions(const std::vector<RE::Actor*>& a_victoires, RE::Actor* a_victim) const
	{
		for (auto&& condition : conditions[ConditionTarget::Victim]) {
			if (!condition.Check(a_victim))
				return false;
		}
		for (auto&& condition : conditions[ConditionTarget::Victoire]) {
			const auto w = std::find_if(a_victoires.begin(), a_victoires.end(), [&](RE::Actor* a_victoire) {
				return condition.Check(a_victoire);
			});
			if (w == a_victoires.end())
				return false;
		}
		return true;
	}

	EventData::CONDITION_DATA::CONDITION_DATA(ConditionType a_type, const std::string& a_conditionobject)
	{
		switch (a_type) {
		case ConditionType::Race:
			{
				value.racetype = a_conditionobject.c_str();
			}
			break;
		case ConditionType::Faction:
			{
				const auto val = FormFromString<RE::TESFaction*>(a_conditionobject);
				if (!val)
					throw ParseException("Invalid Condition: {}", a_conditionobject);

				value.faction = val;
			}
			break;
		default:
			throw ParseException("Invalid Type: {}", a_type);
		}
	}

	bool EventData::CONDITION_DATA::Check(RE::Actor* a_actor) const
	{
		switch (type) {
		case ConditionType::Race:
			{
				const auto r = Animation::GetRaceType(a_actor);
				return r == value.racetype;
			}
			break;
		case ConditionType::Faction:
			{
				return a_actor->IsInFaction(value.faction);
			}
			break;
		default:
			logger::error("Unrecognized type: {}", type);
			return false;
		}
	}

	void Resolution::Initialize()
	{
		auto& h = Events[Type::Hostile].emplace_back(GameForms::DefaultCommon);
		h.flags.set(EventData::Flags::Hidden);

		auto& g = Events[Type::Guard].emplace_back(GameForms::DefaultGuard);
		g.flags.set(EventData::Flags::Hidden);

		try {
			const auto wpath = CONFIGPATH("Consequences\\Weights.yaml");
			auto weights = fs::exists(wpath) ? YAML::LoadFile(wpath) : YAML::Node{};

			const auto read = [&weights](const std::string_view& a_type, std::vector<EventData>& list) {
				const auto& path = CONFIGPATH("Consequences\\"s + a_type.data());
				if (!fs::exists(path)) {
					return;
				}
				for (auto& file : fs::directory_iterator{ path }) {
					const auto filepath = file.path().string();
					logger::info("Reading File: {}", filepath);
					try {
						auto& data = list.emplace_back(filepath);
						if (auto n = weights[a_type.data()]; n.IsDefined()) {
							if (auto node = n[data.name]; node.IsDefined()) {
								data.weight = node.as<decltype(data.weight)>();
								logger::info("Using event weight for event {}: {}", data.name, data.weight);
							}
						}
						logger::info("Successfully added event {}", data.name);
					} catch (const std::exception& e) {
						logger::error("Unable to read file {}; Error: {}", filepath, e.what());
					}
				}
			};
			for (auto&& t : magic_enum::enum_entries<Type>()) {
				read(t.second, Events[t.first]);
			}
		} catch (const std::exception& e) {
			logger::info("Unable to register events; Error: {}", e.what());
		}
	}

	void Resolution::Save()
	{
		YAML::Node save{};
		for (auto&& t : magic_enum::enum_entries<Type>()) {
			const auto& events = Events[t.first];
			for (auto&& e : events) {
				// if (e.name != "NAME_MISSING")
					save[t.second.data()][e.name] = e.weight;
			}
		}
		std::ofstream fout{ CONFIGPATH("Consequences\\Weights.yaml") };
		fout << save;
	}

	RE::TESQuest* Resolution::SelectQuest(Type type, RE::Actor* a_victim, const std::vector<RE::Actor*>& a_victoires, bool a_incombat)
	{
		if (Events[type].empty()) {
			logger::info("No custom events for type {}", type);
			return nullptr;
		}
		const bool tp = Validation::AllowTeleport();
		int priority = 0, weights = 0;
		std::vector<std::pair<RE::TESQuest*, decltype(weights)>> ret{};
		for (auto& e : Events[type]) {
			if (a_incombat && e.flags.none(EventData::Flags::InCombat) || !tp && e.flags.any(EventData::Flags::Teleport))
				continue;
			if (e.priority < priority || e.weight < 0)
				continue;
			if (!e.CheckConditions(a_victoires, a_victim))
				continue;
			if (e.priority > priority) {
				ret.clear();
				priority = e.priority;
				weights = 0;
			}
			weights += e.weight;
			ret.emplace_back(e.quest, weights);
		}
		if (weights && !ret.empty()) {
			const auto where = Random::draw<decltype(weights)>(1, weights);
			const auto there = std::find_if(ret.begin(), ret.end(), [where](std::pair<RE::TESQuest*, int32_t>& pair) { return where <= pair.second; });
			logger::info("Selecting event: {} / {}) ", there->first->GetFormEditorID(), there->first->GetFormID());
			return there->first;
		}
		return nullptr;
	}

	std::vector<std::pair<const std::string&, uint8_t>> Resolution::GetEvents(Type a_type)
	{
		std::vector<std::pair<const std::string&, uint8_t>> ret{};
		for (auto&& e : Events[a_type]) {
			if (e.flags.any(EventData::Flags::Hidden))
				continue;

			ret.emplace_back(e.name, e.weight);
		}
		return ret;
	}

	void Resolution::SetEventWeight(const std::string& a_name, Type a_type, uint8_t a_weight)
	{
		for (auto&& e : Events[a_type]) {
			if (e.name == a_name) {
				e.weight = a_weight;
				return;
			}
		}
		logger::warn("Unable to find event with name {} of type {}. Weight will NOT be set", a_name, a_type);
	}
}
