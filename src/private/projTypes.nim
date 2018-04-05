type
  StringChannel* = Channel[string]
  TSplitFile* = tuple
    dir, name, ext: string
  TDlgEnum* = enum
    INFO = 0
    WARN = 1
    ERR = 3
  
