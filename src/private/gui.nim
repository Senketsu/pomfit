import os, strutils
import projTypes
import projUtils


## TODO: Add hybrid gui and import
##      Consider native win32 gui (bleh, don't do it pls)
when defined(guiWin32):
  discard
elif defined(guiHybrid):
  discard 
else:
  import gui_gtk


proc start*(chanMain, chanUp: ptr StringChannel) =
  try:
    when defined(guiWin32):
      echo "No windows GUI"
    elif defined(guiHybrid):
      echo "No hybrid GUI yet"
    else:
      gui_gtk.createMainWin(chanMain, chanUp)
  except:
    echoInfo("***Error: Starting GUI failed.\n\t'$1'" % [getCurrentExceptionMsg()])


