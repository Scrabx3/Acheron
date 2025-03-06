Scriptname AcheronMCM extends SKI_ConfigBase Hidden 

; ----------- Settings

int Function GetSettingInt(String asSetting) native
float Function GetSettingFloat(String asSetting) native
bool Function GetSettingBool(String asSetting) native
int Function GetSettingColor(String asSetting) native

Function SetSettingInt(String asSetting, int aiValue) native
Function SetSettingFloat(String asSetting, float aiValue) native
Function SetSettingBool(String asSetting, bool abValue) native
Function SetSettingColor(String asSetting, int aiColor) native

Function UpdateKillmoveGlobal() native

String[] _FollowerRescue

; ----------- Events

String[] Function GetEvents(int aiType) native
String Function SetEventWeight(String asEvent, int aiType, int aiNewWeight) native

String[] _typesList
int _type = 0

String[] _events  ; [Flags] Name;Weight;ID

int Function GetEventWeight(String asEvent)
  String[] args = StringUtil.Split(asEvent, ";")
  String w = args[1]
  return w as int
EndFunction

; --------------------- Menu

int Function GetVersion()
	return 2
EndFunction

Event OnVersionUpdate(Int newVersion)
  If (newVersion > CurrentVersion)
    OnConfigInit()
  EndIf
EndEvent

Event OnGameReload()
  Parent.OnGameReload()

  If (SKSE.GetPluginVersion("Acheron") == -1)
    Debug.MessageBox("[Acheron]\n\nMissing Acheron.dll. Acheron will NOT work correctly while this error persists.\nThis is likely due installing a version that doesn't support your game version.")
  EndIf
EndEvent

Event OnConfigInit()
  Pages = new String[4]
  Pages[0] = "$Achr_General"
  Pages[1] = "$Achr_Defeat"
  Pages[2] = "$Achr_Stripping"
  Pages[3] = "$Achr_Events"

  _typesList = new String[5]
  _typesList[0] = "$Achr_Hostile"
  _typesList[1] = "$Achr_Follower"
  _typesList[2] = "$Achr_Civilian"
  _typesList[3] = "$Achr_Guards"
  _typesList[4] = "$Achr_NPC"

  _FollowerRescue = new String[3]
  _FollowerRescue[0] = "$Achr_Rescue_0"
  _FollowerRescue[1] = "$Achr_Rescue_1"
  _FollowerRescue[2] = "$Achr_Rescue_2"
EndEvent

Event OnConfigOpen()
EndEvent

Event OnConfigClose()
EndEvent

