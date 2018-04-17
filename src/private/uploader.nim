import strutils, os, streams, httpclient
# import asyncdispatch
import projUtils
import projTypes
import db_sqlite
import database

var 
  chanUp*: ptr StringChannel
  chanMain*: ptr StringChannel
  pdbConn*: DbConn


proc parse*(resp: Stream, profile: TPuProfile, file: string): TPuData =
  discard


proc start*(all: bool, limit: int = 0) =
  var
    client = newHttpClient()
    profile = pdbConn.getProfileActive()
    queue: seq[string] = @[]

  if all:
    queue = pdbConn.getQueueDataAll()
  else:
    queue = pdbConn.getQueueDataAll("DESC", $limit)
    
  for file in queue:
    var response: Response
    if profile.formType == "multipart/form-data":
      var
        data = newMultipartData()
        fileName = ""

      if profile.formName == "":
        fileName = "uploaded_file"
      else:
        fileName = profile.formName
      data.addFiles({fileName: file})
      response = client.post(profile.api, multipart=data)

    else:
      client.headers = newHttpHeaders({ "Content-Type": profile.formType})
      let body = "" ## todo load file
      response = client.request(profile.api, httpMethod = HttpPost, body = $body)
    
    case int(response.code())
    of 400..599:
      discard # handle error
    of 200..299:
      let upData = response.bodyStream.parse(profile, file)
      if pdbConn.addUploadsData(upData):
        discard # fire upload finished notification
      if not pdbConn.removeQueueData(file):
        discard # fire notification error
    else:
      discard # unhandled status code


proc idle*(channelMain, channelUp:  ptr StringChannel) =
  chanUp = channelUp
  chanMain = channelMain
  pdbConn = openPomfitDatabase()
  
  while true:
    let chanBuffer = chanUp[].recv()
    echoInfo("*Debug: chanUp received: " & chanBuffer)
    case chanBuffer
    of "quit":
      quit(QuitSuccess)
    of "cmd:start":
      uploader.start(true)
    else:
      discard
    

