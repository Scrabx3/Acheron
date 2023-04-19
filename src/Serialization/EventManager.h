#pragma once

namespace Serialization
{
	class EventManager :
		public Singleton<EventManager>
	{
	public:
		enum : std::uint32_t
		{
			ActorDefeated = 'adtd',
			ActorRescued = 'arsd',
			HunterPrideSelect = 'hps'
		};

	public:
		SKSE::RegistrationSet<const RE::Actor*> _actordefeated{ "OnActorDefeated"sv };
		SKSE::RegistrationSet<const RE::Actor*> _actorrescued{ "OnActorRescued"sv };
		SKSE::RegistrationSet<int32_t, const RE::Actor*> _hunterprideselect{ "OnHunterPrideSelect"sv };

	public:
		void Save(SKSE::SerializationInterface* a_intfc, std::uint32_t a_version);
		void Load(SKSE::SerializationInterface* a_intfc, std::uint32_t a_type);
		void Revert(SKSE::SerializationInterface* a_intfc);
		void FormDelete(RE::VMHandle a_handle);
	};

}  // namespace Serialization
