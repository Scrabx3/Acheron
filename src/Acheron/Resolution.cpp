#include "Acheron/Resolution.h"

#include "Acheron/Animation/Animation.h"
#include "Acheron/Validation.h"

namespace Acheron
{
	EventData::EventData(const std::string& a_filepath)
	{
		if (!a_filepath.ends_with(".yaml") && !a_filepath.ends_with(".yml"))
			throw std::exception("Invalid file extension");

		auto root{ YAML::LoadFile(a_filepath) };
		// Requirements
		const auto version = root["Version"].IsDefined() ? root["Version"].as<int>() : 1;
		if (const auto req = root["Requirements"]; req.IsDefined()) {
			if (!req.IsSequence())
				throw std::exception("Requirements defined but not a sequence");

			const auto plugins = req.as<std::vector<std::string>>();
			for (auto&& plugin : plugins) {
				if (RE::TESDataHandler::GetSingleton()->LookupModByName(plugin) == nullptr)
					throw std::exception(fmt::format("Requirement not loaded: {}", plugin).c_str());
			}
		}
		// Quest
		quest = FormFromString<RE::TESQuest*>(root["Quest"].as<std::string>());
		if (!quest) {
			throw std::exception(fmt::format("Unable to find quest: {}", root["Quest"].as<std::string>()).c_str());
		}
		// Attributes
		const auto setattribute = [this, &root]<class T>(const char* a_setting, T& a_attribute) {
			if (!root[a_setting].IsDefined())
				return;

			a_attribute = root[a_setting].as<T>();
		};
		setattribute("Name", name);
		setattribute("Cooldown", cooldown);
		switch (version) {
		case 2:
			{
				uint8_t prio;
				setattribute("Priority", prio);
				if (prio >= Priority::p_Total) {
					throw std::exception(fmt::format("Invalid priority: {}", prio).c_str());
				}
				priority = Priority(prio);
			}
			break;
		case 1:
			{
				uint8_t prio;
				setattribute("Priority", prio);
				if (prio > 85) {
					priority = Priority::StoryPriority;
				} else if (prio > 50) {
					priority = Priority::StoryGeneric;
				} else if (prio > 0) {
					priority = Priority::Common;
				} else {
					priority = Priority::Default;
				}
			}
		default:
			throw std::exception(fmt::format("Invalid version: {}", version).c_str());
		}
		// Flags
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
		// Consequences
		if (const auto cons = root["Conditions"]; cons.IsDefined()) {
			using ConditionType = CONDITION_DATA::ConditionType;
			switch (version) {
			case 2:
				{
					const auto makecondition = [](const YAML::Node& node, std::vector<CONDITION_DATA>& dest, ConditionType a_type) -> void {
						if (!node.IsDefined()) {
							return;
						}
						for (size_t i = 0; i < node.size(); i++) {
							const auto it = node[i];
							if (it.IsMap()) {
								const auto entry = it.begin();
								dest.emplace_back(a_type, entry->first.as<std::string>(), entry->second.as<bool>());
							} else {
								dest.emplace_back(a_type, it.as<std::string>(), true);
							}
						}
					};
					if (const auto node = cons["Assailant"]; node.IsDefined()) {
						auto& dest = conditions[ConditionTarget::Assailant];
						makecondition(node["RaceType"], dest, ConditionType::Race);
						makecondition(node["Faction"], dest, ConditionType::Faction);
						makecondition(node["Keyword"], dest, ConditionType::Keyword);
					}
					if (const auto node = cons["Victim"]; node.IsDefined()) {
						auto& dest = conditions[ConditionTarget::Victim];
						makecondition(node["Faction"], dest, ConditionType::Faction);
						makecondition(node["Keyword"], dest, ConditionType::Keyword);
						makecondition(node["Location"], dest, ConditionType::Location);
						makecondition(node["Worldspace"], dest, ConditionType::WorldSpace);
					}
					if (const auto node = cons["Quest"]; node.IsDefined()) {
						auto& dest = conditions[ConditionTarget::Unspecified];
						makecondition(node["Completed"], dest, ConditionType::QuestDone);
						makecondition(node["Running"], dest, ConditionType::QuestRunning);
					}
				}
				break;
			case 1:
				{
					const auto makecondition = [&cons](std::vector<CONDITION_DATA>& v, const char* a_attribute, ConditionType a_type) -> void {
						if (!cons[a_attribute].IsDefined())
							return;

						const auto conditions = cons[a_attribute].as<std::vector<std::string>>();
						for (auto&& condition : conditions) {
							v.emplace_back(a_type, condition, true);
						}
					};
					auto& c1 = conditions[ConditionTarget::Assailant];
					makecondition(c1, "RaceType", ConditionType::Race);
					makecondition(c1, "Faction", ConditionType::Faction);
					auto& c2 = conditions[ConditionTarget::Victim];
					makecondition(c2, "VictimFaction", ConditionType::Faction);
				}
				break;
			default:
				throw std::exception(fmt::format("Invalid version: {}", version).c_str());
			}
		}
	}

