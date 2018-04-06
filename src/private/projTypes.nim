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
    