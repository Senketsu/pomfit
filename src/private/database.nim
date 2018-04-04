import os, strutils
import db_sqlite
import projTypes
import projUtils

proc createDefaults(conn: DbConn) =
  conn.exec(sql("""INSERT INTO profiles (id, name, api, size, form_id,
    form_type, isDefault) VALUES (0, ?, ?, 150000000, ?, ?, 1)"""), "Uguu.se",  
    "http://uguu.se/api.php?d=upload", "file", "multipart/form-data")


proc createPomfitDatabase(conn: DbConn) =
  try:
    conn.exec(sql("""CREATE TABLE profiles 
      (id integer PRIMARY KEY AUTOINCREMENT, name VARCHAR(255) NOT NULL UNIQUE,
      api VARCHAR(255) NOT NULL, size INTEGER NOT NULL, regex_f VARCHAR(127),
      regex_d VARCHAR(127), form_id VARCHAR(63) NOT NULL,
      form_type VARCHAR(127) NOT NULL, form_name VARCHAR(255),
      prepend VARCHAR(255), append VARCHAR(255),
      isDefault INTEGER NOT NULL DEFAULT 0)"""))

    conn.exec(sql("""CREATE TABLE uploads (id INTEGER PRIMARY KEY AUTOINCREMENT,
      name VARCHAR(255) NOT NULL, url VARCHAR(255) NOT NULL UNIQUE,
      durl VARCHAR(255), fpath VARCHAR(511))"""))

    conn.exec(sql("""CREATE TABLE queue (id INTEGER PRIMARY KEY AUTOINCREMENT,
      path VARCHAR(511) NOT NULL UNIQUE)"""))
    conn.createDefaults()
  except:
    logEvent(true, "***Error DB: Creating Pomfit database..\t\n '$1'" % [
      getCurrentExceptionMsg()])

proc openPomfitDatabase*(): DbConn =
  var path = projUtils.getPath("fileDB")
  if not fileExists(path):
    result = open(connection = path, "", "", "")
    result.createPomfitDatabase()
  else:
    result = open(connection = path, "", "", "")



