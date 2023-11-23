namespace FOEDAG {
   
enum ProgrammerErrorCode {
  NoError = 0,
  InvalidArgument = -100,
  DeviceNotFound = -101,
  CableNotFound = -102,
  CableNotSupported = -104,
  NoSupportedTapFound = -105,
  FailedExecuteCommand = -106,
  FailedToParseOutput = -107,
  BitfileNotFound = -108,
  FailedToProgramFPGA = -109,
  OpenOCDExecutableNotFound = -110,
  FailedToProgramOTP = -111,
  InvalidFlashSize = -112,
  UnsupportedFunc = -113,
};

}