#include "Acheron/Validation.h"

namespace Acheron
{
	void Validation::Initialize()
	{
		logger::info("Loading Exclusion Data");
		exclLocTp = {
			0x00018C91,	 // Cidhna Mine
			0x00018EE6,	 // Azuras Star Interior (DA01)
			0x00018C91,	 // Sovngarde
			0x0005254C,	 // Abandoned Shack Interior
			0x00018C92,	 // Thalmor Embassy
		};
		exclNPC[VTarget::Either] = {
			0x0003C57C,	 // Paarthurnax
			0x0200A877,	 // DLC1 Gelebor
			0x020030D8,	 // Durnehviir
			0x0001D4B9,	 // Emperor
			0x0001D4BA,	 // Emperor Decoy
			0x00044050,	 // GaiusMaro (DB06)
			0x000A733B,	 // Vigilant Tyranus (DA10)
			0x0009CB66,	 // Malkoran (DA09)
			0x000EBE2E,	 // Malkoran's Shade (DA09)
			0x0004D246,	 // The Caller (MG03)
			0x0009C8AA,	 // Weylin (MS01)
			0x0009E07A,	 // MQ306DragonA
			0x0009E07B,	 // MQ306DragonB
			0x0009E07C,	 // MQ306DragonC
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
		exclNPC[VTarget::Assailant] = {
			0x00034D97,	 // Estomo (MG07)
		};
		exclRef[VTarget::Either] = {
			0x000A6F4B,	 // Companion Farkas Werewolf, Ambusher01 (C02)
			0x000A6F1E,	 // Companion Farkas Werewolf, Ambusher02 (C02)
			0x000A6F43,	 // Companion Farkas Werewolf, Ambusher03 (C02)
			0x000A6F0F,	 // Companion Farkas Werewolf, Ambusher04 (C02)
			0x000A6F0E,	 // Companion Farkas Werewolf, Ambusher05 (C02)
		};
		exclFaction[VTarget::Either] = {
			Acheron::GameForms::IgnoredFaction->GetFormID(),
			0x00028347,	 // Alduin fac
			0x00103531,	 // Restoration Master Qst
		};
		const auto exclusionpath = CONFIGPATH("Validation");
		if (fs::exists(exclusionpath)) {
			const auto parseList = [](const YAML::Node& a_node, std::vector<RE::FormID>& a_list) {
				if (!a_node.IsDefined())
					return;

				const auto items = a_node.as<std::vector<std::string>>();
				for (auto& id : items) {
					const auto it = FormFromString<RE::FormID>(id);
					if (!it) {
						logger::info("Cannot exclude \'{}\'. Form does not exist or assoaicted file not loaded", id);
						continue;
					}
					a_list.push_back(it);
				}
			};
			const auto readBranch = [&](const YAML::Node& a_branch, VTarget a_validation) {
				if (!a_branch.IsDefined())
					return;
				parseList(a_branch["ActorBase"], exclNPC[a_validation]);
				parseList(a_branch["Reference"], exclRef[a_validation]);
				parseList(a_branch["Race"], exclRace[a_validation]);
				parseList(a_branch["Faction"], exclFaction[a_validation]);
			};
			for (auto& file : fs::directory_iterator{ exclusionpath }) {
				try {
					const auto filepath = file.path().string();
					if (!filepath.ends_with(".yaml") && !filepath.ends_with(".yml"))
						continue;
					logger::info("Reading file {}", filepath);
					const auto root{ YAML::LoadFile(filepath) };
					parseList(root["LocationA"], exclLocAll);
					parseList(root["LocationT"], exclLocTp);
					parseList(root["MagicEffect"], exclMagicEffect);

					readBranch(root["Assailant"], VTarget::Assailant);
					readBranch(root["Victim"], VTarget::Victim);
					readBranch(root, VTarget::Either);
					logger::info("Added exclusion data from file {}", filepath);
				} catch (const std::exception& e) {
					logger::error("{}", e.what());
				}
			}
		}
		logger::info("Loaded validation data");
	}

	bool Validation::CanProcessDamage()
	{
		if (!Settings::ProcessingEnabled) {
			return false;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (const auto loc = player->GetCurrentLocation()) {
			if (std::find(exclLocAll.begin(), exclLocAll.end(), loc->formID) != exclLocAll.end()) {
				return false;
			}
		}

		if (static const auto DGIntimidateQuest = RE::TESForm::LookupByID<RE::TESQuest>(0x00047AE6); DGIntimidateQuest->currentStage == 10)	 // Brawl Quest
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
		if (a_victim->IsDead() || a_victim->IsInKillMove())
			return false;
		if (a_victim->IsPlayerRef()) {
			if (!Settings::bPlayerDefeat) {
				return false;
			}
		} else if (a_aggressor) {
			if (!UsesHunterPride(a_aggressor) && (!Settings::bNPCDefeat || !a_victim->IsHostileToActor(a_aggressor)))
				return false;
			if (auto ref = a_victim->GetObjectReference(); ref && ref->As<RE::BGSKeywordForm>()->HasKeywordID(0xD205E))	// ActorTypeGhost
				return false;
		}
		if (a_aggressor) {
			if (a_aggressor->IsDead() || a_aggressor->IsInKillMove())
				return false;
			if (!Settings::bCreatureDefeat && !IsNPC(a_aggressor))
				return false;
			if (!ValidateActor(a_aggressor))
				return false;
			if (!CheckAssailantID(a_aggressor->formID))
				return false;
			if (!CheckExclusion(VTarget::Assailant, a_aggressor))
				return false;
		}
		if (!ValidateActor(a_victim))
			return false;
		if (!CheckVictimID(a_victim->formID))
			return false;
		return CheckExclusion(VTarget::Victim, a_victim);
	}

	bool Validation::AllowRescueEffect(RE::EffectSetting* a_effect)
	{
		return !std::ranges::contains(exclMagicEffect, a_effect->GetFormID());
	}

	bool Validation::AllowDetrimentalEffect(RE::EffectSetting* a_effect)
	{
		return std::ranges::contains(exclMagicEffect, a_effect->GetFormID());
	}

	bool Validation::AllowTeleport()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		if (player->GetCurrentScene() != nullptr) {
			return false;
		}
		if (const auto loc = player->GetCurrentLocation()) {
			if (std::ranges::contains(exclLocTp, loc->formID))
				return false;
		}
		static const auto MQ101 = RE::TESForm::LookupByID<RE::TESQuest>(0x0003372B);
		if (MQ101->currentStage > 1 && MQ101->currentStage < 1000)	 // Vanilla Intro
			return false;

		return true;
	}

	bool Validation::ValidateActor(RE::Actor* a_actor)
	{
		if (a_actor->IsChild()) {
			return false;
		}
		const auto furnihandle = a_actor->GetOccupiedFurniture();
		if (const auto furni = furnihandle.get()) {
			static const auto DA02BoethiahPillar = RE::TESForm::LookupByID<RE::BGSKeyword>(0x000F3B64);
			if (furni->HasKeyword(DA02BoethiahPillar))
			return false;
		}
		if (a_actor->IsPlayerRef()) {
			return true;
		}

		const auto base = Acheron::GetLeveledActorBase(a_actor);
		if (!base) {
			logger::info("Invalid Actor {:X}: Missing Base", a_actor->GetFormID());
			return false;
		}

		switch (base->formID) {
		case 0x0001327E:	// Tulius
		case 0x0001414D:	// Ulfric
			{
				static const auto CWSiege = RE::TESForm::LookupByID<RE::TESQuest>(0x00096E71);
				if (!CWSiege->IsEnabled())
					break;
				for (const auto& objective : CWSiege->objectives) {	 // "Bring Ulfric/Tullius to justice" objectives
					if (objective->index != 4001 && objective->index != 4002)
						continue;
					if (objective->state.all(RE::QUEST_OBJECTIVE_STATE::kDisplayed))
						return false;
				}
			}
			break;
		case 0x0001BDB1:	// Cicero
			{
				static const auto DB07 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA56);
				if (DB07->currentStage >= 40 && DB07->currentStage < 200)
					return false;
			}
			break;
		case 0x0001B07C:	// Mercer Frey
			{
				static const auto TG08b = RE::TESForm::LookupByID<RE::TESQuest>(0x00021554);
				if (TG08b->IsEnabled())
					return false;
			}
			break;
		case 0x0001A694:	// Vilkas
			{
				static const auto& C00VilkasTrainingQuest = RE::TESForm::LookupByID<RE::TESQuest>(0x000A3EBC);
				if (C00VilkasTrainingQuest->IsEnabled())
					return false;
			}
			break;
		case 0x20058B0:	 // Dexion
			{
				static const auto DLC1VQ3Hunter = RE::TESForm::LookupByID<RE::TESQuest>(0x020098CB);
				static const auto DLC1VQ03Vamp = RE::TESForm::LookupByID<RE::TESQuest>(0x020098CB);
				if (DLC1VQ3Hunter->IsEnabled() || DLC1VQ03Vamp->IsEnabled())
					return false;
			}
			break;
		}

		return true;
	}

