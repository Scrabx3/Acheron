#pragma once

#include <shared_mutex>

namespace Acheron
{
	class Defeat
	{
	public:
		enum class VictimVistor
		{
			Continue,
			Break
		};

		struct VictimData
		{
			VictimData(float a_registertime) :
				registered_at(a_registertime) {}
			~VictimData() = default;

			float registered_at;							// GameDaysPassed at construction
			bool allow_recovery{ true };			// if this actor may passively recover
			bool mark_for_recovery{ false };	// Rescue this actor the next time they load in?
		};

	public:
		static std::vector<RE::Actor*> GetAllPacified(bool a_loadedonly);
		static std::vector<RE::Actor*> GetAllDefeated(bool a_loadedonly);

		static void ForEachVictim(std::function<VictimVistor(RE::FormID a_victimid, VictimData& a_data)> a_visitor);
		static std::optional<Defeat::VictimData> GetVictimData(RE::FormID a_formid);
		static void DisableRecovery(bool a_loadedonly);

		static void DefeatActor(RE::Actor* a_victim);
		static void RescueActor(RE::Actor* a_victim, bool undo_pacify);
		static void RescueDelayed(RE::Actor* a_victim, bool undo_pacify);
		static bool IsDefeated(const RE::Actor* a_victim);
		static bool IsDamageImmune(RE::Actor* a_victim);

		static void Pacify(RE::Actor* a_victim);
		static void UndoPacify(RE::Actor* a_victim);
		static bool IsPacified(const RE::Actor* a_victim);

	public:
		static void Load(SKSE::SerializationInterface* a_intfc, uint32_t a_type);
		static void Save(SKSE::SerializationInterface* a_intfc, uint32_t a_type);
		static void Revert(SKSE::SerializationInterface* a_intfc);
		static void Delete(RE::FormID a_formid);

	private:
		static void PacifyUnsafe(RE::Actor* a_victim);

	private:
		static inline std::map<RE::FormID, VictimData> Victims;
		static inline std::set<RE::FormID> Pacified;
	};

}	 // namespace Defeat
