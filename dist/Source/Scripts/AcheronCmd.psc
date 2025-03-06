ScriptName AcheronCmd Hidden

String Function ActorVecAsString(Actor[] arr) global
  String ret = "[\n"
  Int i = 0
  While (i < arr.Length)
    ret += "\t" + arr[i].GetFormID() + "\n"
    i += 1
  EndWhile
  return ret + "]"
EndFunction

bool Function RunDefeat(ObjectReference akTarget) global
  Actor act = akTarget as Actor
  If (act == None)
    return 0
  EndIf
  Acheron.DefeatActor(act)
  return 1
EndFunction

bool Function RunRescue(ObjectReference akTarget, bool abAndRelease) global
  Actor act = akTarget as Actor
  If (act == None)
    return 0
  EndIf
  Acheron.RescueActor(act, abAndRelease)
  return 1
EndFunction

bool Function RunIsDefeated(ObjectReference akTarget) global
  Actor act = akTarget as Actor
  If (act == None)
    return 0
  EndIf
  return Acheron.IsDefeated(act)
EndFunction

bool Function RunGetDefeated() global
  Actor[] act = Acheron.GetDefeated()
  ConsoleUtil.PrintConsole(ActorVecAsString(act))
  return 1
EndFunction

bool Function RunPacify(ObjectReference akTarget) global
  Actor act = akTarget as Actor
  If (act == None)
    return 0
  EndIf
  Acheron.PacifyActor(act)
  return 1
EndFunction

bool Function RunRelease(ObjectReference akTarget) global
  Actor act = akTarget as Actor
  If (act == None)
    return 0
  EndIf
  Acheron.ReleaseActor(act)
  return 1
EndFunction

bool Function RunIsPacified(ObjectReference akTarget) global
  Actor act = akTarget as Actor
  If (act == None)
    return 0
  EndIf
  return Acheron.IsPacified(act)
EndFunction

bool Function RunGetPacified() global
  Actor[] act = Acheron.GetPacified()
  ConsoleUtil.PrintConsole(ActorVecAsString(act))
  return 1
EndFunction
