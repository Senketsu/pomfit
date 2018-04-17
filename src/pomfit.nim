import os, strutils
import private/projUtils
import private/projTypes
import private/gui
import private/uploader


when defined(Windows):
  discard
else:
  discard


var
  threadMain: Thread[int]
  threadUploader: Thread[int]
  channelMain: StringChannel
  channelUploader: StringChannel



proc threadUploaderStart (threadID: int) {.thread.} =
  echoInfo("Uploader\t- initializing..")
  let chanMain = channelMain.addr
  let chanUp = channelUploader.addr 

  sleep(1000)
  uploader.idle(chanMain, chanUp)


proc threadMainStart(threadID: int) {.thread.} =
  echoInfo("Pomfit Gui\t- initializing..")
  let chanMain = channelMain.addr
  let chanUp = channelUploader.addr 

  gui.start(chanMain, chanUp)
  

proc launch() =
  echoInfo("\t*** Pomfit starting***")
  if not checkDirectories():
    echoInfo("Quitting...")
    quit()
  
  channelMain.open()
  createThread(threadMain, threadMainStart, 0)
  channelUploader.open()
  createThread(threadUploader, threadUploaderStart, 1)

  joinThreads(threadMain, threadUploader)

  channelMain.close()
  channelUploader.close()

when isMainModule: pomfit.launch()