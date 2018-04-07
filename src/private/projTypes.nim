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