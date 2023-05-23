#pragma once
namespace Acheron
{
	/// @brief Lookup a form object by string
	/// @tparam T The type of the object to lookup
	/// @param a_string a string representing the object, given as "FormID|plugin"
	/// @return if T == FormID, a FormID, otherwise T*. 0 if the object doesnt exist
	template <class T>
	static T FormFromString(const std::string_view& a_string)
	{
		const auto base = a_string.starts_with("0x") ? 16 : 10;
		const auto split = a_string.find("|");
		const auto esp = a_string.substr(split + 1);
		const auto formid = std::stoi(a_string.substr(0, split).data(), nullptr, base);
		if constexpr (std::is_same<T, RE::FormID>::value) {
			return RE::TESDataHandler::GetSingleton()->LookupFormID(formid, esp);
		} else if constexpr (std::is_pointer<T>::value) {
			using t = std::remove_pointer<T>::type;
			return RE::TESDataHandler::GetSingleton()->LookupForm<t>(formid, esp);
		} else {
			return RE::TESDataHandler::GetSingleton()->LookupForm<T>(formid, esp);
		}
	}

	// Debug
	inline void PrintConsole(const std::string& a_msg) { RE::ConsoleLog::GetSingleton()->Print(a_msg.c_str()); }
	inline void PrintConsole(const char* a_msg) { RE::ConsoleLog::GetSingleton()->Print(a_msg); }

	// Actor
	bool HasBeneficialPotion(RE::TESObjectREFR* a_container);
	float GetAVPercent(RE::Actor* a_actor, RE::ActorValue a_av);
	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::Actor* a_actor, uint32_t a_ignoredmasks = 0);
	RE::TESActorBase* GetLeveledActorBase(RE::Actor* a_actor);
	std::vector<RE::Actor*> GetFollowers();

	bool IsHunter(RE::Actor* a_actor);
	bool UsesHunterPride(const RE::Actor* a_actor);
	bool IsNPC(const RE::Actor* a_actor);
	bool IsDaedric(const RE::TESForm* a_form);

	// ObjectReference
	template <class T>
	void SortByDistance(std::vector<T>& array, const RE::TESObjectREFR* center)
	{
		const auto p = center->GetPosition();
		std::sort(array.begin(), array.end(), [&](T& first, T& second) {
			return first->GetPosition().GetDistance(p) < second->GetPosition().GetDistance(p);
		});
	}

	// String
	template <class T>
	constexpr void ToLower(T& str)
	{
		std::transform(str.cbegin(), str.cend(), str.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
	}

}  // namespace Acheron
