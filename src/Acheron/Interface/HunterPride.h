#pragma once

namespace Acheron::Interface
{
	class HunterPride :
			public RE::IMenu
	{
		using GRefCountBaseStatImpl::operator new;
		using GRefCountBaseStatImpl::operator delete;

	public:
		static constexpr std::string_view NAME{ "AcheronHunterPride" };
		static constexpr std::string_view FILEPATH{ "Acheron\\AcheronHunterPride" };

		HunterPride();
		~HunterPride() = default;
		static void Register();
		static RE::IMenu* Create() { return new HunterPride(); }

	public:
		static void SetTarget(RE::Actor* a_target) { _target = a_target;}

		static void Show() { RE::UIMessageQueue::GetSingleton()->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr); }
		static void Hide() { RE::UIMessageQueue::GetSingleton()->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr); }
		static void ForceHide() { RE::UIMessageQueue::GetSingleton()->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kForceHide, nullptr); }
		static bool IsOpen() { return RE::UI::GetSingleton()->IsMenuOpen(NAME); }

		static bool AddOption(const RE::BSFixedString& a_option, const std::string& a_conditionstring, const std::string& a_name, const std::string& a_iconsrc);
		static bool RemoveOption(const RE::BSFixedString& a_option);
		static bool HasOption(const RE::BSFixedString& a_option);

		// Serialization
		static void Save(SKSE::SerializationInterface* a_intfc);
		static void Load(SKSE::SerializationInterface* a_intfc);
		static void Revert(SKSE::SerializationInterface* a_intfc);

	protected:
		// IMenu
		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;

		// Scaleform Callback
		struct OnItemSelected : public RE::GFxFunctionHandler
		{
			void Call(Params& a_args) override;
		};

		struct CloseComplete : public RE::GFxFunctionHandler
		{
			void Call(Params& a_args) override;
		};

	private:
		struct Option
		{
			friend HunterPride;

			struct CONDITION
			{
				union ConditionValue
				{
					RE::TESFaction* faction;
					RE::BGSKeyword* keyword;
				};

				enum class ConditionType : uint8_t
				{
					Faction,
					Keyword
				};

			public:
				CONDITION(ConditionType a_type, const std::string& a_objstring, bool a_reverse);
				CONDITION(ConditionType a_type, ConditionValue a_value, bool a_reverse) :
						reverse(a_reverse), type(a_type), value(a_value) {}
				~CONDITION() = default;

				_NODISCARD bool Check(RE::Actor* a_target) const;

			public:
				bool reverse;
				ConditionType type;
				ConditionValue value;
			};

			enum ConditionTarget
			{
				Player,
				Target,

				Total
			};

		public:
			Option(const RE::BSFixedString& a_option, const std::string& a_conditions, const std::string& a_name, const std::string& a_icon);
			Option() = default;
			~Option() = default;

			void PopulateObjectData(RE::GFxValue& a_object) const;

			_NODISCARD bool Check() const;
			_NODISCARD const RE::BSFixedString& GetID() const;

		private:
			RE::BSFixedString _id;
			std::string _name;
			std::string _iconurl;

			std::vector<CONDITION> conditions[ConditionTarget::Total];
		};

	private:
		static inline std::vector<Option> _options;
		static inline RE::Actor* _target;
	};
}