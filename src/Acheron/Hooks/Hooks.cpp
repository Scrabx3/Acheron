#include "Acheron/Hooks/Hooks.h"

#include "Acheron/Defeat.h"
#include "Acheron/EventSink.h"
#include "Acheron/Hooks/Console.h"
#include "Acheron/Hooks/Processing.h"
#include "Acheron/Validation.h"

using Archetype = RE::EffectArchetypes::ArchetypeID;

namespace Acheron
{
	void Hooks::Install()
	{
		SKSE::AllocTrampoline(1 << 7);
		auto& trampoline = SKSE::GetTrampoline();
		// ==================================================
		REL::Relocation<std::uintptr_t> wh{ RELID(37673, 38627), OFFSET(0x3C0, 0x4a8) };
		_WeaponHit = trampoline.write_call<5>(wh.address(), WeaponHit);
		// ==================================================
		REL::Relocation<std::uintptr_t> magichit{ RELID(33763, 34547), OFFSET(0x52F, 0x7B1) };
		_MagicHit = trampoline.write_call<5>(magichit.address(), MagicHit);
		// ==================================================
		REL::Relocation<std::uintptr_t> mha{ RELID(33742, 34526), OFFSET(0x1E8, 0x20B) };
		_DoesMagicHitApply = trampoline.write_call<5>(mha.address(), DoesMagicHitApply);
		// ==================================================
		REL::Relocation<std::uintptr_t> explH{ RELID(42677, 43849), OFFSET(0x38C, 0x3C2) };
		_ExplosionHit = trampoline.write_call<5>(explH.address(), ExplosionHit);
		// ==================================================
		REL::Relocation<std::uintptr_t> det{ RELID(41659, 42742), OFFSET(0x526, 0x67B) };
		_DoDetect = trampoline.write_call<5>(det.address(), DoDetect);
		// ==================================================
		REL::Relocation<std::uintptr_t> console{ RELID(52065, 52952), OFFSET(0xE2, 0x52) };
		_CompileAndRun = trampoline.write_call<5>(console.address(), CompileAndRun);
		// ==================================================
		REL::Relocation<std::uintptr_t> plu{ RE::PlayerCharacter::VTABLE[0] };
		_PlUpdate = plu.write_vfunc(0xAD, UpdatePlayer);
		// ==================================================
		REL::Relocation<std::uintptr_t> l3d{ RE::Character::VTABLE[0] };
		_Load3D = l3d.write_vfunc(0x6A, Load3D);
		// ==================================================
		REL::Relocation<std::uintptr_t> upc{ RE::Character::VTABLE[0] };
		_UpdateCombat = upc.write_vfunc(0x11B, UpdateCombat);

		logger::info("Hooks installed");
	}

	void Hooks::CompileAndRun(RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef)
	{
		std::string cmd{ a_script->GetCommand() };
		ToLower(cmd);
		if (Console::ParseCmd(cmd, a_targetRef))
			return;

		return _CompileAndRun(a_script, a_compiler, a_name, a_targetRef);
	}

	void Hooks::UpdatePlayer(RE::PlayerCharacter* player, float delta)
	{
		_PlUpdate(player, delta);

		auto w = Defeat::Victims.find(player->GetFormID());
		if (w == Defeat::Victims.end()) {
			if (Validation::CanProcessDamage())
				CalcDamageOverTime(player);

			return;
		}
		if (!w->second.allow_recovery)
			return;

		const auto per = GetAVPercent(player, RE::ActorValue::kHealth);
		if (Settings::fKdHealthThresh > 0.0f && per >= Settings::fKdHealthThresh) {
			Defeat::RescueDelayed(player, true);
			return;
		}

		if (Settings::iKdFallbackTimer) {
			const auto calendar = RE::Calendar::GetSingleton();
			auto days_passed = calendar->GetDaysPassed() - w->second.registered_at;
			auto seconds_passed = days_passed * 24 * 60 * 60;
			if (seconds_passed >= Settings::iKdFallbackTimer * calendar->GetTimescale()) {
				Defeat::RescueDelayed(player, true);
				return;
			}
		}
	}

