import nsu
import times

proc fileChooserKb*(key: cstring, data: pointer) {.procvar.} =
  pfcStart(nil, nil)

proc takeScreenShot*(widget: PWidget, data: Pgpointer) {.procvar.} =
  var
    mode: SSMode = cast[SSMode](data)
    resFilePath: string = ""
    saveTo: string = pdbConn.getSettings(ord(SS_PATH))
    fileName: string = pdbConn.getSettings(ord(SS_NAME))
    timeStampFormat: string = pdbConn.getSettings(ord(SS_TS))
    timeStamp: string = ""
    iTimeNow: int = (int)getTime()
    timeNowUtc = utc(fromUnix(iTimeNow))
  if timeStampFormat != "":
    timeStamp = format(timeNowUtc, timeStampFormat)
  else:
    timeStamp = format(timeNowUtc, "yyyy-MM-dd-HH'_'mm'_'ss")
  fileName.add(("_$1" % [timeStamp]))
  fileName.add(".png")

  case mode
  of FULL_CAP:
    resFilePath = nsu_full_ss(fileName, saveTo)
  of FULL_UP:
    resFilePath = nsu_full_ss(fileName, saveTo)
  of AREA_CAP:
    resFilePath = nsu_area_ss(fileName, saveTo)
  of AREA_UP:
    resFilePath = nsu_area_ss(fileName, saveTo)
  of Win_CAP:
    resFilePath = nsu_select_win_ss(fileName, saveTo)
  of WIN_UP:
    resFilePath = nsu_select_win_ss(fileName, saveTo)
  of AWIN_CAP:
    resFilePath = nsu_active_win_ss(fileName, saveTo)
  of AWIN_UP:
    resFilePath = nsu_active_win_ss(fileName, saveTo)
  else:
    return
  
  if resFilePath == "":
    infoUser(winMain, ERR, "Failed to take screenshot.")
    return
  
  if $mode != "":
    if InstantUpload:
      discard
    else:
      discard pdbConn.addQueueData(resFilePath)
      pbUpdateIdle()


proc takeScreenShotKb*(keystring: cstring, data: pointer) {.procvar.} =
  var mode = cast[SSMode](data)
  var data = cast[Pgpointer](mode)
  takeScreenShot(nil, data)  

