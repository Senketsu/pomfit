type
  StringChannel* = Channel[string]
  TSplitFile* = tuple
    dir, name, ext: string
  TDlgEnum* = enum
    INFO = 0
    WARN = 1
    ERR = 3
  tvQueueEnum* = enum
    TVQ_FILENAME = (0, "Filename")
    TVQ_STATUS = (1, "Status")
    TVQ_SIZE = (2, "Size")
    TVQ_PATH = (3, "Path")
    TVQ_iCol = (4, "nil")