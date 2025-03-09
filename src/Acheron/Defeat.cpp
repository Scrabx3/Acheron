#include "Acheron/Defeat.h"

#include "Acheron/EventSink.h"
#include "Script.h"
#include "Serialization/EventManager.h"

namespace Acheron
{
	struct PackageOverrideCallback : public RE::BSScript::IStackCallbackFunctor
	{
	public:
		PackageOverrideCallback(RE::Actor* a_actor) :
				_actor(a_actor) {}
		~PackageOverrideCallback() = default;

		virtual void operator()([[maybe_unused]] RE::BSScript::Variable a_result) override
		{
			_actor->EvaluatePackage();
		}

		virtual void SetObject([[maybe_unused]] const RE::BSTSmartPointer<RE::BSScript::Object>& a_object)
		{
		}

	private:
		RE::Actor* _actor;
	};

	///
	///	Defeat
	///

	void Defeat::DefeatActor(RE::Actor* a_victim)
	{
		const auto state = a_victim->GetLifeState();
		if (state == RE::ACTOR_LIFE_STATE::kDying || state == RE::ACTOR_LIFE_STATE::kDead) {
			logger::error("{:X} ({}) is dead and cannot be defeated", a_victim->GetFormID(), a_victim->GetDisplayFullName());
			return;
		}

		auto data = std::make_shared<VictimData>(RE::Calendar::GetSingleton()->GetDaysPassed());
		{
			std::unique_lock lock{_m};
			if (!Victims.emplace(a_victim->GetFormID(), data).second) {
				logger::error("{:X} ({}) is already defeated", a_victim->GetFormID(), a_victim->GetDisplayFullName());
				return;
			} else if (auto ref = a_victim->GetObjectReference(); !ref) {
				logger::warn("{:X} ({}) has no associated reference, keywords will NOT be added", a_victim->GetFormID(), a_victim->GetDisplayFullName());
			} else {
				ref->As<RE::BGSKeywordForm>()->AddKeyword(GameForms::Defeated);
			}
		}

		if (a_victim->IsPlayerRef()) {
			const auto cmap = RE::ControlMap::GetSingleton();
			Acheron::ToggleControls(cmap, RE::UserEvents::USER_EVENT_FLAG::kActivate, false);
			Acheron::ToggleControls(cmap, RE::UserEvents::USER_EVENT_FLAG::kJumping, false);
			Acheron::ToggleControls(cmap, RE::UserEvents::USER_EVENT_FLAG::kMainFour, false);
			Acheron::ToggleControls(cmap, RE::UserEvents::USER_EVENT_FLAG::kPOVSwitch, false);
			a_victim->AddAnimationGraphEventSink(EventHandler::GetSingleton());
			RE::PlayerCamera::GetSingleton()->ForceThirdPerson();

			PacifyUnsafe(a_victim);
		} else {
			if (a_victim->IsPlayerTeammate()) {
				a_victim->SetActorValue(RE::ActorValue::kWaitingForPlayer, 1);
			}
			PacifyUnsafe(a_victim);
			Script::CallbackPtr callback(new PackageOverrideCallback(a_victim));
			if (!Script::DispatchStaticCall("ActorUtil", "AddPackageOverride", callback, std::move(a_victim), std::move(GameForms::BlankPackage), std::move(31), std::move(1))) {
				logger::error("Failed to dispatch static call [ActorUtil::AddPackageOverride]. PapyrusUtil missing?");
			}
		}

		a_victim->boolFlags.set(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
		if (a_victim->Is3DLoaded()) {
			if (auto process = a_victim->currentProcess) {
				process->PlayIdle(a_victim, GameForms::BleedoutStart, nullptr);
			} else {
				a_victim->NotifyAnimationGraph("BleedoutStart");
			}
		}
		const auto& process = a_victim->currentProcess;
		const auto middlehigh = process ? process->middleHigh : nullptr;
		if (middlehigh) {
			for (auto& commandedActorData : middlehigh->commandedActors) {
				if (const auto summon = commandedActorData.activeEffect) {
					summon->Dispel(true);
				}
			}
		}

		const auto health = a_victim->GetActorValue(RE::ActorValue::kHealth);
		a_victim->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -health + 0.05f);

		data->state.store(VictimState::Defeated);

		logger::info("{:X} ({}) has been defeated", a_victim->GetFormID(), a_victim->GetDisplayFullName());
		Serialization::EventManager::GetSingleton()->_actordefeated.QueueEvent(a_victim);
	}