	bool EventData::CheckConditions(const std::vector<RE::Actor*>& a_victoires, RE::Actor* a_victim) const
	{
		for (auto&& condition : conditions[ConditionTarget::Victim]) {
			if (!condition.Check(a_victim))
				return false;
		}
		for (auto&& condition : conditions[ConditionTarget::Assailant]) {
			const auto w = std::find_if(a_victoires.begin(), a_victoires.end(), [&](RE::Actor* a_victoire) {
				return condition.Check(a_victoire);
			});
			if (w == a_victoires.end())
				return false;
		}
		for (auto&& condition : conditions[ConditionTarget::Unspecified]) {
			if (!condition.Check(nullptr))
				return false;
		}
		return true;
	}

	EventData::CONDITION_DATA::CONDITION_DATA(ConditionType a_type, std::string a_conditionobject, bool a_compare) :
		type(a_type), compare(a_compare)
	{
		const auto getval = [&]<typename T>(T& dest) {
			const auto val = FormFromString<T>(a_conditionobject);
			if (!val)
				throw std::exception(fmt::format("Object does not represent a valid form: {}", a_conditionobject).c_str());
			dest = val;
		};

		switch (a_type) {
		case ConditionType::Race:
			{
				ToLower(a_conditionobject);
				value.racetype = _strdup(a_conditionobject.c_str());
			}
			break;
		case ConditionType::Faction:
			{
				getval(value.faction);
			}
			break;
		case ConditionType::Keyword:
			{
				getval(value.keyword);
			}
			break;
		case ConditionType::Location:
			{
				getval(value.location);
			}
			break;
		case ConditionType::WorldSpace:
			{
				getval(value.worldspace);
			}
			break;
		case ConditionType::QuestRunning:
		case ConditionType::QuestDone:
			{
				getval(value.quest);
			}
			break;
		default:
			throw std::exception(fmt::format("Invalid Type: {}", a_type).c_str());
		}
	}

	bool EventData::CONDITION_DATA::Check(RE::Actor* a_actor) const
	{
		bool result;
		switch (type) {
		case ConditionType::Race:
			{
				auto r{ Animation::GetRaceType(a_actor) };
				ToLower(r);
				result = r == value.racetype;
			}
			break;
		case ConditionType::Faction:
			{
				result = a_actor->IsInFaction(value.faction);
			}
			break;
		case ConditionType::Keyword:
			{
				result = a_actor->HasKeyword(value.keyword);
			}
			break;
		case ConditionType::Location:
			{
				result = a_actor->GetCurrentLocation() == value.location;
			}
			break;
		case ConditionType::WorldSpace:
			{
				result = a_actor->GetWorldspace() == value.worldspace;
			}
			break;
		case ConditionType::QuestDone:
			{
				result = value.quest->IsCompleted();
			}
			break;
		case ConditionType::QuestRunning:
			{
				result = value.quest->IsRunning();
			}
			break;
		default:
			logger::error("Unrecognized type: {}", type);
			return false;
		}
		return result == compare;
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
			if (t.first == Type::Total)
				break;

			const auto& events = Events[t.first];
			for (auto&& event : events) {
				try {
					if (event.name != EventData::DEFAULT_NAME)
						save[t.second.data()][event.name] = static_cast<int32_t>(event.weight);
				} catch (const std::exception& e) {
					logger::error("Failed to save event weight for event {}: {}", event.name, e.what());
				}
			}
		}
		std::ofstream fout{ CONFIGPATH("Consequences\\Weights.yaml") };
		fout << save;

		logger::info("Saved resolution user data");
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
			if (e.weight <= 0 || a_incombat && e.flags.none(EventData::Flags::InCombat) || !tp && e.flags.any(EventData::Flags::Teleport))
				continue;
			if (e.priority < priority)
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
