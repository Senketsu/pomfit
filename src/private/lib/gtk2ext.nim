{.deadCodeElim: on.}
{.push gcsafe.}
import gtk2, glib2, atk, pango, gdk2pixbuf, gdk2

when defined(win32):
  const lib = "libgtk-win32-2.0-0.dll"
elif declared(gtk_quartz):
  const lib = "libgtk-quartz-2.0.0.dylib"
elif defined(macosx):
  const lib = "libgtk-x11-2.0.dylib"
else:
  const lib = "libgtk-x11-2.0.so(|.0)"

proc set_no_show_all*(widget: PWidget, no_show_all: gboolean) {.cdecl,
    importc: "gtk_widget_set_no_show_all", dynlib: lib.}
proc get_no_show_all*(widget: PWidget): gboolean {.cdecl,
    importc: "gtk_widget_get_no_show_all", dynlib: lib.}

proc count_selected_rows*(selection: PTreeSelection): gint {.cdecl,
    importc: "gtk_tree_selection_count_selected_rows", dynlib: lib.}
proc get_path_at_pos*(tree_view: PTreeView, x:gint, y:gint, path: ptr PTreePath,
    column: PTreeViewColumn, cell_x: gint, cell_y: gint): gint {.cdecl,
    importc: "gtk_tree_view_get_path_at_pos", dynlib: lib.}


type
  TImageData* = object {.union.}
     pixbuf*: PPixbuf
     stock_id*: cstring
     icon_name*: cstring
     gicon*: pointer

  PStatusIconPrivate* = ptr TStatusIconPrivate
  TStatusIconPrivate*{.final, pure.} = object
    dummy_widget*: PWidget
    nid*: pointer
    tray_icon*: PWidget
    image*: PWidget
    last_click_x*: gint
    last_click_y*: gint
    taskbar_top*: gint
    orientation*: POrientation
    tooltip_text*: cstring
    title*: cstring
    size*: gint
    image_width*: gint
    image_height*: gint
    storage_type*: gtk2.PImageType
    image_data*: TImageData
    blank_icon*: PPixbuf
    blinking_timeout*: gint
    blinking*: gint
    blink_off*: gint
    visible*: gint

  PStatusIcon* = ptr TStatusIcon
  TStatusIcon* = object of TGObject
    priv*: PStatusIconPrivate

  PStatusIconClass* = ptr TStatusIconClass
  TStatusIconClass* = object of TGObjectClass
    activate*: proc (status_icon: PStatusIcon){.cdecl.}
    popup_menu*: proc (status_icon: PStatusIcon, button: guint,activate_time: guint32){.cdecl.}
    size_changed*: proc (status_icon: PStatusIcon,size: gint): gboolean{.cdecl.}
    button_press_event*: proc (status_icon: PStatusIcon,event: PEventButton):gboolean{.cdecl.}
    button_release_event*: proc (status_icon: PStatusIcon,event: PEventButton):gboolean{.cdecl.}
    scroll_event*: proc (status_icon: PStatusIcon,event: PEventButton):gboolean{.cdecl.}
    query_tooltip*: proc (status_icon: PStatusIcon,x: gint,y:gint,
      keyboard_mode: gboolean,tooltip: PTooltips):gboolean{.cdecl.}
    reserved1*: proc (){.cdecl.}
    reserved2*: proc (){.cdecl.}


proc status_icon_get_type*(): GType{.cdecl,
    importc: "gtk_status_icon_get_type", dynlib: lib.}

proc status_icon_new*():PStatusIcon {.cdecl,
    importc: "gtk_status_icon_new", dynlib: lib.}
proc status_icon_new_from_pixbuf*(pixbuf: PPixbuf):PStatusIcon {.cdecl,
    importc: "gtk_status_icon_new_from_pixbuf", dynlib: lib.}
proc status_icon_new_from_file*(filename: PPgchar):PStatusIcon {.cdecl,
    importc: "gtk_status_icon_new_from_file", dynlib: lib.}
proc status_icon_new_from_stock*(stock_id: PPgchar):PStatusIcon {.cdecl,
    importc: "gtk_status_icon_new_from_stock", dynlib: lib.}
proc status_icon_new_from_icon_name*(icon_name: PPgchar):PStatusIcon {.cdecl,
    importc: "gtk_status_icon_new_from_icon_name", dynlib: lib.}

proc status_icon_set_from_pixbuf*(status_icon: PStatusIcon,pixbuf: PPixbuf) {.cdecl,
    importc: "gtk_status_icon_set_from_pixbuf", dynlib: lib.}