Event OnPageReset(string page)
  SetCursorFillMode(TOP_TO_BOTTOM)
  If (page == "$Achr_General")
    AddHeaderOption("$Achr_Status")
    AddToggleOptionST("enabled", "$Achr_Enabled", !Acheron.IsProcessingDisabled())
    AddToggleOptionST("consequence", "$Achr_Consequence", !Acheron.IsConsequenceDisabled())
    AddToggleOptionST("creatures", "$Achr_Creatures", GetSettingBool("bCreatureDefeat"))
    AddToggleOptionST("killmoves", "$Achr_Killmoves", GetSettingBool("bKillMove"))

    SetCursorPosition(1)
    AddHeaderOption("$Achr_HunterPride")
    AddKeyMapOptionST("hunterpridekey", "$Achr_HunterPrideKey", GetSettingInt("iHunterPrideKey"))
    AddKeyMapOptionST("hunterpridemodkey", "$Achr_ModifierKey", GetSettingInt("iHunterPrideKeyMod"))
    AddToggleOptionST("hunterpridefollower", "$Achr_HunterPrideFollower", GetSettingBool("bHunterPrideFollower"))

    AddHeaderOption("$Achr_Notification")
    bool bNotifyDefeat = GetSettingBool("bNotifyDefeat")
    bool bNotifyDestroy = GetSettingBool("bNotifyDestroy")
    bool bNotifyColored = GetSettingBool("bNotifyColored")
    AddToggleOptionST("notifydefeat", "$Achr_NotifyDefeat", bNotifyDefeat)
    AddToggleOptionST("notifydestroy", "$Achr_NotifyDestry", bNotifyDestroy) ; item destruction
		AddToggleOptionST("notifycolored", "$Achr_NotifyColored", bNotifyColored, getFlag(bNotifyDefeat || bNotifyDestroy))
    AddColorOptionST("notifycolorchoice", "$Achr_NotifyColorChoice", GetSettingColor("rNotifyColor"), getFlag((bNotifyDefeat || bNotifyDestroy) && bNotifyColored))

  ElseIf (page == "$Achr_Defeat")
    AddHeaderOption("$Achr_Lethal")
    AddToggleoptionST("lethalessential", "$Achr_LethalEssential", GetSettingBool("bLethalEssential"))
    AddSliderOptionST("lethalplayer", "$Achr_LethalPlayer", GetSettingFloat("fLethalPlayer"), "{1}%")
    AddSliderOptionST("lethalfollower", "$Achr_LethalFollower", GetSettingFloat("fLethalFollower"), "{1}%")
    AddSliderOptionST("lethalnpc", "$Achr_LethalNPC", GetSettingFloat("fLethalNPC"), "{1}%")

    AddHeaderOption("$Achr_Trauma")
    AddToggleOptionST("traumenable", "$Achr_TraumaEnab", GetSettingBool("bTraumaEnabled"))
    AddToggleOptionST("traumhealth", "$Achr_TraumaHealth", GetSettingBool("bTraumaHealth"))
    AddSliderOptionST("traumamult", "$Achr_TraumaMult", GetSettingFloat("fTraumaMult"), "{0}")
    AddSliderOptionST("traumabackatt", "$Achr_TraumaBackAttack", GetSettingFloat("fTraumeBackAttack"), "{1}")

    AddHeaderOption("$Achr_Exposed")
    AddSliderOptionST("exposed", "$Achr_StripReq", GetSettingInt("iExposed"), "{0}")
    AddSliderOptionST("exposedchance", "$Achr_StripReqChance", GetSettingFloat("fExposedChance"), "{1}%")

    SetCursorPosition(1)
    AddHeaderOption("$Achr_Knockdown")
    AddSliderOptionST("midcmbtblackout", "$Achr_DefeatEndCmt", GetSettingFloat("fMidCombatBlackout"), "{1}%")
    AddToggleoptionST("kdplayer", "$Achr_KdPlayer", GetSettingBool("bPlayerDefeat"))
    AddToggleoptionST("kdfollower", "$Achr_KdFollower", GetSettingBool("bFollowerDefeat"))
    AddToggleOptionST("kdfolwithplayer", "$Achr_KdFolWithplayer", GetSettingBool("bFolWithPlDefeat"))
    AddToggleoptionST("kdnpc", "$Achr_KdNPC", GetSettingBool("bNPCDefeat"))

    AddHeaderOption("$Achr_Recovery")
    AddSliderOptionST("recoverhealththresh", "$Achr_RecHealThresh", GetSettingFloat("fKdHealthThresh") * 100, "{1}%")
    AddSliderOptionST("recoverhealfallback", "$Achr_RecHealFallback", GetSettingInt("iKdFallbackTimer"), "{0}s")
    AddMenuOptionST("FollowerRescue", "$Achr_FollowerRescue", _FollowerRescue[GetSettingInt("iFollowerRescue")])
    AddToggleoptionST("NPCRescueReload", "$Achr_NPCRescueReload", GetSettingBool("bNPCRescueReload"))

    AddHeaderOption("$Achr_Stripping")
    AddSliderOptionST("stripchance", "$Achr_StripChance", GetSettingFloat("fStripChance"), "{1}%")
    AddSliderOptionST("stripdstry", "$Achr_StripDstry", GetSettingFloat("fStripDestroy"), "{1}%")
    AddToggleoptionST("stripdrop", "$Achr_StripDrop", GetSettingBool("bStripDrop"))

  ElseIf (page == "$Achr_Stripping")
    SetCursorFillMode(LEFT_TO_RIGHT)
    AddTextOptionST("readmestrip", "$Achr_ReadMe", "")
    AddTextOptionST("defaultstrip", "$Achr_RestoreDefaults", "")
    AddHeaderOption("$Achr_Stripping")
    AddHeaderOption("")
    int strips = GetSettingInt("iStrips")
    int i = 0
    While(i < 32)
      int flag = OPTION_FLAG_NONE
      If(i == 9 || i == 20 || i == 21 || i == 31)
        flag = OPTION_FLAG_DISABLED
      EndIf
      int bit = Math.LeftShift(1, i)
      AddToggleOptionST("strips_" + i, "$Achr_Strips_" + i, Math.LogicalAnd(strips, bit), flag)
      i += 1
    EndWhile

  ElseIf(page == "$Achr_Events")
    _events = GetEvents(_type)
    SetCursorFillMode(LEFT_TO_RIGHT)
    AddMenuOptionST("viewingevents", "$Achr_EventView", _typesList[_type])
    AddTextOptionST("readmeevents", "$Achr_ReadMe", "")
    AddHeaderOption("")
    AddHeaderOption("")
    If(!_events.Length)
      AddTextOption("$Achr_NoEvent", "", OPTION_FLAG_DISABLED)
      return
    EndIf
    int i = 0
    int pi = -1
    While(i < _events.Length)
      String[] args = StringUtil.Split(_events[i], ";")
      String w = args[1]
      int p = StringUtil.GetNthChar(args[0], 1) as int
      If (pi < p && i % 2 == 1)
        AddEmptyOption()
      EndIf
      AddSliderOptionST("event_" + i, args[0], w as int, "{0}")
      pi = p
      i += 1
    EndWhile
  EndIf
