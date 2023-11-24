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

#ifndef __OPENOCDADAPTER_H__
#define __OPENOCDADAPTER_H__

#include <atomic>
#include <functional>
#include <string>
#include <vector>

#include "Device.h"
#include "JtagAdapter.h"

namespace FOEDAG {

enum CommandOutputType {
  NOT_OUTPUT = 0,
  CMD_PROGRESS,
  CMD_ERROR,
  CMD_TIMEOUT,
  CBUFFER_TIMEOUT,
  CONFIG_ERROR,
  CONFIG_SUCCESS,
  UNKNOWN_FIRMWARE,
  FSBL_BOOT_FAILURE,
  INVALID_BITSTREAM,
};

class OpenocdAdapter : public JtagAdapter {
 public:
  OpenocdAdapter(std::string openocd);
  virtual ~OpenocdAdapter();
  virtual std::vector<uint32_t> scan(const Cable &cable);
  int program_fpga(const Device &device, DeviceType device_type,
                   const std::vector<Tap> &taplist, std::string bitfile,
                   std::atomic<bool> &stop,
                   std::function<void(float)> progress_callback);
  static CommandOutputType check_output(std::string str,
                                        std::vector<std::string> &output);
  static bool check_regex(std::string str, std::string pattern,
                          std::vector<std::string> &output);
  std::string get_last_output() { return m_last_output; };

 protected:
  int execute(const Cable &cable, std::string cmd, std::string &output);
  std::string convert_transport_to_string(TransportType transport,
                                          std::string defval = "jtag");
  std::string build_cable_config(const Cable &cable);
  std::string build_tap_config(const std::vector<Tap> &taplist);
  std::string build_target_config(const Device &device);
  std::string m_openocd;
  std::string m_last_output;
};

}  // namespace FOEDAG

#endif  //__OPENOCDADAPTER_H__
