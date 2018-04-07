
const
  oKeybinds = 8
  iKeybinds = 9

var
  cbMod1: array[iKeybinds, PComboBoxText]
  cbMod2: array[iKeybinds, PComboBoxText]
  btnKey:  array[iKeybinds, PButton]
  tbEnable: array[iKeybinds, PToggleButton]


proc tbChangeLabel(widget: PWidget, data: Pgpointer) =
  var btn = TOGGLE_BUTTON(widget)
  if btn.get_active:
    btn.set_label("ON ")
  else:
    btn.set_label("OFF")
  

proc readKeyPress(widget: PWidget, event: gdk2.PEvent, data: Pgpointer): bool =
  var
    btnEvent = cast[PEventKey](event)
    keyName = $keyval_name(btnEvent.keyval)
    btn = BUTTON(widget)
  if keyName.startsWith("Shift"):
    return
  else:
    btn.set_label(keyName)

proc clearKeyBind* (keybind: var TPomfitKb) =
  keybind.id = 0
  keybind.mod1 = MOD_NONE
  keybind.mod2 = MOD_NONE
  keybind.key = ""
  keybind.full = ""
  keybind.isActive = false


proc keybindsSetActive(keyBinds: seq[TPomfitKb]) =
  for i in 0..oKeybinds:
    cbMod1[i].set_active(gint(ord(keyBinds[i].mod1)))
    cbMod2[i].set_active(gint(ord(keyBinds[i].mod2)))
    btnKey[i].set_label(keyBinds[i].key)
    tbEnable[i].set_active(keyBinds[i].isActive)


proc keybindsSave(widge: PWidget, data: Pgpointer) =
  var keyBinds: seq[TPomfitKb] = @[]
  for i in 0..oKeybinds:
    var newBind: TPomfitKb
    var iMod1 = if cbMod1[i].get_active() < 0: 0 else: cbMod1[i].getActive()
    var iMod2 = if cbMod2[i].get_active() < 0: 0 else: cbMod1[i].getActive()
    var sKey = $btnKey[i].get_label()
    newBind.id = i
    newBind.mod1 = cast[TBindModEnum](iMod1)
    newBind.mod2 = cast[TBindModEnum](iMod1)
    newBind.key = sKey.toUpperAscii()
    newBind.isActive = tbEnable[i].get_active()  
    newBind.full = "$1$2$3" % [$newBind.mod1, $newBind.mod2, newBind.key]
    keyBinds.add(newBind)
  if pdbConn.setKeyBindsAll(keyBinds):
    discard ## TODO: set em in nkb

proc keybindsReset(widget: PWidget, data: Pgpointer) =
  let window = gtk2.WINDOW(data)
  if yesOrNo(window, "Do you really want to reset your keybinds ?"):
    if not pdbConn.resetKeybinds():
      infoUser(window, ERR, "Failed to reset keybinds database.\t\n$1" % [
        getCurrentExceptionMsg()])


proc keybindsOpen*(widget: PWidget, data: gpointer) =
  var winKeybinds = window_new(gtk2.WINDOW_TOPLEVEL)
  winKeybinds.set_position(WIN_POS_MOUSE)
  winKeybinds.set_default_size(660, 440)
  winKeybinds.set_border_width(10)
  winKeybinds.set_title("Pomf It ! - settings")
  discard winKeybinds.signal_connect("destroy", SIGNAL_FUNC(closeWindow), winKeybinds)
  winKeybinds.set_icon(piIco)

  var vbMain = vbox_new(false, 0)
  winKeybinds.add(vbMain)
  var hbMain = hbox_new(false, 0)
  vbMain.pack_start(hbMain, false, false, 0)

  var vbSection: array[5, PVBox]
  for i in 0..4:
    vbSection[i] = vbox_new(false, 0)
    hbMain.pack_start(vbSection[i], false, false, 0)
  
  vbSection[0].set_size_request(170, 40)
  vbSection[1].set_size_request(150, 40)
  vbSection[2].set_size_request(150, 40)
  vbSection[3].set_size_request(90, 40)
  vbSection[4].set_size_request(90, 40)
  
  var kbLabel: array[iKeybinds, PLabel]
  kbLabel[0] = label_new("File Chooser")
  kbLabel[1] = label_new("Capture Area & UP")
  kbLabel[2] = label_new("Capture Window & UP")
  kbLabel[3] = label_new("Capture Fullscreen & UP")
  kbLabel[4] = label_new("Capture Area")
  kbLabel[5] = label_new("Capture Window")
  kbLabel[6] = label_new("Capture Fullscreen")
  kbLabel[7] = label_new("Start uploading")
  kbLabel[8] = label_new("Pause uploading")

  for i in 0..oKeybinds:
    kbLabel[i].set_size_request(-1, 40)
    vbSection[0].pack_start(kbLabel[i], false, false, 0)
    
    cbMod1[i] = combo_box_text_new()
    cbMod1[i].append_text($MOD_NONE)
    cbMod1[i].append_text($MOD_CTRL)
    cbMod1[i].append_text($MOD_SHIFT)
    cbMod1[i].append_text($MOD_ALT)
    cbMod1[i].append_text($MOD_WIN)
    cbMod1[i].set_size_request(-1, 40)
    vbSection[1].pack_start(cbMod1[i], false, false, 0)

    cbMod2[i] = combo_box_text_new()
    cbMod2[i].append_text($MOD_NONE)
    cbMod2[i].append_text($MOD_CTRL)
    cbMod2[i].append_text($MOD_SHIFT)
    cbMod2[i].append_text($MOD_ALT)
    cbMod2[i].append_text($MOD_WIN)
    cbMod2[i].set_size_request(-1, 40)
    vbSection[2].pack_start(cbMod2[i], false, false, 0)
    
    btnKey[i] = button_new("")
    btnKey[i].set_tooltip_text("Clock on button and press key you want to use.")
    discard btnKey[i].signal_connect("key-press-event",
     SIGNAL_FUNC(readKeyPress), nil)
    btnKey[i].set_size_request(-1, 40)
    vbSection[3].pack_start(btnKey[i], false, false, 0)

    tbEnable[i] = toggle_button_new("OFF")
    discard tbEnable[i].signal_connect("toggled", SIGNAL_FUNC(tbChangeLabel), nil)
    tbEnable[i].set_size_request(-1, 40)
    vbSection[4].pack_start(tbEnable[i], false, false, 0)
  
  
  var hbControl = hbox_new(false, 5)
  hbControl.set_size_request(-1, 30)
  vbMain.pack_end(hbControl, false, false, 5)
    
  var btnReset = button_new("Reset")
  discard OBJECT(btnReset).signal_connect("clicked",
   SIGNAL_FUNC(keybindsReset), winKeybinds)
  btnReset.set_size_request(90, 30)
  hbControl.pack_start(btnReset, false, false, 0)
  
  var btnClose = button_new_from_stock(STOCK_CLOSE)
  discard OBJECT(btnClose).signal_connect("clicked",
    SIGNAL_FUNC(closeWindow), winKeybinds)
  btnClose.set_size_request(90, 30)
  hbControl.pack_end(btnClose, false, false, 0)

  var btnProfSave = button_new_from_stock(STOCK_SAVE)
  btnProfSave.set_size_request(90, 30)
  discard btnProfSave.signal_connect("clicked",
   SIGNAL_FUNC(keybindsSave), winKeybinds)
  hbControl.pack_end(btnProfSave, false, false, 0)
  
  var keyBinds = pdbConn.getKeyBindsAll()
  keybindsSetActive(keyBinds)
  winKeybinds.show_all()