proc status_icon_set_from_file*(status_icon: PStatusIcon,filename: cstring) {.cdecl,
    importc: "gtk_status_icon_set_from_file", dynlib: lib.}
proc status_icon_set_from_stock*(status_icon: PStatusIcon,stock_id: cstring) {.cdecl,
    importc: "gtk_status_icon_set_from_stock", dynlib: lib.}
proc status_icon_set_from_icon_name*(status_icon: PStatusIcon,icon_name: cstring) {.cdecl,
    importc: "gtk_status_icon_set_from_icon_name", dynlib: lib.}

proc status_icon_get_storage_type*(status_icon: PStatusIcon): gtk2.PImageType {.cdecl,
    importc: "gtk_status_icon_get_storage_type", dynlib: lib.}
proc status_icon_get_pixbuf*(status_icon: PStatusIcon):PPixbuf {.cdecl,
    importc: "gtk_status_icon_get_pixbuf", dynlib: lib.}
proc status_icon_get_stock*(status_icon: PStatusIcon):cstring {.cdecl,
    importc: "gtk_status_icon_get_stock", dynlib: lib.}
proc status_icon_get_icon_name*(status_icon: PStatusIcon):cstring {.cdecl,
    importc: "gtk_status_icon_get_icon_name", dynlib: lib.}
proc status_icon_get_size*(status_icon: PStatusIcon):gint {.cdecl,
    importc: "gtk_status_icon_get_size", dynlib: lib.}

proc status_icon_set_screen*(status_icon: PStatusIcon,screen: PScreen) {.cdecl,
    importc: "gtk_status_icon_set_screen", dynlib: lib.}
proc status_icon_get_screen*(status_icon: PStatusIcon):PScreen {.cdecl,
    importc: "gtk_status_icon_get_screen", dynlib: lib.}

proc status_icon_set_has_tooltip*(status_icon: PStatusIcon,has_tooltip: gboolean) {.cdecl,
    importc: "gtk_status_icon_set_has_tooltip", dynlib: lib.}
proc status_icon_set_tooltip_text*(status_icon: PStatusIcon,text: cstring) {.cdecl,
    importc: "gtk_status_icon_set_tooltip_text", dynlib: lib.}
proc status_icon_set_tooltip_markup*(status_icon: PStatusIcon,markup: cstring) {.cdecl,
    importc: "gtk_status_icon_set_tooltip_markup", dynlib: lib.}
proc status_icon_set_title*(status_icon: PStatusIcon,title: cstring) {.cdecl,
    importc: "gtk_status_icon_set_title", dynlib: lib.}
proc status_icon_get_title*(status_icon: PStatusIcon):cstring {.cdecl,
    importc: "gtk_status_icon_get_title", dynlib: lib.}
proc status_icon_set_name*(status_icon: PStatusIcon,name: cstring) {.cdecl,
    importc: "gtk_status_icon_set_name", dynlib: lib.}
proc status_icon_set_visible*(status_icon: PStatusIcon,visible: gboolean) {.cdecl,
    importc: "gtk_status_icon_set_visible", dynlib: lib.}
proc status_icon_get_visible*(status_icon: PStatusIcon):gboolean {.cdecl,
    importc: "gtk_status_icon_get_visible", dynlib: lib.}

proc status_icon_is_embedded*(status_icon: PStatusIcon):gboolean {.cdecl,
    importc: "gtk_status_icon_is_embedded", dynlib: lib.}
proc status_icon_position_menu*(menu: PMenu,x: Pgint,y: Pgint,push_in: Pgboolean,user_data: gpointer) {.cdecl,
    importc: "gtk_status_icon_position_menu", dynlib: lib.}
proc status_icon_get_geometry*(status_icon: PStatusIcon,screen: PScreen,area: gdk2.PRectangle,orientation: POrientation):gboolean {.cdecl,
    importc: "gtk_status_icon_get_geometry", dynlib: lib.}
proc status_icon_get_has_tooltip*(status_icon: PStatusIcon):gboolean {.cdecl,
    importc: "gtk_status_icon_get_has_tooltip", dynlib: lib.}
proc status_icon_get_tooltip_text*(status_icon: PStatusIcon):cstring {.cdecl,
    importc: "gtk_status_icon_get_tooltip_text", dynlib: lib.}