	RE::NiAVObject* Hooks::Load3D(RE::Character& a_this, bool a_arg1)
	{
		const auto ret = _Load3D(a_this, a_arg1);

		const auto getref = [&]() {
			auto ref = a_this.GetObjectReference();
			if (!ref)
				logger::warn("Cannot add keywords to {:X}, form has no associated reference", a_this.formID);
			return ref;
		};

		if (Defeat::IsDefeated(&a_this)) {
			a_this.NotifyAnimationGraph("BleedoutStart");
			if (auto ref = getref(); ref) {
				ref->As<RE::BGSKeywordForm>()->AddKeywords({ GameForms::Defeated, GameForms::Pacified });
			}
		} else if (Defeat::IsPacified(&a_this)) {
			if (auto ref = getref(); ref) {
				ref->As<RE::BGSKeywordForm>()->AddKeyword(GameForms::Pacified);
			}
		}
		return ret;
	}

	void Hooks::UpdateCombat(RE::Character* a_this)
	{
		_UpdateCombat(a_this);

		if (Defeat::IsDamageImmune(a_this)) {
			if (a_this->IsInCombat())
				a_this->StopCombat();
		} else if (IsNPC(a_this) && !a_this->IsCommandedActor()) {
			if (Validation::CanProcessDamage())
				CalcDamageOverTime(a_this);
		}
	}

	void Hooks::CalcDamageOverTime(RE::Actor* a_target)
	{
		const auto effects = a_target->GetActiveEffectList();
		if (!effects)
			return;

		float total = 0.0f;
		for (const auto& effect : *effects) {
			if (!effect || effect->flags.any(RE::ActiveEffect::Flag::kDispelled, RE::ActiveEffect::Flag::kInactive))
				continue;

			if (!a_target->IsPlayerRef()) {
				const auto caster = effect->caster.get();
				if (caster && caster->IsPlayerRef() && !IsHunter(caster.get())) {
					total = 0.0f;
					break;
				}
			}
			if (const float change = GetExpectedHealthModification(effect); change != 0) {
				total += change / 20;	 // Only consider damage the spell would do within the next 50ms
			}
		}
		if (total < 0 && a_target->GetActorValue(RE::ActorValue::kHealth) <= fabs(total)) {
			auto aggressor = GetNearValidAggressor(a_target);
			if (!aggressor || !Validation::ValidatePair(a_target, aggressor))
				return;
			if (!ShouldDefeat(a_target, aggressor, true))
				return;
			if (!Processing::RegisterDefeat(a_target, aggressor))
				return;
			RemoveDamagingSpells(a_target);
		}
	}

	void Hooks::WeaponHit(RE::Actor* a_target, RE::HitData& a_hitData)
	{
		const auto aggressor = a_hitData.aggressor.get();
		if (a_target && aggressor && aggressor.get() != a_target && !a_target->IsCommandedActor() && IsNPC(a_target)) {
			if (Defeat::IsDamageImmune(a_target))
				return;
			if (Validation::CanProcessDamage() && Validation::ValidatePair(a_target, aggressor.get())) {
				const float hp = a_target->GetActorValue(RE::ActorValue::kHealth);
				auto dmg = a_hitData.totalDamage + fabs(GetIncomingEffectDamage(a_target));
				AdjustByDifficultyMult(dmg, aggressor->IsPlayerRef());
				if (ShouldDefeat(a_target, aggressor.get(), hp <= dmg)) {
					if (Processing::RegisterDefeat(a_target, aggressor.get())) {
						RemoveDamagingSpells(a_target);
						return;
					}
				} else if ((a_hitData.flags.underlying() & ((1 << 0) + (1 << 1))) == 0) {	 // blocked, blocked with weapon
					ValidateStrip(a_target);
				}
			}
		}
		return _WeaponHit(a_target, a_hitData);
	}

