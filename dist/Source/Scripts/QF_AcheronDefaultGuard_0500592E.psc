;BEGIN FRAGMENT CODE - Do not edit anything between this and the end comment
;NEXT FRAGMENT INDEX 2
Scriptname QF_AcheronDefaultGuard_0500592E Extends Quest Hidden

;BEGIN ALIAS PROPERTY Guard
;ALIAS PROPERTY TYPE ReferenceAlias
ReferenceAlias Property Alias_Guard Auto
;END ALIAS PROPERTY

;BEGIN ALIAS PROPERTY Player
;ALIAS PROPERTY TYPE ReferenceAlias
ReferenceAlias Property Alias_Player Auto
;END ALIAS PROPERTY

;BEGIN FRAGMENT Fragment_0
Function Fragment_0()
;BEGIN CODE
; Simple Quest to jail the Player after a Short delay
Faction crimeF = Alias_Guard.GetActorReference().GetCrimeFaction()

RegisterForSingleUpdate(5)
FadeToBlackImod.Apply()
Utility.Wait(2)

If(crimeF)
  If(crimeF == CrimeFactionSons)
    crimeF = CrimeFactionEastmarch
  ElseIf(crimeF == CrimeFactionImperial)
    crimeF = CrimeFactionHaafingar
  EndIf
  crimeF.SendPlayerToJail()
EndIf
;END CODE
EndFunction
;END FRAGMENT

;BEGIN FRAGMENT Fragment_1
Function Fragment_1()
;BEGIN CODE
Actor player = Alias_Player.GetReference() as Actor
Acheron.RescueActor(player, false)
Utility.Wait(3)
Acheron.ReleaseActor(player)

Stop()
;END CODE
EndFunction
;END FRAGMENT

;END FRAGMENT CODE - Do not edit anything between this and the begin comment

Event OnUpdate()
  SetStage(25)
EndEvent

ImageSpaceModifier Property FadeToBlackImod  Auto  

Faction Property CrimeFactionSons  Auto  

Faction Property CrimeFactionImperial  Auto  

Faction Property CrimeFactionEastmarch  Auto  

Faction Property CrimeFactionHaafingar  Auto  
