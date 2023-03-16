#include "Acheron/Interface/HunterPride.h"

#include "Acheron/Interface/Interface.h"
#include "Serialization/EventManager.h"

namespace Acheron::Interface
{
	///
	/// Menu
	///

	HunterPride::HunterPride() :
			RE::IMenu()
	{
		this->inputContext = Context::kCursor;
		this->depthPriority = 3;
		this->menuFlags.set(
				// Flag::kPausesGame,
				Flag::kUsesMenuContext,
				Flag::kUsesCursor,
				// Flag::kDisablePauseMenu,
				Flag::kCustomRendering,
				Flag::kApplicationMenu);

		auto scaleform = RE::BSScaleformManager::GetSingleton();
		[[maybe_unused]] bool success = scaleform->LoadMovieEx(this, FILEPATH, [](RE::GFxMovieDef* a_def) -> void {
			a_def->SetState(
					RE::GFxState::StateType::kLog,
					RE::make_gptr<FlashLogger<HunterPride>>().get());
		});
		assert(success);

		auto view = this->uiMovie;
		view->SetMouseCursorCount(1);

		RE::GFxValue _main;
		success = view->GetVariable(&_main, "_root.main");
		assert(success && _main.IsObject());

		RE::GFxFunctionHandler* fn = new OnItemSelected;
		RE::GFxValue dst;
		view->CreateFunction(&dst, fn);
		success = _main.SetMember("OnItemSelected", dst);
		assert(success);

		RE::GFxFunctionHandler* fn2 = new CloseComplete;
		RE::GFxValue dst2;
		view->CreateFunction(&dst2, fn2);
		success = _main.SetMember("CloseComplete", dst2);
		assert(success);
	}

	void HunterPride::Register()
	{
		RE::UI::GetSingleton()->Register(NAME, Create);
		logger::info("Registered Hunter Pride Menu");
	}

	bool HunterPride::AddOption(const RE::BSFixedString& a_option, const std::string& a_conditionstring, const std::string& a_name, const std::string& a_iconsrc)
	{
		if (!HasOption(a_option)) {
			try {
				_options.emplace_back(a_option, a_conditionstring, a_name, a_iconsrc);
				return true;
			} catch (const std::exception& e) {
				logger::error("Error adding option: {}", e.what());
			}
		}
		return false;
	}

	bool HunterPride::RemoveOption(const RE::BSFixedString& a_option)
	{
		const auto where = std::find_if(_options.begin(), _options.end(), [&a_option](Option& option) { return option.GetID() == a_option; });
		if (where != _options.end()) {
			_options.erase(where);
			return true;
		}
		return false;
	}

	bool HunterPride::HasOption(const RE::BSFixedString& a_option)
	{
		return std::find_if(_options.begin(), _options.end(), [&a_option](Option& option) {
			return option.GetID() == a_option;
		}) != _options.end();
	}

	RE::UI_MESSAGE_RESULTS HunterPride::ProcessMessage(RE::UIMessage& a_message)
	{
		using Type = RE::UI_MESSAGE_TYPE;
		using Result = RE::UI_MESSAGE_RESULTS;

		switch (*a_message.type) {
		case Type::kShow:
			{
				constexpr auto vampire_cond = "{\"player\":{\"has\":{\"keywords\":[\"0xa82bb\"]}},\"target\":{\"not\":{\"keywords\":[\"0x13796\"]}}}";

				std::vector<RE::GFxValue> args;
				args.reserve(_options.size() + 4);
				static const std::array defaults{
					Option{ "rescue", "", "$Achr_Plunder", "Plunder.dds" },
					Option{ "plunder", "", "$Achr_Execute", "Execute.dds" },
					Option{ "execute", "", "$Achr_Rescue", "Rescue.dds" },
					Option{ "vampire", vampire_cond, "$Achr_Vampire", "Vampire.dds" }
				};
				for (size_t i = 0; i < defaults.size() + _options.size(); i++) {
					// IDEA: Hidden flag?
					const Option op = i < 4 ? defaults[i] : _options[i - 4];
					bool enabled = false;
					if (op._id == "rescue") {
						enabled = HasBeneficialPotion(RE::PlayerCharacter::GetSingleton());
					} else {
						enabled = op.Check();
					}
					RE::GFxValue arg{};
					this->uiMovie->CreateObject(&arg);
					op.PopulateObjectData(arg);
					arg.SetMember("enabled", { enabled });
					args.push_back(arg);
				}
				this->uiMovie->InvokeNoReturn("_root.main.SetEntriesAndOpen", args.data(), static_cast<uint32_t>(args.size()));
			}
			return Result::kHandled;
		default:
			return RE::IMenu::ProcessMessage(a_message);
		}
	}

