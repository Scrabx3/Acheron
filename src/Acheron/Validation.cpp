#include "Acheron/Validation.h"

namespace Acheron
{
	void Validation::Initialize()
	{
		logger::info("Loading Exclusion Data");
		exLocation = {
			0x00018C91,	 // Sovngarde
			0x0005254C,	 // Abandoned Shack Interior
			0x00018EE6	 // Azuras Star Interior (DA01)
		};
		exTeleport = {
			0x00018C91	// Cidhna Mine
		};
		exNPC = {
			0x0003C57C,	 // Paarthurnax
			0x0200A877,	 // DLC1 Gelebor
			0x020030D8,	 // Durnehviir
			0x0001D4B9,	 // Emperor
			0x0001D4BA,	 // Emperor Decoy
			0x00044050,	 // GaiusMaro (DB06)
			0x00034D97,	 // Estomo (MG07)
			0x000C1908,	 // Red Eagle
			0x000A733B,	 // Vigilant Tyranus (DA10)
			0x0009CB66,	 // Malkoran (DA09)
			0x000EBE2E,	 // Malkoran's Shade (DA09)
			0x0004D246,	 // The Caller (MG03)
			0x0009C8AA,	 // Weylin (MS01)
			0x0009E07A,	 // MQ306DragonA
			0x0009E07B,	 // MQ306DragonB
			0x0009E07C,	 // MQ306DragonC
			0x040285C3,	 // Ebony Warrior
			0x04017F7D,	 // Miraak
			0x04017936,	 // Miraak (MQ01)
			0x04017938,	 // Miraak (MQ02)
			0x04017F81,	 // Miraak (MQ04)
			0x0401FB98,	 // Miraak (MQ06)
			0x040200D9,	 // Miraak (SoulSteal)
			0x04023F7B,	 // MiraakDragon
			0x04031CA5,	 // MiraakDragon (MQ02)
			0x04039B6B,	 // MiraakDragon (MQ06)
			0x0004D6D0	 // Astrid End
		};
		exReference = {
			0x000A6F4B,	 // Companion Farkas Werewolf, Ambusher01 (C02)
			0x000A6F1E,	 // Companion Farkas Werewolf, Ambusher02 (C02)
			0x000A6F43,	 // Companion Farkas Werewolf, Ambusher03 (C02)
			0x000A6F0F,	 // Companion Farkas Werewolf, Ambusher04 (C02)
			0x000A6F0E	 // Companion Farkas Werewolf, Ambusher05 (C02)
		};
		exRace = {
			0x02007AF3	// DLC1 Keeper
		};
		exFaction = {
			RE::TESForm::LookupByID<RE::TESFaction>(0x00028347),	// Alduin fac
			RE::TESForm::LookupByID<RE::TESFaction>(0x00050920),	// Jarl
			RE::TESForm::LookupByID<RE::TESFaction>(0x0002C6C8),	// Greybeards
			RE::TESForm::LookupByID<RE::TESFaction>(0x00103531)		// Restoration Master Qst
		};
		if (fs::exists(CONFIGPATH("Exclusion"))) {
			const auto ReadNode = []<class T>(const std::vector<std::string>& ids, std::vector<T>& list) {
				for (auto& id : ids) {
					const auto it = FormFromString<T>(id);
					if (!it) {
						logger::info("Cannot exclude \'{}\'. Form does not exist or assoaicted file not loaded", id);
						continue;
					}
					logger::info("Successfully excluded \'{}\'", id);
					list.push_back(it);
				}
			};
			const auto p_yaml = [&ReadNode]<class T>(const YAML::Node& a_root, const char* a_type, std::vector<T>& a_list) {
				if (const auto node = a_root[a_type]; node.IsDefined()) {
					const auto items = node.as<std::vector<std::string>>();
					ReadNode(items, a_list);
				}
			};
			const auto p_json = [&ReadNode]<class T>(const json& a_root, const char* a_type, std::vector<T>& a_list) {
				if (a_root.contains(a_type)) {
					const auto items = a_root[a_type].get<std::vector<std::string>>();
					ReadNode(items, a_list);
				}
			};

			for (auto& file : fs::directory_iterator{ CONFIGPATH("Exclusion") }) {
				try {
					const auto filepath = file.path().string();
					logger::info("Reading File = {}", filepath);
					if (filepath.ends_with(".yaml") || filepath.ends_with(".yml")) {
						const auto root{ YAML::LoadFile(filepath) };
						p_yaml(root, "Location", exLocation);
						p_yaml(root, "NoTeleport", exTeleport);
						p_yaml(root, "ActorBase", exNPC);
						p_yaml(root, "Reference", exReference);
						p_yaml(root, "Race", exRace);
						p_yaml(root, "Faction", exFaction);
					} else if (filepath.ends_with(".json")) {
						const auto root{ json::parse(std::ifstream{ filepath }) };
						p_json(root, "Location", exLocation);
						p_json(root, "NoTeleport", exTeleport);
						p_json(root, "ActorBase", exNPC);
						p_json(root, "Reference", exReference);
						p_json(root, "Race", exRace);
						p_json(root, "Faction", exFaction);
					}
				} catch (const std::exception& e) {
					logger::error(e.what());
				}
			}
		}
		std::sort(exLocation.begin(), exLocation.end());
		std::sort(exTeleport.begin(), exTeleport.end());
		std::sort(exNPC.begin(), exNPC.end());
		std::sort(exReference.begin(), exReference.end());
		std::sort(exRace.begin(), exRace.end());
		logger::info("Loaded validation data");
	}