EndEvent

; --------------------- State Options

Event OnSelectST()
  String[] s = StringUtil.Split(GetState(), "_")
  ; --------------- General
  If(s[0] == "enabled")
    bool disabled = Acheron.IsProcessingDisabled()
    Acheron.DisableProcessing(!disabled)
    SetToggleOptionValueST(disabled)
  ElseIf(s[0] == "consequence")
    If(!ShowMessage("$Achr_ConsequenceConfirm"))
      return
    EndIf
    bool disabled = Acheron.IsConsequenceDisabled()
    Acheron.DisableConsequence(!disabled)
    SetToggleOptionValueST(disabled)
  ElseIf(s[0] == "creatures")
    Toggle("bCreatureDefeat")
  ElseIf(s[0] == "killmoves")
    Toggle("bKillMove")
    UpdateKillmoveGlobal()
  ElseIf(s[0] == "notifydefeat")
    Toggle("bNotifyDefeat")
    bool flag = GetSettingBool("bNotifyDefeat") || GetSettingBool("bNotifyDestroy")
    SetOptionFlagsST(getFlag(flag), true, "notifycolored")
    SetOptionFlagsST(getFlag(flag && GetSettingBool("bNotifyColored")), false, "notifycolorchoice")
  ElseIf(s[0] == "notifydestroy")
    Toggle("bNotifyDestroy")
    bool flag = GetSettingBool("bNotifyDefeat") || GetSettingBool("bNotifyDestroy")
    SetOptionFlagsST(getFlag(flag), true, "notifycolored")
    SetOptionFlagsST(getFlag(flag && GetSettingBool("bNotifyColored")), false, "notifycolorchoice")
  ElseIf(s[0] == "notifycolored")
    Toggle("bNotifyColored")
    bool flag = GetSettingBool("bNotifyDefeat") || GetSettingBool("bNotifyDestroy")
    SetOptionFlagsST(getFlag(flag && GetSettingBool("bNotifyColored")), false, "notifycolorchoice")
  ElseIf(s[0] == "hunterpridefollower")
    Toggle("bHunterPrideFollower")
  ; --------------- Defeat
  ElseIf(s[0] == "lethalessential")
    Toggle("bLethalEssential")
  ElseIf(s[0] == "traumenable")
    Toggle("bTraumaEnabled")
  ElseIf(s[0] == "traumhealth")
    Toggle("bTraumaHealth")
  ElseIf(s[0] == "stripdrop")
    Toggle("bStripDrop")
  ElseIf(s[0] == "kdplayer")
    Toggle("bPlayerDefeat")
  ElseIf(s[0] == "kdfollower")
    Toggle("bFollowerDefeat")
  ElseIf(s[0] == "kdfolwithplayer")
    Toggle("bFolWithPlDefeat")
  ElseIf(s[0] == "kdnpc")
    Toggle("bNPCDefeat")
  ElseIf(s[0] == "NPCRescueReload")
    Toggle("bNPCRescueReload")
  ; --------------- Stripping
  ElseIf(s[0] == "readmestrip")
    ShowMessage("$Achr_StripReadMe", false, "$Achr_Ok")
  ElseIf(s[0] == "defaultstrip")
    If(!ShowMessage("$Achr_StripsDefaultsMsg"))
      return
    EndIf
    SetSettingInt("iStrips", 1066390941)
    ForcePageReset()
  ElseIf(s[0] == "strips")
    int strips = GetSettingInt("iStrips")
    int i = s[1] as int
    int bit = Math.LeftShift(1, i)
    strips = Math.LogicalXor(strips, bit)
    SetToggleOptionValueST(Math.LogicalAnd(strips, bit))
    SetSettingInt("iStrips", strips)
  ; --------------- Events
  ElseIf(s[0] == "readmeevents")
    ShowMessage("$Achr_EventReadMe", false, "$Achr_Ok")
  EndIf
