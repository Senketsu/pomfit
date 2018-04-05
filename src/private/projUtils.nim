import os, strutils, times

# macro for cOut
let
 cOut*: bool = true
 logEvents*: bool = true
 logError*: bool = true
 logDebug*: bool = false
 logPM*: bool = false
 logMsg*: bool = false
 logAllMsg*:bool = false


proc isNumber*(s: string): bool =
 var i = 0
 while s[i] in {'0'..'9'}: inc(i)
 result = i == s.len and s.len > 0

proc getPath*(name: string): string =
  result = ""
  var
    dirHome = getHomeDir()
    dirMain = joinPath(getConfigDir(), "pomfit")
  case name
  of "dirHome":
    result = dirHome
  of "dirMain":
    result = dirMain
  of "dirData":
    result = joinPath(dirMain, "data")
  of "dirConf":
    result = joinPath(dirMain, "cfg")
  of "dirLog":
    result = joinPath(dirMain, "logs")
  of "fileDB":
    result = joinPath(dirMain, "data/pomfit.db")
  else:
    discard
  
proc echoInfo*(msg: string) =
 var
  isUploader,isGUI,isManager,isDebug,isUnk,isPrompt: bool = false

 if msg.startsWith("Uploader"):
  isUploader = true
 elif msg.startsWith("GUI"):
  isGUI = true
 elif msg.startsWith("Manager"):
  isManager = true
 elif msg.startsWith("Debug"):
  isDebug = true
 elif msg[0] == '*':
  isPrompt = true
 else:
  isUnk = true

 when defined(Windows):
  stdout.writeLine("[Info]: $1" % msg)
 else:
  if cOut:
   if isManager:
    stdout.writeLine("[Info]: \27[0;35m$1\27[0m" % [msg])
   elif isGUI:
    stdout.writeLine("[Info]: \27[0;94m$1\27[0m" % [msg])
   elif isUploader:
    stdout.writeLine("[Info]: \27[0;95m$1\27[0m" % [msg])
   elif isDebug:
    stdout.writeLine("[Info]: \27[0;92m$1\27[0m" % [msg])
   elif isPrompt:
    stdout.writeLine("\27[0;96m$1\27[0m" % [msg])
   else:
    stdout.writeLine("[Info]: \27[0;93m$1\27[0m" % msg)
  else:
   stdout.writeLine("[Info]: $1" % msg)


proc logEvent*(logThis: bool, msg: string) =
 var
  isError,isDebug,isWarn,isNotice,isUnk: bool = false
  fileName: string = ""
  logPath: string = getPath("dirLog")

 if msg.startsWith("***Error"):
  isError = true
  fileName = "Error.log"
 elif msg.startsWith("*Notice"):
  isNotice = true
 elif msg.startsWith("**Warning"):
  isWarn = true
  fileName = "Error.log"
 elif msg.startsWith("*Debug"):
  isDebug = true
  fileName = "Debug.log"
 else:
  isUnk = true

 if cOut:
  if isError:
   stdout.writeLine("\27[1;31m$1\27[0m" % [msg])
  elif isNotice:
   stdout.writeLine("\27[0;34m$1\27[0m" % [msg])
  elif isWarn:
   stdout.writeLine("\27[0;33m$1\27[0m" % [msg])
  elif isDebug:
   stdout.writeLine("\27[0;32m$1\27[0m" % [msg])
  else:
   stdout.writeLine(msg)
 else:
  stdout.writeLine(msg)

 if logEvents and logThis and not isUnk: # TODO finish after config
  var
   tStamp: string = ""
   iTimeNow: int = (int)getTime()
  let timeNewTrackWhen = utc(fromUnix(iTimeNow))
  tStamp = format(timeNewTrackWhen,"[yyyy-MM-dd] (HH:mm:ss)")

  var eventFile: File
  if isError or isDebug or isWarn:
   if eventFile.open(joinPath(logPath,fileName) ,fmAppend):
    eventFile.writeLine("$1: $2" % [tStamp,msg])
    eventFile.flushFile()
    eventFile.close()


proc checkDirectories*(): bool =
  if not existsDir(projUtils.getPath("dirMain")):
    try:
      createDir(projUtils.getPath("dirMain"))
      createDir(projUtils.getPath("dirData"))
      createDir(projUtils.getPath("dirConf"))
      createDir(projUtils.getPath("dirLog"))
      result = true
    except:
      logEvent(true, "***Error: Coudn't create Pomfit directory.\n\t '$1'" % [
        getCurrentExceptionMsg()])
  else:
    result = true

