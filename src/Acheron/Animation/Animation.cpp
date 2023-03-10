#include "Acheron/Animation/Animation.h"

namespace Acheron
{
	std::vector<std::string> Animation::LookupAssaultAnimations(RE::Actor* a_victim, RE::Actor* a_partner, AssaultType a_type)
	{
		if (!IsNPC(a_victim))
			return {};

		const std::string racetype{ Animation::GetRaceType(a_partner) };
		if (racetype.empty())
			return {};

		try {
			static const auto root{ json::parse(std::ifstream{ CONFIGPATH("Assault.json") }) };
			const auto where = root.find(racetype);
			if (where == root.end() || !where->is_object())
				return {};

			const auto& node = *where;
			switch (a_type) {
			case AssaultType::LeadIn:
				if (node.contains("LeadIn") && node["LeadIn"].is_array()) {
					return node["LeadIn"].get<std::vector<std::string>>();
				}
				__fallthrough;
			case AssaultType::Instant:
				if (node.contains("Instant") && node["Instant"].is_array()) {
					return node["Instant"].get<std::vector<std::string>>();
				}
				break;
			case AssaultType::BreakFree:
				if (node.contains("BreakFree") && node["BreakFree"].is_array()) {
					return node["BreakFree"].get<std::vector<std::string>>();
				} else {
					logger::error("No break free animations for race type {}", racetype);
					return { "IdleForceDefaultState"s, "StaggerStart"s };
				}
				break;
			}
		} catch (const std::exception& e) {
			logger::error(e.what());
		}
		return {};
	}

