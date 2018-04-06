const iProfOpt = 10
var
  cbProfiles: PComboBoxText
  profEntry: array[iProfOpt, PEntry]
  iProfiles: int = 0 # gotta keep counting since gtk2 can't
  labActive: PLabel

proc profilesUpdateLabelActive() =
  var active = pdbConn.getProfilesActive()
  labActive.set_text(cstring(active))


proc profilesUpdateComboBox() =
  var names = pdbConn.getProfilesNames()
  for i in 0..iProfiles:
    cbProfiles.remove(gint(0))
  iProfiles = 0
  for i in 0..names.high:
    cbProfiles.append_text(cstring(names[i]))
    inc(iProfiles)
  profilesUpdateLabelActive()


proc profileEdit(widget: PWidget, data: Pgpointer) =
  let pName = $cbProfiles.get_active_text()
  if pName == nil:
    return
  var profile = pdbConn.getProfileData(pName)
  for i in 0..iProfOpt-1:
    profEntry[i].set_text(cstring(profile[i]))


proc profileActivate(widget: PWidget, data: Pgpointer) =
  let pName = $cbProfiles.get_active_text()
  let window = WINDOW(data)
  if pName == nil:
    return  
  if pdbConn.updateProfilesActive(pName):
    profilesUpdateComboBox()
  else:
    infoUser(window, ERR, "Failed to set active profile.\t\n$1" % [
        getCurrentExceptionMsg()])


proc profilesReset(widget: PWidget, data: Pgpointer) =
  let window = WINDOW(data)
  if yesOrNo(window, """Do you really want to reset to default profile(s) ?
    (This will delete all custom made profiles !)"""):
    if pdbConn.resetProfiles():
      profilesUpdateComboBox()
    else:
      infoUser(window, ERR, "Failed to reset profiles database.\t\n$1" % [
        getCurrentExceptionMsg()])
      