EndEvent

Event OnSliderOpenST()
	String[] s = StringUtil.Split(GetState(), "_")
  ; --------------- Defeat
	If(s[0] == "lethalplayer")
		SetSliderDialogStartValue(GetSettingFloat("fLethalPlayer"))
		SetSliderDialogDefaultValue(100)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(0.5)
	ElseIf(s[0] == "lethalfollower")
		SetSliderDialogStartValue(GetSettingFloat("fLethalFollower"))
		SetSliderDialogDefaultValue(30)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(0.5)
	ElseIf(s[0] == "lethalnpc")
		SetSliderDialogStartValue(GetSettingFloat("fLethalNPC"))
		SetSliderDialogDefaultValue(30)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(0.5)
	ElseIf(s[0] == "traumamult")
		SetSliderDialogStartValue(GetSettingFloat("fTraumaMult"))
		SetSliderDialogDefaultValue(32)
		SetSliderDialogRange(5, 200)
		SetSliderDialogInterval(5)
	ElseIf(s[0] == "traumabackatt")
		SetSliderDialogStartValue(GetSettingFloat("fTraumeBackAttack"))
		SetSliderDialogDefaultValue(2)
		SetSliderDialogRange(1, 5)
		SetSliderDialogInterval(0.2)
	ElseIf(s[0] == "stripchance")
		SetSliderDialogStartValue(GetSettingFloat("fStripChance"))
		SetSliderDialogDefaultValue(7)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(0.5)
	ElseIf(s[0] == "stripdstry")
		SetSliderDialogStartValue(GetSettingFloat("fStripDestroy"))
		SetSliderDialogDefaultValue(5)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(0.5)
	ElseIf(s[0] == "exposed")
		SetSliderDialogStartValue(GetSettingInt("iExposed"))
		SetSliderDialogDefaultValue(2)
		SetSliderDialogRange(0, 20)
		SetSliderDialogInterval(1)
	ElseIf(s[0] == "exposedchance")
		SetSliderDialogStartValue(GetSettingFloat("fExposedChance"))
		SetSliderDialogDefaultValue(75)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(0.5)
	ElseIf(s[0] == "midcmbtblackout")
		SetSliderDialogStartValue(GetSettingFloat("fMidCombatBlackout"))
		SetSliderDialogDefaultValue(30)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(0.5)
	ElseIf(s[0] == "recoverhealththresh")
		SetSliderDialogStartValue(GetSettingFloat("fKdHealthThresh") * 100)
		SetSliderDialogDefaultValue(30)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(2.5)
	ElseIf(s[0] == "recoverhealfallback")
		SetSliderDialogStartValue(GetSettingInt("iKdFallbackTimer"))
		SetSliderDialogDefaultValue(90)
		SetSliderDialogRange(0, 300)
		SetSliderDialogInterval(5)
  ; --------------- Events
  ElseIf(s[0] == "event")
    int i = s[1] as int
    string e = _events[i]
    int w = GetEventWeight(e)
		SetSliderDialogStartValue(w)
		SetSliderDialogDefaultValue(50)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(1)
  EndIf
EndEvent

