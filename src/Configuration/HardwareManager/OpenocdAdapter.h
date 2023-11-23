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

#include "JtagAdapter.h"
#include "ProgramingAdapter.h"
namespace FOEDAG {
using CommandExecutorFuncType = std::function<int(
    const std::string &, std::string &, std::ostream *, std::atomic<bool> &)>;

class OpenocdAdapter : public JtagAdapter, public ProgramingAdapter {
 public:
  OpenocdAdapter(std::string openocd_filepath,
                 CommandExecutorFuncType command_executor)
      : m_openocd_filepath(openocd_filepath),
        m_command_executor(command_executor) {}
  virtual std::vector<uint32_t> scan(const Cable &cable) override;

  virtual int pld(const Device &device, std::string bitfile, std::atomic<bool>& stop,
                                  progress_func_type progress_callback) override;
  virtual int otp(const Device &device, std::string bitfile, std::atomic<bool>& stop,
                                  progress_func_type progress_callback) override;
  virtual int flash(const Device &device, std::string bitfile, std::atomic<bool>& stop,
                                  progress_func_type progress_callback) override;
 private:
  int execute(const Cable &cable, std::string &output);
  std::string convert_transport_to_string(TransportType transport,
                                          std::string defval = "jtag");
  std::string m_openocd_filepath;
  CommandExecutorFuncType m_command_executor;
};
}  // namespace FOEDAG
#endif  //__OPENOCDADAPTER_H__
