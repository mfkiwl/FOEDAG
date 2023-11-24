/*
Copyright 2022 The Foedag team

GPL License

Copyright (c) 2022 The Open-Source FPGA Foundation

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
#include "ProgrammerGuiIntegration.h"

#include <QDebug>

namespace FOEDAG {

bool operator==(const Cable &c1, const Cable &c2) {
  return c1.vendor_id == c2.vendor_id && c1.product_id == c2.product_id &&
         c1.bus_addr == c2.bus_addr && c1.port_addr == c2.port_addr &&
         c1.device_addr == c2.device_addr && c1.channel == c2.channel &&
         c1.serial_number == c2.serial_number &&
         c1.description == c2.description && c1.speed == c2.speed &&
         c1.transport == c2.transport && c1.name == c2.name &&
         c1.index == c2.index;
}

bool operator==(const Tap &t1, const Tap &t2) {
  return t1.idcode == t2.idcode && t1.index == t2.index &&
         t1.irlength == t2.irlength;
}

bool operator==(const Device &d1, const Device &d2) {
  return d1.index == d2.index && d1.name == d2.name &&
         d1.flashSize == d2.flashSize && d1.type == d2.type &&
         d1.cable == d2.cable && d1.tap == d2.tap;
}

// bool operator==(const Tap &t1, const Tap &t2) {
//   return //t1.enabled == t2.enabled &&
//          //t1.expected == t2.expected &&
//          t1.idcode == t2.idcode && t1.index == t2.index &&
//          //t1.irCap == t2.irCap &&
//          t1.irlength == t2.irlength;// &&
//          //t1.irMask == t2.irMask;
//          //&& t1.tapName == t2.tapName;
// }

bool operator<(const Device &d1, const Device &d2) {
  return d1.name.size() < d2.name.size();
}

ProgrammerGuiIntegration::ProgrammerGuiIntegration(QObject *parent)
    : QObject(parent) {
  qRegisterMetaType<std::string>("std::string");
}

const sequential_map<Cable, std::vector<Device> >
    &ProgrammerGuiIntegration::devices() const {
  return m_devices;
}

void ProgrammerGuiIntegration::Cables(const std::vector<Cable> &cables) {
  for (const auto &cable : cables) {
    m_devices[cable] = {};
  }
}

void ProgrammerGuiIntegration::Devices(const Cable &cable,
                                       const std::vector<Device> &devices) {
  for (const auto &[cab, dev] : m_devices.values()) {
    if (cab == cable) {
      m_devices[cab] = devices;
      break;
    }
  }
  emit autoDetect();
}

void ProgrammerGuiIntegration::Progress(const std::string &progress) {
  emit this->progress({m_current.first, m_current.second, m_type}, progress);
}

void ProgrammerGuiIntegration::ProgramFpga(const Cable &cable,
                                           const Device &device,
                                           const std::string &file) {
  m_current = std::make_pair(cable, device);
  m_type = Type::Fpga;
  m_files[device].bitstream = file;
  m_stop = false;
  emit programStarted({cable, device, m_type});
}

void ProgrammerGuiIntegration::ProgramOtp(const Cable &cable,
                                          const Device &device,
                                          const std::string &file) {
  m_current = std::make_pair(cable, device);
  m_type = Type::Otp;
  m_files[device].bitstream = file;
  m_stop = false;
  emit programStarted({cable, device, m_type});
}

void ProgrammerGuiIntegration::Flash(const Cable &cable, const Device &device,
                                     const std::string &file) {
  m_current = std::make_pair(cable, device);
  m_type = Type::Flash;
  m_files[device].flashBitstream = file;
  m_stop = false;
  emit programStarted({cable, device, m_type});
}

void ProgrammerGuiIntegration::Status(const Cable &cable, const Device &device,
                                      int status) {
  emit this->status({cable, device, m_type}, status);
}

std::atomic_bool &ProgrammerGuiIntegration::Stop() { return m_stop; }

std::string ProgrammerGuiIntegration::File(const Device &dev,
                                           bool flash) const {
  auto it = m_files.find(dev);
  if (it != m_files.end()) {
    return flash ? it->second.flashBitstream : it->second.bitstream;
  }
  return {};
}

void ProgrammerGuiIntegration::StopLastProcess() { m_stop = true; }

}  // namespace FOEDAG
