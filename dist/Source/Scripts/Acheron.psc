Scriptname Acheron Hidden
{ Global API for Acheron }

; Disable/Enable hit processing
Function DisableProcessing(bool abDisable) native global
bool Function IsProcessingDisabled() native global
; Disable/Enable consequence events
Function DisableConsequence(bool abDisable) native global
bool Function IsConsequenceDisabled() native global
; If teleport events are currently permitted as by Acherons config data
bool Function IsTeleportAllowed() native global

; ================================ Defeat
; A defeated actor is bleeding out and immune to damage
; A defeated actor is always pacified
; A pacified actor is ignoring & ignored by combat
; All defeated actors carry the "AcheronDefeated" keyword
; All pacified actors carry the "AcheronPacified" keyword
Function DefeatActor(Actor akActor) native global
Function RescueActor(Actor akActor, bool abRelease = true) native global
Function PacifyActor(Actor akActor) native global
Function ReleaseActor(Actor akActor) native global
bool Function IsDefeated(Actor akActor) native global
bool Function IsPacified(Actor akActor) native global

; get all currently defeated actors
Actor[] Function GetDefeated(bool abLoadedOnly = false) native global
Actor[] Function GetPacified(bool abLoadedOnly = false) native global

; Invoked whenever an actor is defeated
Function RegisterForActorDefeated(Form akForm) native global
Function UnregisterForActorDefeated(Form akForm) native global
Function RegisterForActorDefeated_Alias(ReferenceAlias akAlias) native global
Function UnregisterForActorDefeated_Alias(ReferenceAlias akAlias) native global
Function RegisterForActorDefeated_MgEff(ActiveMagicEffect akEffect) native global
Function UnregisterForActorDefeated_MgEff(ActiveMagicEffect akEffect) native global
Event OnActorDefeated(Actor akVictim)
EndEvent

; Invoked whenever an actor is rescued (recovering from defeat)
Function RegisterForActorRescued(Form akForm) native global
Function UnregisterForActorRescued(Form akForm) native global
Function RegisterForActorRescued_Alias(ReferenceAlias akAlias) native global
Function UnregisterForActorRescued_Alias(ReferenceAlias akAlias) native global
Function RegisterForActorRescued_MgEff(ActiveMagicEffect akEffect) native global
Function UnregisterForActorRescued_MgEff(ActiveMagicEffect akEffect) native global
Event OnActorRescued(Actor akVictim)
EndEvent

; Invoked when a death event is started (these only fire for the player) global native
Function RegisterForPlayerDeathEvent(Form akForm) native global
Function UnregisterForPlayerDeathEvent(Form akForm) native global
Function RegisterForPlayerDeathEvent_Alias(ReferenceAlias akAlias) native global
Function UnregisterForPlayerDeathEvent_Alias(ReferenceAlias akAlias) native global
Function RegisterForPlayerDeathEvent_MgEff(ActiveMagicEffect akEffect) native global
Function UnregisterForPlayerDeathEvent_MgEff(ActiveMagicEffect akEffect) native global
Event OnPlayerDeathEvent()
EndEvent

; ================================ Hunter Pride
; 'Hunter Pride' is the menu called when the player activates a defeated victim. The menu can be dynamically expanded with the below functions
; When registering a new option, use the callback event to be notified when your option is being selected
; See https://github.com/Scrabx3/Acheron/wiki/Hunter-Pride for a more throughout documentation
; --- Params:
; asOptionID:   An ID-String with the option to add. Has to be unique among all options, recommended to prefix
; asOptionName: A human readable, descriptive name for your option. Shorter is better, supports localization
; asIconSource: A file path to an icon that is used for the option, relative to 'Interface\\Acheron'
; asConditions: A json string of conditions, stating when the option is available/disabled
; -- Return:
; The associated integer representing this option, or -1 if the option could not be added
int Function AddOption(String asOptionID, String asOptionName, String asIconSource, String asConditions = "") native global
bool Function RemoveOption(String asOptionID) native global
bool Function HasOption(String asOptionID) native global
int Function GetOptionID(String asOptionID) native global

; Invoked whenever an option is selected
Function RegisterForHunterPrideSelect(Form akForm) native global
Function UnregisterForHunterPrideSelect(Form akForm) native global
Function RegisterForHunterPrideSelect_Alias(ReferenceAlias akAlias) native global
Function UnregisterForHunterPrideSelect_Alias(ReferenceAlias akAlias) native global
Function RegisterForHunterPrideSelect_MgEff(ActiveMagicEffect akEffect) native global
Function UnregisterForHunterPrideSelect_MgEff(ActiveMagicEffect akEffect) native global
Event OnHunterPrideSelect(int aiOptionID, Actor akTarget)
EndEvent

; ================================ ObjectReference
; Link source to target with the given keyword. Setting 'target' to 'none' unsets the Link
Function SetLinkedRef(ObjectReference akSource, ObjectReference akTarget, Keyword akLink = none) native global
; Similar to ObjectRef.RemoveAll but will always skip quest items & can be set to ignore worn armor
Function RemoveAllItems(ObjectReference akTransferFrom, ObjectReference akTransferTo = none, bool abExcludeWorn = true) native global
; Get a list of all items in the given reference container which have any of the specified keywords AND a value of at least aiMinValue
Form[] Function GetItemsByKeywords(ObjectReference akContainer, Keyword[] akKeywords, int aiMinValue = 0, bool abIgnoreQuestItems = true) native global

; ================================ Actor
; return all armor this actor is currently wearing, excluding those covered by ignored slot masks
Armor[] Function GetWornArmor(Actor akActor, int aiIgnoredMasks = 0) native global
; strip all of this actors worn armor, excluding those covered by ignored slot masks or using a "nostrip" keyword
Armor[] Function StripActor(Actor akActor, int aiIgnoredFlags = 0) global native
; Get the most efficien Potion (= the Potion which gets the Hp closest to max) for this actor from the given container
; The function recognizes all Healing Potions in the container inventory which are purely beneficial
Potion Function GetMostEfficientPotion(Actor akActor, ObjectReference akContainer) native global
; Return all of the Players currently loaded followers
Actor[] Function GetFollowers() native global
; Get the race type of the given Actor. Race Types are used to bundle difference of Races together, 
; Example: Snow, Brown and Cave bears are race type 'Bear', Dogs and Wolves both are race type 'Wolf'
String Function GetRaceType(Actor akActor) native global

; ================================ Utility
; Write a message to the game console
Function PrintConsole(String asMsg) global native
; Similar to SKSE's custom Menu but will not pause the game while open. menuName: 'AcheronCustomMenu'
; Filepath relative to the Interface folder, without extension
bool Function OpenCustomMenu(String asFilePath) native global
; Close the custom menu if its currently open
Function CloseCustomMenu() native global