proc convertSizeStringToBytes(sSize: string): string =
  result = ""
  if sSize.isNilOrWhitespace():
    return
  var
    prefix = newString(len(sSize))
    number = newString(len(sSize))
    p,s: int = 0
    iResult: BiggestUInt
  for i in 0..sSize.len-1:
    if sSize[i].isDigit:
      number[s] = sSize[i]
      inc(s)
    else:
      prefix[p] = sSize[i]
      inc(p)
  
  number.delete(s, number.len-1)
  prefix.delete(p, prefix.len-1)
  case prefix.toLowerAscii()
  of "kb":
    iResult = (number.parseBiggestUINT() * 1000'u64)
  of "mb":
    iResult = (number.parseBiggestUINT() * 1000000'u64)
  of "gb":
    iResult = (number.parseBiggestUINT() * 1000000000'u64)
  of "tb":
    iResult = (number.parseBiggestUINT() * 1000000000000'u64)
  of "kib":
    iResult = (number.parseBiggestUINT() * 1024'u64)
  of "mib":
    iResult = (number.parseBiggestUINT() * 1048576'u64)
  of "gib":
    iResult = (number.parseBiggestUINT() * 1073741824'u64)
  of "tib":
    iResult = (number.parseBiggestUINT() * 1099511627776'u64)
  of "":
    result = number
  else:
    return "err" # return 'err' when prefix is 'unknown'
  result = $(iResult)


proc profileSave(widget: PWidget, data: Pgpointer) =
  var
    window = WINDOW(data)
    newName = $profEntry[0].get_text()
    storedNames = pdbConn.getProfilesNames()
    overwrite = false
  for name in storedNames:
    if newName == "Uguu.se":
      infoUser(window, WARN, "Profile 'Uguu.se' cannot be edited.")
      return
    if newName == name:
      if yesOrNo(window, "Profile '$1' already exists.\n Overwrite ?" % [newName]):
        overwrite = true
        break
      else:
        return

  var profileData: seq[string] = @[]
  for i in 0..iProfOpt-1:
    profileData.add($profEntry[i].get_text())
  if profileData[0] == "" or profileData[1] == "":
    infoUser(window, WARN, "'Name' or 'API' fields can't be empty.")
    return
  profileData[2] = convertSizeStringToBytes(profileData[2])
  if profileData[2] == "err":
    infoUser(window, ERR, "Invalid size input or prefix.")
    return
    
  if overwrite:
    if pdbConn.updateProfileData(profileData):
      profilesUpdateComboBox()
  else:
    if pdbConn.addProfileData(profileData):
      profilesUpdateComboBox()
  
  
proc profileDelete(widget: PWidget, data: Pgpointer) =
  let pName = $cbProfiles.get_active_text()
  let window = WINDOW(data)
  if pName == nil:
    return
  if pName == "Uguu.se":
    infoUser(window, WARN, "Deleting profile 'Uguu.se' is forbidden !")
    return
  if yesOrNo(window, "Do you want to delete profile '$1' ?" % [pName]):
    if pdbConn.deleteProfile(pName):
      profilesUpdateComboBox()
    else:
      infoUser(window, ERR, "Failed to delete profile.\t\n$1" % [
          getCurrentExceptionMsg()])
        

proc profilesOpen(widget: PWidget, data: Pgpointer) =
  var winProf = window_new(WINDOW_TOPLEVEL)
  winProf.set_position(WIN_POS_MOUSE)
  winProf.set_default_size(640, 320)
  winProf.set_border_width(10)
  winProf.set_title("Pomf It ! - profiles")
  discard winProf.signal_connect("destroy", SIGNAL_FUNC(closeWindow), winProf)
  winProf.set_icon(piIco)

  var label = label_new("")
  label.set_size_request(150, 30)
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
    descLabel: array[iProfOpt, PLabel]
  for i in 0..iProfOpt-1:
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
    Valid suffixes: 'MiB' 'MB' 'KiB' 'KB' etc..
    Leave empty for no limit.""")
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

  label = label_new("Profile list:")
  label.set_size_request(150, -1)
  hbProfCtrl.pack_start(label, false, false, 0)

  cbProfiles = combo_box_text_new()
  cbProfiles.set_tooltip_text("List of available profiles.")
  cbProfiles.set_size_request(150, -1)
  discard cbProfiles.signal_connect("changed", SIGNAL_FUNC(profileEdit), nil)
  hbProfCtrl.pack_start(cbProfiles, false, false, 0)

  var btnProfAct = button_new("Activate")
  btnProfAct.set_size_request(90, -1)
  discard btnProfAct.signal_connect("clicked", SIGNAL_FUNC(profileActivate), winProf)
  hbProfCtrl.pack_start(btnProfAct, false, false, 0)
  
  var btnProfDel = button_new("Delete")
  btnProfDel.set_size_request(90, -1)
  discard btnProfDel.signal_connect("clicked", SIGNAL_FUNC(profileDelete), winProf)
  hbProfCtrl.pack_start(btnProfDel, false, false, 0)
  
  label = label_new("")
  label.set_size_request(20, 30)
  hbProfCtrl.pack_start(label, false, false, 0)

  var btnProfSave = button_new_from_stock(STOCK_SAVE)
  btnProfSave.set_size_request(90, -1)
  discard btnProfSave.signal_connect("clicked", SIGNAL_FUNC(profileSave), winProf)
  hbProfCtrl.pack_start(btnProfSave, false, false, 0)

  label = label_new("")
  label.set_size_request(150, 30)
  vbMain.pack_start(label, false, false, 0)

  var hbControl = hbox_new(false, 5)
  hbControl.set_size_request(-1, 30)
  vbMain.pack_end(hbControl, false, false, 5)
  
  var btnReset = button_new("Reset")
  discard OBJECT(btnReset).signal_connect("clicked",
   SIGNAL_FUNC(profilesReset), winProf)
  btnReset.set_size_request(90, 30)
  hbControl.pack_start(btnReset, false, false, 0)

  label = label_new("Active Profile:")
  label.set_size_request(150, -1)
  hbControl.pack_start(label, false, false, 0)
  
  labActive = label_new("")
  labActive.set_size_request(-1, 30)
  hbControl.pack_start(labActive, false, false, 0)

  var btnClose = button_new_from_stock(STOCK_CLOSE)
  discard OBJECT(btnClose).signal_connect("clicked",
   SIGNAL_FUNC(closeWindow), winProf)
  btnClose.set_size_request(90, 30)
  hbControl.pack_end(btnClose, false, false, 0)
  
  profilesUpdateComboBox()
  winProf.show_all()