Event OnSliderAcceptST(float value)
	string[] s = StringUtil.Split(GetState(), "_")
  ; --------------- Defeat
	If(s[0] == "lethalplayer")
		SetSettingFloat("fLethalPlayer", value)
		SetSliderOptionValueST(value, "{1}%")
	ElseIf(s[0] == "lethalfollower")
		SetSettingFloat("fLethalFollower", value)
		SetSliderOptionValueST(value, "{1}%")
	ElseIf(s[0] == "lethalnpc")
		SetSettingFloat("fLethalNPC", value)
		SetSliderOptionValueST(value, "{1}%")
	ElseIf(s[0] == "traumamult")
		SetSettingFloat("fTraumaMult", value)
		SetSliderOptionValueST(value, "{1}")
	ElseIf(s[0] == "traumabackatt")
		SetSettingFloat("fTraumeBackAttack", value)
		SetSliderOptionValueST(value, "{1}")
	ElseIf(s[0] == "stripchance")
		SetSettingFloat("fStripChance", value)
		SetSliderOptionValueST(value, "{1}%")
	ElseIf(s[0] == "stripdstry")
		SetSettingFloat("fStripDestroy", value)
		SetSliderOptionValueST(value, "{1}%")
	ElseIf(s[0] == "exposed")
		SetSettingInt("iExposed", value as int)
		SetSliderOptionValueST(value, "{0}")
	ElseIf(s[0] == "exposedchance")
		SetSettingFloat("fExposedChance", value)
		SetSliderOptionValueST(value, "{1}%")    
	ElseIf(s[0] == "midcmbtblackout")
		SetSettingFloat("fMidCombatBlackout", value)
		SetSliderOptionValueST(value, "{1}%")  
	ElseIf(s[0] == "recoverhealththresh")
		SetSettingFloat("fKdHealthThresh", value / 100)
		SetSliderOptionValueST(value, "{1}%")
	ElseIf(s[0] == "recoverhealfallback")
		SetSettingInt("iKdFallbackTimer", value as int)
		SetSliderOptionValueST(value, "{0}s")
  ; --------------- Events
  ElseIf(s[0] == "event")
    int i = s[1] as int
    string e = _events[i]
    int w = GetEventWeight(e)
    If (value as int == w)
      return
    EndIf
    _events[i] = SetEventWeight(e, _type, value as int)
		SetSliderOptionValueST(value, "{0}")
  EndIf
EndEvent

Event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
  String[] s = StringUtil.Split(GetState(), "_")
  If(newKeyCode == 1 || newKeyCode == 277)
    newKeyCode = -1
  EndIf
  If(s[0] == "hunterpridemodkey")
    SetSettingInt("iHunterPrideKeyMod", newKeyCode)
  Else
    If(newKeyCode != -1 && conflictControl != "")
      string msg
      If(conflictName != "")
        msg = "$Achr_ConflictControl{" + conflictControl + "}{" + conflictName + "}"
      Else
        msg = "$Achr_ConflictControl{" + conflictControl + "}"
      EndIf
      If(!ShowMessage(msg, true, "$Yes", "$No"))
        return
      EndIf
    EndIf
    If(s[0] == "hunterpridekey")
      SetSettingInt("iHunterPrideKey", newKeyCode)
    EndIf
	EndIf
  SetKeyMapOptionValueST(newKeyCode)
EndEvent

Event OnHighlightST()
  String[] s = StringUtil.Split(GetState(), "_")
  ; --------------- General
  If(s[0] == "enabled")
    SetInfoText("$Achr_EnabledHighlight")
  ElseIf(s[0] == "consequence")
    SetInfoText("$Achr_ConsequenceHighlight")
  ElseIf(s[0] == "creatures")
    SetInfoText("$Achr_CreaturesHighlight")
  ElseIf(s[0] == "killmoves")
    SetInfoText("$Achr_KillmovesHighlight")
  ElseIf(s[0] == "notifydefeat")
    SetInfoText("$Achr_NotifyDefeatHighlight")
  ElseIf(s[0] == "notifydestroy")
    SetInfoText("$Achr_NotifyDestryHighlight")
  ElseIf(s[0] == "notifycolored")
    SetInfoText("$Achr_NotifyColoredHighlight")
  ElseIf(s[0] == "hunterpridekey")
    SetInfoText("$Achr_HunterPrideKeyHighlight")
  ElseIf(s[0] == "hunterpridemodkey")
    SetInfoText("$Achr_ModifierKeyHighlight")
  ElseIf(s[0] == "hunterpridefollower")
    SetInfoText("$Achr_HunterPrideFollowerHighlight")
  ; --------------- Defeat
  ElseIf(s[0] == "lethalessential")
    SetInfoText("$Achr_LethalEssentialHighlight")
  ElseIf(s[0] == "lethalplayer")
    SetInfoText("$Achr_LethalPlayerHighlight")
  ElseIf(s[0] == "lethalfollower")
    SetInfoText("$Achr_LethalFollowerHighlight")
  ElseIf(s[0] == "lethalnpc")
    SetInfoText("$Achr_LethalNPCHighlight")
  ElseIf(s[0] == "traumenable")
    SetInfoText("$Achr_TraumaEnabHighlight")
  ElseIf(s[0] == "traumhealth")
    SetInfoText("$Achr_TraumaHealthHighlight")
  ElseIf(s[0] == "traumamult")
    SetInfoText("$Achr_TraumaMultHighlight")
  ElseIf(s[0] == "traumabackatt")
    SetInfoText("$Achr_TraumaBackAttackHighlight")
  ElseIf(s[0] == "stripchance")
    SetInfoText("$Achr_StripChanceHighlight")
  ElseIf(s[0] == "stripdstry")
    SetInfoText("$Achr_StripDstryHighlight")
  ElseIf(s[0] == "stripdrop")
    SetInfoText("$Achr_StripDropHighlight")
  ElseIf(s[0] == "exposed")
    SetInfoText("$Achr_StripReqHighlight")
  ElseIf(s[0] == "exposedchance")
    SetInfoText("$Achr_StripReqChanceHighlight")
  ElseIf(s[0] == "midcmbtblackout")
    SetInfoText("$Achr_DefeatEndCmtHighlight")
  ElseIf(s[0] == "kdplayer")
    SetInfoText("$Achr_KdPlayerHighlight")
  ElseIf(s[0] == "kdfollower")
    SetInfoText("$Achr_KdFollowerHighlight")
  ElseIf(s[0] == "kdfolwithplayer")
    SetInfoText("$Achr_KdFolWithplayerHighlight")
  ElseIf(s[0] == "kdnpc")
    SetInfOText("$Achr_KdNPCHighlight")
  ElseIf(s[0] == "recoverhealththresh")
    SetInfoText("$Achr_RecHealThreshHighlight")
  ElseIf(s[0] == "recoverhealfallback")
    SetInfoText("$Achr_RecHealFallbackHighlight")
  ElseIf(s[0] == "NPCRescueReload")
    SetInfoText(("$Achr_NPCRescueReloadHighlight"))
  ; --------------- Stripping
  ElseIf(s[0] == "strips")
    int i = s[1] as int
    int bit = Math.LeftShift(1, i)
    Form worn = Game.GetPlayer().GetWornForm(bit)
    String name
    If(!worn)
      name = "---"
    Else
      name = worn.GetName()
      If(name == "" || name == " ")
        name = "---"
      EndIf
    EndIf
    SetInfoText("$Achr_StripsHighlight{" + name + "}")
  EndIf
