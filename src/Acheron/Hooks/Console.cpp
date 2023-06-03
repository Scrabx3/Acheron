#include "Console.h"

namespace Acheron
{
	bool Console::ParseCmd(std::string_view a_cmd, RE::TESObjectREFR* a_targetRef)
	{
		if (!a_cmd.starts_with("acheron"))
			return false;

		const auto trimWhitespace = [&](size_t& i) -> size_t {
			for (; i < a_cmd.size(); i++)
				if (!std::isspace(a_cmd[i]))
					break;
			return i;
		};

		std::vector<std::string_view> words{};
		size_t i = 8, start_idx_crrent = trimWhitespace(i), start_idx_previous = 0;
		for (; i < a_cmd.size(); i++) {
			if (std::isspace(a_cmd[i])) {
				if (start_idx_crrent != start_idx_previous) {
					const auto newword = a_cmd.substr(start_idx_crrent, i - start_idx_crrent);
					words.push_back(newword);
					start_idx_previous = start_idx_crrent;
				}
				start_idx_crrent = trimWhitespace(i);
			}
			if (a_cmd[i] == '"') {
				auto w = a_cmd.find('"', i + 1);
				if (w == std::string_view::npos) {
          PrintConsole("[Acheron] Unexpexted EOF");
					return false;
				}
				words.push_back(a_cmd.substr(i + 1, w - i - 1));
				start_idx_crrent = trimWhitespace(i = w + 1);
			}
		}
		if (start_idx_crrent < i) {
			auto word = a_cmd.substr(start_idx_crrent);
			words.push_back(word);
		}

		if (words.empty()) {
			PrintConsole("[Acheron] Expected command but got EOF. Use \"Acheron help\" for a list of possible commands");
			return false;
		}
		std::string command{ words[0] };
		const auto where = commands.find(command);
		if (where == commands.end()) {
			PrintConsole("[Acheron] Unrecognized command \"{}\", use \"Help\" for a list possible commands", words[0]);
			return false;
		}
		return where->second->Run(words, a_targetRef);
	}

	RE::TESObjectREFR* CommandBase::ParseTargetRef(std::string_view a_targetStr) const
	{
		if (a_targetStr == "player") {
			return RE::PlayerCharacter::GetSingleton();
		}
		try {
			return FormFromString<RE::TESObjectREFR*>(a_targetStr, 16);
		} catch (const std::exception&) {
			return nullptr;
		}
	}

	RE::Actor* CommandBase::GetTargetActor(std::string_view a_targetStr, RE::TESObjectREFR* a_targetRef) const
	{
		RE::Actor* targetRef = nullptr;
		if (a_targetRef) {
			targetRef = a_targetRef->As<RE::Actor>();
		}
		if (!targetRef && !a_targetStr.empty()) {
			const auto tmp = ParseTargetRef(a_targetStr);
			return tmp ? tmp->As<RE::Actor>() : nullptr;
		}
		return nullptr;
	}

	bool CmdHelp::Run(std::vector<std::string_view>&, RE::TESObjectREFR*) const
	{
		PrintConsole(
			"-------------------------------------------------\n"
			"\tdefeat (actor): Defeat an actor, causing them to become unable to fight\n"
			"\trescue (actor, bAndRelease = 1): Rescue (undo defeat) an actor, optionally also releasing them\n"
			"\tisdefeated (actor): Test if an actor is currently defeated\n"
			"\tpacify (actor): Pacify an actor, disallowing them to join combat\n"
			"\trelease (actor): Release (undo pacify) an actor\n"
			"\tispacified (actor): Test is an actor is currently pacified\n"
			"-------------------------------------------------\n");
		return true;
	}

	bool CmdDefeat::Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const
	{
		const auto target = GetTargetActor(a_args.size() > 1 ? a_args[1] : ""sv, a_targetRef);
		if (!target) {
			PrintConsole("[Acheron] Missing paramaeter \"actor\" at position 1");
			return false;
		}
		Defeat::DefeatActor(target);
		return true;
	}

	bool CmdRescue::Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const
	{
		const auto target = GetTargetActor(a_args.size() > 1 ? a_args[1] : ""sv, a_targetRef);
		if (!target) {
			PrintConsole("[Acheron] Missing argument \"actor\" at position 1");
			return false;
		}
		bool release;
		if (a_args.size() > 2) {
			if (a_args[2] == "0" || a_args[2] == "false") {
				release = false;
			} else if (a_args[2] == "1" || a_args[2] == "true") {
				release = true;
			} else {
				PrintConsole("[Acheron] Invalid argument at position 2. Argument should be either \"1\" or \"0\"");
				return false;
			}
		} else {
			release = true;
		}
		Defeat::RescueActor(target, release);
		return true;
	}

	bool CmdIsDefeated::Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const
	{
		const auto target = GetTargetActor(a_args.size() > 1 ? a_args[1] : ""sv, a_targetRef);
		if (!target) {
			PrintConsole("[Acheron] Missing paramaeter \"actor\" at position 1");
			return false;
		}
		Defeat::IsDefeated(target);
		return true;
	}

	bool CmdPacify::Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const
	{
		const auto target = GetTargetActor(a_args.size() > 1 ? a_args[1] : ""sv, a_targetRef);
		if (!target) {
			PrintConsole("[Acheron] Missing paramaeter \"actor\" at position 1");
			return false;
		}
		Defeat::Pacify(target);
		return true;
	}

	bool CmdRelease::Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const
	{
		const auto target = GetTargetActor(a_args.size() > 1 ? a_args[1] : ""sv, a_targetRef);
		if (!target) {
			PrintConsole("[Acheron] Missing paramaeter \"actor\" at position 1");
			return false;
		}
		Defeat::UndoPacify(target);
		return true;
	}

	bool CmdIsPacified::Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const
	{
		const auto target = GetTargetActor(a_args.size() > 1 ? a_args[1] : ""sv, a_targetRef);
		if (!target) {
			PrintConsole("[Acheron] Missing paramaeter \"actor\" at position 1");
			return false;
		}
		Defeat::IsPacified(target);
		return true;
	}

}  // namespace Acheron