	void Hooks::MagicHit(uint64_t* unk1, RE::ActiveEffect& effect, uint64_t* unk3, uint64_t* unk4, uint64_t* unk5)
	{
		const auto target = effect.GetTargetActor();
		const auto& base = effect.effect ? effect.effect->baseEffect : nullptr;
		if (!target || !base || target->IsCommandedActor() || !IsNPC(target))
			return _MagicHit(unk1, effect, unk3, unk4, unk5);

		enum
		{
			damaging,
			healing,
			none
		};
		switch ([&]() {
			if (effect.magnitude != 0 && (base->data.primaryAV == RE::ActorValue::kHealth || base->data.secondaryAV == RE::ActorValue::kHealth))
				return effect.magnitude < 0 ? damaging : healing;
			return none;
		}()) {
		case healing:
			if (Defeat::IsDefeated(target))
				Defeat::RescueActor(target, true);
			break;
		case damaging:
			if (Defeat::IsDamageImmune(target))
				return;
			if (Validation::CanProcessDamage()) {
				const auto caster = effect.caster.get();
				if (caster && caster.get() != target && Validation::ValidatePair(target, caster.get())) {
					const float health = target->GetActorValue(RE::ActorValue::kHealth);
					float dmg = base->data.secondaryAV == RE::ActorValue::kHealth ? effect.magnitude * base->data.secondAVWeight : effect.magnitude;
					dmg += GetIncomingEffectDamage(target);	 // + GetTaperDamage(effect.magnitude, data->data);
					AdjustByDifficultyMult(dmg, caster->IsPlayerRef());
					if (ShouldDefeat(target, caster.get(), health <= fabs(dmg) + 2)) {
						if (Processing::RegisterDefeat(target, caster.get())) {
							RemoveDamagingSpells(target);
							return;
						}
					} else if (effect.spell->GetSpellType() != RE::MagicSystem::SpellType::kEnchantment) {
						ValidateStrip(target);
					}
				}
			}
			break;
		}
		return _MagicHit(unk1, effect, unk3, unk4, unk5);
	}

	bool Hooks::DoesMagicHitApply(RE::MagicTarget* a_target, RE::MagicTarget::AddTargetData* a_data)
	{
		const auto target = a_target->MagicTargetIsActor() ? static_cast<RE::Actor*>(a_target) : nullptr;
		if (target && !target->IsDead() && Defeat::IsDamageImmune(target)) {
			auto spell = a_data ? a_data->magicItem : nullptr;
			if (!spell)
				return _DoesMagicHitApply(a_target, a_data);

			for (auto& e : spell->effects) {
				auto base = e ? e->baseEffect : nullptr;
				if (base && base->data.flags.underlying() & 4) {	// detremental
					return false;
				}
			}
		}
		return _DoesMagicHitApply(a_target, a_data);
	}

	// return false if hit should not be processed
	bool Hooks::ExplosionHit(RE::Explosion& a_explosion, float* a_flt, RE::Actor* a_actor)
	{
		if (a_actor && Defeat::IsDamageImmune(a_actor))
			return false;

		return _ExplosionHit(a_explosion, a_flt, a_actor);
	}

