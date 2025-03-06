#include "Acheron/Hooks/Hooks.h"

#include "Acheron/Defeat.h"
#include "Acheron/EventSink.h"
#include "Acheron/Hooks/Console.h"
#include "Acheron/Hooks/Processing.h"
#include "Acheron/Validation.h"

#include <xbyak/xbyak.h>

using Archetype = RE::EffectArchetypes::ArchetypeID;

namespace Acheron
{
	void Hooks::Install()
	{
		REL::Relocation<std::uintptr_t> phd{ RELID(37633, 38586), OFFSET(0x76, 0x7e) };
		// MOV R13,RDX	; HitData* a_hitData
		// MOV R15,RCX	; Actor* a_target
		// ...
		// CMP qword ptr [RCX + 0xf8],0x0  ; if (a_target->currentProcess == 0)
		// JZ LAB_1406b8d42                ; 	return;
		struct ProcessHitData_Patch : Xbyak::CodeGenerator {
			ProcessHitData_Patch(size_t a_retAddr) {
				Xbyak::Label retLbl;
				Xbyak::Label retPtr;
				Xbyak::Label callPtr;

				cmp(qword[rcx + OFFSET(0xf0, 0xf8)], 0);
				jz(retLbl);

				call(ptr[rip + callPtr]);
				test(al, al);

				mov(rcx, OFFSET(r14, r15));
				mov(rdx, OFFSET(r15, r13));
				L(retLbl);
				jmp(ptr[rip + retPtr]);

				L(callPtr);
				dq(std::uintptr_t(&Hooks::ProcessHitData));

				L(retPtr);
				dq(a_retAddr);
			}
		};
		ProcessHitData_Patch _ProcessHitData(phd.address() + 8);
		_ProcessHitData.ready();

		SKSE::AllocTrampoline((static_cast<size_t>(1) << 7) + _ProcessHitData.getSize());
		auto& trampoline = SKSE::GetTrampoline();
		// ==================================================
		auto patchDst = trampoline.allocate(_ProcessHitData);
		trampoline.write_branch<5>(phd.address(), patchDst);
		REL::safe_fill(phd.address() + 5, 0x90, 3);
		// ==================================================
		REL::Relocation<std::uintptr_t> magichit{ RELID(33763, 34547), OFFSET(0x52F, 0x7B1) };
		_MagicHit = trampoline.write_call<5>(magichit.address(), MagicHit);
		// ==================================================
		REL::Relocation<std::uintptr_t> mha{ RELID(33742, 34526), OFFSET(0x1E8, 0x20B) };
		_DoesMagicHitApply = trampoline.write_call<5>(mha.address(), DoesMagicHitApply);
		// ==================================================
		REL::Relocation<std::uintptr_t> det{ RELID(41659, 42742), OFFSET(0x526, 0x67B) };
		_DoDetect = trampoline.write_call<5>(det.address(), DoDetect);
		// ==================================================
		// REL::Relocation<std::uintptr_t> console{ RELID(52065, 52952), OFFSET(0xE2, 0x52) };
		// _CompileAndRun = trampoline.write_call<5>(console.address(), CompileAndRun);
		// ==================================================
		REL::Relocation<std::uintptr_t> ragdoll_dmg{ RELOCATION_ID(36346, 37336), 0x35 };
		_FallAndPhysicsDamage = trampoline.write_call<5>(ragdoll_dmg.address(), FallAndPhysicsDamage<false>);
		// ==================================================
		REL::Relocation<std::uintptr_t> movefinish{ RELOCATION_ID(36973, 37998), OFFSET(0xAE, 0xAB) };
		_FallAndPhysicsDamage = trampoline.write_call<5>(movefinish.address(), FallAndPhysicsDamage<true>);
		// ==================================================
		REL::Relocation<std::uintptr_t> plu{ RE::PlayerCharacter::VTABLE[0] };
		_PlUpdate = plu.write_vfunc(0xAD, UpdatePlayer);
		// ==================================================
		REL::Relocation<std::uintptr_t> char_vt{ RE::Character::VTABLE[0] };
		_Load3D = char_vt.write_vfunc(0x6A, Load3D);
		_UpdateCombat = char_vt.write_vfunc(0xE4, UpdateCombat);
		_UpdateCharacter = char_vt.write_vfunc(0xAD, UpdateCharacter);

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

	void Hooks::UpdatePlayer(RE::PlayerCharacter* a_player, float a_delta)
	{
		_PlUpdate(a_player, a_delta);

		static bool __combat = a_player->playerFlags.isInCombat;
		if (__combat != a_player->playerFlags.isInCombat) {
			__combat = a_player->playerFlags.isInCombat;
			if (!__combat && Settings::iFollowerRescue == 1 && !Defeat::IsDamageImmune(a_player)) {
				const auto defeated = Defeat::GetAllDefeated(false);
				for (auto&& actor : defeated) {
					if (!actor->IsPlayerTeammate())
						continue;
					Defeat::RescueActor(actor, true);
				}
			}
		}

		const auto data = Defeat::GetVictimData(a_player->GetFormID());
		if (!data) {
			if (Validation::CanProcessDamage())
				CalcDamageOverTime(a_player);
			return;
		} else if (!data->allow_recovery) {
			return;
		}

		if (Settings::fKdHealthThresh) {
			if (GetAVPercent(a_player, RE::ActorValue::kHealth) >= Settings::fKdHealthThresh) {
				Defeat::RescueActor(a_player, true);
				return;
			}
		}

		if (Settings::iKdFallbackTimer) {
			const auto calendar = RE::Calendar::GetSingleton();
			auto days_passed = calendar->GetDaysPassed() - data->registered_at;
			auto seconds_passed = days_passed * 24 * 60 * 60;
			if (seconds_passed >= Settings::iKdFallbackTimer * calendar->GetTimescale()) {
				Defeat::RescueDelayed(a_player, true);
				return;
			}
		}
	}

	RE::NiAVObject* Hooks::Load3D(RE::Character& a_this, bool a_arg1)
	{
		const auto ret = _Load3D(a_this, a_arg1);

		if (const auto data = Defeat::GetVictimData(a_this.GetFormID())) {
			if (data->mark_for_recovery && data->allow_recovery && Settings::bNPCRescueReload) {
				// isLoading is true when loading from a prev location / is false when loading save
				if (RE::PlayerCharacter::GetSingleton()->playerFlags.isLoading) {
					Defeat::RescueActor(&a_this, true);
					return ret;
				}
			}
			if (auto process = a_this.currentProcess) {
				process->PlayIdle(&a_this, GameForms::BleedoutStart, &a_this);
			} else {
				a_this.NotifyAnimationGraph("BleedoutStart");
			}
			if (auto ref = a_this.GetObjectReference()) {
				ref->As<RE::BGSKeywordForm>()->AddKeywords({ GameForms::Defeated, GameForms::Pacified });
			}
		} else if (Defeat::IsPacified(&a_this)) {
			if (auto ref = a_this.GetObjectReference()) {
				ref->As<RE::BGSKeywordForm>()->AddKeyword(GameForms::Pacified);
			}
		}
		return ret;
	}

	void Hooks::UpdateCharacter(RE::Character* a_this, float a_delta)
	{
		_UpdateCharacter(a_this, a_delta);
		if (!Validation::CanProcessDamage() || a_this->IsCommandedActor() || !IsNPC(a_this))
			return;
		if (Defeat::IsDamageImmune(a_this))
			return;

		CalcDamageOverTime(a_this);
	}

	void Hooks::UpdateCombat(RE::Character* a_this)
	{
		if (Defeat::IsPacified(a_this)) {
			if (a_this->IsInCombat())
				a_this->StopCombat();
			return;
		}
		_UpdateCombat(a_this);
	}

	void Hooks::CalcDamageOverTime(RE::Actor* a_target)
	{
		const auto effects = a_target->GetActiveEffectList();
		if (!effects)
			return;

		float inc_damage = 0.0f;
		for (const auto& effect : *effects) {
			if (!effect || effect->flags.any(RE::ActiveEffect::Flag::kDispelled, RE::ActiveEffect::Flag::kInactive))
				continue;

			if (!a_target->IsPlayerRef()) {
				const auto caster = effect->caster.get();
				const auto player = RE::PlayerCharacter::GetSingleton();
				if (caster && UsesHunterPride(caster.get()) && !IsHunter(player)) {
					inc_damage = 0.0f;
					break;
				}
			}
			if (const float change = GetExpectedHealthModification(effect); change != 0) {
				inc_damage += change / 20;	// damage value is per second
			}
		}
		if (inc_damage < 0 && a_target->GetActorValue(RE::ActorValue::kHealth) <= fabs(inc_damage)) {
			auto aggressor = Processing::AggressorInfo(nullptr, a_target);
			if (!Validation::ValidatePair(a_target, aggressor.actor))
				return;
			if (GetProcessType(aggressor.actor, true) != ProcessType::Lethal)
				return;
			if (!HandleLethal(a_target, aggressor.actor))
				return;
			Processing::RegisterDefeat(a_target, aggressor);
		}
	}

	Hooks::HitResult Hooks::ProcessHitData(RE::Actor* a_target, RE::HitData& a_hitData)
	{
		// If hooked function will queue itself, process the queued execution instead
		if (RE::TaskQueueInterface::ShouldUseTaskQueue())
			return HitResult::Allow;

		if (!a_target || a_target->IsCommandedActor() || !IsNPC(a_target))
			return HitResult::Allow;
		if (Defeat::IsDamageImmune(a_target))
			return HitResult::Prevent;

		if (!Validation::CanProcessDamage())
			return HitResult::Allow;

		const auto aggressorPtr = a_hitData.aggressor.get();
		const auto aggressor = aggressorPtr ? aggressorPtr.get() : nullptr;
		if (!Validation::ValidatePair(a_target, aggressor))
			return HitResult::Allow;

		const float hp = a_target->GetActorValue(RE::ActorValue::kHealth);
		auto dmg = a_hitData.totalDamage + fabs(GetIncomingEffectDamage(a_target));
		AdjustByDifficultyMult(dmg, a_target->IsPlayerRef());
		bool negate;
		switch (GetProcessType(aggressor, hp <= dmg)) {
		case ProcessType::Lethal:
			negate = HandleLethal(a_target, aggressor);
			break;
		case ProcessType::Any:
			if (!aggressor || a_hitData.totalDamage < 0.0001) {
				negate = false;
				break;
			}
			negate = [&]() {
				if (a_hitData.stagger == 0 || !Settings::bTraumaEnabled)
					return false;
				// f(x,y) = (x^2 * (1 + d * (1 - y))) / 64; with x in (-inf, inf), y in [0; 1], d in [0; inf)
				const auto y = Settings::bTraumaHealth ? GetAVPercent(a_target, RE::ActorValue::kHealth) : 0.7;
				const auto f = (powf(a_hitData.stagger, 2) * (1 + Settings::fTraumaMult * (1 - y))) / 64;
				const auto random = Random::draw<float>(0.1f, 0.999f);
				if (random < f)
					return true;
				if (Settings::fTraumeBackAttack <= 1)
					return false;
				const auto head = a_target->GetNodeByName("NPC Head [Head]");
				if (head) {
					const auto& matrix = head->world.rotate;
					RE::NiPoint2 vecTarget{ matrix.entry[0][1], matrix.entry[0][0] };
					RE::NiPoint2 vecRelative{ aggressor->GetPositionX() - a_hitData.hitPosition.x, aggressor->GetPositionY() - a_hitData.hitPosition.y };
					vecRelative.Unitize();
					const auto res = vecTarget.Dot(vecRelative);
					if (res < -0.92f) {	 // 157°
						return random < f * Settings::fTraumeBackAttack;
					}
				}
				return false;
			}() || HandleExposed(a_target);
			break;
		default:
			negate = false;
		}
		if (negate && Processing::RegisterDefeat(a_target, { aggressor, a_target })) {
			return HitResult::Prevent;
		} else if (a_hitData.flags.none(RE::HitData::Flag::kBlocked, RE::HitData::Flag::kBlockWithWeapon)) {
			ValidateStrip(a_target);
		}
		return HitResult::Allow;
	}

	void Hooks::MagicHit(uint64_t* unk1, RE::ActiveEffect& effect, uint64_t* unk3, uint64_t* unk4, uint64_t* unk5)
	{
		const auto target = effect.GetTargetActor();
		const auto& base = effect.effect ? effect.effect->baseEffect : nullptr;
		if (!target || !base || target->IsCommandedActor() || !IsNPC(target))
			return _MagicHit(unk1, effect, unk3, unk4, unk5);

		bool dispel = false;
		enum
		{
			damaging,
			healing,
			none
		};
		switch ([&]() {
			if (base->HasArchetype(RE::EffectArchetype::kPeakValueModifier))
				return none;
			if (effect.magnitude != 0 && (base->data.primaryAV == RE::ActorValue::kHealth || base->data.secondaryAV == RE::ActorValue::kHealth))
				return effect.magnitude < 0 ? damaging : healing;
			return none;
		}()) {
		case healing:
			if (Defeat::IsDefeated(target) && Validation::AllowRescueEffect(base))
				Defeat::RescueActor(target, true);
			break;
		case damaging:
			if (Defeat::IsDamageImmune(target)) {
				dispel = true;
				break;
			}
			if (Validation::CanProcessDamage()) {
				auto caster = Processing::AggressorInfo(effect.caster.get().get(), target);
				if (!Validation::ValidatePair(target, caster.actor))
					break;

				const float health = target->GetActorValue(RE::ActorValue::kHealth);
				float dmg = base->data.secondaryAV == RE::ActorValue::kHealth ? effect.magnitude * base->data.secondAVWeight : effect.magnitude;
				dmg += GetIncomingEffectDamage(target);	 // + GetTaperDamage(effect.magnitude, data->data);
				AdjustByDifficultyMult(dmg, target->IsPlayerRef());
				bool negate;
				switch (GetProcessType(caster.actor, health <= fabs(dmg) + 2)) {
				case ProcessType::Lethal:
					negate = HandleLethal(target, caster.actor);
					break;
				case ProcessType::Any:
					{
						static Cache cache{ 800ms };
						if (!cache.Debounce(target->formID)) {
							negate = HandleExposed(target);
						} else {
							negate = false;
						}
					}
					break;
				default:
					negate = false;
					break;
				}
				if (negate && Processing::RegisterDefeat(target, caster)) {
					dispel = true;
				} else if (effect.spell->GetSpellType() != RE::MagicSystem::SpellType::kEnchantment) {
					ValidateStrip(target);
				}
			}
			break;
		}
		if (dispel)
			effect.Dispel(true);
		_MagicHit(unk1, effect, unk3, unk4, unk5);
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
				if (base && base->data.flags.all(RE::EffectSetting::EffectSettingData::Flag::kDetrimental) && !Validation::AllowDetrimentalEffect(base)){
					return false;
				}
			}
		}
		return _DoesMagicHitApply(a_target, a_data);
	}

	template <bool MoveFinish>
	float Hooks::FallAndPhysicsDamage(RE::Actor* a_this, float a_fallDistance, float a_defaultMult)
	{
		float dmg = _FallAndPhysicsDamage(a_this, a_fallDistance, a_defaultMult);
		if (dmg <= 0.0f)
			return dmg;
		float adj_dmg = dmg;
		AdjustByDifficultyMult(adj_dmg, a_this->IsPlayerRef(), MoveFinish);
		const float hp = a_this->GetActorValue(RE::ActorValue::kHealth);
		if (adj_dmg < hp)
			return dmg;
		if (!Validation::CanProcessDamage() || Defeat::IsDamageImmune(a_this))
			return dmg;
		if (!IsNPC(a_this) || a_this->IsCommandedActor() || !Validation::ValidatePair(a_this, nullptr))
			return dmg;
		if (a_this->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
			return dmg;
		if (a_this->HasEffectWithArchetype(RE::MagicTarget::Archetype::kEtherealize))
			return dmg;
		auto aggressor = Processing::AggressorInfo(nullptr, a_this);
		return Processing::RegisterDefeat(a_this, aggressor) ? 0.0f : dmg;
	}

	uint8_t* Hooks::DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04, uint8_t& unk05, uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09, float& unk10)
	{
		if (target && Defeat::IsPacified(target) || viewer && Defeat::IsPacified(viewer)) {
			detectval = -1000;
			return nullptr;
		}
		return _DoDetect(viewer, target, detectval, unk04, unk05, unk06, pos, unk08, unk09, unk10);
	}

	Hooks::ProcessType Hooks::GetProcessType(RE::Actor* a_aggressor, bool a_lethal)
	{
		if (a_aggressor && UsesHunterPride(a_aggressor)) {
			auto player = RE::PlayerCharacter::GetSingleton();
			if (!a_lethal || !IsHunter(player)) {
				return ProcessType::None;
			}
		}

		return a_lethal ? ProcessType::Lethal : ProcessType::Any;
	}

	bool Hooks::HandleLethal(RE::Actor* a_victim, RE::Actor* a_aggressor)
	{
		using Flag = RE::Actor::BOOL_FLAGS;

		if (a_victim->IsPlayerRef())
			return Random::draw<float>(0, 99.5f) < Settings::fLethalPlayer;

		if (a_victim->IsPlayerTeammate())
			return Random::draw<float>(0, 99.5f) < Settings::fLethalFollower;

		if (Settings::bLethalEssential && (a_victim->boolFlags.all(Flag::kEssential) || !(a_aggressor && a_aggressor->IsPlayerRef()) && a_victim->boolFlags.all(Flag::kProtected)))
			return true;

		return Random::draw<float>(0, 99.5f) < Settings::fLethalNPC;
	}

	bool Hooks::HandleExposed(RE::Actor* a_victim)
	{
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
		if (base->HasArchetype(RE::EffectArchetype::kPeakValueModifier))
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

	void Hooks::AdjustByDifficultyMult(float& damage, const bool playerPOV, const bool onlyReduce)
	{
		const auto s = RE::GetINISetting("iDifficulty:GamePlay");
		if (s->GetType() != RE::Setting::Type::kSignedInteger)
			return;

		std::string diff{ "fDiffMultHP"s + (playerPOV ? "ToPC"s : "ByPC"s) };
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
		if (!onlyReduce || mult < 1.0)
			damage *= mult;
	}

	void Hooks::ValidateStrip(RE::Actor* a_victim)
	{
		if (Random::draw<float>(0, 99.5f) >= Settings::fStripChance)
			return;

		static Cache cache{ 3s };
		if (cache.Debounce(a_victim->formID))
			return;

		const auto gear = GetWornArmor(a_victim, Settings::iStrips);
		if (gear.empty())
			return;
		const auto item = gear.at(Random::draw<size_t>(0, gear.size() - 1));
		if (auto k = item->As<RE::BGSKeywordForm>(); k && k->ContainsKeywordString("NoStrip"))
			return;

		RE::ActorEquipManager::GetSingleton()->UnequipObject(a_victim, item);
		if (Random::draw<float>(0, 99.5f) < Settings::fStripDestroy && !IsDaedric(item)) {
			if (a_victim->IsPlayerRef() && Settings::bNotifyDestroy) {
				auto base = std::format("{} got teared off and destroyed", item->GetName());
				if (Settings::bNotifyColored) {
					base = std::format("<font color = '{}'>{}</font color>", Settings::rNotifyColor, base);
				}
				RE::DebugNotification(base.c_str());
			}
			a_victim->RemoveItem(item, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
		} else if (Settings::bStripDrop) {
			a_victim->RemoveItem(item, 1, RE::ITEM_REMOVE_REASON::kDropping, nullptr, nullptr);
		} else if (!a_victim->IsPlayerRef()) {
			// store armor for re-equipping on combat end, since NPC normally dont do it on their own..
			// IDEA: use vanilla NPC gear re-evaluation
			auto& v = EventHandler::GetSingleton()->worn_cache;
			if (auto where = v.find(a_victim->GetFormID()); where != v.end())
				where->second.push_back(item);
			else
				v.insert(std::make_pair(a_victim->GetFormID(), std::vector<RE::TESObjectARMO*>{ item }));
		}
	}

	bool Hooks::Cache::Debounce(RE::FormID a_id)
	{
		{
			std::scoped_lock lk{ _m };
			if (std::ranges::contains(cache, a_id))
				return true;
			cache.push_back(a_id);
		}
		std::thread([this, a_id]() {
			std::this_thread::sleep_for(cacheTime);
			std::scoped_lock lk{ _m };
			cache.pop_front();
		}).detach();
		return false;
	}

}	 // namespace Hooks