	///
	/// Callback
	///

	void HunterPride::OnItemSelected::Call(Params& a_args)
	{
		RE::BSFixedString id{ a_args.args[0].GetString() };
		logger::info("Selected HunterPride option: {}", id);
		Serialization::EventManager::GetSingleton()->_hunterprideselect.QueueEvent(id, _target);
	}

	void HunterPride::CloseComplete::Call(Params&)
	{
		Hide();
	}

	///
	/// Option
	///

	HunterPride::Option::Option(const RE::BSFixedString& a_option, const std::string& a_conditions, const std::string& a_name, const std::string& a_icon) :
			_id(a_option),
			_name(a_name),
			_iconurl(a_icon)
	{
		if (a_conditions.empty())
			return;

		// conditions are a json string, see .json file in folder
		using CType = CONDITION::ConditionType;
		const auto root = json::parse(a_conditions);
		const auto make_conditions = [&root](const char* a_branch) {
			std::vector<CONDITION> ret{};
			const auto node = root.find(a_branch);
			if (node == root.end() || !node->is_object())
				return ret;

			const auto helper = [&node, &ret](bool a_reverse) -> void {
				const char* branch = a_reverse ? "not" : "has";
				const auto lists = node->find(branch);
				if (lists == node->end() || !lists->is_object())
					return;

				const auto addattributes = [&](const char* a_attribute, CType a_type) -> void {
					auto objects = lists->find(a_attribute);
					if (objects == lists->end() || !objects->is_array())
						return;

					for (auto&& obj : *objects) {
						if (!obj.is_string()) {
							continue;
						}
						const auto str = obj.get<std::string>();
						ret.emplace_back(a_type, str, a_reverse);
					}
				};
				addattributes("factions", CType::Faction);
				addattributes("keywords", CType::Keyword);
			};
			helper(true);
			helper(false);
			return ret;
		};
		conditions[ConditionTarget::Player] = make_conditions("player");
		conditions[ConditionTarget::Target] = make_conditions("target");
	}

	HunterPride::Option::CONDITION::CONDITION(ConditionType a_type, const std::string& a_objstring, bool a_reverse) :
			reverse(a_reverse),
			type(a_type)
	{
		const auto GetValue = [&]<class T>(T& a_value) {
			if (a_objstring.find('|') == std::string::npos) {
				const auto base = a_objstring.starts_with("0x") ? 16 : 10;
				auto id = std::stoi(a_objstring.data(), nullptr, base);
				using t = std::remove_pointer<T>::type;
				a_value = RE::TESForm::LookupByID<t>(id);
			} else {
				a_value = FormFromString<T>(a_objstring);
			}
			if (!a_value) {
				throw std::exception{ fmt::format("Condition received invalid value argument {}", a_objstring).c_str() };
			}
		};
		switch (a_type) {
		case ConditionType::Faction:
			GetValue(value.faction);
			break;
		case ConditionType::Keyword:
			GetValue(value.keyword);
			break;
		}
	}

	bool HunterPride::Option::CONDITION::Check(RE::Actor* a_target) const
	{
		bool ret;
		switch (type) {
		case ConditionType::Keyword:
			ret = a_target->HasKeyword(value.keyword);
			break;
		case ConditionType::Faction:
			ret = a_target->IsInFaction(value.faction);
			break;
		default:
			return false;
		}
		return reverse ? !ret : ret;
	}

	bool HunterPride::Option::Check() const
	{
		for (size_t i = 0; i < ConditionTarget::Total; i++) {
			auto target = i == ConditionTarget::Player ? RE::PlayerCharacter::GetSingleton() : _target;
			for (auto&& c : conditions[i]) {
				if (!c.Check(target))
					return false;
			}
		}
		return true;
	}

	const RE::BSFixedString& HunterPride::Option::GetID() const
	{
		return _id;
	}

