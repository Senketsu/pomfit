type
  StringChannel* = Channel[string]

  TSplitFile* = tuple
    dir, name, ext: string

  TDlgEnum* = enum
    INFO = 0
    WARN = 1
    ERR = 3

  tvQueueEnum* = enum
    TVQ_NAME = (0, "Filename")
    TVQ_SIZE = (1, "Size")
    TVQ_STATUS = (2, "Status")
    TVQ_PATH = (3, "Path")
    TVQ_iCol = (4, "nil")

  tvQueueDo* = enum
    TVQ_OFI = 0
    TVQ_OFO = 1
    TVQ_REM = 2

  TBindModEnum* = enum
    MOD_NONE = (0,"")
    MOD_CTRL = (1,"<CTRL>")
    MOD_SHIFT = (2,"<SHIFT>")
    MOD_ALT = (3,"<ALT>")
    MOD_WIN = (4,"<WIN>")
  
  TPomfitKb* = tuple
    id: int
    mod1, mod2:TBindModEnum
    key: string
    full: string
    isActive: bool
  
  SSMode* = enum
    FULL_CAP = (0, "")
    FULL_UP = (1, "up")
    AREA_CAP = (2, "")
    AREA_UP = (3, "up")
    WIN_CAP = (4, "")
    WIN_UP = (5, "up")
    AWIN_CAP = (6, "")
    AWIN_UP = (7, "up")

  TPomfitSett* = tuple
    id: int
    name, value: string

  SettingsEnum* = enum
    SS_PATH = (0, "SS Folder")
    SS_NAME = (1, "SS Filename")
    SS_TS = (2, "SS Timestamp")
    UNKNOW_02 = (3, "")
    UNKNOW_03 = (4, "")
    UNKNOW_04 = (5, "")
    UNKNOW_05 = (6, "")
    UNKNOW_06 = (7, "")
    UNKNOW_07 = (8, "")
    UNKNOW_08 = (9, "")
  
  TPuProfile* = tuple
    name, api, size: string
    regex_f, regex_d: string
    formID, formType, formName: string
    prepStr, appStr: string
  
  TPuData* = tuple
    name, url, durl, path: string
  
