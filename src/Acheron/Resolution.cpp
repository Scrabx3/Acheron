#include "Acheron/Resolution.h"

#include "Acheron/Animation/Animation.h"
#include "Acheron/Validation.h"
#include "Acheron/Defeat.h"

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
			break;
		default:
			throw std::exception(fmt::format("Invalid version: {}", version).c_str());
		}
		// Flags
		const auto setflag = [this, &root](Flag a_flag) {
			std::string key{ magic_enum::enum_name(a_flag) };
			if (!root[key].IsDefined())
				return false;

			auto ret = root[key].as<bool>();
			if (ret) {
				flags.set(a_flag);
			} else {
				flags.reset(a_flag);
			}
			return ret;
		};
		if (setflag(Flag::StartTeleport)) {
			flags.set(Flag::Teleport);
		} else {
			setflag(Flag::Teleport);
		}
		setflag(Flag::InCombat);
		setflag(Flag::Hidden);
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
						makecondition(node["CrimeFaction"], dest, ConditionType::CrimeFaction);
					}
					if (const auto node = cons["Victim"]; node.IsDefined()) {
						auto& dest = conditions[ConditionTarget::Victim];
						makecondition(node["Faction"], dest, ConditionType::Faction);
						makecondition(node["Keyword"], dest, ConditionType::Keyword);
						makecondition(node["Location"], dest, ConditionType::Location);
						makecondition(node["Worldspace"], dest, ConditionType::WorldSpace);
						makecondition(node["CrimeFaction"], dest, ConditionType::CrimeFaction);
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
			const auto fixed = RE::BSFixedString(a_conditionobject);
			if (fixed == "none" || fixed == "null") {
				dest = nullptr;
				return;
			}
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
		case ConditionType::CrimeFaction:
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
			{
				throw std::exception(fmt::format("Invalid Type: {}", static_cast<int>(a_type)).c_str());
			}
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
				result = value.quest->IsEnabled();
			}
			break;
		case ConditionType::CrimeFaction:
			{
				result = a_actor->GetCrimeFaction() == value.faction;
				break;
			}
		default:
			logger::error("Unrecognized type: {}", static_cast<int>(type));
			return false;
		}
		return result == compare;
	}

	void Resolution::Initialize()
	{
		const auto defaultFlags = stl::enumeration{ EventData::Flag::Teleport, EventData::Flag::StartTeleport, EventData::Flag::InCombat }.get();
		Events[Type::Hostile].emplace_back(GameForms::DefaultCommon, "Acheron Default", defaultFlags);
		Events[Type::Guard].emplace_back(GameForms::DefaultGuard, "Acheron Default Guard", defaultFlags);

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

	bool Resolution::SelectQuest(Type type, RE::Actor* a_victim, const std::vector<RE::Actor*>& a_victoires, bool a_incombat, bool a_doteleport)
	{
		logger::info("Looking up quest of type {} for {:X} and {} aggressors. Combat? {} / Teleport? {}", type, a_victim->formID, a_victoires.size(), a_incombat, a_doteleport);
		const bool tp = Validation::AllowTeleport();
		if (!tp && a_doteleport) {
			logger::info("Teleport event requested but teleportation is disabled");
			return false;
		}
		stl::enumeration<EventData::Flag> argFlags{};
		if (a_incombat)
			argFlags.set(EventData::Flag::InCombat);
		if (a_doteleport)
			argFlags.set(EventData::Flag::StartTeleport);
		if (type == Type::Any) {
			if (!a_victim->IsPlayerRef()) {
				return SelectQuestImpl(Type::NPC, a_victim, a_victoires, argFlags.get());
			}
			auto types = std::vector{ Type::Civilian, Type::Hostile };
			auto fl = GetFollowers();
			for (auto&& f : fl) {
				if (!Defeat::IsDamageImmune(f)) {
					types.push_back(Type::Follower);
					break;
				}
			}
			Random::shuffle(types);
			for (auto&& t : types) {
				if (SelectQuestImpl(t, a_victim, a_victoires, argFlags.get())) {
					return true;
				}
			}
			return false;
		}
		return SelectQuestImpl(type, a_victim, a_victoires, argFlags.get());
	}

	bool Resolution::SelectQuestImpl(Type type, RE::Actor* a_victim, const std::vector<RE::Actor*>& a_victoires, const EventData::Flag& a_flags)
	{
		assert(type != Type::Any && type != Type::Total);
		if (Events[type].empty()) {
			logger::info("No custom events for type {}", type);
			return false;
		}
		std::vector<std::pair<RE::TESQuest*, int>> ret[EventData::Priority::p_Total];
		for (auto& e : Events[type]) {
			if (e.weight <= 0 || !e.flags.all(a_flags))
				continue;
			if (!e.CheckConditions(a_victoires, a_victim))
				continue;
			ret[e.priority].emplace_back(e.quest, e.weight);
		}
		for (auto i = EventData::Priority::p_Total - 1; i >= 0; i--) {
			auto& it = ret[i];
			while (!it.empty()) {
				const auto weights = std::ranges::fold_left(it | std::ranges::views::values, 0, std::plus<>());
				auto where = Random::draw<int>(1, weights);
				const auto there = std::find_if(it.begin(), it.end(), [where](std::pair<RE::TESQuest*, int32_t>& pair) mutable {
					where -= pair.second;
					return where <= 0;
				});
				if (there->first->Start()) {
					logger::info("Started event: {:X} ({})", there->first->GetFormID(), there->first->GetFormEditorID());
					return true;
				}
				logger::info("Cannot start event: {:X} ({})", there->first->GetFormID(), there->first->GetFormEditorID());
				it.erase(there);
			}
		}
		logger::info("No event quest found");
		return false;
	}

	inline std::string MakeMCMEventKey(const EventData& e) {
		return fmt::format("[{}{}{}] {};{};{}",
				e.priority,
				e.flags.all(EventData::Flag::InCombat) ? ", C" : "",
				e.flags.all(EventData::Flag::Teleport) ? ", T" : "",
				e.name,
				e.weight,
				e.quest->formID);
	}

	std::vector<std::string> Resolution::GetEvents(Type a_type)
	{
		std::vector<std::string> ret{};
		for (auto&& e : Events[a_type]) {
			if (e.flags.any(EventData::Flag::Hidden))
				continue;
			const auto argS = MakeMCMEventKey(e);
			ret.push_back(argS);
		}
		std::sort(ret.begin(), ret.end(), [](std::string& a, std::string& b) {
			return a[1] < b[1];
		});
		return ret;
	}

	std::string Resolution::SetEventWeight(const std::string& a_name, Type a_type, uint8_t a_weight)
	{
		const auto where = a_name.rfind(';');
		const auto idstr = a_name.substr(where + 1);
		const auto id = static_cast<RE::FormID>(std::stoul(idstr));
		for (auto&& e : Events[a_type]) {
			if (e.quest->formID == id) {
				e.weight = a_weight;
				return MakeMCMEventKey(e);
			}
		}
		logger::warn("Unable to find event with name {} of type {}. Weight will NOT be set", a_name, a_type);
		return "";
	}
}
