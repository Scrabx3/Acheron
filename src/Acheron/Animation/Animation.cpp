#include "Acheron/Animation/Animation.h"

namespace Acheron
{
	std::string Animation::GetRaceType(RE::Actor* a_actor)
	{
		const auto base = a_actor->GetActorBase();
		const auto race = a_actor->GetRace();
		const auto sex = base->GetSex();
		if (!race || !base || sex == RE::SEXES::kNone) {
			logger::critical("Actor {:X} (Base: {:X}) has no race/base/sex", a_actor->GetFormID(), base ? base->GetFormID() : 0);
			return "";
		}
		const std::string_view rootTMP{ race->rootBehaviorGraphNames[sex].data() };
		const auto root{ rootTMP.substr(rootTMP.rfind('\\') + 1) };
		if (root == "0_Master.hkx") {
			return "Human";
		}

		const std::unordered_map<std::string_view, std::string> behaviorfiles{
			{ "WolfBehavior.hkx", "Wolf"  },
			{ "DogBehavior.hkx", "Dog"  },
			{ "ChickenBehavior.hkx", "Chicken"  },
			{ "HareBehavior.hkx", "Hare"  },
			{ "AtronachFlameBehavior.hkx", "FlameAtronach"  },
			{ "AtronachFrostBehavior.hkx", "FrostAtronach"  },
			{ "AtronachStormBehavior.hkx", "StormAtronach"  },
			{ "BearBehavior.hkx", "Bear"  },
			{ "ChaurusBehavior.hkx", "Chaurus"  },
			{ "H-CowBehavior.hkx", "Cow"  },
			{ "DeerBehavior.hkx", "Deer"  },
			{ "CHaurusFlyerBehavior.hkx", "ChaurusHunter"  },
			{ "VampireBruteBehavior.hkx", "Gargoyle"  },
			{ "BenthicLurkerBehavior.hkx", "Lurker"  },
			{ "BoarBehavior.hkx", "Boar"  },
			{ "BCBehavior.hkx", "DwarvenBallista"  },
			{ "HMDaedra.hkx", "Seeker"  },
			{ "NetchBehavior.hkx", "Netch"  },
			{ "RieklingBehavior.hkx", "Riekling"  },
			{ "ScribBehavior.hkx", "AshHopper"  },
			{ "DragonBehavior.hkx", "Dragon"  },
			{ "Dragon_Priest.hkx", "DragonPriest"  },
			{ "DraugrBehavior.hkx", "Draugr"  },
			{ "SCBehavior.hkx", "DwarvenSphere"  },
			{ "DwarvenSpiderBehavior.hkx", "DwarvenSpider"  },
			{ "SteamBehavior.hkx", "DwarvenCenturion"  },
			{ "FalmerBehavior.hkx", "Falmer"  },
			{ "FrostbiteSpiderBehavior.hkx", "Spider"  },
			{ "GiantBehavior.hkx", "Giant"  },
			{ "GoatBehavior.hkx", "Goat"  },
			{ "HavgravenBehavior.hkx", "Hagraven"  },
			{ "HorkerBehavior.hkx", "Horker"  },
			{ "HorseBehavior.hkx", "Horse"  },
			{ "IceWraithBehavior.hkx", "IceWraith"  },
			{ "MammothBehavior.hkx", "Mammoth"  },
			{ "MudcrabBehavior.hkx", "Mudcrab"  },
			{ "SabreCatBehavior.hkx", "Sabrecat"  },
			{ "SkeeverBehavior.hkx", "Skeever"  },
			{ "SlaughterfishBehavior.hkx", "Slaughterfish"  },
			{ "SprigganBehavior.hkx", "Spriggan"  },
			{ "TrollBehavior.hkx", "Troll"  },
			{ "VampireLord.hkx", "VampireLord"  },
			{ "WerewolfBehavior.hkx", "Werewolf"  },
			{ "WispBehavior.hkx", "Wispmother"  },
			{ "WitchlightBehavior.hkx", "Wisp"  },
		};
		auto where = behaviorfiles.find(root);
		return where == behaviorfiles.end() ? "" : where->second;
	}
}