proc status_icon_get_tooltip_markup*(status_icon: PStatusIcon):cstring {.cdecl,
    importc: "gtk_status_icon_get_tooltip_markup", dynlib: lib.}

proc status_icon_get_x11_window_id*(status_icon: PStatusIcon):guint32 {.cdecl,
    importc: "gtk_status_icon_get_x11_window_id", dynlib: lib.}

proc TYPE_STATUS_ICON*(): GType =
  result = status_icon_get_type()
proc STATUS_ICON_TYPE*(wid: pointer): GType =
  result = OBJECT_TYPE(wid)
proc `STATUS_ICON`*(anObject: pointer): PStatusIcon =
  result = cast[PStatusIcon](CHECK_CAST(anObject, TYPE_STATUS_ICON()))
proc STATUS_ICON_CLASS*(klass: pointer): PStatusIconClass =
  result = cast[PStatusIconClass](CHECK_CLASS_CAST(klass, TYPE_STATUS_ICON()))
proc IS_STATUS_ICON*(anObject: pointer): bool =
  result = CHECK_TYPE(anObject, TYPE_STATUS_ICON())
proc IS_STATUS_ICON_CLASS*(klass: pointer): bool =
  result = CHECK_CLASS_TYPE(klass, TYPE_STATUS_ICON())
proc STATUS_ICON_GET_CLASS*(anObject: pointer): PStatusIconClass =
  result = cast[PStatusIconClass](CHECK_GET_CLASS(anObject, TYPE_STATUS_ICON()))

type
  PFileChooserButtonPrivate* = ptr TFileChooserButtonPrivate
  TFileChooserButtonPrivate*{.final, pure.} = object
    dialog*: PWidget
    button*: PWidget
    image*: PWidget
    label*: PWidget
    combo_box*: PWidget
    icon_cell*: PCellRenderer
    name_cell*: PCellRenderer
    model*: PTreeModel
    filter_model*: PTreeModel
    fs*: pointer

    selection_while_inactive*: pointer
    current_folder_while_inactive*: pointer

    combo_box_changed_id*: gulong
    fs_volumes_changed_id*: gulong
    fs_bookmarks_changed_id*: gulong

    dnd_select_folder_cancellable*: pointer
    update_button_cancellable*: pointer
    change_icon_theme_cancellable*: PGSList

    icon_size*: gint
    n_special*: guint8
    n_volumes*: guint8
    n_shortcuts*: guint8
    n_bookmarks*: guint8
    has_bookmark_separator*: guint
    has_current_folder_separator*: guint
    has_current_folder*: guint
    has_other_separator*: guint
    active*: guint
    focus_on_click*: guint
    is_changing_selection*: guint



  PFileChooserButton* = ptr TFileChooserButton
  TFileChooserButton* = object of THbox
    priv*: PFileChooserButtonPrivate

  PFileChooserButtonClass* = ptr TFileChooserButtonClass
  TFileChooserButtonClass* = object of THBoxClass
    file_set* : proc (fc: PFileChooserButton){.cdecl.}

proc file_chooser_button_get_type*():GType {.cdecl,
    importc: "gtk_file_chooser_button_get_type", dynlib: lib.}
proc file_chooser_button_new*(): PWidget {.cdecl,
    importc: "gtk_file_chooser_button_new", dynlib: lib.}
proc file_chooser_button_new_with_dialog*(dialog: PWidget): PWidget {.cdecl,
    importc: "gtk_file_chooser_button_new_with_dialog", dynlib: lib.}
proc file_chooser_button_get_title*(button: PFileChooserButton): cstring {.cdecl,
    importc: "gtk_file_chooser_button_get_title", dynlib: lib.}
proc file_chooser_button_set_title*(button: PFileChooserButton, title: cstring) {.cdecl,
    importc: "gtk_file_chooser_button_set_title", dynlib: lib.}
proc file_chooser_button_get_width_chars*(button: PFileChooserButton):gint {.cdecl,
    importc: "gtk_file_chooser_button_get_width_chars", dynlib: lib.}
proc file_chooser_button_set_width_chars*(button: PFileChooserButton,n_chars: gint) {.cdecl,
    importc: "gtk_file_chooser_button_set_width_chars", dynlib: lib.}
proc file_chooser_button_get_focus_on_click*(button: PFileChooserButton): gboolean {.cdecl,
    importc: "gtk_file_chooser_button_get_focus_on_click", dynlib: lib.}
proc file_chooser_button_set_focus_on_click*(button: PFileChooserButton,focus_on_click: gboolean) {.cdecl,
    importc: "gtk_file_chooser_button", dynlib: lib.}

