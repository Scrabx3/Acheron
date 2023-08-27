#pragma once

namespace Acheron
{
	class Defeat
	{
	public:
		enum VictimType
		{
			Player,
			Follower,
			NPC
		};

		struct VictimData
		{
			VictimData(float a_registertime) :
				registered_at(a_registertime) {}
			~VictimData() = default;

			float registered_at;					// GameDaysPassed at construction
			bool allow_recovery{ true };	// if this actor may passively recover
		};

	public:
		static std::vector<RE::Actor*> GetAllPacified(bool a_loadedonly);
		static std::vector<RE::Actor*> GetAllDefeated(bool a_loadedonly);
		static void DisableRecovery(bool a_loadedonly);

		static void DefeatActor(RE::Actor* a_victim);
		static void RescueActor(RE::Actor* a_victim, bool undo_pacify);
		static void RescueDelayed(RE::Actor* a_victim, bool undo_pacify);
		static bool IsDefeated(const RE::Actor* a_victim);
		static bool IsDamageImmune(RE::Actor* a_victim);

		static void Pacify(RE::Actor* a_victim);
		static void PacifyUnsafe(RE::Actor* a_victim);
		static void UndoPacify(RE::Actor* a_victim);
		static bool IsPacified(const RE::Actor* a_victim);

		static void Load(SKSE::SerializationInterface* a_intfc, uint32_t a_type);
		static void Save(SKSE::SerializationInterface* a_intfc, uint32_t a_type);
		static void Revert(SKSE::SerializationInterface* a_intfc);

	public:
		static inline std::unordered_map<RE::FormID, VictimData> Victims;
		static inline std::set<RE::FormID> Pacified;
	};

}	 // namespace Defeat
