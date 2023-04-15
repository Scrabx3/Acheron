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
		static void Show(std::string_view a_filepath);
		static void Hide();
		static bool IsOpen();

	protected:
		// IMenu
		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;

	private:
		static inline std::string_view filepath{ ""sv };
	};
}