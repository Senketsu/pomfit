proc profileActivate(widget: PWidget, data: Pgpointer) =
  echoInfo("*Debug: TODO: Activate profile")

proc profileSave(widget: PWidget, data: Pgpointer) =
  echoInfo("*Debug: TODO: Save profile")

proc profileDelete(widget: PWidget, data: Pgpointer) =
  echoInfo("*Debug: TODO: Delete profile")

proc profilesClose(widget: PWidget, data: Pgpointer) =
  WINDOW(data).destroy()

proc profilesOpen(widget: PWidget, data: Pgpointer) =
  var winProf = window_new(WINDOW_TOPLEVEL)
  winProf.set_position(WIN_POS_MOUSE)
  winProf.set_default_size(640, 360)
  winProf.set_border_width(10)
  winProf.set_title("Pomf It ! - profiles")
  discard winProf.signal_connect("destroy", SIGNAL_FUNC(profilesClose), winProf)
  winProf.set_icon(piIco)

  var label = label_new("")
  var vbMain = vbox_new(false, 0)
  vbMain.set_size_request(-1, -1)
  winProf.add(vbMain)
  
  var hbMain = hbox_new(false , 5)
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
    descLabel: array[10, PLabel]
    profEntry: array[10, PEntry]
  for i in 0..9:
    descLabel[i] = label_new("")
    profEntry[i] = entry_new()
    if i < 5:
      vbMainTab[0].pack_start(descLabel[i], false, false, 0)
      vbMainTab[1].pack_start(profEntry[i], false, false, 0)
    else:
      vbMainTab[2].pack_start(descLabel[i], false, false, 0)
      vbMainTab[3].pack_start(profEntry[i], false, false, 0)
      
  descLabel[0].set_text("Name")
  descLabel[0].set_tooltip_text("Unique name identifier for this profile.")
  descLabel[1].set_text("API Link")
  descLabel[1].set_tooltip_text("""Should looks something like this:
    http://pomf.se/upload.php""")
  descLabel[2].set_text("Size Limit")
  descLabel[2].set_tooltip_text("""Can be raw number or with suffix.
    Valid suffixes: 'Mb' 'MB' 'Kb' 'KB' etc..""")
  descLabel[3].set_text("Regex URL")
  descLabel[3].set_tooltip_text("[Optional] Replaces internal parser.")
  descLabel[4].set_text("Regex Delete URL")
  descLabel[4].set_tooltip_text("[Optional] Replaces internal parser.")


  descLabel[5].set_text("Form-ID")
  descLabel[5].set_tooltip_text("Sites input element id.\n\te.g: 'file' or 'files[]'")
  descLabel[6].set_text("Form-Type")
  descLabel[6].set_tooltip_text("Sites content type\n\te.g: 'application/octet-stream'")
  descLabel[7].set_text("Form-Filename")
  descLabel[7].set_tooltip_text("[Optional] Custom filename. Blank for default.")
  descLabel[8].set_text("Prepend to URI")
  descLabel[8].set_tooltip_text("[Optional] If you want to prepend to the URL.")
  descLabel[9].set_text("Append to URI")
  descLabel[9].set_tooltip_text("[Optional] If you want to append to the URL.")
    
  vbMain.pack_start(label, false, false, 0)
  var hbProfCtrl = hbox_new(false, 5)
  hbProfCtrl.set_size_request(-1, 30)
  vbMain.pack_start(hbProfCtrl, false, false, 0)

  label = label_new("Active Profile:")
  label.set_size_request(150, -1)
  hbProfCtrl.pack_start(label, false, false, 0)

  var cbProfiles = combo_box_text_new()
  cbProfiles.set_tooltip_text("List of available profiles.")
  cbProfiles.set_size_request(150, -1)
  hbProfCtrl.pack_start(cbProfiles, false, false, 0)

  var btnProfAct = button_new("Activate")
  btnProfAct.set_size_request(90, -1)
  discard btnProfAct.signal_connect("clicked", SIGNAL_FUNC(profileActivate), nil)
  hbProfCtrl.pack_start(btnProfAct, false, false, 0)
  
  var btnProfDel = button_new("Delete")
  btnProfDel.set_size_request(90, -1)
  discard btnProfDel.signal_connect("clicked", SIGNAL_FUNC(profileDelete), nil)
  hbProfCtrl.pack_start(btnProfDel, false, false, 0)
  
  label = label_new("")
  label.set_size_request(20, -1)
  hbProfCtrl.pack_start(label, false, false, 0)

  var btnProfSave = button_new_from_stock(STOCK_SAVE)
  btnProfSave.set_size_request(90, -1)
  discard btnProfSave.signal_connect("clicked", SIGNAL_FUNC(profileSave), nil)
  hbProfCtrl.pack_start(btnProfSave, false, false, 0)

  var hbControl = hbox_new(false, 5)
  hbControl.set_size_request(-1, 30)
  vbMain.pack_end(hbControl, false, false, 5)

  var btnClose = button_new_from_stock(STOCK_CLOSE)
  discard OBJECT(btnClose).signal_connect("clicked",
   SIGNAL_FUNC(profilesClose), winProf)
  btnClose.set_size_request(90, 30)
  hbControl.pack_end(btnClose, false, false, 0)

  winProf.show_all()

