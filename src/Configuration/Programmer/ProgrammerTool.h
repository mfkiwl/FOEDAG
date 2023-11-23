// ProgrammerTool.h
#ifndef PROGRAMMERTOOL_H
#define PROGRAMMERTOOL_H

#include "IProgrammerTool.h"
#include "../HardwareManager/JtagAdapter.h" // Assuming JtagAdapter is defined in another file

namespace FOEDAG {
   
class JtagAdapter;
struct Device;

enum class ProgramFlashOperation : uint32_t;

class ProgrammerTool : public IProgrammerTool {
private:
    JtagAdapter* m_adapter;

public:
    ProgrammerTool(JtagAdapter* adapter);
    ~ProgrammerTool() override;

    int ProgrammerFPGA(const Device& device, const std::string& bitfile, std::atomic<bool>& stop) override;
    int ProgramFlash(const Device& device, const std::string& bitfile, ProgramFlashOperation operations, std::atomic<bool>& stop) override;
    int ProgramOTP(const Device& device, const std::string& bitfile, std::atomic<bool>& stop) override;
};

}


#endif // PROGRAMMERTOOL_H
