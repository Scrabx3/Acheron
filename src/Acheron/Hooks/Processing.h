#pragma once

namespace Acheron
{
	class Processing
	{
public:
		enum class DefeatResult
		{
			Cancel,
			Resolution,
			Defeat,
			Assault
		};

		/// @brief Have the given victim be defeated by the given aggressor
		/// @param victim The victim to defeat
		/// @param aggressor The aggressor which did the final attack
		/// @return if the victim was successfully defeated
		static bool RegisterDefeat(RE::Actor* victim, RE::Actor* aggressor);

private:
		static DefeatResult GetDefeatType(RE::Actor* aggressor);
		static bool CreateResolution(RE::Actor* a_victim, RE::Actor* a_victoire, bool a_incombat);
	};	// class Zone

}