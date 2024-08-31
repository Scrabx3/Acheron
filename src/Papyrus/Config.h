#pragma once

namespace Papyrus
{
	int GetSettingInt(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting);
	float GetSettingFloat(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting);
	bool GetSettingBool(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting);
	int GetSettingColor(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting);

	void SetSettingInt(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting, int a_value);
	void SetSettingFloat(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting, float a_value);
	void SetSettingBool(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting, bool a_value);
	void SetSettingColor(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_setting, int a_value);

	std::vector<std::string> GetEvents(VM* a_vm, StackID a_stackID, RE::TESQuest*, int a_type);
	std::string SetEventWeight(VM* a_vm, StackID a_stackID, RE::TESQuest*, std::string a_event, int a_type, int a_newweight);

	void UpdateKillmoveGlobal(VM* a_vm, StackID a_stackID, RE::TESQuest*);

	inline bool RegisterConfig(VM* a_vm)
	{
		REGISTERFUNCND(GetSettingInt, "AcheronMCM");
		REGISTERFUNCND(GetSettingFloat, "AcheronMCM");
		REGISTERFUNCND(GetSettingBool, "AcheronMCM");
		REGISTERFUNCND(GetSettingColor, "AcheronMCM");
  
		REGISTERFUNCND(SetSettingInt, "AcheronMCM");
		REGISTERFUNCND(SetSettingFloat, "AcheronMCM");
		REGISTERFUNCND(SetSettingBool, "AcheronMCM");
		REGISTERFUNCND(SetSettingColor, "AcheronMCM");

		REGISTERFUNCND(GetEvents, "AcheronMCM");
		REGISTERFUNCND(SetEventWeight, "AcheronMCM");

		REGISTERFUNCND(UpdateKillmoveGlobal, "AcheronMCM");

		return true;
	}

}	 // namespace Papyrus
