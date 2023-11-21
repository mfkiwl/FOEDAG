#include "ProgrammerTool.h"

#include "Configuration/CFGCommon/CFGCommon.h"
#include "Programmer_errror_code.h"

namespace FOEDAG {

std::map<int, std::string> ErrorMessages = {
    {NoError, "Success"},
    {InvalidArgument, "Invalid argument"},
    {DeviceNotFound, "Device not found"},
    {CableNotFound, "Cable not found"},
    {CableNotSupported, "Cable not supported"},
    {NoSupportedTapFound, "No supported tap found"},
    {FailedExecuteCommand, "Failed to execute command"},
    {FailedToParseOutput, "Failed to parse output"},
    {BitfileNotFound, "Bitfile not found"},
    {FailedToProgramFPGA, "Failed to program FPGA"},
    {OpenOCDExecutableNotFound, "OpenOCD executable not found"},
    {FailedToProgramOTP, "Failed to program device OTP"},
    {InvalidFlashSize, "Invalid flash size"},
    {UnsupportedFunc, "Unsupported function"}};

ProgrammerTool::ProgrammerTool(ProgrammingAdapter* adapter)
    : m_adapter(adapter) {
  CFG_ASSERT(m_adapter != nullptr);
}

ProgrammerTool::~ProgrammerTool() {
  // Clean-up code if needed
}

int ProgrammerTool::program_fpga(
    const Device& device, const std::string& bitfile, std::atomic<bool>& stop,
    std::ostream* outStream /*= nullptr*/,
    OutputMessageCallback callbackMsg /*= nullptr*/,
    ProgressCallback callbackProgress /*= nullptr*/) {
  int statusCode = ProgrammerErrorCode::NoError;
  std::error_code ec;
  std::string errorMessage;
  if (!std::filesystem::exists(bitfile, ec)) {
    return ProgrammerErrorCode::BitfileNotFound;
  }
  statusCode = m_adapter->program_fpga(device, bitfile, stop, outStream,
                                       callbackMsg, callbackProgress);
  return statusCode;
}

int ProgrammerTool::program_flash(
    const Device& device, const std::string& bitfile, std::atomic<bool>& stop,
    ProgramFlashOperation modes, std::ostream* outStream /*= nullptr*/,
    OutputMessageCallback callbackMsg /*= nullptr*/,
    ProgressCallback callbackProgress /*= nullptr*/) {
  int statusCode = ProgrammerErrorCode::NoError;
  std::error_code ec;
  std::string errorMessage;
  if (!std::filesystem::exists(bitfile, ec)) {
    return ProgrammerErrorCode::BitfileNotFound;
  }
  statusCode = m_adapter->program_flash(device, bitfile, stop, modes, outStream,
                                        callbackMsg, callbackProgress);
  return statusCode;
}

int ProgrammerTool::program_otp(
    const Device& device, const std::string& bitfile, std::atomic<bool>& stop,
    std::ostream* outStream /*= nullptr*/,
    OutputMessageCallback callbackMsg /*= nullptr*/,
    ProgressCallback callbackProgress /*= nullptr*/) {
  int statusCode = ProgrammerErrorCode::NoError;
  std::error_code ec;
  std::string errorMessage;
  if (!std::filesystem::exists(bitfile, ec)) {
    return ProgrammerErrorCode::BitfileNotFound;
  }
  statusCode = m_adapter->program_otp(device, bitfile, stop, outStream,
                                      callbackMsg, callbackProgress);
  return statusCode;
}

int ProgrammerTool::query_fpga_status(const Device& device,
                                      CfgStatus& cfgStatus,
                                      std::string& outputMessage) {
  int statusCode = ProgrammerErrorCode::NoError;
  statusCode = m_adapter->query_fpga_status(device, cfgStatus, outputMessage);
  return statusCode;
}

}  // namespace FOEDAG
