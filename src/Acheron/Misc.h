#pragma once
namespace Acheron
{
	/// @brief Lookup a form object by string
	/// @tparam T Result type of the object
	/// @param a_string a string representing the object, given as "FormID|plugin"
	/// @return The object represented by the string or 0 if the string is invalid
	template <typename T>
	static T FormFromString(const std::string_view a_string, int base)
	{
		const auto split = a_string.find("|");
		const auto formid = std::stoi(a_string.substr(0, split).data(), nullptr, base);
		if (split == std::string_view::npos) {
			if constexpr (std::is_same<T, RE::FormID>::value) {
				return formid;
			} else {
				static_assert(std::is_pointer<T>::value);
				using U = std::remove_pointer<T>::type;
				return RE::TESForm::LookupByID<U>(formid);
			}
		}

		const auto esp = a_string.substr(split + 1);
		if constexpr (std::is_same<T, RE::FormID>::value) {
			return RE::TESDataHandler::GetSingleton()->LookupFormID(formid, esp);
		} else {
			static_assert(std::is_pointer<T>::value);
			using U = std::remove_pointer<T>::type;
			return RE::TESDataHandler::GetSingleton()->LookupForm<U>(formid, esp);
		}
	}

	template <typename T>
	static T FormFromString(const std::string_view a_string)
	{
		const size_t base = a_string.starts_with("0x") ? 16 : 10;
		return FormFromString<T>(a_string, base);
	}

	// Debug
	inline void PrintConsole(std::string_view a_msg) { RE::ConsoleLog::GetSingleton()->Print(a_msg.data()); }
	inline void PrintConsole(const char* a_msg) { RE::ConsoleLog::GetSingleton()->Print(a_msg); }
	template <typename... Args>
	inline void PrintConsole(fmt::format_string<Args...> a_fmt, Args&&... args)
	{
		const auto msg = fmt::format(a_fmt, std::forward<Args>(args)...);
		RE::ConsoleLog::GetSingleton()->Print(msg.data());
	}

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

	// ControlMap
	void ToggleControls(RE::ControlMap* controlMap, RE::ControlMap::UEFlag a_flags, bool a_enable);
	bool IsMovementControlsEnabled(RE::ControlMap* controlMap);

}  // namespace Acheron
