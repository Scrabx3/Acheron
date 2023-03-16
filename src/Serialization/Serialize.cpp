#include "Serialization/Serialize.h"

#include "Acheron/Defeat.h"
#include "Acheron/EventSink.h"
#include "Acheron/Interface/HunterPride.h"
#include "Serialization/EventManager.h"

namespace Serialization
{
	void Serialize::SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		EventManager::GetSingleton()->Save(a_intfc, _Version);

		if (!a_intfc->OpenRecord(_Defeated, _Version))
			logger::error("Failed to open record <Defeated>"sv);
		else
			Acheron::Defeat::Save(a_intfc, _Defeated);

		if (!a_intfc->OpenRecord(_Pacified, _Version))
			logger::error("Failed to open record <Pacified>"sv);
		else
			Acheron::Defeat::Save(a_intfc, _Pacified);

		if (!a_intfc->OpenRecord(_HunterPride, _Version))
			logger::error("Failed to open record <HunterPride>"sv);
		else
			Acheron::Interface::HunterPride::Save(a_intfc);

		if (!a_intfc->OpenRecord(_Processing, _Version))
			logger::error("Failed to open record <Processing>"sv);
		else if (!a_intfc->WriteRecordData(Settings::ProcessingEnabled))
			logger::error("Failed to serialize record <Processing>"sv);

		if (!a_intfc->OpenRecord(_Consequence, _Version))
			logger::error("Failed to open record <Processing>"sv);
		else if (!a_intfc->WriteRecordData(Settings::ConsequenceEnabled))
			logger::error("Failed to serialize record <Processing>"sv);

		logger::info("Finished writing data to cosave");
	}

	void Serialize::LoadCallback(SKSE::SerializationInterface* a_intfc)
	{    
		uint32_t type;
		uint32_t version;
		uint32_t length;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			if (version != _Version) {
				logger::info("Invalid Version for loaded Data of Type = {}. Expected = {}; Got = {}", GetTypeName(type), _Version, version);
				continue;
			}
			logger::info("Loading record {}"sv, GetTypeName(type));
			switch (type) {
			case _Defeated:
			case _Pacified:
				Acheron::Defeat::Load(a_intfc, type);
				break;
			case _HunterPride:
				Acheron::Interface::HunterPride::Load(a_intfc);
				break;
			case _Processing:
				a_intfc->ReadRecordData(Settings::ProcessingEnabled);
				break;
			case _Consequence:
				a_intfc->ReadRecordData(Settings::ConsequenceEnabled);
				break;
			default:
				EventManager::GetSingleton()->Load(a_intfc, type);
				break;
			}
		}

		logger::info("Finished loading data from cosave");
	}

	void Serialize::RevertCallback(SKSE::SerializationInterface* a_intfc)
	{
		EventManager::GetSingleton()->Revert(a_intfc);
		Acheron::Defeat::Revert(a_intfc);
		Acheron::Interface::HunterPride::Revert(a_intfc);
	}

	void Serialize::FormDeleteCallback(RE::VMHandle a_handle)
	{
		EventManager::GetSingleton()->FormDelete(a_handle);
  }

	inline std::string GetTypeName(uint32_t a_type)
	{
		constexpr auto size = sizeof(uint32_t);
		std::string ret{};
		ret.resize(size);
		const char* it = reinterpret_cast<char*>(&a_type);
		for (size_t i = 0, j = size - 2; i < size - 1; i++, j--)
			ret[j] = it[i];

		return ret;
	}

}  // namespace Serialization
