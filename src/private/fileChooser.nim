when defined(guiHybrid):
  discard ## TODO

else:
  proc yesOrNo*(window: PWindow, question: string): bool =
    let label = label_new(question)
    let ynDialog = dialog_new_with_buttons(question, window,
      DIALOG_DESTROY_WITH_PARENT, STOCK_NO, RESPONSE_NO,
      STOCK_YES, RESPONSE_YES,nil)
    ynDialog.vbox.pack_start(label, true, true, 30)
    ynDialog.show_all()

    if ynDialog.run() == RESPONSE_YES:
      result = true
    ynDialog.destroy()


  proc infoUser*(window: PWindow, msgType: TDlgEnum, msg: string) =
    var dialog = message_dialog_new(window,
        DIALOG_MODAL or DIALOG_DESTROY_WITH_PARENT,
        cast[TMessageType](int(msgType)), BUTTONS_OK, "%s", cstring(msg))
    dialog.setTitle("Pomfit Info")
    discard dialog.run()
    dialog.destroy()
  
  
  proc pfcUpdate(widget: PWidget, data: Pgpointer) =
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
        if ext == ".gif":
          var pvImage = pixbuf_animation_new_from_file(pvPath, nil)
          pvWidget.set_from_animation(pvImage)
        else:
          var pvImage = pixbuf_new_from_file_at_size(pvPath, 224, -1, nil)
          pvWidget.set_from_pixbuf(pvImage)
        dialog.set_preview_widget_active(true)
        return
    dialog.set_preview_widget_active(false)
  
  
  proc pfcCreate(window: PWindow, curDir: string = ""): PFileChooser =
    var preview = image_new()
    preview.set_size_request(224, 224)

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
     G_CALLBACK(gui_gtk.pfcUpdate), nil)


  proc pfcOpen(window: PWindow, root: string = ""): seq[string] =
    var dialog = pfcCreate(window)
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
  
  proc pfcStart*(widget: PWidget, data: Pgpointer){.procvar.} =
    let fileURIs = pfcOpen(WINDOW(data))
    if fileURIs != @[]:
      var fNO = 0
      var info: string = ""
      for uri in fileURIs:
        var filePath = uri
        filePath.delete(0,6)
        if IsInstantUpload:
          chanUp[].send("file:$1" % [filePath])
        else:
          if pdbConn.addQueueData(filePath):
            inc(fNo)
      if IsInstantUpload:
        info = ("Selected $1 file$2 for upload." % [
          $fNo, if fNo == 1: "" else: "s"])
      else:
        info = ("Added $1 new file$2 into the queue." % [
          $fNo, if fNo == 1: "" else: "s"])
      discard sbMain.push(0, info)
      pbUpdateIdle()

