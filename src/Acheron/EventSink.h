#pragma once

namespace Acheron
{
	using EventResult = RE::BSEventNotifyControl;

	class EventHandler :
			public Singleton<EventHandler>,
			public RE::BSTEventSink<RE::TESActorLocationChangeEvent>,
			public RE::BSTEventSink<RE::BSAnimationGraphEvent>,
			public RE::BSTEventSink<RE::TESFormDeleteEvent>,
			public RE::BSTEventSink<RE::TESCombatEvent>,
			public RE::BSTEventSink<RE::TESDeathEvent>,
			public RE::BSTEventSink<RE::TESResetEvent>,
			public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		void Register()
		{
			logger::info("Registering Event Sinks");

			const auto script = RE::ScriptEventSourceHolder::GetSingleton();
			script->AddEventSink<RE::TESActorLocationChangeEvent>(this);
			script->AddEventSink<RE::TESFormDeleteEvent>(this);
			script->AddEventSink<RE::TESCombatEvent>(this);
			script->AddEventSink<RE::TESDeathEvent>(this);
			script->AddEventSink<RE::TESResetEvent>(this);

			const auto input = RE::BSInputDeviceManager::GetSingleton();
			input->AddEventSink<RE::InputEvent*>(this);
		}

	public:
		EventResult ProcessEvent(const RE::TESActorLocationChangeEvent* a_event, RE::BSTEventSource<RE::TESActorLocationChangeEvent>*) override;
		EventResult ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>*) override;
		EventResult ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>*) override;
		EventResult ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>*) override;
		EventResult ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*) override;
		EventResult ProcessEvent(const RE::TESResetEvent* a_event, RE::BSTEventSource<RE::TESResetEvent>*) override;

		EventResult ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override;

		void CacheWornArmor(const RE::FormID a_form, RE::TESObjectARMO* a_armor);
		std::vector<RE::TESObjectARMO*> ExtractCachedArmor(const RE::FormID a_form);
	private:
		std::map<RE::FormID, std::vector<RE::TESObjectARMO*>> worn_cache{};
		std::mutex cache_lock{};
	};
}	 // namespace Acheron
