#pragma once

namespace Acheron::Interface
{
	class CustomMenu :
		public RE::IMenu
	{
		using GRefCountBaseStatImpl::operator new;
		using GRefCountBaseStatImpl::operator delete;

	public:
		static constexpr std::string_view NAME{ "AcheronCustomMenu" };

		CustomMenu();
		~CustomMenu() = default;
		static void Register();
		static RE::IMenu* Create() { return new CustomMenu(); }

	public:
		static void SetSwfPath(std::string_view a_path) { filepath = a_path; }

		static void Show() { RE::UIMessageQueue::GetSingleton()->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr); }
		static void Hide() { RE::UIMessageQueue::GetSingleton()->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr); }
		static void ForceHide() { RE::UIMessageQueue::GetSingleton()->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kForceHide, nullptr); }
		static bool IsOpen() { return RE::UI::GetSingleton()->IsMenuOpen(NAME); }

	protected:
		// IMenu
		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;

	private:
		static inline std::string_view filepath{ ""sv };
	};
}