	bool Validation::CanProcessDamage()
	{
		if (!Settings::ProcessingEnabled) {
			return false;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (const auto loc = player->GetCurrentLocation(); loc) {
			if (std::binary_search(exLocation.begin(), exLocation.end(), loc->formID))
				return false;
		}

		if (static const auto DGIntimidateQuest = RE::TESForm::LookupByID<RE::TESQuest>(0x00047AE6); DGIntimidateQuest->IsEnabled())	// Brawl Quest
			return false;
		if (static const auto MQ101 = RE::TESForm::LookupByID<RE::TESQuest>(0x0003372B); MQ101->currentStage > 1 && MQ101->currentStage < 1000)	 // Vanilla Intro
			return false;
		if (static const auto DLCVQ08 = RE::TESForm::LookupByID<RE::TESQuest>(0x02007C25); DLCVQ08->currentStage == 60)	 // Harkon
			return false;
		if (static const auto DLC1VQ07 = RE::TESForm::LookupByID<RE::TESQuest>(0x02002853); DLC1VQ07->currentStage == 120)	// Gelebor
			return false;
		if (static const auto DB10 = RE::TESForm::LookupByID<RE::TESQuest>(0x0003CEDA); DB10->currentStage < 200 && DB10->currentStage >= 10)	 // Sanctuary Raid
			return false;
		if (static const auto MG08 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001F258); MG08->currentStage == 30)	 // Ancano
			return false;
		if (static const auto DLC2MQ06 = RE::TESForm::LookupByID<RE::TESQuest>(0x040179D7); DLC2MQ06->currentStage >= 400 && DLC2MQ06->currentStage < 500)	// Miraak
			return false;

		return true;
	}

	bool Validation::ValidatePair(RE::Actor* a_victim, RE::Actor* a_aggressor)
	{
		if (a_victim->IsDead() || a_aggressor->IsDead())
			return false;
		if (a_victim->IsInKillMove() || a_aggressor->IsInKillMove())
			return false;
		if (a_victim->IsPlayerRef()) {
			if (!Settings::bPlayerDefeat)
				return false;
		} else {
			if (!Settings::bNPCDefeat)
				return false;
		}
		if (auto ref = a_victim->GetObjectReference(); ref && ref->As<RE::BGSKeywordForm>()->HasKeywordID(0xD205E))	// ActorTypeGhost
			return false;
		if (!Settings::bCreatureDefeat && !IsNPC(a_aggressor))
			return false;
		if (!a_aggressor->IsPlayerRef() && !a_victim->IsHostileToActor(a_aggressor))
			return false;

		return ValidateActor(a_victim) && ValidateActor(a_aggressor);
	}

	bool Validation::AllowTeleport()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		if (player->GetCurrentScene() != nullptr) {
			return false;
		}

