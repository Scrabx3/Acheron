#include "Acheron/Interface/CustomMenu.h"

#include "Acheron/Interface/Interface.h"
#include "Serialization/EventManager.h"

namespace Acheron::Interface
{
	///
	/// Menu
	///

	CustomMenu::CustomMenu() :
		RE::IMenu()
	{
		this->inputContext = Context::kNone;
		this->depthPriority = 3;
		this->menuFlags.set(
			Flag::kCustomRendering,
			Flag::kAssignCursorToRenderer,
			Flag::kUsesMenuContext,
			Flag::kDisablePauseMenu);

		auto scaleform = RE::BSScaleformManager::GetSingleton();
		[[maybe_unused]] bool success = scaleform->LoadMovieEx(this, filepath, [](RE::GFxMovieDef* a_def) -> void {
			a_def->SetState(
				RE::GFxState::StateType::kLog,
				RE::make_gptr<FlashLogger<CustomMenu>>().get());
		});
		assert(success);

		auto view = this->uiMovie;
		view->SetMouseCursorCount(0);
		FunctionManager::AttachSKSEFunctions(view);
	}

	void CustomMenu::Register()
	{
		RE::UI::GetSingleton()->Register(NAME, Create);
		logger::info("Registered Custom Menu");
	}

	RE::UI_MESSAGE_RESULTS CustomMenu::ProcessMessage(RE::UIMessage& a_message)
	{
		using Type = RE::UI_MESSAGE_TYPE;
		using Result = RE::UI_MESSAGE_RESULTS;

		// switch (*a_message.type) {
		// default:
		// 	return RE::IMenu::ProcessMessage(a_message);
		// }
		return RE::IMenu::ProcessMessage(a_message);
	}
}