	void Defeat::RescueActor(RE::Actor* a_victim, bool undo_pacify)
	{

		if (auto data = GetVictimData(a_victim->GetFormID()); !data) {
			logger::error("{:X} ({}) is not a defeated actor and cannot be rescued", a_victim->GetFormID(), a_victim->GetDisplayFullName());
			return;
		} else if (auto expected = VictimState::Defeated; !data->state.compare_exchange_strong(expected, VictimState::Rescuing)) {
			switch (expected)
			{
			case VictimState::Defeating:
				logger::info("{:X} ({}) is still being defeated, delaying rescue", a_victim->GetFormID(), a_victim->GetDisplayFullName());
				{
					auto victim = a_victim->GetHandle();
					SKSE::GetTaskInterface()->AddTask([victim, undo_pacify]() {
						RescueActor(victim.get().get(), undo_pacify);
					});
				}
				return;
			case VictimState::Rescuing:
				logger::error("{:X} ({}) is already being rescued", a_victim->GetFormID(), a_victim->GetDisplayFullName());
				return;
			}
		}

		if (a_victim->IsPlayerRef()) {
			auto cmap = RE::ControlMap::GetSingleton();
			Acheron::ToggleControls(cmap, RE::UserEvents::USER_EVENT_FLAG::kActivate, true);
			Acheron::ToggleControls(cmap, RE::UserEvents::USER_EVENT_FLAG::kJumping, true);
			Acheron::ToggleControls(cmap, RE::UserEvents::USER_EVENT_FLAG::kMainFour, true);
			Acheron::ToggleControls(cmap, RE::UserEvents::USER_EVENT_FLAG::kPOVSwitch, true);
			a_victim->RemoveAnimationGraphEventSink(EventHandler::GetSingleton());
		} else {
			if (a_victim->IsPlayerTeammate()) {
				a_victim->SetActorValue(RE::ActorValue::kWaitingForPlayer, 0);
			}
			Script::CallbackPtr callback(new PackageOverrideCallback(a_victim));
			if (!Script::DispatchStaticCall("ActorUtil", "RemovePackageOverride", callback, std::move(a_victim), std::move(GameForms::BlankPackage))) {
				logger::error("Unable to dispatch static call [ActorUtil::RemovePackageOverride]. PapyrusUtil missing?");
			}
		}

		a_victim->boolFlags.reset(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
		if (a_victim->Is3DLoaded() && !a_victim->IsDead()) {
			if (auto process = a_victim->currentProcess) {
				process->PlayIdle(a_victim, GameForms::BleedoutStop, a_victim);
			} else {
				a_victim->NotifyAnimationGraph("BleedoutStop");
			}
		}

		if (undo_pacify) {
			UndoPacify(a_victim);
		}

		{
			std::unique_lock lock{_m};
			if (auto ref = a_victim->GetObjectReference(); !ref) {
				logger::warn("{:X} ({}) has no associated reference, keywords will not be removed", a_victim->GetFormID(), a_victim->GetDisplayFullName());
			} else {
				ref->As<RE::BGSKeywordForm>()->RemoveKeyword(GameForms::Defeated);
			}
			Victims.erase(a_victim->GetFormID());
		}

		logger::info("{:X} ({}) has been rescued", a_victim->GetFormID(), a_victim->GetDisplayFullName());
		Serialization::EventManager::GetSingleton()->_actorrescued.QueueEvent(a_victim);
	}

	void Defeat::RescueDelayed(RE::Actor* a_victim, bool a_undo_pacify)
	{
		RescueActor(a_victim, false);
		if (a_undo_pacify) {
			auto victim = a_victim->GetHandle();
			std::thread([victim]() {
				std::this_thread::sleep_for(4s);
				SKSE::GetTaskInterface()->AddTask([=]() {
					UndoPacify(victim.get().get());
				});
			}).detach();
		}
	}

	void Defeat::Pacify(RE::Actor* a_victim)
	{
		const auto state = a_victim->GetLifeState();
		if (state == RE::ACTOR_LIFE_STATE::kDying || state == RE::ACTOR_LIFE_STATE::kDead) {
			logger::error("{:X} ({}) is dead and cannot be pacified", a_victim->GetFormID(), a_victim->GetDisplayFullName());
			return;
		} else if (IsPacified(a_victim)) {
			logger::error("{:X} ({}) is already pacified", a_victim->GetFormID(), a_victim->GetDisplayFullName());
			return;
		}

		PacifyUnsafe(a_victim);
	}

	void Defeat::PacifyUnsafe(RE::Actor* a_victim)
	{
		{
			std::unique_lock lock{_m};
			if (auto ref = a_victim->GetObjectReference(); !ref) {
				logger::warn("{:X} ({}) has no associated reference, keywords will NOT be added", a_victim->GetFormID(), a_victim->GetDisplayFullName());
			} else {
				ref->As<RE::BGSKeywordForm>()->AddKeyword(GameForms::Pacified);
			}
			if (!Pacified.insert(a_victim->GetFormID()).second) {
				return;
			}
		}

		const auto process = RE::ProcessLists::GetSingleton();
		process->runDetection = false;
		process->ClearCachedFactionFightReactions();
		process->StopCombatAndAlarmOnActor(a_victim, false);
		process->runDetection = true;

		logger::info("{:X} ({}) has been pacified", a_victim->GetFormID(), a_victim->GetDisplayFullName());
	}

	void Defeat::UndoPacify(RE::Actor* a_victim)
	{
		{
			std::unique_lock lock{_m};
			if (!Pacified.contains(a_victim->GetFormID())) {
				logger::error("{:X} ({}) is not a pacified actor and cannot be released", a_victim->GetFormID(), a_victim->GetDisplayFullName());
				return;
			} else if (auto ref = a_victim->GetObjectReference(); !ref) {
				logger::warn("{:X} ({}) has no associated reference, keywords will not be removed", a_victim->GetFormID(), a_victim->GetDisplayFullName());
			} else {
				ref->As<RE::BGSKeywordForm>()->RemoveKeyword(GameForms::Pacified);
			}
			Pacified.erase(a_victim->GetFormID());
		}

		logger::info("{:X} ({}) has been released", a_victim->GetFormID(), a_victim->GetDisplayFullName());
	}

	bool Defeat::IsDefeated(const RE::Actor* a_victim)
	{
		const auto key = a_victim->GetFormID();
		std::shared_lock lock{_m};
		return Victims.contains(key) && Pacified.contains(key);
	}

	bool Defeat::IsPacified(const RE::Actor* a_victim)
	{
		std::shared_lock lock{_m};
		return Pacified.contains(a_victim->GetFormID());
	}

	bool Defeat::IsDamageImmune(RE::Actor* a_victim)
	{
		std::shared_lock lock{_m};
		return Victims.contains(a_victim->GetFormID());
	}

	///
	/// Utility
	///

	std::vector<RE::Actor*> Defeat::GetAllPacified(bool a_loadedonly)
	{
		std::shared_lock lock{_m};
		std::vector<RE::Actor*> ret;
		for (auto &&formid : Pacified)
		{
			auto actor = RE::TESForm::LookupByID<RE::Actor>(formid);
			if (!actor)
				continue;

			if (a_loadedonly && !actor->Is3DLoaded())
				continue;

			ret.push_back(actor);
		}
		return ret;
	}

	std::vector<RE::Actor*> Defeat::GetAllDefeated(bool a_loadedonly)
	{
		std::shared_lock lock{_m};
		std::vector<RE::Actor*> ret;
		for (auto&& [formid, data] : Victims) {
			auto actor = RE::TESForm::LookupByID<RE::Actor>(formid);
			if (!actor)
				continue;

			if (a_loadedonly && !actor->Is3DLoaded())
				continue;

			ret.push_back(actor);
		}
		return ret;
	}

	void Defeat::ForEachVictim(std::function<VictimVistor(RE::FormID a_victimid, std::shared_ptr<VictimData> a_data)> a_visitor)
	{
		std::vector<std::pair<RE::FormID, std::shared_ptr<VictimData>>> v;
		{
			std::shared_lock lock{_m};
			v.assign(Victims.begin(), Victims.end());
		}
		for (auto& [formid, data] : v) {
			if (a_visitor(formid, data) == VictimVistor::Break)
				break;
		}
	}

	std::shared_ptr<Defeat::VictimData> Defeat::GetVictimData(RE::FormID a_formid)
	{
		std::shared_lock lock{_m};
		const auto where = Victims.find(a_formid);
		if (where == Victims.end())
			return {};

		return where->second;
	}

	void Defeat::DisableRecovery(bool a_loadedonly)
	{
		std::shared_lock lock{_m};
		for (auto&& [formid, data] : Victims) {
			if (a_loadedonly) {
				auto victim = RE::TESForm::LookupByID<RE::Actor>(formid);
				if (victim && !victim->Is3DLoaded())
					continue;
			}
			data->allow_recovery = false;
		}
	}

	///
	/// Serialization
	///

	void Defeat::Load(SKSE::SerializationInterface* a_intfc, uint32_t a_type)
	{
		std::unique_lock lock{_m};
		switch (a_type) {
		case Serialization::Serialize::_Defeated:
			{
				std::size_t numRegs;
				a_intfc->ReadRecordData(numRegs);

				Victims.clear();

				RE::FormID formID;
				float time;

				for (size_t i = 0; i < numRegs; i++) {
					a_intfc->ReadRecordData(formID);
					if (!a_intfc->ResolveFormID(formID, formID)) {
						logger::warn("Error reading formID ({:X})", formID);
						continue;
					}
					a_intfc->ReadRecordData(time);

					auto victim = RE::TESForm::LookupByID<RE::Actor>(formID);
					if (victim) {
						if (victim->IsPlayerRef()) {
							victim->AddAnimationGraphEventSink(EventHandler::GetSingleton());
						}
						auto ref = victim->GetObjectReference();
						if (!ref) {
							logger::warn("Unable to apply keyword to {:X}, form has no associated referece", victim->GetFormID());
							return;
						}
						ref->As<RE::BGSKeywordForm>()->AddKeyword(GameForms::Defeated);
					}

					Victims.emplace(formID, std::make_shared<VictimData>(time));
				}
				logger::info("Loaded {} Victims from cosave", Victims.size());
			}
			break;
		case Serialization::Serialize::_Pacified:
			{
				std::size_t numRegs;
				a_intfc->ReadRecordData(numRegs);

				Pacified.clear();

				RE::FormID formID;

				for (size_t i = 0; i < numRegs; i++) {
					a_intfc->ReadRecordData(formID);
					if (!a_intfc->ResolveFormID(formID, formID)) {
						logger::warn("Error reading formID ({:X})", formID);
						continue;
					}
					auto victim = RE::TESForm::LookupByID<RE::Actor>(formID);
					if (victim) {
						auto ref = victim->GetObjectReference();
						if (!ref) {
							logger::warn("Unable to apply keyword to {:X}, form has no associated referece", victim->GetFormID());
							return;
						}
						ref->As<RE::BGSKeywordForm>()->AddKeyword(GameForms::Pacified);
					}
					Pacified.insert(formID);
				}
				logger::info("Loaded {} Pacified from cosave", Pacified.size());
			}
			break;
		}
	}

	void Defeat::Save(SKSE::SerializationInterface* a_intfc, uint32_t a_type)
	{
		std::shared_lock lock{_m};
		switch (a_type) {
		case Serialization::Serialize::_Defeated:
			{
				const std::size_t numRegs = Victims.size();
				if (!a_intfc->WriteRecordData(numRegs)) {
					logger::error("Failed to save number of regs ({})", numRegs);
					return;
				}
				for (auto&& [formID, data] : Victims) {
					if (!a_intfc->WriteRecordData(formID)) {
						logger::error("Failed to save reg ({:X})", formID);
						continue;
					}
					if (!a_intfc->WriteRecordData(data->registered_at)) {
						logger::error("Failed to save reg ({})", data->registered_at);
						continue;
					}
				}
			}
			break;
		case Serialization::Serialize::_Pacified:
			{
				const std::size_t numRegs = Pacified.size();
				if (!a_intfc->WriteRecordData(numRegs)) {
					logger::error("Failed to save number of regs ({})", numRegs);
					return;
				}
				for (auto&& formID : Pacified) {
					if (!a_intfc->WriteRecordData(formID)) {
						logger::error("Failed to save reg ({:X})", formID);
						continue;
					}
				}
			}
			break;
		}
	}

	void Defeat::Revert(SKSE::SerializationInterface*)
	{
		std::unique_lock lock{_m};
		for (auto&& [formid, data] : Victims) {
			auto actor = RE::TESForm::LookupByID<RE::Actor>(formid);
			if (!actor)
				continue;

			if (actor->IsPlayerRef()) {
				actor->RemoveAnimationGraphEventSink(EventHandler::GetSingleton());
			}

			if (auto ref = actor->GetObjectReference(); ref) {
				ref->As<RE::BGSKeywordForm>()->RemoveKeywords({ GameForms::Defeated, GameForms::Pacified });
			}
		}
		for (auto&& formid : Pacified) {
			if (Victims.contains(formid))
				continue;

			auto actor = RE::TESForm::LookupByID<RE::Actor>(formid);
			if (!actor)
				continue;

			if (auto ref = actor->GetObjectReference(); ref) {
				ref->As<RE::BGSKeywordForm>()->RemoveKeyword(GameForms::Pacified);
			}
		}
		Victims.clear();
		Pacified.clear();
	}

	void Defeat::Delete(RE::FormID a_formid)
	{
		std::unique_lock lock{_m};
		if (Pacified.erase(a_formid) > 0) {
			Victims.erase(a_formid);
			logger::info("Form {:X} has been deleted and removed from Pacified & Victim lists", a_formid);
		}
	}


}	 // namespace Acheron
