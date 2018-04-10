const iSettOpt = 10
var settEntry: array[iSettOpt, PEntry]



proc settingsSave(widget: PWidget, data: Pgpointer) =
  var window = WINDOW(data)
  var settingsData: seq[TPomfitSett] = @[]
  for i in 0..iSettOpt-1:
    var data: TPomfitSett
    data.id = i
    data.name = $(cast[SettingsEnum](i))
    data.value = $settEntry[i].get_text()
    settingsData.add(data)
  
  if not pdbConn.setSettingsAll(settingsData):
    infoUser(window, ERR, "Failed to save settings.")
    

proc settingsReset(widget: PWidget, data: Pgpointer) =
  var window = WINDOW(data)
  if not pdbConn.resetSettings():
    infoUser(window, ERR, "Failed to reset settings.")


proc settingsLoad() =
  var settings = pdbConn.getSettingsAll()
  for item in settings:
    settEntry[item.id].set_text(cstring(item.value))


proc tbToggleLabel(widget: PWidget, data: Pgpointer) =
  let button = TOGGLE_BUTTON(widget)
  if button.get_active():
    button.set_label("ON ")
  else:
    button.set_label("OFF")

proc settingsOpen(widget: PWidget, data: Pgpointer) {.procvar.} =
  
  var label: PLabel
  var winSett = window_new(WINDOW_TOPLEVEL)
  winSett.set_position(WIN_POS_MOUSE)
  winSett.set_border_width(10)
  winSett.set_title("Pomf It ! - Settings")
  winSett.set_default_size(640, 360)
  discard winSett.signal_connect("destroy", SIGNAL_FUNC(closeWindow), winSett)
  winSett.set_icon(piIco)

  var vbMain = vbox_new(false, 0)
  vbMain.set_size_request(-1, -1)
  winSett.add(vbMain)

  var hbMain = hbox_new(false , 5)
  # hbMain.set_size_request(-1, -1)
  vbMain.pack_start(hbMain, false, false, 0)
  var vbMainTab: array[4, PVBox]
  for i in 0..3:
    vbMainTab[i] = vbox_new(true, 0)
    hbMain.pack_start(vbMainTab[i], false, false, 0)
  
  vbMainTab[0].set_size_request(150, -1)
  vbMainTab[1].set_size_request(150, -1)
  vbMainTab[2].set_size_request(150, -1)
  vbMainTab[3].set_size_request(150, -1)
  
  var descLabel: array[iSettOpt, PLabel]
  for i in 0..iSettOpt-1:
    descLabel[i] = label_new("")
    settEntry[i] = entry_new()
    if i < 5:
      vbMainTab[0].pack_start(descLabel[i], false, false, 0)
      vbMainTab[1].pack_start(settEntry[i], false, false, 0)
    else:
      vbMainTab[2].pack_start(descLabel[i], false, false, 0)
      vbMainTab[3].pack_start(settEntry[i], false, false, 0)
      
  descLabel[0].set_text($SS_PATH)
  descLabel[0].set_tooltip_text("Where to save your screenshots")
  descLabel[1].set_text($SS_NAME)
  descLabel[1].set_tooltip_text("Custom filename for your screenshots")
  descLabel[2].set_text($SS_TS)
  descLabel[2].set_tooltip_text("Timestamp format string\ndefault: 'yyyy-MM-dd-HH'_'mm'_'ss'")
  descLabel[3].set_text("4")
  descLabel[3].set_tooltip_text("")
  descLabel[4].set_text("5")
  descLabel[4].set_tooltip_text("")

  descLabel[5].set_text("6")
  descLabel[5].set_tooltip_text("")
  descLabel[6].set_text("7")
  descLabel[6].set_tooltip_text("")
  descLabel[7].set_text("8")
  descLabel[7].set_tooltip_text("")
  descLabel[8].set_text("9")
  descLabel[8].set_tooltip_text("")
  descLabel[9].set_text("10")
  descLabel[9].set_tooltip_text("")

  var hbControl = hbox_new(false, 5)
  hbControl.set_size_request(-1, 30)
  vbMain.pack_end(hbControl, false, false, 0)

  var btnReset = button_new("Reset")
  discard OBJECT(btnReset).signal_connect("clicked",
   SIGNAL_FUNC(settingsReset), winSett)
  btnReset.set_size_request(90, 30)
  hbControl.pack_start(btnReset, false, false, 0)

  var btnClose = button_new_from_stock(STOCK_CLOSE)
  discard OBJECT(btnClose).signal_connect("clicked",
   SIGNAL_FUNC(closeWindow), winSett)
  btnClose.set_size_request(90, 30)
  hbControl.pack_end(btnClose, false, false, 0)

  var btnSave = button_new_from_stock(STOCK_APPLY)
  discard OBJECT(btnSave).signal_connect("clicked",
   SIGNAL_FUNC(settingsSave), winSett)
  btnSave.set_size_request(90, 30)
  hbControl.pack_end(btnSave, false, false, 10)
  
  settingsLoad()
  winSett.show_all()