#pragma once

#pragma warning(push)
#pragma warning(disable : 4200)
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#pragma warning(pop)

#include <atomic>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <magic_enum.hpp>
static_assert(magic_enum::is_magic_enum_supported);

#pragma warning(push)
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#pragma warning(pop)

namespace logger = SKSE::log;
namespace fs = std::filesystem;
using namespace std::literals;
using json = nlohmann::json;

#include "Acheron/Misc.h"
#include "GameForms.h"
#include "Random.h"
#include "Serialization/Serialize.h"
#include "Singleton.h"
#include "Settings.h"

static constexpr auto CONFIGPATH = [](std::string file) -> std::string { return "Data\\SKSE\\Acheron\\"s + file; };

#ifdef SKYRIM_SUPPORT_AE
#define RELID(SE, AE) REL::ID(AE)
#define OFFSET(SE, AE) AE
#else
#define RELID(SE, AE) REL::ID(SE)
#define OFFSET(SE, AE) SE
#endif

using Serialize = Serialization::Serialize;
namespace stl
{
	using namespace SKSE::stl;
	
	inline bool read_string(SKSE::SerializationInterface* a_intfc, std::string& a_str)
	{
		std::size_t size = 0;
		if (!a_intfc->ReadRecordData(size)) {
			return false;
		}
		a_str.reserve(size);
		if (!a_intfc->ReadRecordData(a_str.data(), static_cast<std::uint32_t>(size))) {
			return false;
		}
		return true;
	}

	template <class S>
	inline bool write_string(SKSE::SerializationInterface* a_intfc, const S& a_str)
	{
		std::size_t size = a_str.length() + 1;
		return a_intfc->WriteRecordData(size) && a_intfc->WriteRecordData(a_str.data(), static_cast<std::uint32_t>(size));
	}
}

namespace Papyrus
{
#define REGISTERFUNC(func, classname) a_vm->RegisterFunction(#func##sv, classname, func)
#define REGISTERFUNCND(func, classname) a_vm->RegisterFunction(#func##sv, classname, func, true)

	using VM = RE::BSScript::IVirtualMachine;
	using StackID = RE::VMStackID;
}

template <>
struct std::formatter<RE::BSFixedString> : std::formatter<const char*>
{
	template <typename FormatContext>
	auto format(const RE::BSFixedString& myStr, FormatContext& ctx) const
	{
		return std::formatter<const char*>::format(myStr.data(), ctx);
	}
};

#define DLLEXPORT __declspec(dllexport)
