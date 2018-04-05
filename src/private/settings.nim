
proc settingsSave(widget: PWidget, data: Pgpointer) =
  echoInfo("*Debug: TODO: Save settings")

proc settingsReset(widget: PWidget, data: Pgpointer) =
  echoInfo("*Debug: TODO: Reset settings")

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
  
  var
    descLabel: array[6, PLabel]
    tbOption: array[6, PToggleButton]
  for i in 0..5:
    descLabel[i] = label_new("")
    tbOption[i] = toggle_button_new("OwO") ## REMOVE
    discard tbOption[i].signal_connect("toggled", SIGNAL_FUNC(tbToggleLabel), nil)
    if i < 3:
      vbMainTab[0].pack_start(descLabel[i], false, false, 0)
      vbMainTab[1].pack_start(tbOption[i], false, false, 0)
    else:
      vbMainTab[2].pack_start(descLabel[i], false, false, 0)
      vbMainTab[3].pack_start(tbOption[i], false, false, 0)
      
  descLabel[0].set_text("1")
  descLabel[0].set_tooltip_text("")
  descLabel[1].set_text("2")
  descLabel[1].set_tooltip_text("")
  descLabel[2].set_text("3")
  descLabel[2].set_tooltip_text("")


  descLabel[3].set_text("4")
  descLabel[3].set_tooltip_text("")
  descLabel[4].set_text("5")
  descLabel[4].set_tooltip_text("")
  descLabel[5].set_text("6")
  descLabel[5].set_tooltip_text("")

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
  
  winSett.show_all()