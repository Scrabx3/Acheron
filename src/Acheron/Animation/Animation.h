#pragma once

namespace Acheron::Animation
{
	enum class AssaultType
	{
		LeadIn = 1,
		Instant = 2,
		BreakFree = 3,

		Total
	};

	void PlayAnimation(RE::TESObjectREFR* a_reference, const char* a_animation);								 // Wrapper for sending thread save animation events cuz Im lazy
	std::string GetRaceType(RE::Actor* a_actor);																								 // RaceType definition to get struggle motion
	void SetPositions(const std::vector<RE::Actor*>& a_positions, RE::TESObjectREFR* a_center);	 // Function to quickly center a given array around a given center

	/// @brief Get assault animations for the given actors
	/// @param a_victim The actor which is being attacked. MUST be a npc
	/// @param a_partner The actor attacking the victim, can be any race. Though some races arent supported
	/// @param a_type The class type of the animation
	/// @return An array of animations to play (empty array if no animations are found)
	std::vector<std::string> LookupAssaultAnimations(RE::Actor* a_victim, RE::Actor* a_partner, AssaultType a_type);

}
