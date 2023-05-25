#pragma once

namespace Acheron
{
	class Hooks
	{
	public:
		static void Install();

	private:
		// Hooks
		static void CompileAndRun(RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef);
		static void UpdatePlayer(RE::PlayerCharacter* player, float delta);
		static RE::NiAVObject* Load3D(RE::Character& a_this, bool a_arg1);
		static void UpdateCombat(RE::Character* a_this);
		static void WeaponHit(RE::Actor* a_target, RE::HitData& a_hitData);
		static void MagicHit(uint64_t* unk1, RE::ActiveEffect& effect, uint64_t* unk3, uint64_t* unk4, uint64_t* unk5);
		static bool DoesMagicHitApply(RE::MagicTarget* a_target, RE::MagicTarget::AddTargetData* a_data);
		static bool ExplosionHit(RE::Explosion& explosion, float* flt, RE::Actor* actor);
		static uint8_t* DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04, uint8_t& unk05, uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09, float& unk10);

		// Hit Processing
		static bool ShouldDefeat(RE::Actor* a_victim, RE::Actor* a_aggressor, bool lethal);
		static float GetTaperDamage(const float magnitude, const RE::EffectSetting::EffectSettingData& data);
		static float GetExpectedHealthModification(RE::ActiveEffect* a_effect);
		static float GetIncomingEffectDamage(RE::Actor* subject);
		static void RemoveDamagingSpells(RE::Actor* subject);
		static void CalcDamageOverTime(RE::Actor* a_target);
		static void AdjustByDifficultyMult(float& damage, const bool playerPOV);
		static RE::Actor* GetNearValidAggressor(RE::Actor* a_victim);
		static void ValidateStrip(RE::Actor* target);

		static inline REL::Relocation<decltype(CompileAndRun)> _CompileAndRun;
		static inline REL::Relocation<decltype(UpdatePlayer)> _PlUpdate;
		static inline REL::Relocation<decltype(Load3D)> _Load3D;
		static inline REL::Relocation<decltype(UpdateCombat)> _UpdateCombat;
		static inline REL::Relocation<decltype(WeaponHit)> _WeaponHit;
		static inline REL::Relocation<decltype(MagicHit)> _MagicHit;
		static inline REL::Relocation<decltype(DoesMagicHitApply)> _DoesMagicHitApply;
		static inline REL::Relocation<decltype(ExplosionHit)> _ExplosionHit;
		static inline REL::Relocation<decltype(DoDetect)> _DoDetect;
	};

}	 // namespace Hooks
