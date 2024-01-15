#include "Serialization/EventManager.h"

namespace Serialization
{
	void EventManager::Save(SKSE::SerializationInterface* a_intfc, std::uint32_t a_version)
  {
	  _actordefeated.Save(a_intfc, ActorDefeated, a_version);
		_actorrescued.Save(a_intfc, ActorRescued, a_version);
		_playerdeathevent.Save(a_intfc, PlayerDeathEvent, a_version);
		_hunterprideselect.Save(a_intfc, HunterPrideSelect, a_version);
	}

	void EventManager::Load(SKSE::SerializationInterface* a_intfc, std::uint32_t a_type)
  {
	  switch (a_type) {
	  case ActorDefeated:
		  _actordefeated.Load(a_intfc);
			break;
		case ActorRescued:
			_actorrescued.Load(a_intfc);
			break;
		case PlayerDeathEvent:
			_playerdeathevent.Load(a_intfc);
			break;
		case HunterPrideSelect:
			_hunterprideselect.Load(a_intfc);
			break;
		default:
      logger::warn("Unknown Type = {}", a_type);
		  break;
	  }
  }

	void EventManager::Revert(SKSE::SerializationInterface* a_intfc)
  {
	  _actordefeated.Revert(a_intfc);
	  _actorrescued.Revert(a_intfc);
		_playerdeathevent.Revert(a_intfc);
		_hunterprideselect.Revert(a_intfc);
	}

	void EventManager::FormDelete(RE::VMHandle a_handle)
  {
	  _actordefeated.Unregister(a_handle);
	  _actorrescued.Unregister(a_handle);
		_playerdeathevent.Unregister(a_handle);
		_hunterprideselect.Unregister(a_handle);
	}

} // namespace Serialization