		if (const auto loc = player->GetCurrentLocation(); loc) {
			if (std::binary_search(exTeleport.begin(), exTeleport.end(), loc->formID))
				return false;
		}
		return true;
	}

	bool Validation::ValidateActor(RE::Actor* a_actor)
	{
		if (a_actor->IsChild() || a_actor->IsInFaction(Acheron::GameForms::IgnoredFaction))
			return false;
		if (a_actor->IsPlayerRef())
			return true;
		if (std::binary_search(exReference.begin(), exReference.end(), a_actor->GetFormID()))
			return false;

		const auto base = Acheron::GetLeveledActorBase(a_actor);
		const auto formid = base ? base->GetFormID() : 0;
		if (!formid || std::binary_search(exNPC.begin(), exNPC.end(), formid))
			return false;

		switch (formid) {
		case 0x0001327E:	// Tulius
		case 0x0001414D:	// Ulfric
			{
				const auto CWSiege = RE::TESForm::LookupByID<RE::TESQuest>(0x00096E71);
				if (CWSiege->IsEnabled()) {
					for (const auto& objective : CWSiege->objectives) {
						// "Bring Ulfric/Tullius to justice" objectives
						if (objective->state.all(RE::QUEST_OBJECTIVE_STATE::kDisplayed) && objective->index > 4000 && objective->index <= 4102)
							return false;
					}
				}
			}
			break;
		case 0x00013387:	// Anton
		case 0x00038C6E:	// BalagogGroNolob
			{
				const auto DB08 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA57);
				if (DB08->currentStage == 4 || DB08->currentStage == 6)
					return false;
			}
			break;
		case 0x0001BDB1:	// Cicero
			{
				const auto DB07 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA56);
				if (DB07->currentStage >= 40 && DB07->currentStage < 200)
					return false;
			}
			break;
		case 0x0001327A:	// Vittoria Vici
			{
				const auto DB05 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA54);
				if (DB05->IsEnabled())
					return false;
			}
			break;
		case 0x0001B074:	// Alain Dufont
		case 0x0001412C:	// Nilsine Shatter-Shield
			{
				const auto DB03 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA53);
				if (DB03->IsEnabled())
					return false;
			}
			break;
		case 0x0001B07C:	// Mercer Frey
			{
				const auto TG08b = RE::TESForm::LookupByID<RE::TESQuest>(0x00021554);
				if (TG08b->IsEnabled())
					return false;
			}
			break;
		case 0x0001A694:	// Vilkas
			{
				const auto& C00VilkasTrainingQuest = RE::TESForm::LookupByID<RE::TESQuest>(0x000A3EBC);
				if (C00VilkasTrainingQuest->IsEnabled())
					return false;
			}
			break;
		case 0x02003368:	// Stalf
		case 0x02003369:	// Salonia Caelia
			{
				const auto& DLC1VampIntro = RE::TESForm::LookupByID<RE::TESQuest>(0x0200594C);
				if (DLC1VampIntro->currentStage == 40)
					return false;
			}
			break;
		case 0x20058B0:	 // Dexion
			{
				const auto DLC1VQ3Hunter = RE::TESForm::LookupByID<RE::TESQuest>(0x020098CB);
				const auto DLC1VQ03Vamp = RE::TESForm::LookupByID<RE::TESQuest>(0x020098CB);
				if (DLC1VQ3Hunter->IsEnabled() || DLC1VQ03Vamp->IsEnabled())
					return false;
			}
			break;
		case 0x00013268:	// Deeja
		case 0x0001328D:	// Jaree-Ra
			{
				const auto& LightsOut = RE::TESForm::LookupByID<RE::TESQuest>(0x00023A64);
				if (LightsOut->currentStage > 100)
					return false;
			}
			break;
		case 0x0001414A:	// Calixto
			{
				const auto& MS11 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001F7A3);
				if (MS11->IsEnabled())
					return false;
			}
			break;
		case 0x000136C0:	// Narfi
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA5B);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x0001360C:	// Ennodius Papius
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA5E);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00013612:	// Beitild
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA5F);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x0001367B:	// Hern
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA60);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x0001AA63:	// Lubuk
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA61);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00020040:	// Deekus
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA62);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x0001B1D7:	// Ma'randru-jo
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA63);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00013B97:	// Anoriath
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA64);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00020044:	// Agnis
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA65);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00020046:	// Maluril
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA66);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00013657:	// Helvar
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA67);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00013267:	// Safia
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA68);
				if (contract->IsEnabled())
					return false;
			}
			break;
		}

		const auto furnihandle = a_actor->GetOccupiedFurniture();
		if (const auto furni = furnihandle.get(); furni) {
			const auto DA02BoethiahPillar = RE::TESForm::LookupByID<RE::BGSKeyword>(0x000F3B64);
			if (furni->HasKeyword(DA02BoethiahPillar))
				return false;
		}

		const auto race = a_actor->GetRace();
		if (std::find(exRace.begin(), exRace.end(), race->GetFormID()) != exRace.end()) {
			return false;
		}
		std::string raceid{ race->GetFormEditorID() };
		ToLower(raceid);
		constexpr std::array editorids{ "child", "enfant", "little", "teen" };
		for (auto&& i : editorids) {
			if (raceid.find(i) != std::string::npos)
				return false;
		}

		for (auto& f : exFaction) {
			if (a_actor->IsInFaction(f))
				return false;
		}
		return true;
	}

}	 // namespace Acheron