	uint8_t* Hooks::DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04, uint8_t& unk05, uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09, float& unk10)
	{
		if (viewer && Defeat::IsPacified(viewer) || target && Defeat::IsPacified(target)) {
			detectval = -1000;
			return nullptr;
		}
		return _DoDetect(viewer, target, detectval, unk04, unk05, unk06, pos, unk08, unk09, unk10);
	}

	bool Hooks::ShouldDefeat(RE::Actor* a_victim, RE::Actor* a_aggressor, const bool lethal)
	{
		if ([a_aggressor]() -> bool {	// player related attack?
					const auto helper = [](RE::Actor* a_actor) {
						return a_actor->IsPlayerRef() || Settings::bHunterPrideFollower && a_actor->IsPlayerTeammate();
					};
					if (!a_aggressor->IsCommandedActor()) {
						return helper(a_aggressor);
					}
					if (auto tmp = a_aggressor->GetCommandingActor(); tmp) {
						return helper(tmp.get());
					}
					return false;
				}()) {
			auto player = RE::PlayerCharacter::GetSingleton();
			if (!IsHunter(player) || !lethal)
				return false;
		}

		if (lethal) {
			// lethal
			using Flag = RE::Actor::BOOL_FLAGS;
			bool protecc;
			if (Settings::bLethalEssential && (a_victim->boolFlags.all(Flag::kEssential) || !a_aggressor->IsPlayerRef() && a_victim->boolFlags.all(Flag::kProtected)))
				protecc = true;
			else if (a_victim->IsPlayerRef())
				protecc = Random::draw<float>(0, 99.5f) < Settings::fLethalPlayer;
			else
				protecc = Random::draw<float>(0, 99.5f) < Settings::fLethalNPC;
			return protecc;
		} else {
			// Exposed
			if (Settings::iExposed > 0 && Random::draw<float>(0, 99.5) < Settings::fExposedChance) {
				const auto gear = GetWornArmor(a_victim, Settings::iStrips);
				uint32_t occupied = 0;
				for (auto& e : gear) {
					auto kwd = e->As<RE::BGSKeywordForm>();
					if (kwd && kwd->ContainsKeywordString("NoStrip"))
						continue;
					occupied += static_cast<decltype(occupied)>(e->GetSlotMask());
				}
				constexpr auto ign{ (1U << 1) + (1U << 5) + (1U << 6) + (1U << 9) + (1U << 11) + (1U << 12) + (1U << 13) + (1U << 15) + (1U << 20) + (1U << 21) + (1U << 31) };
				auto t = std::popcount(occupied & (~ign));
				if (t < Settings::iExposed)
					return true;
			}
			// TODO: Trauma
		}
		return false;
	}

	float Hooks::GetIncomingEffectDamage(RE::Actor* subject)
	{
		const auto effects = subject->GetActiveEffectList();
		if (!effects)
			return 0.0f;

		float ret = 0.0f;
		for (const auto& effect : *effects) {
			if (!effect || effect->flags.any(RE::ActiveEffect::Flag::kDispelled, RE::ActiveEffect::Flag::kInactive))
				continue;
			else if (const float change = GetExpectedHealthModification(effect); change != 0.0f) {
				ret += change;
			}
		}
		// if ret > 0, subject is getting healed
		return ret < 0 ? ret : 0;
	}

	float Hooks::GetExpectedHealthModification(RE::ActiveEffect* a_effect)
	{
		const auto base = a_effect->GetBaseObject();
		if (!base || base->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kRecover))
			return 0.0f;
		// Damage done every second by the effect
		const auto getmagnitude = [&]() -> float {
			if (a_effect->duration - base->data.taperDuration < a_effect->elapsedSeconds)
				return GetTaperDamage(a_effect->magnitude, base->data);
			else
				return a_effect->magnitude;
		};
		if (base->data.primaryAV == RE::ActorValue::kHealth)
			return getmagnitude();
		else if (base->data.secondaryAV == RE::ActorValue::kHealth)	 // only DualValueModifier can have an ActorValue as 2nd Item
			return getmagnitude() * base->data.secondAVWeight;
		return 0.0f;
	}

	float Hooks::GetTaperDamage(const float magnitude, const RE::EffectSetting::EffectSettingData& data)
	{
		return magnitude * data.taperWeight * data.taperDuration / (data.taperCurve + 1);
	}

	void Hooks::RemoveDamagingSpells(RE::Actor* subject)
	{
		auto effects = subject->GetActiveEffectList();
		if (!effects)
			return;

		for (auto& eff : *effects) {
			if (!eff || eff->flags.all(RE::ActiveEffect::Flag::kDispelled))
				continue;
			auto base = eff->GetBaseObject();
			if (!base)
				continue;

			const auto damaging = [](const RE::EffectSetting::EffectSettingData& data) {
				if (data.primaryAV == RE::ActorValue::kHealth || data.secondaryAV == RE::ActorValue::kHealth)
					return (data.flags.underlying() & 6) == 4;
				return false;
			};

			if (damaging(base->data)) {
				logger::info("Dispelling Spell = {}", base->GetFormID());
				eff->Dispel(true);
			} else if (base->data.archetype == RE::EffectSetting::Archetype::kCloak) {
				const auto associate = base->data.associatedForm;
				if (associate == nullptr)
					continue;
				const auto magicitem = associate->As<RE::MagicItem>();
				for (auto& e : magicitem->effects) {
					if (e && e->baseEffect && damaging(e->baseEffect->data)) {
						eff->Dispel(true);
						break;
					}
				}
			}
		}
	}

	void Hooks::AdjustByDifficultyMult(float& damage, const bool playerPOV)
	{
		const auto s = RE::GetINISetting("iDifficulty:GamePlay");
		if (s->GetType() != RE::Setting::Type::kSignedInteger)
			return;

		std::string diff{ "fDiffMultHP"s + (playerPOV ? "ByPC"s : "ToPC"s) };
		switch (s->GetSInt()) {
		case 0:
			diff.append("VE");
			break;
		case 1:
			diff.append("E");
			break;
		case 2:
			diff.append("N");
			break;
		case 3:
			diff.append("H");
			break;
		case 4:
			diff.append("VH");
			break;
		case 5:
			diff.append("L");
			break;
		default:
			logger::error("Invalid Difficulty Setting");
			return;
		}
		const auto smult = RE::GameSettingCollection::GetSingleton()->GetSetting(diff.c_str());
		if (smult->GetType() != RE::Setting::Type::kFloat)
			return;

		const auto mult = smult->GetFloat();
		damage *= mult;
	}

	RE::Actor* Hooks::GetNearValidAggressor(RE::Actor* a_victim)
	{
		const auto valid = [&](RE::Actor* subject) {
			return subject->IsHostileToActor(a_victim) && subject->IsInCombat();
		};
		std::vector<RE::Actor*> valids{};
		if (const auto player = RE::PlayerCharacter::GetSingleton(); valid(player) && IsHunter(player))
			valids.push_back(player);

		const auto& processLists = RE::ProcessLists::GetSingleton();
		for (auto& pHandle : processLists->highActorHandles) {
			auto potential = pHandle.get().get();
			if (!potential || potential->IsDead() || Defeat::IsDamageImmune(potential) || !valid(potential))
				continue;

			if (const auto group = potential->GetCombatGroup(); group) {
				for (auto& e : group->targets) {
					if (e.targetHandle.get().get() == a_victim) {
						valids.push_back(potential);
						break;
					}
				}
			}
		}
		if (valids.empty()) {
			return nullptr;
		}
		const auto targetposition = a_victim->GetPosition();
		auto distance = valids[0]->GetPosition().GetDistance(targetposition);
		size_t where = 0;
		for (size_t i = 1; i < valids.size(); i++) {
			const auto d = valids[i]->GetPosition().GetDistance(targetposition);
			if (d < distance) {
				distance = d;
				where = i;
			}
		}
		return distance < 4096.0f ? valids[where] : nullptr;
	}

	void Hooks::ValidateStrip(RE::Actor* a_victim)
	{
		if (Random::draw<float>(0, 99.5f) >= Settings::fStripChance)
			return;

		static std::vector<RE::FormID> cache{};
		if (std::find(cache.begin(), cache.end(), a_victim->formID) != cache.end())
			return;

		const auto id = a_victim->formID;
		cache.push_back(id);
		std::thread([id]() {
			std::this_thread::sleep_for(3s);
			SKSE::GetTaskInterface()->AddTask([id]() {
				cache.erase(std::find(cache.begin(), cache.end(), id));
			});
		}).detach();

		const auto gear = GetWornArmor(a_victim, Settings::iStrips);
		if (gear.empty())
			return;
		const auto item = gear.at(Random::draw<size_t>(0, gear.size() - 1));
		if (auto k = item->As<RE::BGSKeywordForm>(); k && k->ContainsKeywordString("NoStrip"))
			return;

		RE::ActorEquipManager::GetSingleton()->UnequipObject(a_victim, item);
		if (Random::draw<float>(0, 99.5f) < Settings::fStripDestroy && !IsDaedric(item)) {
			if (a_victim->IsPlayerRef() && Settings::bNotifyDestroy) {
				auto base = fmt::format("{} got teared off and destroyed", item->GetName());
				if (Settings::bNotifyColored) {
					base = fmt::format("<font color = '{}'>{}</font color>", Settings::rNotifyColor, base);
				}
				RE::DebugNotification(base.c_str());
			}
			a_victim->RemoveItem(item, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
		} else if (Settings::bStripDrop) {
			a_victim->RemoveItem(item, 1, RE::ITEM_REMOVE_REASON::kDropping, nullptr, nullptr);
		} else if (!a_victim->IsPlayerRef()) {
			// store armor for re-equipping on combat end, since NPC normally dont do it on their own..
			// IDEA: would use the default equip func if I knew where its hidin
			auto& v = EventHandler::GetSingleton()->worn_cache;
			if (auto where = v.find(a_victim->GetFormID()); where != v.end())
				where->second.push_back(item);
			else
				v.insert(std::make_pair(a_victim->GetFormID(), std::vector<RE::TESObjectARMO*>{ item }));
		}
	}

}	 // namespace Hooks
