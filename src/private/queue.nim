import gdk2

var
  tvQueue: PTreeView
  tvQueueStore: PTreeStore
  tvQueuePath: PTreePath
  sbQueue: PStatusBar

proc queueMenuItemDo(widget: PWidget, data: Pgpointer) =
  var
    model = get_model(tvQueue)
    iter: TTreeIter
    filePath: cstring
    cmd = cast[tvQueueDo](data)
  
  if model.get_iter(addr iter, tvQueuePath):
    model.get(addr iter, TVQ_PATH, addr filePath, -1)
    case cmd
    of TVQ_OFI:
      discard execShellCmd("xdg-open $1" % [$filePath])
    of TVQ_OFO:
      discard execShellCmd("xdg-open $1" % [($filePath).splitFile().dir])
    of TVQ_REM:
      if pdbConn.removeQueueData($filePath):
        TREE_STORE(model).remove(addr iter)
        pbUpdateIdle()
    else:
      discard


proc queueTreeViewShowMenu(widget: PTreeView, event: PEventButton,
                            data: Pgpointer) =
  let menu = MENU(data)
  menu.popup(nil, nil, nil, nil, event.button, get_time(cast[PEvent](event)))


proc queueTreeViewBtnCb(widget: PWIDGET, event: PEventButton,
                         data: Pgpointer):bool =
  if event.type == BUTTON_PRESS and event.button == 3:
    var
      view = TREE_VIEW(widget)
      selection = view.get_selection()

    if selection.count_selected_rows() <= 1:
      if view.get_path_at_pos(gint(event.x), gint(event.y),
       addr tvQueuePath, nil, 0, 0) != 0:
        selection.unselect_all()
        selection.select_path(tvQueuePath)
      if tvQueuePath != nil:
        view.queueTreeViewShowMenu(event, data)
      return true
  return false


proc queueTreeViewMenuCb(widget: PWidget, data: Pgpointer): bool =
  TREE_VIEW(widget).queueTreeViewShowMenu(nil, data)
  return true


proc queueTreeViewRefresh() =
  var
    iter: TTreeIter
    limit: uint64 = parseBiggestUInt(pdbConn.getProfileDataActive("size"))
    d: int = 0
  tvQueueStore.clear()
  var fileList = pdbConn.getQueueDataAll()
  for item in fileList:
    if item.existsFile():
      let size: uint64 = uint64(getFileSize(item))
      let splFile = item.splitFile()
      let name = splFile.name & splFile.ext
      var status = ""
      if size > limit and limit != 0:
        status = "BIG"
      else:
        status = " OK"
      var strSize = "$1 MB" % [ (float(size) / 1000000) | (10,2)]
      tvQueueStore.append(addr iter, nil)
      tvQueueStore.set(addr iter, TVQ_NAME, name, TVQ_SIZE, strSize, TVQ_STATUS,
        status, TVQ_PATH, item, -1)
    else:
      if pdbConn.removeQueueData(item):
        inc(d)
  if d != 0:
    discard sbQueue.push(1, "Removed $1 invalid queue item$2.." % [$d,
      if d == 1: "" else: "s"])
    pbUpdateIdle()

proc queueCreateTreeViewMenu(window: gtk2.PWindow): PMenu =
  result = menu_new()
  var
    miOpenFile = menu_item_new("Open File")
    miOpenFolder = menu_item_new("Open Folder")
    miRemove = menu_item_new("Remove from queue")
  
  discard OBJECT(miOpenFile).signal_connect("activate",
   SIGNAL_FUNC(queueMenuItemDo), cast[Pgpointer](TVQ_OFI))
  discard OBJECT(miOpenFolder).signal_connect("activate",
   SIGNAL_FUNC(queueMenuItemDo), cast[Pgpointer](TVQ_OFO))
  discard OBJECT(miRemove).signal_connect("activate",
   SIGNAL_FUNC(queueMenuItemDo), cast[Pgpointer](TVQ_REM)) 
  
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
  uploadControl(widget, data)
  closeWindow(widget, data)

proc queueStartPfc(widget: PWidget, data: Pgpointer) =
  pfcStart(nil, data)
  queueTreeViewRefresh()


proc queueReset(widget: PWidget, data: Pgpointer) =
  let window = gtk2.WINDOW(data)
  if yesOrNo(window, "Do you really want to delete all items from queue ?"):
    if pdbConn.resetQueue():
      queueTreeViewRefresh()
      pbUpdateIdle()
      discard sbQueue.push(1, "Queue has been cleared of all items.")
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

  sbQueue = statusbar_new()
  vbMain.pack_start(sbQueue, true, true, 0)
  discard sbQueue.push(0, "")

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
   SIGNAL_FUNC(queueStartPfc), winQueue)
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
  
  queueTreeViewRefresh()
  winQueue.show_all()

