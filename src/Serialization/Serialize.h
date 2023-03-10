#pragma once

#include "Singleton.h"

namespace Serialization
{
	class Serialize final :
		public Singleton<Serialize>
	{
	public:
		enum : std::uint32_t
		{
			_Version = 1,

			_Processing = 'prc',
			_Consequence = 'cnsq',
			
			_Defeated = 'dfts',
			_Pacified = 'pfcy',

			_HunterPride = 'htpr'
		};

		static void SaveCallback(SKSE::SerializationInterface* a_intfc);
		static void LoadCallback(SKSE::SerializationInterface* a_intfc);
		static void RevertCallback(SKSE::SerializationInterface* a_intfc);
		static void FormDeleteCallback(RE::VMHandle a_handle);
	};	// class Serialize

	inline void LoadSet(SKSE::SerializationInterface* a_intfc, std::set<RE::FormID>& a_set);
	inline void SaveSet(SKSE::SerializationInterface* a_intfc, std::set<RE::FormID>& a_set);
	inline std::string GetTypeName(uint32_t a_type);

}  // namespace Serialize