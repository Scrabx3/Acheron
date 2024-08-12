#pragma once

namespace Acheron
{
	class Processing
	{
public:
		struct AggressorInfo
		{
			AggressorInfo(RE::Actor* a_actor, RE::Actor* a_victim);
			~AggressorInfo() = default;

			RE::Actor* actor;
			bool legal;

			constexpr RE::Actor* operator->() const { return actor; }
			constexpr operator RE::Actor*() const { return actor; }
			constexpr operator bool() const { return actor != nullptr; }
		};

		enum class DefeatResult
		{
			Cancel,
			Resolution,
			Defeat,
			Assault,
			Teleport
		};

		/// @brief Have the given victim be defeated by the given aggressor
		/// @param victim The victim to defeat
		/// @param aggressor The aggressor which did the final attack
		/// @return if the victim was successfully defeated
		static bool RegisterDefeat(RE::Actor* victim, const AggressorInfo& aggressor);

	private:
		static RE::Actor* GetNearValidAggressor(RE::Actor* a_victim);
		static DefeatResult GetDefeatType(RE::Actor* a_aggressor);
		static bool CreateResolution(RE::Actor* a_victim, const AggressorInfo& a_victoire, bool a_incombat);
	};	// class Zone

}