/*
Copyright 2023 The Foedag team

GPL License

Copyright (c) 2023 The Open-Source FPGA Foundation

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "OpenocdAdapter.h"

#include <filesystem>
#include <regex>
#include <sstream>

#include "Configuration/CFGCommon/CFGCommon.h"
#include "Configuration/HardwareManager/HardwareManager.h"

namespace FOEDAG {

OpenocdAdapter::OpenocdAdapter(std::string openocd) : m_openocd(openocd) {}

OpenocdAdapter::~OpenocdAdapter() {}

std::vector<uint32_t> OpenocdAdapter::scan(const Cable &cable) {
  std::vector<uint32_t> idcode_array;
  std::string line;
  std::cmatch matches;
  std::string output;

  const std::string pattern(
      R"((\d+) +(\w+.\w+) +([YN]) +(0x[0-9a-f]+) +(0x[0-9a-f]+) +(\d+) +(0x[0-9a-f]+) +(0x[0-9a-f]+))");

  // OpenOCD "scan_chain" command output text example:-
  //    TapName            Enabled IdCode     Expected   IrLen IrCap IrMask
  // -- ------------------ ------- ---------- ---------- ----- ----- ------
  //  0 omap5912.dsp          Y    0x03df1d81 0x03df1d81    38 0x01  0x03
  //  1 omap5912.arm          Y    0x0692602f 0x0692602f     4 0x01  0x0f
  //  2 omap5912.unknown      Y    0x00000000 0x00000000     8 0x01  0x03
  //  3 auto0.tap             Y    0x20000913 0x00000000     5 0x01  0x03

  // use "scan_chain" cmd to collect tap ids
  CFG_ASSERT_MSG(execute(cable, "scan_chain", output) == 0, "cmdexec error: %s",
                 output.c_str());
  std::stringstream ss(output);

  while (std::getline(ss, line)) {
    if (std::regex_search(line.c_str(), matches,
                          std::regex(pattern, std::regex::icase)) == true) {
      uint32_t idcode = (uint32_t)CFG_convert_string_to_u64(matches[4]);
      idcode_array.push_back(idcode);
    }
  }

  return idcode_array;
}

int OpenocdAdapter::program_fpga(const Device &device,
                                 const std::vector<Tap> &taplist,
                                 std::string bitfile, std::atomic<bool> &stop,
                                 std::function<void(double)> output_callback) {
  std::ostringstream ss;
  std::string output;

  CFG_ASSERT(std::filesystem::exists(m_openocd));
  CFG_ASSERT(std::filesystem::exists(bitfile));
  CFG_ASSERT(output_callback != nullptr);
  CFG_ASSERT(device.type == GEMINI || device.type == VIRGO);

  ss << " -l /dev/stdout"  //<-- not windows friendly
     << " -d2";

  ss << build_cable_config(device.cable) << build_tap_config(taplist)
     << build_target_config(device);

  std::string cmd = "gemini load 1 fpga " + bitfile + " -p 1 -d " +
                    (device.type == GEMINI ? "gemini" : "virgo");

  ss << " -c \"init\"";
  ss << " -c \"" << cmd << "\"";
  ss << " -c \"exit\"";

  CFG_POST_MSG("[cmd: %d]", ss.str().c_str());

  int count = 0;

  // run the command
  int res = CFG_execute_cmd_with_callback(
      "OPENOCD_DEBUG_LEVEL=-3 " + m_openocd + ss.str(), output, nullptr,
      std::regex{}, stop, nullptr, [&count](const std::string &line) {
        CFG_POST_MSG("[line %d: %s]", ++count, line.c_str());
      });

  CFG_ASSERT_MSG(res == 0, "cmdexec error: %s", output.c_str());

  return 0;
}

int OpenocdAdapter::execute(const Cable &cable, std::string cmd,
                            std::string &output) {
  std::atomic<bool> stop = false;
  std::ostringstream ss;

  CFG_ASSERT(std::filesystem::exists(m_openocd));

  ss << " -l /dev/stdout"  //<-- not windows friendly
     << " -d2";
  ss << build_cable_config(cable);
  ss << " -c \"init\"";
  ss << " -c \"" << cmd << "\"";
  ss << " -c \"exit\"";

  // run the command
  int res = CFG_execute_cmd("OPENOCD_DEBUG_LEVEL=-3 " + m_openocd + ss.str(),
                            output, nullptr, stop);
  return res;
}

std::string OpenocdAdapter::convert_transport_to_string(TransportType transport,
                                                        std::string defval) {
  switch (transport) {
    case TransportType::JTAG:
      return "jtag";
      // Handle other transport types as needed
  }
  return defval;
}

std::string OpenocdAdapter::build_cable_config(const Cable &cable) {
  std::ostringstream ss;

  // setup cable type specific configuration
  if (cable.cable_type == FTDI) {
    ss << " -c \"adapter driver ftdi;"
       << "ftdi vid_pid " << std::hex << std::showbase << cable.vendor_id << " "
       << cable.product_id << ";" << std::noshowbase << std::dec
       << "ftdi layout_init 0x0c08 0x0f1b;";

    if (!cable.serial_number.empty()) {
      ss << "adapter serial " << cable.serial_number << ";";
    }
  } else if (cable.cable_type == JLINK) {
    ss << " -c \"adapter driver jlink;";
  }

  // setup general cable configuration
  ss << "adapter speed " << cable.speed << ";"
     << "transport select " << convert_transport_to_string(cable.transport)
     << ";"
     << "telnet_port disabled;"
     << "gdb_port disabled;\"";

  return ss.str();
}

std::string OpenocdAdapter::build_tap_config(const std::vector<Tap> &taplist) {
  std::ostringstream ss;

  // setup tap configuration
  if (!taplist.empty()) {
    ss << " -c \"";
    for (const auto &tap : taplist) {
      ss << "jtag newtap tap" << tap.index << " tap"
         << " -irlen " << tap.irlength << " -expected-id " << std::hex
         << std::showbase << tap.idcode << ";" << std::noshowbase << std::dec;
    }
    ss << "\"";
  }

  return ss.str();
}

std::string OpenocdAdapter::build_target_config(const Device &device) {
  std::ostringstream ss;

  // setup target configuration
  if (device.type == GEMINI || device.type == VIRGO) {
    ss << " -c \"target create gemini" << device.index
       << " riscv -endian little -chain-position tap" << device.tap.index
       << ".tap;\""
       // add pld driver
       << " -c \"pld device gemini gemini" << device.index << "\"";
  } else if (device.type == OCLA) {
    ss << " -c \"target create gemini" << device.index
       << " testee -chain-position tap" << device.tap.index << ".tap;\"";
  }

  return ss.str();
}

std::unique_ptr<HardwareManager> HardwareManager::create_instance(
    CFGCommon_ARG *cmdarg) {
  return std::make_unique<HardwareManager>(
      std::make_unique<OpenocdAdapter>(cmdarg->toolPath.string()));
}

}  // namespace FOEDAG

/* for testing purpose */
void test_hwmgr(CFGCommon_ARG *cmdarg, std::string bitfile, std::string cable,
                uint32_t device_index) {
  using namespace FOEDAG;

  CFG_POST_MSG("I m here!");

  auto hardware_manager = HardwareManager::create_instance(cmdarg);
  OpenocdAdapter adapter{cmdarg->toolPath};
  std::atomic<bool> stop = false;
  std::vector<Tap> taplist{};
  Device device{};
  int error_code = 0;

  if (!hardware_manager->is_cable_exists(cable, true)) {
    CFG_POST_ERR("Cable '%s' not found", cable.c_str());
    return;
  }

  if (!hardware_manager->find_device(cable, device_index, device, taplist,
                                     true)) {
    CFG_POST_ERR("Device %d not found", device_index);
    return;
  }

  error_code = adapter.program_fpga(
      device, taplist, bitfile, stop, [](double percentage) {
        CFG_post_msg(CFG_print("Progress...%2.2f%%", percentage),
                     "INFO: ", false);
      });
  if (!error_code) {
    CFG_POST_MSG("Programmed '%s' successfully.", bitfile.c_str());
  } else {
    CFG_POST_MSG("Failed to program '%s.'. Error code %d.", bitfile.c_str(),
                 error_code);
  }
}
