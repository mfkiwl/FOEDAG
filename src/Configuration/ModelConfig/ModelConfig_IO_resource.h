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

#ifndef MODEL_CONFIG_IO_RESOURCE_H
#define MODEL_CONFIG_IO_RESOURCE_H

#include <Configuration/CFGCommon/CFGCommon.h>

namespace FOEDAG {

struct PIN_INFO {
  PIN_INFO(std::string name);
  std::string type = "";
  std::string rtl_name = "";
  uint32_t bank = 0;
  bool is_clock = false;
  uint32_t index = 0;
  uint32_t pair_index = 0;
  uint32_t rx_io = 0;
};

struct ModelConfig_IO_MODEL {
  ModelConfig_IO_MODEL(const std::string& name, const std::string& ric_name,
                       const std::string& type, const std::string& subtype,
                       uint32_t bank);
  void assign(const std::string* const_ptr, const std::string& value) const;
  void backup() const;
  void restore() const;
  void set_instantiator(const std::string& instantiator) const;
  const std::string m_name = "";
  const std::string m_ric_name = "";
  const std::string m_type = "";
  const std::string m_subtype = "";
  const uint32_t m_bank = 0;
  std::string m_instantiator = "";
  std::string m_backup_instantiator = "";
};

struct ModelConfig_IO_PLL : ModelConfig_IO_MODEL {
  ModelConfig_IO_PLL(const std::string& name, const std::string& ric_name,
                     const std::string& type, const std::string& subtype,
                     uint32_t bank)
      : ModelConfig_IO_MODEL(name, ric_name, type, subtype, bank) {}
};

struct ModelConfig_IO_FCLK : ModelConfig_IO_MODEL {
  ModelConfig_IO_FCLK(const std::string& name, const std::string& ric_name,
                      const std::string& type, const std::string& subtype,
                      uint32_t bank)
      : ModelConfig_IO_MODEL(name, ric_name, type, subtype, bank) {}
};

struct ModelConfig_IO_RESOURCE {
  ModelConfig_IO_RESOURCE(std::vector<ModelConfig_IO_PLL> plls,
                          std::vector<ModelConfig_IO_FCLK> fclks);
  bool use_resource(std::vector<const ModelConfig_IO_MODEL*>& models,
                    const std::string& instantiator, const std::string& name,
                    const std::string& type);
  bool use_fclk(const std::string& instantiator, const std::string& name);
  bool use_pll(const std::string& instantiator, const std::string& name);
  uint32_t fclk_use_pll(const std::string& fclk);
  std::vector<const ModelConfig_IO_MODEL*> get_used_resource(
      std::vector<const ModelConfig_IO_MODEL*>& models,
      const std::string& instantiator);
  std::vector<const ModelConfig_IO_MODEL*> get_used_fclk(
      const std::string& instantiator);
  std::vector<const ModelConfig_IO_MODEL*> get_used_pll(
      const std::string& instantiator);
  uint32_t get_pll_availability();
  void backup();
  void restore();
  const std::vector<ModelConfig_IO_PLL> m_plls;
  const std::vector<ModelConfig_IO_FCLK> m_fclks;
  std::string m_msg = "";
};

struct ModelConfig_IO_62x44_RESOURCE : ModelConfig_IO_RESOURCE {
  ModelConfig_IO_62x44_RESOURCE();
};

}  // namespace FOEDAG

#endif