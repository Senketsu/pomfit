import os, strutils
import ndbex/db_sqlite_ex
import projTypes
import projUtils

const KeyBindsO = 8

proc createDefaultProfiles(conn: DbConn) =
  conn.exec(sql("""INSERT INTO profiles (id, name, api, size, form_id,
    form_type, form_name, prepend, append, regex_f, regex_d, isActive) VALUES 
    (0, ?, ?, 150000000, ?, ?, ?, ?, ?, ?, ?, 1)"""), "Uguu.se",  
    "http://uguu.se/api.php?d=upload", "file", "multipart/form-data",
     "", "", "", "", "")

proc createDefaultKeybinds(conn: DBConn) =
  for i in 0..11:
    conn.exec(sql("""INSERT INTO keybinds (id, mod1, mod2, key, full, isActive)
     VALUES (?, ?, ?, ?, ?, ?)"""),i, 0, 0, "", "", 0)

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
    
    conn.exec(sql("""CREATE TABLE keybinds (id INTEGER PRIMARY KEY,
      mod1 INTEGER NOT NULL DEFAULT 0, mod2 INTEGER NOT NULL DEFAULT 0,
      key VARCHAR(7), full VARCHAR(63),
      isActive INTEGER NOT NULL DEFAULT 0)"""))

    conn.exec(sql("""CREATE TABLE settings (id INTEGER PRIMARY KEY,
      name VARCHAR(127) NOT NULL, value VARCHAR(127) NOT NULL)"""))
    
    conn.createDefaultProfiles()
    conn.createDefaultKeybinds()
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

proc getProfileActive*(conn: DbConn): TPuProfile =
  try:
    let row = conn.getAllRowsNew(sql("""SELECT * FROM profiles WHERE isActive = 1"""))
    result.name = row[0].data[1]
    result.api = row[0].data[2]
    result.size = row[0].data[3]
    result.regex_f = row[0].data[4]
    result.regex_d = row[0].data[5]
    result.formID = row[0].data[6]
    result.formType = row[0].data[7]
    result.formName = row[0].data[8]
    result.prepStr = row[0].data[9]
    result.appStr = row[0].data[10]
  except:
    logEvent(true, "***Error DB: Reading profile data..\t\n '$1'" % [
      getCurrentExceptionMsg()])


proc getProfileDataActive*(conn: DbConn, name: string): string =
  try:
    result = ""
    var queryRaw = "SELECT $1 FROM profiles WHERE isActive = 1" % name
    let res = conn.getValueNew(sql(queryRaw))
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
    conn.createDefaultProfiles()
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc resetQueue*(conn: DbConn): bool =
  try:
    conn.exec(sql("""DELETE FROM queue"""))
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc getQueueDataAll*(conn: DbConn, order: string = "ASC", limit: string = "-1"): seq[string] =
  try:
    result = @[]
    var queryRaw = "SELECT path FROM queue ORDER BY id $1 LIMIT ?" % order
    var rows = conn.getAllRowsNew(sql(queryRaw), limit)
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


proc getKeyBindsAll*(conn: DbConn): seq[TPomfitKb] =
  try:
    var rows = conn.getAllRowsNew(sql("""SELECT * FROM keybinds"""))
    result = @[]
    var item: TPomfitKb
    for i in 0..KeyBindsO:
      item.id = parseInt(rows[i].data[0])
      item.mod1 = cast[TBindModEnum](parseInt(rows[i].data[1]))
      item.mod2 = cast[TBindModEnum](parseInt(rows[i].data[2]))
      item.key = rows[i].data[3]
      item.full = rows[i].data[4]
      item.isActive = if rows[i].data[5] == "1": true else: false
      result.add(item)
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc setKeyBindsAll*(conn: DbConn, keyBinds: seq[TPomfitKb]): bool =
  try:
    for i in 0..KeyBindsO:
      conn.exec(sql("""INSERT OR REPLACE INTO keybinds (id, mod1, mod2, key,
       full, isActive) VALUES (?, ?, ?, ?, ?, ?)"""), keyBinds[i].id,
       ord(keyBinds[i].mod1), ord(keyBinds[i].mod2), keyBinds[i].key,
      keyBinds[i].full, if keyBinds[i].isActive: 1 else: 0)
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc resetKeyBinds*(conn: DbConn): bool =
  try:
    conn.exec(sql("""DELETE FROM keybinds"""))
    conn.createDefaultKeybinds()
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc getSettings*(conn: DbConn, choice: int): string =
  result = ""
  try:
    var value = conn.getValueNew(sql("""SELECT value FROM settings 
      WHERE id = ?"""), choice)
    if value.hasData:
      result = value.data
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc getSettingsAll*(conn: DbConn): seq[TPomfitSett] =
  try:
    result = @[]
    var rows = conn.getAllRowsNew(sql("""SELECT * FROM settings"""))
    for item in rows:
      var data: TPomfitSett
      data.id = parseInt(item.data[0])
      data.name = item.data[1]
      data.value = item.data[2]
      result.add(data)
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc setSettingsAll*(conn: DbConn, data: seq[TPomfitSett]): bool =
  try:
    for item in data:
      if item.name != "":
        conn.exec(sql("""INSERT OR REPLACE INTO settings (id, name, value)
          VALUES (?, ?, ?)"""),item.id, item.name, item.value)
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc resetSettings*(conn: DbConn): bool =
  try:
    conn.exec(sql("""DELETE FROM settins"""))
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])


proc addUploadsData*(conn: DbConn, data: TPuData): bool =
  try:
    conn.exec(sql("""INSERT INTO uploads (name, url, durl, path) VALUES 
      (?, ?, ?, ?)"""), data.name, data.url, data.durl, data.path)
    result = true
  except:
    logEvent(true, "***Error: $1\n$2" % [getCurrentExceptionMsg(), repr getCurrentException()])
    