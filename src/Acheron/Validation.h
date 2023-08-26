#pragma once

namespace Acheron
{
	enum VTarget
	{
		Victim = 0,
		Assailant = 1,
		Either = 2,

		Total
	};

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

		static bool CheckVictimID(RE::FormID a_formid);												 // Conditional exclusion of some unique actor ids
		static bool CheckExclusion(VTarget a_validation, RE::Actor* a_actor);	 // Check actor for exclusion in arrays

		static inline std::vector<RE::FormID> exclLocAll{};									// Always disabled locations
		static inline std::vector<RE::FormID> exclLocTp{};									// Teleport only disabled locations
		static inline std::vector<RE::FormID> exclNPC[VTarget::Total];			// Excluded Actor Bases
		static inline std::vector<RE::FormID> exclRef[VTarget::Total];			// Excluded object refs
		static inline std::vector<RE::FormID> exclRace[VTarget::Total];			// Excluded races
		static inline std::vector<RE::FormID> exclFaction[VTarget::Total];	// Excluded factions
	};

}	 // namespace Acheron