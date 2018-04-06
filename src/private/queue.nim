import gdk2

var
  tvQueue: PTreeView
  tvQueueStore: PTreeStore

proc queueMenuItemRemove(widget: PWidget, data: Pgpointer) =
  var
    model = TREE_VIEW(widget.get_parent()).get_model()
    iter: PTreeIter
    path: PTreePath
  if model.get_iter(iter, path):
    TREE_STORE(model).remove(iter)


proc queueMenuItemOpenFile(widget: PWidget, data: Pgpointer) =
  echoInfo("TODO: queue menu item open file")


proc queueMenuItemOpenFolder(widget: PWidget, data: Pgpointer) =
  echoInfo("TODO: queue menu item open folder")


proc queueTreeViewShowMenu(widget: PTreeView, event: PEventButton,
                            data: Pgpointer, path: PTreePath) =
  let menu = MENU(data)
  menu.popup(nil, nil, nil, nil, event.button, get_time(cast[PEvent](event)))


proc queueTreeViewBtnCb(widget: PWIDGET, event: PEventButton,
                         data: Pgpointer):bool =
  if event.type == BUTTON_PRESS and event.button == 3:
    var
      view = TREE_VIEW(widget)
      selection = view.get_selection()
      path: PTreePath

    if selection.count_selected_rows() <= 1:
      if view.get_path_at_pos(gint(event.x), gint(event.y),
       addr path, nil, 0, 0) != 0:
        selection.unselect_all()
        selection.select_path(path)
      if path != nil:
        view.queueTreeViewShowMenu(event, data, path)
      return true
  return false


proc queueTreeViewMenuCb(widget: PWidget, data: Pgpointer): bool =
  TREE_VIEW(widget).queueTreeViewShowMenu(nil, data, nil)
  return true


proc queueTreeViewRefresh() =
  var
    iter: PTreeIter
    limit: uint64 = parseBiggestUInt(pdbConn.getProfileDataActive("size"))
  tvQueueStore.clear()
  var fileList = pdbConn.getQueueDataAll()
  for item in fileList:
    if item.existsFile():
      let size: uint64 = uint64(getFileSize(item))
      let name = item.splitFile().name
      var status = ""
      if size > limit and limit != 0:
        status = "BIG"
      else:
        status = " OK"
      var strSize = "$1 MB" % [ (float(size) / 1000000) | (10,2)]
      tvQueueStore.append(iter, nil)
      tvQueueStore.set(iter, TVQ_NAME, name, TVQ_SIZE, strSize, TVQ_STATUS,
        status, TVQ_PATH, item, -1)


proc queueCreateTreeViewMenu(window: gtk2.PWindow): PMenu =
  result = menu_new()
  var
    miOpenFile = menu_item_new("Open File")
    miOpenFolder = menu_item_new("Open File")
    miRemove = menu_item_new("Remove from Queue")
  
  discard OBJECT(miOpenFile).signal_connect("activate",
   SIGNAL_FUNC(queueMenuItemOpenFile), window)
  discard OBJECT(miOpenFolder).signal_connect("activate",
   SIGNAL_FUNC(queueMenuItemOpenFolder), window)
  discard OBJECT(miRemove).signal_connect("activate",
   SIGNAL_FUNC(queueMenuItemRemove), window) 
  
  MENU_SHELL(result).append(miOpenFile)
  MENU_SHELL(result).append(miOpenFolder)
  MENU_SHELL(result).append(miRemove)
  result.show_all()


proc queueCreateTreeView(window: gtk2.PWindow, view: var PTreeView,
                          store: var PTreeStore, menu: PMenu) =
  var
    rend: PCellRenderer = cell_renderer_text_new()
    column: PTreeViewColumn
  
  store = tree_store_new(gint(TVQ_iCol),
   TYPE_STRING, TYPE_STRING, TYPE_STRING, TYPE_STRING)
  view = tree_view_new(TREE_MODEL(store))
  discard view.g_signal_connect("button-press-event",
   G_CALLBACK(queueTreeViewBtnCb), menu)
  discard view.g_signal_connect("popup-menu",
   G_CALLBACK(queueTreeViewMenuCb), menu)
  
  for col in tvQueueEnum:
    if ord(col) < ord(TVQ_iCol):
      column = tree_view_column_new_with_attributes($col, rend, "text", ord(col), nil)
      column.column_set_sort_column_id(gint(ord(col)))
      discard view.append_column(column)
  view.set_headers_clickable(true)


proc queueStartUpload(widget: PWidget, data: Pgpointer) =
  chanUp[].send("cmd_up")


proc queueReset(widget: PWidget, data: Pgpointer) =
  let window = gtk2.WINDOW(data)
  if pdbConn.resetQueue():
    queueTreeViewRefresh()
  else:
    infoUser(window, ERR, "Failed to delete all items from queue\t\n$1" % [
      getCurrentExceptionMsg()])


proc queueOpen(widget: PWidget, data: Pgpointer) =
  var winQueue = window_new(gtk2.WINDOW_TOPLEVEL)
  winQueue.set_position(WIN_POS_MOUSE)
  winQueue.set_default_size(640, 320)
  winQueue.set_border_width(10)
  winQueue.set_title("Pomf It ! - profiles")
  discard winQueue.signal_connect("destroy", SIGNAL_FUNC(closeWindow), winQueue)
  winQueue.set_icon(piIco)

  var vbMain = vbox_new(false, 0)
  winQueue.add(vbMain)

  var swQueue = scrolled_window_new(nil, nil)
  swQueue.set_size_request(-1, 270)
  vbMain.pack_start(swQueue, true, true, 0)

  var tvMenu: PMenu = queueCreateTreeViewMenu(winQueue)
  queueCreateTreeView(winQueue, tvQueue, tvQueueStore, tvMenu)
  CONTAINER(swQueue).add(WIDGET(tvQueue))

  var sbQueue = statusbar_new()
  vbMain.pack_start(sbQueue, true, true, 0)
  discard sbQueue.push(1, "Queue status bar..")

  var hbControl = hbox_new(false, 5)
  hbControl.set_size_request(-1, 30)
  vbMain.pack_end(hbControl, false, false, 5)

  var btnClear = button_new_from_stock(STOCK_CLEAR)
  discard OBJECT(btnClear).signal_connect("clicked",
   SIGNAL_FUNC(queueReset), winQueue)
  btnClear.set_size_request(90, 30)
  hbControl.pack_start(btnClear, false, false, 0)

  var btnBrowse = button_new("Add files")
  discard OBJECT(btnBrowse).signal_connect("clicked",
   SIGNAL_FUNC(fileChooser.start), winQueue)
  btnBrowse.set_size_request(90, 30)
  hbControl.pack_start(btnBrowse, false, false, 20)

  var btnClose = button_new_from_stock(STOCK_CLOSE)
  discard OBJECT(btnClose).signal_connect("clicked",
   SIGNAL_FUNC(closeWindow), winQueue)
  btnClose.set_size_request(90, 30)
  hbControl.pack_end(btnClose, false, false, 0)
  
  var btnUpload = button_new("Upload")
  discard OBJECT(btnUpload).signal_connect("clicked",
   SIGNAL_FUNC(queueStartUpload), winQueue)
  btnUpload.set_size_request(90, 30)
  hbControl.pack_end(btnUpload, false, false, 20)
  
  winQueue.show_all()

