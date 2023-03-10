#pragma once

#include "Acheron/Defeat.h"

namespace Acheron::Console
{
	inline bool ParseCmd(const std::string_view& a_cmd, RE::TESObjectREFR* a_targetRef)
	{
		if (!a_cmd.starts_with("acheron"))
			return false;

		std::vector<std::string> words{};
		std::string next = ""s;
		for (size_t i = 8; i < a_cmd.size(); i++) {
			if (std::isspace(a_cmd[i])) {
				if (!next.empty()) {
					words.push_back(next);
					next = ""s;
				}
				continue;
			}

			if (a_cmd[i] == '"') {
				auto w = a_cmd.find('"', i + 1);
				if (w == std::string_view::npos) {
					PrintConsole("[Acheron] Unexpexted EOF");
					return false;
				}
				words.emplace_back(a_cmd.substr(i + 1, w - i - 1));
				i = w;
				continue;
			}

			next.push_back(a_cmd[i]);
		}
		if (!next.empty()) {
			words.push_back(next);
		}
		if (words.empty()) {
			PrintConsole("[Acheron] Unrecognized line. Use 'help' for a list of possible interactions");
			return false;
		}

		const auto gettarget = [&](int idx) -> RE::Actor* {
			if (words.size() > idx) {
				auto& formid = words[idx];
				if (formid == "player")
					return RE::PlayerCharacter::GetSingleton();

				if (formid.find_first_not_of("0123456789abcdef") == std::string::npos && formid.length() <= 8) {
					try {
						int id = std::stoi(formid, nullptr, 16);
						return RE::TESForm::LookupByID<RE::Actor>(id);
					} catch (const std::exception&) {
						return nullptr;
					}
				}
				return nullptr;
			}
			if (a_targetRef && a_targetRef->Is(RE::FormType::ActorCharacter))
				return a_targetRef->As<RE::Actor>();

			return RE::PlayerCharacter::GetSingleton();
		};
		if (words[0] == "help") {
			PrintConsole(
					"[Acheron] Possible commands are:\n"
					"\t\"Defeat <reference>\"\n"
					"\t\"Rescue <reference>\"\n"
					"\t\"Pacify <reference>\"\n"
					"\t\"Release <reference>\"");
		} else if (auto target = gettarget(1); !target) {
			PrintConsole("[Acheron] Missing target reference");
		} else if (words[0] == "defeat") {
			if (Defeat::IsDefeated(target)) {
				PrintConsole("[Acheron] Selected target actor is already defeated");
			} else {
				Defeat::DefeatActor(target);
			}
		} else if (words[0] == "rescue") {
			if (Defeat::IsDefeated(target)) {
				Defeat::RescueActor(target, true);
			} else {
				PrintConsole("[Acheron] Actor is not defeated");
			}
		} else if (words[0] == "pacify") {
			if (Defeat::IsPacified(target)) {
				PrintConsole("[Acheron] Selected target actor is already pacified");
			} else {
				Defeat::Pacify(target);
			}
		} else if (words[0] == "release") {
			if (Defeat::IsPacified(target)) {
				Defeat::UndoPacify(target);
			} else {
				PrintConsole("[Acheron] Actor is not pacified");
			}
		} else {
			PrintConsole("[Acheron] Unrecognized line. Use 'help' for a list of possible interactions");
		}
		return true;
	}
}	 // namespace Acheron::Console
