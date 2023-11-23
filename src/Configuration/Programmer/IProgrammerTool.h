// IProgrammerTool.h
#ifndef IPROGRAMMERTOOL_H
#define IPROGRAMMERTOOL_H

#include <string>
#include <atomic>


namespace FOEDAG {

struct Device;
enum class ProgramFlashOperation : uint32_t;

class IProgrammerTool {
public:
  virtual ~IProgrammerTool() = default;

  virtual int ProgrammerFPGA(const Device& device, const std::string& bitfile, std::atomic<bool>& stop) = 0;
  virtual int ProgramFlash(const Device& device, const std::string& bitfile, ProgramFlashOperation operations, std::atomic<bool>& stop) = 0;
  virtual int ProgramOTP(const Device& device, const std::string& bitfile, std::atomic<bool>& stop) = 0;
};

} // namespace FOEDAG


#endif // IPROGRAMMERTOOL_H