EndEvent

; --------------------- Default State

State FollowerRescue
  Event OnMenuOpenST()
    SetMenuDialogStartIndex(GetSettingInt("iFollowerRescue"))
    SetMenuDialogDefaultIndex(1)
    SetMenuDialogOptions(_FollowerRescue)
  EndEvent
  Event OnMenuAcceptST(Int aiIndex)
    SetSettingInt("iFollowerRescue", aiIndex)
    SetMenuOptionValueST(_FollowerRescue[aiIndex])
  EndEvent
  Event OnDefaultST()
    SetSettingInt("iFollowerRescue", 1)
    SetMenuOptionValueST(_FollowerRescue[1])
  EndEvent
  Event OnHighlightST()
    SetInfoText("$Achr_FollowerRescueHighlight")
  EndEvent
EndState

State viewingevents
  Event OnMenuOpenST()
    SetMenuDialogStartIndex(_type)
    SetMenuDialogDefaultIndex(0)
    SetMenuDialogOptions(_typesList)
  EndEvent
  Event OnMenuAcceptST(Int aiIndex)
    _type = aiIndex
    ForcePageReset()
  EndEvent
  Event OnDefaultST()
    _type = 0
    ForcePageReset()
  EndEvent
  Event OnHighlightST()
    SetInfoText("$Achr_EventViewHighlight")
  EndEvent
EndState

State notifycolorchoice
	Event OnColorOpenST()
		SetColorDialogStartColor(GetSettingColor("rNotifyColor"))
		SetColorDialogDefaultColor(0xFF0000)
	EndEvent
	Event OnColorAcceptST(int color)
    SetSettingColor("rNotifyColor", color)
		SetColorOptionValueST(color)
	EndEvent
  Event OnHighlightST()
    SetInfoText("$Achr_NotifyColorChoiceHighlight")
  EndEvent
EndState

; --------------------- Misc

Function Toggle(String asSetting)
  bool b = GetSettingBool(asSetting)
  SetSettingBool(asSetting, !b)
  SetToggleOptionValueST(!b)
EndFunction

String Function GetCustomControl(int keyCode)
	If(keyCode == GetSettingInt("iHunterPrideKey"))
    return "Hunter's Pride"
  Else
		return ""
	EndIf
EndFunction

int Function getFlag(bool option)
	If(option)
		return OPTION_FLAG_NONE
  Else
		return OPTION_FLAG_DISABLED
	EndIf
EndFunction
