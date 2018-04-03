import os, strutils
import projTypes
import projUtils
import gtk2, glib2

const
  VERSION = "v0.5.0-alpha"
  INFO = ("Pomf It ! $1" % VERSION)
  LINK = "https://github.com/Senketsu/pomfit"

var 
  chanUp: ptr StringChannel
  chanMain: ptr StringChannel


proc destroy(widget: PWidget, data: Pgpointer) {.cdecl.} =
  chanUp[].send("quit")
  while gtk2.events_pending() > 0:
      discard gtk2.main_iteration()
  main_quit()

proc createMainWin*(channelMain, channelUp:  ptr StringChannel) =

  var winMain: PWindow
  var label: PLabel
  nim_init()
  winMain = window_new(WINDOW_TOPLEVEL)
  # winMain.set_position(WIN_POS_CENTER)
  winMain.set_title(INFO)
  winMain.set_default_size(480,270)
  discard winMain.signal_connect("destroy", SIGNAL_FUNC(gui_gtk.destroy), nil)

  var vbMain = vbox_new(false,2)
  winMain.add(vbMain)

  var ntbMain = notebook_new()
  vbMain.pack_start(ntbMain, true, true, 0)

  # Tab Upload
  var vbNtbUp = vbox_new(false, 0)
  ntbMain.add(vbNtbUp)
  label = label_new("Upload")
  OBJECT(label).set("can-focus", false, nil)
  ntbMain.set_tab_label(get_nth_page(ntbMain, gint(0)), label)
  ntbMain.set_size_request(480, 210)

  var hboxUp = hbox_new(false, 4)
  vbNtbUp.add(hboxUp)

  var btnChooser = button_new_from_stock(STOCK_OPEN)
  # discard OBJECT(btnChooser).signal_connect("clicked",
  #  SIGNAL_FUNC(chooseFiles), nil)
  hboxUp.pack_start(btnChooser, true, true, 0)

  var btnUpload = button_new("Upload")
  # discard OBJECT(btnUpload).signal_connect("clicked",
  #  SIGNAL_FUNC(addToQueue), nil)
  hboxUp.pack_start(btnUpload, true, true, 0)

  var progBarUpload = progress_bar_new()
  progBarUpload.set_text("No ongoing operations..")
  vbNtbUp.pack_start(progBarUpload, true, true, 0)

  # Tab Settings
  var vbNtbSetting = vbox_new(false, 0)
  ntbMain.add(vbNtbSetting)
  label = label_new("Settings")
  OBJECT(label).set("can-focus", false, nil)
  ntbMain.set_tab_label(get_nth_page(ntbMain, gint(1)), label)

  # Tab Tools
  var vbNtbTools = vbox_new(false, 0)
  ntbMain.add(vbNtbTools)
  label = label_new("Tools")
  OBJECT(label).set("can-focus", false, nil)
  ntbMain.set_tab_label(get_nth_page(ntbMain, gint(2)), label)



  var btnQuit = button_new("Quit")
  discard OBJECT(btnQuit).signal_connect("clicked",
   SIGNAL_FUNC(gui_gtk.destroy), nil)
  btnQuit.set_size_request(480, 60)
  vbMain.pack_start(btnQuit, true, true, 0)


  chanUp = channelUp
  chanMain = channelMain

  winMain.show_all()
  main()

