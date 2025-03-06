Scriptname AcheronHunterPride extends Quest Hidden 

Function OpenHunterPrideMenu(Actor akTarget) native

PlayerVampireQuestScript Property PlayerVampireQuest Auto
Activator Property HealTargetFX Auto
Actor Property PlayerRef Auto

Message Property IsEssentialMsg Auto
Message Property NoPotionMsg Auto

Function OpenMenu(Actor akTarget)
  Acheron.RegisterForHunterPrideSelect(self)
  OpenHunterPrideMenu(akTarget)
EndFunction

Event OnHunterPrideSelect(int aiOptionID, Actor akTarget)
  Debug.Trace("OnHunterPrideSelect: " + aiOptionID + " / " + akTarget)
  If(aiOptionID == 0)
    Potion p = Acheron.GetMostEfficientPotion(akTarget, PlayerRef)
    If(!p)
      NoPotionMsg.Show()
      return
    EndIf
    PlayerRef.RemoveItem(p, 1, true, akTarget)
    akTarget.EquipItem(p, false, true)
    If(Acheron.IsDefeated(akTarget))
      ; Some actors are immune to potion effect
      akTarget.PlaceAtMe(HealTargetFX)
      akTarget.RestoreActorValue("Health", 25.0)
      Acheron.RescueActor(akTarget, true)
    EndIf
  ElseIf(aiOptionID == 1)
    akTarget.OpenInventory(true)
    akTarget.SendAssaultAlarm()
  ElseIf(aiOptionID == 2)
    If(CheckEssential(akTarget))
      return
    EndIf
    ; TODO: Add Animation
    akTarget.Kill(PlayerRef)
  ElseIf(aiOptionID == 3)
    If(CheckEssential(akTarget))
      return
    EndIf
    PlayerRef.StartVampireFeed(akTarget)
    PlayerVampireQuest.VampireFeed()
    akTarget.Kill(PlayerRef)
  EndIf
EndEvent

bool Function CheckEssential(Actor akTarget)
  ActorBase base = akTarget.GetLeveledActorBase()
  If(!base.IsEssential())
    return false
  EndIf
  If(IsEssentialMsg.Show() == 1)
    base.SetEssential(false)
    return false
  EndIf
  return true
EndFunction