	void HunterPride::Option::PopulateObjectData(RE::GFxValue& a_object) const
	{
		a_object.SetMember("entryid", { _id.data() });
		a_object.SetMember("entrynm", { _name.data() });
		a_object.SetMember("iconsrc", { _iconurl.data() });
	}

	///
	/// Serialization
	///

	void HunterPride::Save(SKSE::SerializationInterface* a_intfc)
	{
		const size_t numRegs = _options.size();
		if (!a_intfc->WriteRecordData(numRegs)) {
			logger::error("Failed to save number of regs ({})", numRegs);
			return;
		}

		for (auto&& option : _options) {
			if (!stl::write_string(a_intfc, option._id)) {
				logger::error("Failed to save option id ({})", option._id);
				continue;
			}
			if (!stl::write_string(a_intfc, option._name)) {
				logger::error("Failed to save option name ({})", option._name);
				continue;
			}
			if (!stl::write_string(a_intfc, option._iconurl)) {
				logger::error("Failed to save option url ({})", option._iconurl);
				continue;
			}
			for (size_t i = 0; i < Option::ConditionTarget::Total; i++) {
				const size_t numCons = option.conditions[i].size();
				if (!a_intfc->WriteRecordData(numCons)) {
					logger::error("Failed to save number of conditions ({})", numCons);
					continue;
				}
				for (auto&& condition : option.conditions[i]) {
					if (!a_intfc->WriteRecordData(condition.reverse)) {
						logger::error("Failed to save condition attribute reverse ({})", condition.reverse);
						break;
					}
					if (!a_intfc->WriteRecordData(static_cast<uint8_t>(condition.type))) {
						logger::error("Failed to save condition attribute type ({})", static_cast<uint8_t>(condition.type));
						break;
					}
					RE::FormID formID;
					switch (condition.type) {
					case Option::CONDITION::ConditionType::Faction:
						formID = condition.value.faction->GetFormID();
						break;
					case Option::CONDITION::ConditionType::Keyword:
						formID = condition.value.keyword->GetFormID();
						break;
					}
					if (!a_intfc->WriteRecordData(formID)) {
						logger::error("Failed to save condition attribute value ({})", formID);
						break;
					}
				}
			}
		}

		logger::info("Saved {} options to cosave", numRegs);
	}

	void HunterPride::Load(SKSE::SerializationInterface* a_intfc)
	{
		_options.clear();

		std::size_t numRegs;
		a_intfc->ReadRecordData(numRegs);

		std::string id{}, name{}, url{};

		for (size_t i = 0; i < numRegs; i++) {
			Option next{};
			stl::read_string(a_intfc, id);
			stl::read_string(a_intfc, name);
			stl::read_string(a_intfc, url);

			next._id = id;
			next._name = name;
			next._iconurl = url;

			for (size_t j = 0; j < Option::ConditionTarget::Total; j++) {
				size_t numCons;
				a_intfc->ReadRecordData(numCons);
				next.conditions[j].reserve(numCons);

				bool reverse;
				uint8_t type;
				RE::FormID formID;

				for (size_t n = 0; n < numCons; n++) {
					a_intfc->ReadRecordData(reverse);
					a_intfc->ReadRecordData(type);
					a_intfc->ReadRecordData(formID);
					if (!a_intfc->ResolveFormID(formID, formID)) {
						logger::error("Unable to resolve formID {:X}", formID);
						continue;
					}
					Option::CONDITION::ConditionValue value;
					Option::CONDITION::ConditionType __type{ type };
					switch (__type) {
					case Option::CONDITION::ConditionType::Faction:
						value.faction = RE::TESForm::LookupByID<RE::TESFaction>(formID);
						if (!value.faction) {
							logger::error("Unable to retrieve form with ID {:X}", formID);
							continue;
						}
						break;
					case Option::CONDITION::ConditionType::Keyword:
						value.keyword = RE::TESForm::LookupByID<RE::BGSKeyword>(formID);
						if (!value.keyword) {
							logger::error("Unable to retrieve form with ID {:X}", formID);
							continue;
						}
						break;
					}
					next.conditions[j].emplace_back(__type, value, reverse);
				}
			}
			_options.push_back(next);
		}
		
		logger::info("Restored {} options from cosave", _options.size());
	}

	void HunterPride::Revert(SKSE::SerializationInterface*)
	{
		_options.clear();
	}

}
