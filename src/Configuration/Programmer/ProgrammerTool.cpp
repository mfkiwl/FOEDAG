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

ProgrammerTool::ProgrammerTool(JtagAdapter* adapter) : m_adapter(adapter) {
  CFG_ASSERT(m_adapter != nullptr);
}

ProgrammerTool::~ProgrammerTool() {
    // Clean-up code if needed
}

int ProgrammerTool::ProgrammerFPGA(const Device& device, const std::string& bitfile, std::atomic<bool>& stop) {
  // Implementation for programming FPGA
  int status = ProgrammerErrorCode::NoError;
  std::error_code ec;
  std::string errorMessage;
  if (!std::filesystem::exists(bitfile, ec)) {
    return ProgrammerErrorCode::BitfileNotFound;
  }
  return status; // Return appropriate status
}

int ProgrammerTool::ProgramFlash(const Device& device, const std::string& bitfile, ProgramFlashOperation operations, std::atomic<bool>& stop) {
    // Implementation for programming Flash
    return 0; // Return appropriate status
}

int ProgrammerTool::ProgramOTP(const Device& device, const std::string& bitfile, std::atomic<bool>& stop) {
    // Implementation for programming OTP
    return 0; // Return appropriate status
}

} // namespace FOEDAG end

