#pragma once

namespace Acheron::Animation
{
	void PlayAnimation(RE::TESObjectREFR* a_reference, const char* a_animation);								 // Wrapper for sending thread save animation events cuz Im lazy
	std::string GetRaceType(RE::Actor* a_actor);																								 // RaceType definition to get struggle motion
}
