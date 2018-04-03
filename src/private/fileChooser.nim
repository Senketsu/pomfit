import projTypes
import projUtils

when defined(guiHybrid):
  discard ## TODO
else:

  import os, strutils
  import glib2, gtk2, gdk2pixbuf
  from gui_gtk import chanMain, chanUp



  proc update(widget: PWidget, data: Pgpointer) =
    var dialog = FILE_CHOOSER(widget)
    let pvPath = get_preview_filename(dialog)
    if pvPath == nil or pvPath == "":
      return
    
    var
      acceptTypes: seq[string] = @[".png", ".jpeg", ".jpg",
       ".jpe", ".bmp", ".tiff", ".tif", ".gif"]
      pvWidget = IMAGE(dialog.get_preview_widget())
      spFile: TSplitFile = splitFile($pvPath)
    
    for ext in acceptTypes:
      if ext == spFile.ext:
        let pvImage = pixbuf_new_from_file_at_size(pvPath, 192, -1, nil)
        pvWidget.set_from_pixbuf(pvImage)
        dialog.set_preview_widget_active(true)
        return
    dialog.set_preview_widget_active(false)
  
  
  proc create*(window: PWindow, curDir: string = ""): PFileChooser =
    var preview = image_new()
    preview.set_size_request(192,192)

    var filterAll = file_filter_new()
    filterAll.add_pattern("*")
    filterAll.set_name("All files")
    var filterImg = file_filter_new()
    filterImg.add_mime_type("image/*")
    filterImg.set_name("Images")
    var filterAud = file_filter_new()
    filterAud.add_mime_type("audio/*")
    filterAud.set_name("Audio")
    var filterVid = file_filter_new()
    filterVid.add_mime_type("video/*")
    filterVid.set_name("Video")

    result = file_chooser_dialog_new("Upload file(s)", window,
      FILE_CHOOSER_ACTION_OPEN,
      "Cancel", RESPONSE_CANCEL,
      "Select", RESPONSE_ACCEPT, nil)
    discard result.set_current_folder_uri(getHomeDir())
    result.set_use_preview_label(false)
    result.add_filter(filterAll)
    result.add_filter(filterImg)
    result.add_filter(filterAud)
    result.add_filter(filterVid)
    result.set_select_multiple(true)
    result.set_preview_widget(preview)
    discard result.g_signal_connect("update-preview",
     G_CALLBACK(fileChooser.update), nil)
    
    return result


  proc open*(window: PWindow, root: string = ""): seq[string] =
    var dialog = fileChooser.create(window)
    if root.len > 0:
      discard dialog.set_current_folder_uri(root)
    
    result = @[]
    if dialog.run() == cint(RESPONSE_ACCEPT):
      var uriList = dialog.get_uris()
      while uriList != nil:
        result.add($cast[cstring](uriList.data))
        g_free(uriList.data)
        uriList = uriList.next
      free(uriList)
    dialog.destroy()
  
  proc start*(widget: PWidget, data: Pgpointer){.procvar.} =
    let filePaths = fileChooser.open(WINDOW(data))
    if filePaths != @[]:
      for path in filePaths:
        chanUp[].send("file: $1" % path)
      let fNo = (filePaths.high + 1)
      let info = ("Added $1 file$2 into the queue." % [
        $fNo, if fNo == 0: "" else: "s"])
      ## TODO: a way to push this into status bar /  progress bar
      ##  maybe a global pointer .. is that even gc safe ?
