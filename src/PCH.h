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
}

namespace Papyrus
{
#define REGISTERFUNC(func, classname) a_vm->RegisterFunction(#func##sv, classname, func)
#define REGISTERFUNCND(func, classname) a_vm->RegisterFunction(#func##sv, classname, func, true)

	using VM = RE::BSScript::IVirtualMachine;
	using StackID = RE::VMStackID;
}

#define DLLEXPORT __declspec(dllexport)
#include "Plugin.h"