	void Animation::SetPositions(const std::vector<RE::Actor*>& a_positions, RE::TESObjectREFR* a_center)
	{
		const auto centerPos = a_center->GetPosition();
		const auto centerAng = a_center->GetAngle();

		for (auto&& subject : a_positions) {
			subject->data.angle = centerAng;
			subject->SetPosition(centerPos, true);
			subject->Update3DPosition(true);
		}

		const auto setposition = [centerAng, centerPos](RE::Actor* actor) {
			for (size_t i = 0; i < 6; i++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(300));
				actor->data.angle.z = centerAng.z;
				actor->SetPosition(centerPos, false);
			}
		};
		std::for_each(a_positions.begin(), a_positions.end(), [&setposition](RE::Actor* subject) { std::thread(setposition, subject).detach(); });
	}

	void Animation::PlayAnimation(RE::TESObjectREFR* a_reference, const char* a_animation)
	{
		SKSE::GetTaskInterface()->AddTask([=]() { a_reference->NotifyAnimationGraph(a_animation); });
	}

	// TODO: get racetype based on skeleton data?
	std::string Animation::GetRaceType(RE::Actor* subject)
	{
		if (IsNPC(subject))
			return "Human"s;

		auto race = subject->GetRace();
		if (!race)
			return ""s;

		switch (race->GetFormID()) {
		case 0x0401B658:  // Ash Hopper
			return "AshHopper"s;
		case 0x000131F5:  // Atronach Flame
			return "AtronachFlame"s;
		case 0x000131F6:  // Atronach Frost
			return "AtronachFrost"s;
		case 0x000131F7:  // Atronach Storm
		case 0x04027BFC:  // Ash Guardian
			return "AtronachStorm"s;
		case 0x000131E8:  // Cave Bear
		case 0x000131E7:  // Brown Bear
		case 0x000131E9:  // Snow Bear
			return "Bear"s;
		case 0x04024038:  // Boar
			return "Boar"s;
		case 0x000131EB:  // Chaurus
		case 0x02015136:  // Frozen Chaurus
			return "Chaurus"s;
		case 0x000A5601:  // Chaurus Reapeer
			return "ChaurusReapeer"s;
		case 0x020051FB:  // Chaurus Hunter
			return "ChaurusHunter"s;
		case 0x000A919D:  // Chicken
			return "Chicken"s;
		case 0x0004E785:  // Cow
			return "Cow"s;
		case 0x000CF89B:  // Deer
		case 0x000131ED:  // Elk
		case 0x0200D0B2:  // DLC1 Vale Deer
			return "Deer"s;
		case 0x00012E82:  // Dragon
		case 0x001052A3:  // Dragon Undead
			return "Dragon"s;
		case 0x000131EF:  // Dragon Priest
		case 0x000EBE18:  // Necro Dragon Priest
		case 0x0403911A:  // DLC2 Acolyte Dragon Priest
			return "DragonPriest"s;
		case 0x0402A6FD:  // DLC2 Hulking Draugr
		case 0x0403CECB:  // DLC2 Rigid Skeleton
		case 0x0401B637:  // DLC2 Ash Spawn
		case 0x0200894D:  // DLC1 Soul Cairn Armored Skeleton
		case 0x020023E2:  // DLC1 Armored Skeleton
		case 0x02019FD3:  // DLC1 Black Skeleton Skeleton
		case 0x0200A94B:  // DLC1 Soul Cairn Boneman
		case 0x02006AFA:  // DLC1 Necro Skeleton
		case 0x000B7998:  // Skeleton
		case 0x000B9FD7:  // Rigid Skeleton
		case 0x000EB872:  // Necro Skeleton
		case 0x00000D53:  // Draugr
		case 0x000F71DC:  // Draugr Magic
			return "Draugr"s;
		case 0x000131F1:  // DwarvenCenturion
		case 0x02015C34:  // DLC1 Forgemaster
			return "DwarvenCenturion"s;
		case 0x0402B014:  // DwarvenBallista
			return "DwarvenBallista"s;
		case 0x000131F2:  // DwarvenSphere
			return "DwarvenSphere"s;
		case 0x000131F3:  // DwarvenSpider
			return "DwarvenSpider"s;
		case 0x000131F4:  // Falmer
		case 0x0201AACC:  // DLC1 Frozen Falmer
			return "Falmer"s;
		case 0x000131F8:  // FrostbiteSpider
			return "FrostbiteSpider"s;
		case 0x00053477:  // FrostbideSpiderLarge
			return "FrostbideSpiderLarge"s;
		case 0x0004E507:  // FrostbiteSpiderGiant
			return "FrostbiteSpiderGiant"s;
		case 0x0200A2C6:  // Gargoyle
		case 0x02010D00:  // Gargoyle Variant Boss
		case 0x02019D86:  // Gargoyle Variant Green
			return "Gargoyle"s;
		case 0x0401CAD8:  // DLC2 Frost Giant
		case 0x04014495:  // DLC2 Lurker
		case 0x000131F9:  // Giant
			return "Giant"s;
		case 0x0006FC4A:  // Domestic Goat
		case 0x000131FA:  // Goat
			return "Goat"s;
		case 0x000131FB:  // Hagraven
			return "Hagraven"s;
		case 0x0006DC99:  // Hare
			return "Hare"s;
		case 0x000131FC:  // Horker
			return "Horker"s;
		case 0x000131FD:  // Horse
			return "Horse"s;
		case 0x0401F98F:  // DLC2 Spectral
		case 0x04029EE7:  // DLC2 Karstaag IceWrath
		case 0x000131FE:  // IceWrath
			return "IceWrath"s;
		case 0x000131FF:  // Mammoth
			return "Mammoth"s;
		case 0x040179CF:  // MountedRiekling
			return "MountedRiekling"s;
		case 0x0401B647:  // Solstheim Mudcrab
		case 0x000BA545:  // Mudcrab
			return "Mudcrab"s;
		case 0x04028580:  // Netch Calf
		case 0x0401FEB8:  // Netch
			return "Netch"s;
		case 0x0401A50A:  // Thirst Riekling
		case 0x04017F44:  // Riekling
			return "Riekling"s;
		case 0x00013202:  // Snow SabreCat
		case 0x00013200:  // SabreCat
			return "SabreCat"s;
		case 0x0401DCB9:  // Seeker
			return "Seeker"s;
		case 0x000C3EDF:  // White Skeever
		case 0x00013201:  // Skeever
			return "Skeever"s;
		case 0x00013203:  // Slaughterfish
			return "Slaughterfish"s;
		case 0x0401B644:  // Burned Spriggan
		case 0x02013B77:  // Spriggan Earth Mother
		case 0x000F3903:  // Spriggan Matron
		case 0x00013204:  // Spriggan
			return "Spriggan"s;
		case 0x020117F4:  // Armored Frost Troll
		case 0x02011F75:  // Armored Troll
		case 0x00013206:  // Frost Troll
		case 0x00013205:  // Troll
			return "Troll"s;
		case 0x0200283A:  // VampireBeast (Vampire Lord)
			return "VampireBeast"s;
		case 0x0401E17B:  // Werebear
		case 0x000CDD84:  // Werewolf
			return "Werewolf"s;
		case 0x02003D02:  // Death Hound Companion
		case 0x0200C5F0:  // Death Hound
		case 0x02003D01:  // DLC1 Armored Husky Companion
		case 0x02018B33:  // DLC1 Armored Husky
		case 0x020122B7:  // DLC1 Husky Bare Companion
		case 0x02018B36:  // DLC1 Husky Bare
		case 0x000CD657:  // Barbas Race
		case 0x000F1AC4:  // Dog Companion
		case 0x000131EE:  // Dog
		case 0x000F905F:  // Dog (MG07)
		case 0x0001320A:  // Wolf
			return "Wolf"s;
		case 0x00109C7C:	// Fox
			return "Fox"s;
		default:
			{
				std::string racename{ race->GetFormEditorID() };
				logger::info("Requested RaceKey for unrecognized Race = {}", racename);
				ToLower(racename);
				const auto contains = [&racename](const char* str) -> bool { return racename.find(str) != std::string_view::npos; };
				if (contains("atronach")) {
					if (contains("flame"))
						return "AtronachFlame"s;
					else if (contains("frost"))
						return "AtronachFrost"s;
					else if (contains("storm"))
						return "AtronachStorm"s;
				} else if (contains("ash")) {
					if (contains("hopper"))
						return "AshHopper"s;
					else if (contains("guardian"))
						return "AtronachStorm"s;
				} else if (contains("bear")) {
					return "Bear"s;
				} else if (contains("riekling")) {
					if (contains("mounted") || contains("boar"))
						return "MountedRiekling"s;
					else
						return "Riekling"s;
				} else if (contains("boar")) {
					return "Boar"s;
				} else if (contains("chaurus")) {
					if (contains("reaper"))
						return "ChaurusReaper"s;
					else if (contains("hunter") || contains("flying"))
						return "ChaurusHunter"s;
					else
						return "Chaurus"s;
				} else if (contains("chicken")) {
					return "Chicken"s;
				} else if (contains("cow")) {
					return "Cow"s;
				} else if (contains("deer") || contains("elk")) {
					return "Deer"s;
				} else if (contains("dragon")) {
					if (contains("priest"))
						return "DragonPriest"s;
					else
						return "Dragon"s;
				} else if (contains("dwarven")) {
					if (contains("spider"))
						return "DwarvenSpider"s;
					else if (contains("ballista"))
						return "DwarvenBallista"s;
					else if (contains("sphere"))
						return "DwarvenSphere"s;
					else if (contains("centurion"))
						return "DwarvenCenturion"s;
				} else if (contains("falmer")) {
					return "Falmer"s;
				} else if (contains("spider")) {
					if (contains("large"))
						return "FrostbideSpiderLarge"s;
					else if (contains("giant"))
						return "FrostbiteSpiderGiant"s;
					else
						return "FrostbiteSpider"s;
				} else if (contains("gargoyle")) {
					return "Gargoyle"s;
				} else if (contains("giant") || contains("lurker")) {
					return "Giant"s;
				} else if (contains("goat")) {
					return "Goat"s;
				} else if (contains("hagraven")) {
					return "Hagraven"s;
				} else if (contains("hare") || contains("rabbit") || contains("bunny")) {
					return "Hare"s;
				} else if (contains("horker")) {
					return "Horker"s;
				} else if (contains("horse")) {
					return "Horse"s;
				} else if (contains("icewrath")) {
					return "IceWrath"s;
				} else if (contains("mammoth")) {
					return "Mammoth"s;
				} else if (contains("mudcrab")) {
					return "Mudcrab"s;
				} else if (contains("netch")) {
					return "Netch"s;
				} else if (contains("sabrecat")) {
					return "Sabrecat"s;
				} else if (contains("skeever")) {
					return "Skeever"s;
				} else if (contains("seeker")) {
					return "Seeker"s;
				} else if (contains("slaughterfish")) {
					return "Slaughterfish"s;
				} else if (contains("spriggan")) {
					return "Spriggan"s;
				} else if (contains("troll")) {
					return "Troll"s;
				} else if (contains("vampire")) {
					return "VampireBeast"s;
				} else if (contains("werewolf") || contains("werebear")) {
					return "Werewolf"s;
				} else if (contains("wolf") || contains("dog") || contains("husky") || contains("hound")) {
					return "Wolf"s;
				} else if (contains("fox")) {
					return "Fox"s;
				} else if (contains("draugr") || contains("skeleton")) {
					// skeleton can technically be used in combination with many creature types, so this is checked last
					return "Draugr"s;
				}
				return ""s;
			}
		}
	}
}
