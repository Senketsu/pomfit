import os, strutils
import db_sqlite
import gtk2, glib2, gdk2pixbuf
import projTypes
import projUtils
import database

const
  VERSION = "v0.5.0-alpha"
  NAME = "Pomf It !"
  INFO = "$1 $2" % [NAME, VERSION]
  LINK = "https://github.com/Senketsu/pomfit"
  TWITTER = "https://twitter.com/Senketsu_dev"
  LICENSE = LINK & "/blob/devel/LICENSE.txt"

var
  winMain*: gtk2.PWindow
  chanUp*: ptr StringChannel
  chanMain*: ptr StringChannel
  pbMain*: PProgressBar
  sbMain*: PStatusBar
  piIco*: PPixBuf
  pdbConn*: DbConn
  btnUpload: PButton
  IsUploading = false
  InstantUpload = false

import lib/gtk2ext

proc destroy(widget: PWidget, data: Pgpointer) {.cdecl.} =
  chanUp[].send("quit")
  while gtk2.events_pending() > 0:
      discard gtk2.main_iteration()
  main_quit()

proc closeWindow(widget: PWidget, data: Pgpointer) =
  WINDOW(data).destroy()


proc uploadControl(widget: PWidget, data: Pgpointer) =
  if IsUploading:
    btnUpload.set_relief(1)
    btnUpload.set_label("Stopping upload..")
    chanUp[].send("cmd:stop")
  else:
    btnUpload.set_relief(1)
    btnUpload.set_label("Starting upload..")
    chanUp[].send("cmd:start")


proc pbUpdateIdle() =
  var qlen = pdbConn.getQueueLen()
  if qlen == 0:
    pbMain.set_text("No ongoing operations & 0 files in the queue.")
  else:
    pbMain.set_text("There $1 $2 file$3 in the queue.." % [
      if qlen == 1: "is" else: "are", $qlen, if qlen == 1: "" else: "s"])


proc showTrayMenu(icon: PStatusIcon, btnID: guint,
                   actTime: guint32, data: Pgpointer) {.procvar.} =
  MENU(data).popup(nil, nil, status_icon_position_menu, icon, btnID, actTime)


proc toggleVisible(widget: PWidget, data: Pgpointer){.procvar.} =
  var window = WINDOW(data)
  let noShow = get_no_show_all(window)
  if noShow:
    window.show()
    window.set_no_show_all(false)
  else:
    window.hide()
    window.set_no_show_all(true)


proc createTrayWidget(win: gtk2.PWindow, icon: PPixbuf, menu: PMenu): PStatusIcon =
  result = status_icon_new()
  discard result.g_signal_connect("activate", G_CALLBACK(toggleVisible), win)
  discard result.g_signal_connect("popup-menu", G_CALLBACK(showTrayMenu), menu)
  result.status_icon_set_from_pixbuf(icon)
  result.status_icon_set_tooltip_text(INFO)
  result.status_icon_set_visible(true)

include fileChooser
include settings
include profiles
include queue
include keybinds_cb
include keybinds


proc createTrayMenu(win: gtk2.PWindow): PMenu =
  result = menu_new()
  var
    miFile = menu_item_new("Choose file(s)\t\t")
    miSep1 = separator_menu_item_new()
    miFullUp = menu_item_new("Capture & Upload Fullscreen\t\t")
    miAreaUp = menu_item_new("Capture & Upload Area / Window\t")
    miSep2 = separator_menu_item_new()
    miFull = menu_item_new("Capture Fullscreen\t\t")
    miArea = menu_item_new("Capture Area / Window\t\t")
    miSep3 = separator_menu_item_new()
    miShow = menu_item_new("Show / Hide")
    miQuit = menu_item_new("Quit")

  discard OBJECT(miFile).signal_connect("activate", SIGNAL_FUNC(pfcStart), win)
  discard OBJECT(miFullUp).signal_connect("activate",
   SIGNAL_FUNC(takeScreenShotKb), cast[pointer](FULL_UP))
  discard OBJECT(miAreaUp).signal_connect("activate",
   SIGNAL_FUNC(takeScreenShotKb), cast[pointer](AREA_UP))
  discard OBJECT(miFull).signal_connect("activate",
   SIGNAL_FUNC(takeScreenShotKb), cast[pointer](FULL_CAP))
  discard OBJECT(miArea).signal_connect("activate",
   SIGNAL_FUNC(takeScreenShotKb), cast[pointer](AREA_CAP))
  discard OBJECT(miShow).signal_connect("activate",
   SIGNAL_FUNC(gui_gtk.toggleVisible), win)
  discard OBJECT(miQuit).signal_connect("activate",
   SIGNAL_FUNC(gui_gtk.destroy), win)

  MENU_SHELL(result).append(miFile)
  MENU_SHELL(result).append(miSep1)
  MENU_SHELL(result).append(miFullUp)
  MENU_SHELL(result).append(miAreaUp)
  MENU_SHELL(result).append(miSep2)
  MENU_SHELL(result).append(miFull)
  MENU_SHELL(result).append(miArea)
  MENU_SHELL(result).append(miSep3)
  MENU_SHELL(result).append(miShow)
  MENU_SHELL(result).append(miQuit)
  result.show_all()


