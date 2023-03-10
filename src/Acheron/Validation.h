#pragma once

namespace Acheron
{
	class Validation
	{
	public:
		static void Initialize();

		/// @brief Check that the mod is considered enabled in current location
		/// @return if attacks may be processed
		_NODISCARD static bool CanProcessDamage();

		/// @brief Validate both actors and tell whether or not victim may be defeated
		/// @param a_victim The actor being attacked
		/// @param a_aggressor The actor attacking
		/// @return if victim may be defeated by aggressor
		_NODISCARD static bool ValidatePair(RE::Actor* a_victim, RE::Actor* a_aggressor);

		/// @brief Ensure that the player can be teleported away from their current location
		/// @return if teleporting the player is permitted
		_NODISCARD static bool AllowTeleport();

	private:
		static bool ValidateActor(RE::Actor* a_actor);

		static inline std::vector<RE::FormID> exLocation{};
		static inline std::vector<RE::FormID> exTeleport{};
		static inline std::vector<RE::FormID> exNPC{};
		static inline std::vector<RE::FormID> exReference{};
		static inline std::vector<RE::FormID> exRace{};
		static inline std::vector<RE::TESFaction*> exFaction{};
	};

}	 // namespace Acheron