proc TYPE_FILE_CHOOSER_BUTTON*(): GType =
  result = file_chooser_button_get_type()
proc FILE_CHOOSER_TYPE*(wid: pointer): GType =
  result = OBJECT_TYPE(wid)
proc `FILE_CHOOSER_BUTTON`*(anObject: pointer): PFileChooserButton =
  result = cast[PFileChooserButton](CHECK_CAST(anObject, TYPE_FILE_CHOOSER_BUTTON()))
proc FILE_CHOOSER_BUTTON_CLASS*(klass: pointer): PFileChooserButtonClass =
  result = cast[PFileChooserButtonClass](CHECK_CLASS_CAST(klass, TYPE_FILE_CHOOSER_BUTTON()))
proc IS_FILE_CHOOSER_BUTTON*(anObject: pointer): bool =
  result = CHECK_TYPE(anObject, TYPE_FILE_CHOOSER_BUTTON())
proc IS_FILE_CHOOSER_BUTTON_CLASS*(klass: pointer): bool =
  result = CHECK_CLASS_TYPE(klass, TYPE_FILE_CHOOSER_BUTTON())
proc FILE_CHOOSER_BUTTON_GET_CLASS*(anObject: pointer): PFileChooserButtonClass =
  result = cast[PFileChooserButtonClass](CHECK_GET_CLASS(anObject, TYPE_FILE_CHOOSER_BUTTON()))

type
  PLinkButtonPrivate* = ptr TLinkButtonPrivate
  TLinkButtonPrivate*{.final, pure.} = object
    uri*: cstring
    visited*: gboolean
    popup_menu*: PWidget

  PLinkButton* = ptr TLinkButton
  TLinkButton* = object of TButton
    priv*: PLinkButtonPrivate

  PLinkButtonClass* = ptr TLinkButtonClass
  TLinkButtonClass* = object of TButtonClass
    padding1*: proc (){.cdecl.}
    padding2*: proc (){.cdecl.}
    padding3*: proc (){.cdecl.}
    padding4*: proc (){.cdecl.}

proc link_button_get_type*():GType {.cdecl,
    importc: "gtk_link_button_get_type", dynlib: lib.}

proc link_button_new*(uri: cstring): PWidget {.cdecl,
    importc: "gtk_link_button_new", dynlib: lib.}
proc link_button_new_with_label*(uri: cstring,label: cstring): PWidget {.cdecl,
    importc: "gtk_link_button_new_with_label", dynlib: lib.}
proc link_button_get_uri*(link_button: PLinkButton): cstring {.cdecl,
    importc: "gtk_link_button_get_uri", dynlib: lib.}
proc link_button_set_uri*(link_button: PLinkButton,uri: cstring) {.cdecl,
    importc: "gtk_link_button_set_uri", dynlib: lib.}
proc link_button_get_visited*(link_button: PLinkButton): gboolean {.cdecl,
    importc: "gtk_link_button_get_visited", dynlib: lib.}
proc link_button_set_visited*(link_button: PLinkButton,visited: gboolean) {.cdecl,
    importc: "gtk_link_button_set_visited", dynlib: lib.}

proc TYPE_LINK_BUTTON*(): GType =
  result = link_button_get_type()
proc `LINK_BUTTON`*(anObject: pointer): PLinkButton =
  result = cast[PLinkButton](CHECK_CAST(anObject, TYPE_LINK_BUTTON()))
proc LINK_BUTTON_CLASS*(klass: pointer): PLinkButtonClass =
  result = cast[PLinkButtonClass](CHECK_CLASS_CAST(klass, TYPE_LINK_BUTTON()))
proc IS_LINK_BUTTON*(anObject: pointer): bool =
  result = CHECK_TYPE(anObject, TYPE_LINK_BUTTON())
proc IS_LINK_BUTTON_CLASS*(klass: pointer): bool =
  result = CHECK_CLASS_TYPE(klass, TYPE_LINK_BUTTON())
proc LINK_BUTTON_GET_CLASS*(anObject: pointer): PLinkButtonClass =
  result = cast[PLinkButtonClass](CHECK_GET_CLASS(anObject, TYPE_LINK_BUTTON()))

proc show_uri* (screen: gdk2.PScreen, uri: cstring,
    timestamp: guint32,error: pointer): gboolean {.cdecl,
    importc: "gtk_show_uri", dynlib: lib.}