proc createMainWin*(channelMain, channelUp:  ptr StringChannel) =

  var label: PLabel
  nim_init()
  nkb_init()
  winMain = window_new(gtk2.WINDOW_TOPLEVEL)
  winMain.set_position(WIN_POS_MOUSE)
  winMain.set_title(INFO)
  winMain.set_default_size(480, 270)
  discard winMain.signal_connect("destroy", SIGNAL_FUNC(gui_gtk.destroy), nil)
  piIco = pixbuf_new_from_file_at_size(joinPath(getAppDir(), 
    "../data/logo.png"), 64, 64, nil)
  winMain.set_icon(piIco)
  var trayMenu = createTrayMenu(winMain)
  var pomfitTray = createTrayWidget(winMain, piIco, trayMenu)

  var vbMain = vbox_new(false, 2)
  winMain.add(vbMain)
  var ntbMain = notebook_new()
  vbMain.pack_start(ntbMain, true, true, 0)

  # Tab Upload
  var vbNtbUp = vbox_new(false, 0)
  ntbMain.add(vbNtbUp)
  label = label_new("Upload")
  label.set_size_request(100, 30)
  OBJECT(label).set("can-focus", false, nil)
  ntbMain.set_tab_label(get_nth_page(ntbMain, gint(0)), label)
  ntbMain.set_size_request(480, 240)

  var hboxUp = hbox_new(false, 4)
  vbNtbUp.add(hboxUp)

  var btnChooser = button_new_from_stock(STOCK_OPEN)
  btnChooser.set_size_request(230, -1)
  discard OBJECT(btnChooser).signal_connect("clicked",
   SIGNAL_FUNC(pfcStart), winMain)
  hboxUp.pack_start(btnChooser, true, true, 0)

  btnUpload = button_new("Upload")
  btnUpload.set_size_request(230, -1)
  discard OBJECT(btnUpload).signal_connect("clicked",
   SIGNAL_FUNC(uploadControl), nil)
  hboxUp.pack_start(btnUpload, true, true, 0)

  pbMain = progress_bar_new()
  vbNtbUp.pack_start(pbMain, true, true, 0)

  # Tab Settings
  var vbNtbSetting = vbox_new(false, 0)
  ntbMain.add(vbNtbSetting)
  label = label_new("Settings")
  label.set_size_request(100, 30)
  OBJECT(label).set("can-focus", false, nil)
  ntbMain.set_tab_label(get_nth_page(ntbMain, gint(1)), label)

  var btnSettings = button_new("Settings")
  btnSettings.set_size_request(-1, 50)
  discard OBJECT(btnSettings).signal_connect("clicked",
   SIGNAL_FUNC(gui_gtk.settingsOpen), winMain)
  vbNtbSetting.pack_start(btnSettings, false, false, 0)

  var btnProfiles = button_new("Profiles")
  btnProfiles.set_size_request(-1, 50)
  discard OBJECT(btnProfiles).signal_connect("clicked",
   SIGNAL_FUNC(gui_gtk.profilesOpen), winMain)
  vbNtbSetting.pack_start(btnProfiles, false, false, 0)

  var btnKeybinds = button_new("Keybinds")
  btnKeybinds.set_size_request(-1, 50)
  discard OBJECT(btnKeybinds).signal_connect("clicked",
   SIGNAL_FUNC(gui_gtk.keybindsOpen), winMain)
  vbNtbSetting.pack_start(btnKeybinds, false, false, 0)

  # Tab Tools
  var vbNtbTools = vbox_new(false, 0)
  ntbMain.add(vbNtbTools)
  label = label_new("Tools")
  label.set_size_request(100, 30)
  OBJECT(label).set("can-focus", false, nil)
  ntbMain.set_tab_label(get_nth_page(ntbMain, gint(2)), label)
  
  var btnQueue = button_new("Upload queue")
  btnQueue.set_size_request(-1, 50)
  discard OBJECT(btnQueue).signal_connect("clicked",
   SIGNAL_FUNC(gui_gtk.queueOpen), winMain)
  vbNtbTools.pack_start(btnQueue, false, false, 0)

  var btnManager = button_new("Uploads DB")
  btnManager.set_size_request(-1, 50)
  discard OBJECT(btnManager).signal_connect("clicked",
   SIGNAL_FUNC(gui_gtk.queueOpen), winMain)
  vbNtbTools.pack_start(btnManager, false, false, 0)

  # Tab About
  var vbNtbAbout = vbox_new(false, 0)
  ntbMain.add(vbNtbAbout)
  label = label_new("About")
  label.set_size_request(100, 30)
  OBJECT(label).set("can-focus", false, nil)
  ntbMain.set_tab_label(get_nth_page(ntbMain, gint(3)), label)

  var hbAboutSplit = hbox_new(false, 0)
  hbAboutSplit.set_size_request(-1, 160)
  vbNtbAbout.pack_start(hbAboutSplit, false, false, 0)
  
  var pathLogo = joinPath(getAppDir(), "../data/logo_simple.png")
  var pvImage = pixbuf_new_from_file_at_scale(pathLogo, 150, 150, false, nil)
  var imgLogo = image_new_from_pixbuf(pvImage)
  hbAboutSplit.pack_start(imgLogo, false, false, 0)

  var vbAboutText = vbox_new(false, 0)
  vbAboutText.set_size_request(300, -1)
  hbAboutSplit.pack_start(vbAboutText, false, false, 0)
  
  var labAbout = label_new("")
  labAbout.set_justify(JUSTIFY_CENTER)
  labAbout.set_markup("<span weight='bold' font='Sans Mono 24'>$1</span>" % [NAME])
  vbAboutText.pack_start(labAbout, false, false, 0)

  labAbout = label_new("")
  labAbout.set_justify(JUSTIFY_CENTER)
  labAbout.set_markup("<span style='italic' font='Sans 12'>$1</span>" % [VERSION])
  vbAboutText.pack_start(labAbout, false, false, 0)

  labAbout = label_new("")
  labAbout.set_justify(JUSTIFY_CENTER)
  labAbout.set_markup("<span style='italic' font='Sans 10'>$1</span>" % [
    "Designed by Senketsu"])
  vbAboutText.pack_start(labAbout, false, false, 0)

  var hbLinks = hbox_new(false, 10)
  hbLinks.set_size_request(-1, 30)
  vbNtbAbout.pack_end(hbLinks, false, false, 0)

  var btnLicense = link_button_new_with_label(LICENSE, "License Information")
  btnLicense.set_size_request(150, 30)
  hbLinks.pack_start(btnLicense, false, false, 0)
  
  var btnGitHub = link_button_new_with_label(LINK, "GitHub Page")
  btnGitHub.set_size_request(150, 30)
  hbLinks.pack_start(btnGitHub, false, false, 0)
  
  var btnSenketsu = link_button_new_with_label(TWITTER, "@Senketsu_dev")
  btnSenketsu.set_size_request(150, 30)
  hbLinks.pack_start(btnSenketsu, false, false, 0)
  
  var hboxBottom = hbox_new(false, 10)
  vbMain.pack_end(hboxBottom, true, false, 0)
  hboxBottom.set_size_request(-1, 30)

  sbMain = status_bar_new()
  sbMain.set_tooltip_text("General information is shown here...")
  discard sbMain.push(1, INFO)
  hboxBottom.pack_start(sbMain, true, true, 10)

  var btnQuit = button_new("Quit")
  discard OBJECT(btnQuit).signal_connect("clicked",
   SIGNAL_FUNC(gui_gtk.destroy), nil)
  btnQuit.set_size_request(90, 30)
  hboxBottom.pack_start(btnQuit, false, false, 10)

  chanUp = channelUp
  chanMain = channelMain
  pdbConn = openPomfitDatabase()
  
  pbUpdateIdle()
  keybindsSetActive()
  winMain.show_all()
  main()

