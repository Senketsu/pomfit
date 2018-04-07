import os, strutils
import ndbex/db_sqlite_ex
import projTypes
import projUtils

proc createDefaults(conn: DbConn) =
  conn.exec(sql("""INSERT INTO profiles (id, name, api, size, form_id,
    form_type, isActive) VALUES (0, ?, ?, 150000000, ?, ?, 1)"""), "Uguu.se",  
    "http://uguu.se/api.php?d=upload", "file", "multipart/form-data")


proc createPomfitDatabase(conn: DbConn) =
  try:
    conn.exec(sql("""CREATE TABLE profiles 
      (id integer PRIMARY KEY AUTOINCREMENT, name VARCHAR(255) NOT NULL UNIQUE,
      api VARCHAR(255) NOT NULL, size INTEGER NOT NULL, regex_f VARCHAR(127),
      regex_d VARCHAR(127), form_id VARCHAR(63) NOT NULL,
      form_type VARCHAR(127) NOT NULL, form_name VARCHAR(255),
      prepend VARCHAR(255), append VARCHAR(255),
      isActive INTEGER NOT NULL DEFAULT 0)"""))

    conn.exec(sql("""CREATE TABLE uploads (id INTEGER PRIMARY KEY AUTOINCREMENT,
      name VARCHAR(255) NOT NULL, url VARCHAR(255) NOT NULL UNIQUE,
      durl VARCHAR(255), fpath VARCHAR(511))"""))

    conn.exec(sql("""CREATE TABLE queue (id INTEGER PRIMARY KEY AUTOINCREMENT,
      path VARCHAR(511) NOT NULL UNIQUE)"""))
    conn.createDefaults()
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])

proc openPomfitDatabase*(): DbConn =
  var path = projUtils.getPath("fileDB")
  if not fileExists(path):
    result = open(connection = path, "", "", "")
    result.createPomfitDatabase()
  else:
    result = open(connection = path, "", "", "")


proc getProfilesActive*(conn: DbConn): string =
  try:
    let val = conn.getValueNew(sql("""SELECT name FROM profiles WHERE isActive = 1"""))
    if val.hasData:
      result = val.data
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc getProfilesNames*(conn: DbConn): seq[string] =
  try:
    var rows = conn.getAllRowsNew(sql("""SELECT name FROM profiles"""))
    result = @[]
    for row in rows:
      if row.hasData:
        result.add(row.data[0])
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])

proc getProfileDataActive*(conn: DbConn, name: string): string =
  try:
    result = ""
    let res = conn.getValueNew(sql("""SELECT size FROM profiles WHERE isActive = 1"""))
    if res.hasData:
      result = res.data
    else:
      result = "0"
  except:
    logEvent(true, "***Error DB: Reading profile data..\t\n '$1'" % [
      getCurrentExceptionMsg()])


proc getProfileDataAll*(conn: DbConn, name: string): seq[string] =
  try:
    result = @[]
    var row: RowNew
    row = conn.getRowNew(sql("""SELECT * FROM profiles WHERE name =  ?"""),name)
    result = row.data
    result.delete(0)
  except:
    logEvent(true, "***Error DB: Reading profile data..\t\n '$1'" % [
      getCurrentExceptionMsg()])

proc updateProfilesActive*(conn: DbConn, name: string): bool =
  try:
    conn.exec(sql("""UPDATE profiles SET isActive = 0 WHERE isActive = 1"""))
    conn.exec(sql("""UPDATE profiles SET isActive = 1 WHERE name = ?"""), name)
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])

proc deleteProfile*(conn: DbConn, name: string): bool =
  try:
    var isAct = conn.getValue(sql("""SELECT isActive FROM profiles WHERE name = ?"""), name)
    if isAct == "1":
      conn.exec(sql("""UPDATE profiles SET isActive = 1 WHERE id = 0"""))
      
    conn.exec(sql("""DELETE FROM profiles WHERE name = ?"""), name)
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])

proc addProfileData*(conn: DbConn, pData: seq[string]): bool =
  try:
    conn.exec(sql("""INSERT INTO profiles (name, api, size, regex_f, regex_d,
      form_id, form_type, form_name, prepend, append) VALUES (?,
      ?, ?, ?, ?, ?, ?, ?, ?, ?)"""), pData[0], pData[1], pData[2], pData[3],
      pData[4], pData[5], pData[6], pData[7], pData[8], pData[9])
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc updateProfileData*(conn: DbConn, pData: seq[string]): bool =
  try:
    conn.exec(sql("""INSERT OR REPLACE INTO profiles (name, api, size, regex_f,
      regex_d, form_id, form_type, form_name, prepend, append) VALUES (?,
      ?, ?, ?, ?, ?, ?, ?, ?, ?)"""), pData[0], pData[1], pData[2], pData[3],
      pData[4], pData[5], pData[6], pData[7], pData[8], pData[9])
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc resetProfiles*(conn: DbConn): bool =
  try:
    conn.exec(sql("""DELETE FROM profiles"""))
    conn.createDefaults()
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc resetQueue*(conn: DbConn): bool =
  try:
    conn.exec(sql("""DELETE FROM queue"""))
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc getQueueDataAll*(conn: DbConn): seq[string] =
  try:
    result = @[]
    var rows = conn.getAllRowsNew(sql("""SELECT path FROM queue"""))
    for row in rows:
      if row.hasData:
        result.add(row.data)
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc removeQueueData*(conn: DbConn, path: string): bool =
  try:
    conn.exec(sql("""DELETE FROM queue WHERE path = ?"""), path)
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])
    

proc addQueueData*(conn: DbConn, path: string): bool =
  try:
    conn.exec(sql("""INSERT INTO queue (path) VALUES (?)"""), path)
    result = true
  except:
    let errMsg = getCurrentExceptionMsg()
    if errMsg.startsWith("UNIQUE constraint"):
      discard
    else:
      logEvent(true, "***Error: $1\n$2" % [errMsg, repr getCurrentException()])


proc getQueueLen*(conn: DbConn): int =
  try:
    var len = conn.getValue(sql("""SELECT COUNT(*) FROM queue"""))
    result = parseInt(len)
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])

    