	bool Validation::CheckVictimID(RE::FormID a_formid)
	{
		switch (a_formid) {
		case 0x00013387:	// Anton
		case 0x00038C6E:	// BalagogGroNolob
			{
				const auto DB08 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA57);
				if (DB08->currentStage == 4 || DB08->currentStage == 6)
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

		return true;
	}

	bool Validation::CheckAssailantID(RE::FormID a_formid)
	{
		switch (a_formid) {
		case 0x02003368:	// Stalf
		case 0x02003369:	// Salonia Caelia
			{
				const auto& DLC1VampIntro = RE::TESForm::LookupByID<RE::TESQuest>(0x0200594C);
				if (DLC1VampIntro->currentStage == 40)
					return false;
			}
			break;
		}

		return true;
	}

	bool Validation::CheckExclusion(VTarget a_validation, RE::Actor* a_actor)
	{
		const auto find = [&](RE::FormID a_id, const std::vector<RE::FormID> a_list[3]) -> bool {
			return std::ranges::contains(a_list[VTarget::Either], a_id) || std::ranges::contains(a_list[a_validation], a_id);
		};
		if (find(a_actor->GetFormID(), exclRef)) {
			return false;
		}
		const auto base = Acheron::GetLeveledActorBase(a_actor);
		if (base && find(base->GetFormID(), exclNPC)) {
			return false;
		}
		const auto race = a_actor->GetRace();
		if (race && find(a_actor->GetFormID(), exclRace)) {
			return false;
		}
		// visitor returns true on the first iteration that returns true, false otherwise
		return !a_actor->VisitFactions([&](RE::TESFaction* faction, uint8_t rank) {
			if (!faction || rank < 0)
				return false;

			return find(faction->formID, exclFaction);
		});
	}

}	 // namespace Acheron
