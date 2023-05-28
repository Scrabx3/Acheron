#include "Acheron/EventSink.h"
#include "Acheron/Hooks/Hooks.h"
#include "Acheron/Interface/CustomMenu.h"
#include "Acheron/Interface/HunterPride.h"
#include "Acheron/Resolution.h"
#include "Acheron/Validation.h"
#include "Papyrus/Config.h"
#include "Papyrus/Events.h"
#include "Papyrus/Functions.h"

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
	switch (message->type) {
	case SKSE::MessagingInterface::kSaveGame:
		Settings::Save();
		Acheron::Resolution::Save();
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		if (!Acheron::GameForms::LoadForms()) {
			logger::critical("Unable to load plugin objects");
			if (SKSE::WinAPI::MessageBox(nullptr, "Some game objects could not be loaded. This is usually due to a required game plugin not being loaded in your game. Please ensure that you have all requirements installed\n\nExit Game now? (Recommended yes)", "Acheron Load Data", 0x00000004) == 6)
				std::_Exit(EXIT_FAILURE);
			return;
		}
		Settings::bKillMove = Acheron::GameForms::KillMove->value != 0.0f;
		Settings::Initialize();
		Acheron::Validation::Initialize();
		Acheron::Resolution::Initialize();
		Acheron::EventHandler::GetSingleton()->Register();
		Acheron::GameForms::KillMove->value = Settings::bKillMove;
		break;
	case SKSE::MessagingInterface::kNewGame:
	case SKSE::MessagingInterface::kPostLoadGame:
		{
			const auto player = RE::PlayerCharacter::GetSingleton();
			const auto base = player->GetActorBase();
			bool success = base && base->AddPerk(Acheron::GameForms::InteractionPerk, 1);
			if (success) {
				logger::info("Added Interaction Perk to player");
				for (auto& perkEntry : Acheron::GameForms::InteractionPerk->perkEntries) {
					if (perkEntry) {
						perkEntry->ApplyPerkEntry(player);
					}
				}
			}
		}
		break;
	}
}

#ifdef SKYRIM_SUPPORT_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);
	v.AuthorName("Scrab Joséline"sv);
	v.UsesAddressLibrary();
	v.UsesUpdatedStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });
	// v.CompatibleVersions({ SKSE::RUNTIME_1_6_353 });
	return v;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION.pack();
	return true;
}
#endif

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	const auto InitLogger = []() -> bool {
#ifndef NDEBUG
		auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
		auto path = logger::log_directory();
		if (!path)
			return false;
		*path /= fmt::format(FMT_STRING("{}.log"), Plugin::NAME);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif
		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
#ifndef NDEBUG
		log->set_level(spdlog::level::trace);
		log->flush_on(spdlog::level::trace);
#else
		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::info);
#endif
		spdlog::set_default_logger(std::move(log));
#ifndef NDEBUG
		spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);
#else
		spdlog::set_pattern("[%^%l%$] %v"s);
#endif

		logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());
		return true;
	};
	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	} else if (!InitLogger()) {
		return false;
	}

	SKSE::Init(a_skse);

	const auto papyrus = SKSE::GetPapyrusInterface();
	papyrus->Register(Papyrus::RegisterFuncs);
	papyrus->Register(Papyrus::RegisterEvents);
	papyrus->Register(Papyrus::RegisterConfig);

	const auto msging = SKSE::GetMessagingInterface();
	if (!msging->RegisterListener("SKSE", SKSEMessageHandler)) {
		logger::critical("Failed to register Listener");
		return false;
	}

	Acheron::Interface::HunterPride::Register();
	Acheron::Interface::CustomMenu::Register();
	Acheron::Hooks::Install();

	const auto serialization = SKSE::GetSerializationInterface();
	serialization->SetUniqueID('achr');
	serialization->SetSaveCallback(Serialization::Serialize::SaveCallback);
	serialization->SetLoadCallback(Serialization::Serialize::LoadCallback);
	serialization->SetRevertCallback(Serialization::Serialize::RevertCallback);
	serialization->SetFormDeleteCallback(Serialization::Serialize::FormDeleteCallback);

	logger::info("{} loaded"sv, Plugin::NAME);

	return true;
}
