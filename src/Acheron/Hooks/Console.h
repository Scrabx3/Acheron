#pragma once

namespace Acheron
{
	struct CommandBase
	{
		virtual ~CommandBase() = default;

		virtual bool Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const = 0;
		RE::TESObjectREFR* ParseTargetRef(std::string_view a_targetStr) const;
		RE::Actor* GetTargetActor(std::string_view a_targetStr, RE::TESObjectREFR* a_targetRef) const;
	};

	struct CmdHelp : public CommandBase
	{
		virtual bool Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const override;
	};

	struct CmdDefeat : public CommandBase
	{
		virtual bool Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const override;
	};

	struct CmdRescue : public CommandBase
	{
		virtual bool Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const override;
	};

	struct CmdIsDefeated : public CommandBase
	{
		virtual bool Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const override;
	};

	struct CmdPacify : public CommandBase
	{
		virtual bool Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const override;
	};

	struct CmdRelease : public CommandBase
	{
		virtual bool Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const override;
	};

	struct CmdIsPacified : public CommandBase
	{
		virtual bool Run(std::vector<std::string_view>& a_args, RE::TESObjectREFR* a_targetRef) const override;
	};

	struct Console
	{
		static bool ParseCmd(std::string_view a_cmd, RE::TESObjectREFR* a_targetRef);

	private:
		static inline const std::map<std::string, std::shared_ptr<CommandBase>> commands{
			{ "help"s, std::make_shared<CmdHelp>() },
			{ "defeat"s, std::make_shared<CmdDefeat>() },
			{ "rescue"s, std::make_shared<CmdRescue>() },
			{ "isdefeated"s, std::make_shared<CmdIsDefeated>() },
			{ "pacify"s, std::make_shared<CmdPacify>() },
			{ "release"s, std::make_shared<CmdRelease>() },
			{ "ispacified"s, std::make_shared<CmdIsPacified>() },
		};
	};
}  // namespace Acheron::Console
