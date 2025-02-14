/*
Copyright 2021-2022 The Foedag team

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

#include "Simulator.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <charconv>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <sstream>
#include <thread>

#include "Compiler/Compiler.h"
#include "Compiler/Log.h"
#include "NewProject/ProjectManager/project_manager.h"
#include "ProjNavigator/tcl_command_integration.h"
#include "Utils/FileUtils.h"
#include "Utils/StringUtils.h"

using namespace FOEDAG;

Simulator::SimulationType Simulator::ToSimulationType(const std::string& str,
                                                      bool& ok) {
  ok = true;
  if (str == "rtl") return SimulationType::RTL;
  if (str == "pnr") return SimulationType::PNR;
  if (str == "gate") return SimulationType::Gate;
  if (str == "bitstream_bd") return SimulationType::BitstreamBackDoor;
  if (str == "bitstream_fd") return SimulationType::BitstreamFrontDoor;
  ok = false;
  return SimulationType::RTL;
}

Simulator::SimulatorType Simulator::ToSimulatorType(
    const std::string& str, bool& ok, SimulatorType defaultValue) {
  Simulator::SimulatorType sim_tool = defaultValue;
  ok = true;
  if (str == "verilator") {
    sim_tool = Simulator::SimulatorType::Verilator;
  } else if (str == "icarus") {
    sim_tool = Simulator::SimulatorType::Icarus;
  } else if (str == "ghdl") {
    sim_tool = Simulator::SimulatorType::GHDL;
  } else if (str == "vcs") {
    sim_tool = Simulator::SimulatorType::VCS;
  } else if (str == "questa") {
    sim_tool = Simulator::SimulatorType::Questa;
  } else if (str == "xcelium") {
    sim_tool = Simulator::SimulatorType::Xcelium;
  } else {
    ok = false;
  }
  return sim_tool;
}

std::string Simulator::ToString(SimulatorType type) {
  switch (type) {
    case Simulator::SimulatorType::GHDL:
      return "ghdl";
    case Simulator::SimulatorType::Icarus:
      return "icarus";
    case Simulator::SimulatorType::Questa:
      return "questa";
    case Simulator::SimulatorType::VCS:
      return "vcs";
    case Simulator::SimulatorType::Verilator:
      return "verilator";
    case Simulator::SimulatorType::Xcelium:
      return "xcelium";
  }
  return std::string{};
}

Simulator::Simulator(TclInterpreter* interp, Compiler* compiler,
                     std::ostream* out,
                     TclInterpreterHandler* tclInterpreterHandler)
    : m_interp(interp),
      m_compiler(compiler),
      m_out(out),
      m_tclInterpreterHandler(tclInterpreterHandler) {}

void Simulator::AddGateSimulationModel(const std::filesystem::path& path) {
  m_gateSimulationModels.push_back(path);
}

void Simulator::SetSimulatorCompileOption(const std::string& simulation,
                                          SimulatorType type,
                                          const std::string& options) {
  bool ok{false};
  auto level = ToSimulationType(simulation, ok);
  if (ok) {
    m_simulatorCompileOptionMap[level][type] = options;
  } else {
    for (auto level : {SimulationType::RTL, SimulationType::PNR,
                       SimulationType::Gate, SimulationType::BitstreamBackDoor,
                       SimulationType::BitstreamFrontDoor})
      m_simulatorCompileOptionMap[level][type] = options;
  }
}

void Simulator::SetSimulatorElaborationOption(const std::string& simulation,
                                              SimulatorType type,
                                              const std::string& options) {
  bool ok{false};
  auto level = ToSimulationType(simulation, ok);
  if (ok) {
    m_simulatorElaborationOptionMap[level][type] = options;
  } else {
    for (auto level : {SimulationType::RTL, SimulationType::PNR,
                       SimulationType::Gate, SimulationType::BitstreamBackDoor,
                       SimulationType::BitstreamFrontDoor})
      m_simulatorElaborationOptionMap[level][type] = options;
  }
}

void Simulator::SetSimulatorExtraOption(const std::string& simulation,
                                        SimulatorType type,
                                        const std::string& options) {
  bool ok{false};
  auto level = ToSimulationType(simulation, ok);
  if (ok) {
    m_simulatorExtraOptionMap[level][type] = options;
  } else {
    for (auto level : {SimulationType::RTL, SimulationType::PNR,
                       SimulationType::Gate, SimulationType::BitstreamBackDoor,
                       SimulationType::BitstreamFrontDoor})
      m_simulatorExtraOptionMap[level][type] = options;
  }
}

void Simulator::SetSimulatorSimulationOption(const std::string& simulation,
                                             SimulatorType type,
                                             const std::string& options) {
  bool ok{false};
  auto level = ToSimulationType(simulation, ok);
  if (ok) {
    m_simulatorSimulationOptionMap[level][type] = options;
  } else {
    for (auto level : {SimulationType::RTL, SimulationType::PNR,
                       SimulationType::Gate, SimulationType::BitstreamBackDoor,
                       SimulationType::BitstreamFrontDoor})
      m_simulatorSimulationOptionMap[level][type] = options;
  }
}

void Simulator::ResetGateSimulationModel() { m_gateSimulationModels.clear(); }

bool Simulator::RegisterCommands(TclInterpreter* interp) {
  bool ok = true;
  auto set_top_testbench = [](void* clientData, Tcl_Interp* interp, int argc,
                              const char* argv[]) -> int {
    Simulator* simulator = (Simulator*)clientData;
    std::ostringstream out;
    bool ok = simulator->m_compiler->GuiTclSync()->TclSetTopTestBench(
        argc, argv, out);
    if (!ok) {
      simulator->m_compiler->ErrorMessage(out.str());
      return TCL_ERROR;
    }
    return TCL_OK;
  };
  interp->registerCmd("set_top_testbench", set_top_testbench, this, 0);

  auto simulation_options = [](void* clientData, Tcl_Interp* interp, int argc,
                               const char* argv[]) -> int {
    Simulator* simulator = (Simulator*)clientData;
    if (argc > 3) {
      std::string options;
      std::string phase;
      std::string level;
      Simulator::SimulatorType sim_tool = Simulator::SimulatorType::Icarus;
      for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        bool ok{false};
        auto tmp = Simulator::ToSimulatorType(arg, ok);
        if (ok) sim_tool = tmp;
        if (arg == "compilation") {
          phase = "compilation";
        } else if (arg == "comp") {
          phase = "compilation";
        } else if (arg == "elaboration") {
          phase = "elaboration";
        } else if (arg == "elab") {
          phase = "elaboration";
        } else if (arg == "simul") {
          phase = "simulation";
        } else if (arg == "simulation") {
          phase = "simulation";
        } else if (arg == "extra_options") {
          phase = "extra_options";
        } else if (arg == "rtl" || arg == "pnr" || arg == "gate" ||
                   arg == "bitstream_bd" || arg == "bitstream_fd") {
          level = arg;
        } else {
          // skip level if present
          if (arg == "rtl" || arg == "pnr" || arg == "gate" ||
              arg == "bitstream_bd" || arg == "bitstream_fd") {
            continue;
          }
          // skip simulation name if present
          if (arg == "verilator" || arg == "icarus" || arg == "ghdl" ||
              arg == "vcs" || arg == "questa" || arg == "xcelium") {
            continue;
          }
          options += arg + " ";
        }
      }
      options = StringUtils::rtrim(options);
      if (phase == "compilation") {
        simulator->SetSimulatorCompileOption(level, sim_tool, options);
      } else if (phase == "elaboration") {
        simulator->SetSimulatorElaborationOption(level, sim_tool, options);
      } else if (phase == "simulation") {
        simulator->SetSimulatorSimulationOption(level, sim_tool, options);
      } else if (phase == "extra_options") {
        simulator->SetSimulatorExtraOption(level, sim_tool, options);
      }
      if (!phase.empty()) {
        if (simulator->m_compiler->GuiTclSync())
          simulator->m_compiler->GuiTclSync()->saveSettings();
        return TCL_OK;
      }
    }
    Tcl_AppendResult(interp,
                     "Invalid arguments. Usage: simulation_options <simulator> "
                     "<phase> ?<level>? <options>",
                     nullptr);
    return TCL_ERROR;
  };
  interp->registerCmd("simulation_options", simulation_options, this, 0);

  return ok;
}

bool Simulator::Clean(SimulationType action) {
  Message("Cleaning simulation results for " + ProjManager()->projectName());
  auto base = m_compiler->FilePath(Compiler::ToCompilerAction(action));
  if (!base.empty()) FileUtils::removeAll(base);
  SimulationOption(SimulationOpt::None);
  return true;
}

void Simulator::Message(const std::string& message) {
  m_compiler->Message(message);
}
void Simulator::ErrorMessage(const std::string& message) {
  m_compiler->ErrorMessage(message);
}

std::string Simulator::GetSimulatorCompileOption(SimulationType simulation,
                                                 SimulatorType type) {
  if (m_simulatorCompileOptionMap.count(simulation) != 0) {
    auto itr = m_simulatorCompileOptionMap[simulation].find(type);
    if (itr != m_simulatorCompileOptionMap[simulation].end())
      return (*itr).second;
  }
  return std::string{};
}

std::string Simulator::GetSimulatorElaborationOption(SimulationType simulation,
                                                     SimulatorType type) {
  if (m_simulatorElaborationOptionMap.count(simulation) != 0) {
    auto itr = m_simulatorElaborationOptionMap[simulation].find(type);
    if (itr != m_simulatorElaborationOptionMap[simulation].end())
      return (*itr).second;
  }
  return std::string{};
}

std::string Simulator::GetSimulatorExtraOption(SimulationType simulation,
                                               SimulatorType type) {
  if (m_simulatorExtraOptionMap.count(simulation) != 0) {
    auto itr = m_simulatorExtraOptionMap[simulation].find(type);
    if (itr != m_simulatorExtraOptionMap[simulation].end())
      return (*itr).second;
  }
  return std::string{};
}

std::string Simulator::GetSimulatorSimulationOption(SimulationType simulation,
                                                    SimulatorType type) {
  if (m_simulatorSimulationOptionMap.count(simulation) != 0) {
    auto itr = m_simulatorSimulationOptionMap[simulation].find(type);
    if (itr != m_simulatorSimulationOptionMap[simulation].end())
      return (*itr).second;
  }
  return std::string{};
}

void Simulator::SimulationOption(SimulationOpt option) {
  m_simulationOpt = option;
}

Simulator::SimulationOpt Simulator::SimulationOption() const {
  return m_simulationOpt;
}

void Simulator::WaveFile(SimulationType type, const std::string& file) {
  if (!file.empty()) m_waveFiles[type] = file;
}

std::string Simulator::WaveFile(SimulationType type) const {
  if (m_waveFiles.count(type) != 0) return m_waveFiles.at(type);
  return std::string{};
}

void Simulator::UserSimulationType(SimulationType simulation,
                                   SimulatorType simulator) {
  // create new item if not exists
  m_simulatorTypes[simulation] = simulator;
}

Simulator::SimulatorType Simulator::UserSimulationType(
    SimulationType simulation, bool& ok) const {
  if (m_simulatorTypes.count(simulation) != 0) {
    ok = true;
    return m_simulatorTypes.at(simulation);
  }
  ok = false;
  return SimulatorType::Verilator;
}

bool Simulator::Simulate(SimulationType action, SimulatorType type,
                         const std::string& wave_file) {
  if (!ProjManager()->HasDesign()) {
    ErrorMessage("No design specified");
    return false;
  }

  m_simType = action;
  if (SimulationOption() == SimulationOpt::Clean) return Clean(action);
  if (ProjManager()->SimulationFiles().empty()) {
    m_compiler->ErrorMessage("Simulation file(s) missing.");
    return false;
  }
  WaveFile(action, wave_file);
  UserSimulationType(action, type);
  m_waveFile = wave_file;
  if (wave_file.empty()) {
    m_waveFile = WaveFile(action);
  }
  if (m_waveFile.find(".vcd") != std::string::npos) {
    m_waveType = WaveformType::VCD;
  } else if (m_waveFile.find(".fst") != std::string::npos) {
    m_waveType = WaveformType::FST;
  } else if (m_waveFile.find(".ghw") != std::string::npos) {
    m_waveType = WaveformType::GHW;
  }
  switch (action) {
    case SimulationType::RTL: {
      return SimulateRTL(type);
      break;
    }
    case SimulationType::Gate: {
      return SimulateGate(type);
      break;
    }
    case SimulationType::PNR: {
      return SimulatePNR(type);
      break;
    }
    case SimulationType::BitstreamFrontDoor: {
      return SimulateBitstream(action, type);
      break;
    }
    case SimulationType::BitstreamBackDoor: {
      return SimulateBitstream(action, type);
      break;
    }
  }
  return false;
}

class ProjectManager* Simulator::ProjManager() const {
  return m_compiler->ProjManager();
}

std::string Simulator::FileList(SimulationType action) {
  std::string list;

  return list;
}

std::string Simulator::LogFile(SimulationType type) {
  switch (type) {
    case Simulator::SimulationType::RTL:
      return std::string{"simulation_rtl.rpt"};
    case Simulator::SimulationType::Gate:
      return std::string{"simulation_gate.rpt"};
    case Simulator::SimulationType::PNR:
      return std::string{"simulation_pnr.rpt"};
    case Simulator::SimulationType::BitstreamFrontDoor:
      return std::string{"simulation_bitstream_front.rpt"};
    case Simulator::SimulationType::BitstreamBackDoor:
      return std::string{"simulation_bitstream_back.rpt"};
  }
  return std::string{};
}

std::string Simulator::CommandLogFile(const std::string& prefix) const {
  std::string fileName{};
  if (m_compiler && m_compiler->ProjManager())
    fileName += m_compiler->ProjManager()->projectName() + "_";
  if (!prefix.empty()) fileName += prefix + "_";
  fileName += "simulation.cmd";
  return fileName;
}

std::string Simulator::SimulatorName(SimulatorType type) {
  switch (type) {
    case SimulatorType::Verilator:
      return "verilator";
    case SimulatorType::Icarus:
      return "iverilog";
    case SimulatorType::GHDL:
      return "ghdl";
    case SimulatorType::Questa:
      return "questa";
    case SimulatorType::VCS:
      return "vcs";
    case SimulatorType::Xcelium:
      return "xcelium";
  }
  return "Invalid";
}

std::string Simulator::IncludeDirective(SimulatorType type) {
  switch (type) {
    case SimulatorType::Verilator:
      return "-I";
    case SimulatorType::Icarus:
      return "-I";
    case SimulatorType::GHDL:
      m_compiler->SetError(StringUtils::format(
          "Include directive not supported by %", ToString(type)));
      return "Invalid";
    case SimulatorType::Questa:
      return "-I";
    case SimulatorType::VCS:
      return "+incdir+";
    case SimulatorType::Xcelium:
      return "-I";
  }
  return "Invalid";
}

std::string Simulator::LibraryPathDirective(SimulatorType type) {
  switch (type) {
    case SimulatorType::Verilator:
      return "-y ";
    case SimulatorType::Icarus:
      return "-y ";
    case SimulatorType::GHDL:
      return "-P";
    case SimulatorType::Questa:
      return "-y ";
    case SimulatorType::VCS:
      return "-y ";
    case SimulatorType::Xcelium:
      return "-y ";
  }
  return "Invalid";
}

std::string Simulator::LibraryFileDirective(SimulatorType type) {
  switch (type) {
    case SimulatorType::Verilator:
      return "-v ";
    case SimulatorType::Icarus:
      return "-l ";
    case SimulatorType::GHDL:
      return "";
    case SimulatorType::Questa:
      return "-v ";
    case SimulatorType::VCS:
      return "-v ";
    case SimulatorType::Xcelium:
      return "-v ";
  }
  return "Invalid";
}

std::string Simulator::LibraryExtDirective(SimulatorType type) {
  switch (type) {
    case SimulatorType::Verilator:
      return "+libext+";
    case SimulatorType::Icarus:
      return "-Y ";
    case SimulatorType::GHDL:
      m_compiler->SetError(StringUtils::format(
          "Library ext directive not supported by %", ToString(type)));
      return "Invalid";
    case SimulatorType::Questa:
      return "+libext+";
    case SimulatorType::VCS:
      return "+libext+";
    case SimulatorType::Xcelium:
      return "+libext+";
  }
  return "Invalid";
}

void Simulator::SetSimulatorPath(SimulatorType type, const std::string path) {
  m_simulatorPathMap.emplace(type, path);
}

std::filesystem::path Simulator::SimulatorExecPath(SimulatorType type) {
  std::map<SimulatorType, std::filesystem::path>::iterator itr =
      m_simulatorPathMap.find(type);
  if (itr != m_simulatorPathMap.end()) {
    return (*itr).second;
  }
  return "";
}

std::string Simulator::SimulatorCompilationOptions(SimulationType simulation,
                                                   SimulatorType type) {
  switch (type) {
    case SimulatorType::Verilator: {
      std::string options =
          "-cc --assert -Wall -Wno-DECLFILENAME "
          "-Wno-UNUSEDSIGNAL "
          "-Wno-TIMESCALEMOD "
          "-Wno-WIDTH -Wno-fatal -Wno-BLKANDNBLK -Wno-BLKLOOPINIT "
          "-Wno-MULTIDRIVEN -DSIM_VERILATOR ";
      if (simulation == SimulationType::PNR ||
          simulation == SimulationType::Gate)
        options += "--timing ";

      switch (m_waveType) {
        case WaveformType::VCD:
          options += "--trace ";
          break;
        case WaveformType::FST:
          options += "--trace-fst ";
          break;
        case WaveformType::GHW:
          break;
      }
      return options;
      break;
    }
    case SimulatorType::Icarus:
      return "-DIVERILOG=1 -v";
    case SimulatorType::GHDL:
      return "-a -fsynopsys -fexplicit";
    case SimulatorType::Questa:
      return "";
    case SimulatorType::VCS:
      return "-full64";
    case SimulatorType::Xcelium:
      return "";
  }
  return "Invalid";
}

std::string Simulator::MacroDirective(SimulatorType type) {
  switch (type) {
    case SimulatorType::Verilator:
      return "-D";
    case SimulatorType::Icarus:
      return "-D";
    case SimulatorType::GHDL:
      m_compiler->SetError(StringUtils::format(
          "Macro directive not supported by %", ToString(type)));
      return "Invalid";
    case SimulatorType::Questa:
      return "-D";
    case SimulatorType::VCS:
      return "-D";
    case SimulatorType::Xcelium:
      return "-D";
  }
  return "Invalid";
}

std::string Simulator::TopModuleCmd(SimulatorType type) {
  switch (type) {
    case SimulatorType::Verilator:
      return "--top-module ";
    case SimulatorType::Icarus:
      return "-s ";
    case SimulatorType::GHDL:
      return " ";
    case SimulatorType::Questa:
      return "Todo";
    case SimulatorType::VCS:
      return "-top ";
    case SimulatorType::Xcelium:
      return "Todo";
  }
  return "Invalid";
}

std::string Simulator::SimulationTypeMacro(SimulationType sim_type,
                                           SimulatorType simulator_type) {
  switch (simulator_type) {
    case SimulatorType::Verilator:
    case SimulatorType::Icarus:
    case SimulatorType::Questa:
    case SimulatorType::VCS:
    case SimulatorType::Xcelium: {
      std::string result = MacroDirective(simulator_type);
      switch (sim_type) {
        case SimulationType::RTL:
          return (result + "RTL_SIM=1");
        case SimulationType::Gate:
          return (result + "GATE_SIM=1");
        case SimulationType::PNR:
          return (result + "PNR_SIM=1");
        case SimulationType::BitstreamFrontDoor:
          return (result + "BITSTREAM_FD_SIM=1");
        case SimulationType::BitstreamBackDoor:
          return (result + "BITSTREAM_BD_SIM=1");
      }
      break;
    }
    case SimulatorType::GHDL:
      return "";
  }
  return "Invalid";
}

std::string Simulator::LanguageDirective(SimulatorType type,
                                         Design::Language lang) {
  switch (type) {
    case SimulatorType::Verilator:
      switch (lang) {
        case Design::Language::VERILOG_1995:
          return "+1364-1995ext+.v";
        case Design::Language::VERILOG_2001:
          return "+1364-2001ext+.v";
        case Design::Language::SYSTEMVERILOG_2005:
          return "+1364-2005ext+.v +1800-2005ext+.sv";
        case Design::Language::SYSTEMVERILOG_2009:
          return "+1800-2009ext+.v +1800-2009ext+.sv";
        case Design::Language::SYSTEMVERILOG_2012:
          return "+1800-2012ext+.v +1800-2012ext+.sv";
        case Design::Language::SYSTEMVERILOG_2017:
          return "+1800-2017ext+.v +1800-2017ext+.sv";
        case Design::Language::VERILOG_NETLIST:
          return "";
        case Design::Language::C:
          return "";
        case Design::Language::CPP:
          return "";
        default:
          m_compiler->SetError(StringUtils::format(
              "%: Invalid language for %", FOEDAG::ToString(lang),
              StringUtils::toUpper(ToString(type))));
          return "--invalid-lang-for-verilator";
      }
      break;
    case SimulatorType::Icarus:
      switch (lang) {
        case Design::Language::VERILOG_1995:
          return "-g1995";
        case Design::Language::VERILOG_2001:
          return "-g2001";
        case Design::Language::SYSTEMVERILOG_2005:
          return "-g2005";
        case Design::Language::SYSTEMVERILOG_2009:
          return "-g2009";
        case Design::Language::SYSTEMVERILOG_2012:
          return "-g2012";
        case Design::Language::SYSTEMVERILOG_2017:
          return "-g2012";
        case Design::Language::VERILOG_NETLIST:
          return "";
        case Design::Language::C:
          return "";
        case Design::Language::CPP:
          return "";
        default:
          m_compiler->SetError(StringUtils::format(
              "%: Invalid language for %", FOEDAG::ToString(lang),
              StringUtils::toUpper(ToString(type))));
          return "--invalid-lang-for-icarus";
      }
      break;
    case SimulatorType::GHDL:
      switch (lang) {
        case Design::Language::VERILOG_1995:
        case Design::Language::VERILOG_2001:
        case Design::Language::SYSTEMVERILOG_2005:
        case Design::Language::SYSTEMVERILOG_2009:
        case Design::Language::SYSTEMVERILOG_2012:
        case Design::Language::SYSTEMVERILOG_2017:
        case Design::Language::VERILOG_NETLIST:
        case Design::Language::C:
        case Design::Language::CPP:
          m_compiler->SetError(StringUtils::format(
              "%: Invalid language for %", FOEDAG::ToString(lang),
              StringUtils::toUpper(ToString(type))));
          return "--invalid-lang-for-ghdl";
        case Design::Language::VHDL_1987:
          if (m_simType == SimulationType::Gate ||
              m_simType == SimulationType::PNR) {
            return "--std=08";
          }
          return "--std=87";
        case Design::Language::VHDL_1993:
          if (m_simType == SimulationType::Gate ||
              m_simType == SimulationType::PNR) {
            return "--std=08";
          }
          return "--std=93";
        case Design::Language::VHDL_2000:
          if (m_simType == SimulationType::Gate ||
              m_simType == SimulationType::PNR) {
            return "--std=08";
          }
          return "--std=00";
        case Design::Language::VHDL_2008:
          return "--std=08";
        case Design::Language::VHDL_2019:
        default:
          m_compiler->SetError(StringUtils::format(
              "%: Invalid language for %", FOEDAG::ToString(lang),
              StringUtils::toUpper(ToString(type))));
          return "--invalid-lang-for-ghdl";
      }
      break;
    case SimulatorType::Questa:
      break;
    case SimulatorType::VCS:
      switch (lang) {
        case Design::Language::VERILOG_1995:
          return "";
        case Design::Language::VERILOG_2001:
          return "+v2k";
        case Design::Language::SYSTEMVERILOG_2005:
          return "-sverilog";
        case Design::Language::SYSTEMVERILOG_2009:
          return "-sverilog";
        case Design::Language::SYSTEMVERILOG_2012:
          return "-sverilog";
        case Design::Language::SYSTEMVERILOG_2017:
          return "-sverilog";
        case Design::Language::VERILOG_NETLIST:
          return "-sverilog";
        case Design::Language::C:
          return "";
        case Design::Language::CPP:
          return "";
        default:
          m_compiler->SetError(StringUtils::format(
              "%: Invalid language for %", FOEDAG::ToString(lang),
              StringUtils::toUpper(ToString(type))));
          return "--invalid-lang-for-vcs";
      }
      break;
    case SimulatorType::Xcelium:
      break;
    default:
      return "Invalid";
  }
  return "Invalid";
}

std::string Simulator::SimulatorRunCommand(SimulationType simulation,
                                           SimulatorType type) {
  std::string execPath =
      (SimulatorExecPath(type) / SimulatorName(type)).string();
  auto simulationTop{ProjManager()->SimulationTopModule()};
  switch (type) {
    case SimulatorType::Verilator: {
      std::string command = "obj_dir/V" + simulationTop;
      if (!GetSimulatorSimulationOption(simulation, type).empty())
        command += " " + GetSimulatorSimulationOption(simulation, type);
      if (!m_waveFile.empty()) command += " " + m_waveFile;
      return command;
    }
    case SimulatorType::Icarus: {
      std::string command =
          (SimulatorExecPath(type) / "vvp").string() + " ./a.out";
      if (m_waveType == WaveformType::FST) {
        command += " -fst";
      }
      if (!m_waveFile.empty()) command += " -dumpfile=" + m_waveFile;
      return command;
    }
    case SimulatorType::GHDL: {
      std::string command = execPath + " -r -fsynopsys -fexplicit";
      command += " --workdir=" +
                 m_compiler->FilePath(Compiler::Action::SimulateRTL).string();
      if (!GetSimulatorExtraOption(simulation, type).empty())
        command += " " + GetSimulatorExtraOption(simulation, type);
      if (!simulationTop.empty()) {
        command += TopModuleCmd(type) + simulationTop;
      }
      if (!m_waveFile.empty()) {
        command += " ";
        switch (m_waveType) {
          case WaveformType::VCD:
            command += "--vcd=";
            break;
          case WaveformType::FST:
            command += "--fst=";
            break;
          case WaveformType::GHW:
            command += "--wave=";
            break;
        };
        command += m_waveFile;
      }
      if (!GetSimulatorSimulationOption(simulation, type).empty())
        command += " " + GetSimulatorSimulationOption(simulation, type);
      return command;
    }
    case SimulatorType::Questa:
      return "Todo";
    case SimulatorType::VCS:
      return "simv";
    case SimulatorType::Xcelium:
      return "Todo";
  }
  return "Invalid";
}

std::string Simulator::SimulationFileList(SimulationType action,
                                          SimulatorType type,
                                          const std::string& designFiles) {
  std::string fileList;
  m_compiler->CustomSimulatorSetup(action);
  if (type != SimulatorType::GHDL) {
    auto simulationTop{ProjManager()->SimulationTopModule()};
    if (!simulationTop.empty()) {
      fileList += TopModuleCmd(type) + simulationTop + " ";
    }
  }

  // macroses
  for (auto& macro_value : ProjManager()->macroList()) {
    fileList += MacroDirective(type) + macro_value.first + "=" +
                macro_value.second + " ";
  }

  // includes
  for (const auto& path : ProjManager()->includePathList()) {
    fileList +=
        IncludeDirective(type) +
        FileUtils::AdjustPath(path, ProjManager()->projectPath()).string() +
        " ";
  }

  if (type != SimulatorType::GHDL) {
    // VHDL has no include files
    std::set<std::string> designFileDirs;
    for (const auto& lang_file : ProjManager()->DesignFiles()) {
      const std::string& fileNames = lang_file.second;
      std::vector<std::string> files;
      StringUtils::tokenize(fileNames, " ", files);
      for (const auto& file : files) {
        std::filesystem::path filePath = file;
        filePath = filePath.parent_path();
        const std::string& path = filePath.string();
        if (designFileDirs.find(path) == designFileDirs.end()) {
          fileList += IncludeDirective(type) +
                      FileUtils::AdjustPath(path, ProjManager()->projectPath())
                          .string() +
                      " ";
          designFileDirs.insert(path);
        }
      }
    }

    // Add simulation files directory as an include dir
    for (const auto& lang_file : ProjManager()->SimulationFiles()) {
      const std::string& fileNames = lang_file.second;
      std::vector<std::string> files;
      StringUtils::tokenize(fileNames, " ", files);
      for (const auto& file : files) {
        std::filesystem::path filePath = file;
        filePath = filePath.parent_path();
        const std::string& path = filePath.string();
        if (designFileDirs.find(path) == designFileDirs.end()) {
          fileList += IncludeDirective(type) +
                      FileUtils::AdjustPath(path, ProjManager()->projectPath())
                          .string() +
                      " ";
          designFileDirs.insert(path);
        }
      }
    }
  }

  // libraries
  for (const auto& path : ProjManager()->libraryPathList()) {
    fileList +=
        LibraryPathDirective(type) +
        FileUtils::AdjustPath(path, ProjManager()->projectPath()).string() +
        " ";
  }

  // extensions
  for (const auto& ext : ProjManager()->libraryExtensionList()) {
    fileList += LibraryExtDirective(type) + ext + " ";
  }

  bool langDirective = false;
  // design files
  if (!designFiles.empty()) {
    fileList += designFiles;
    if (type == SimulatorType::GHDL) {
      if (fileList.find("--std=") != std::string::npos) {
        langDirective = true;
      }
    }
  }

  // simulation files
  bool exeSpecified = false;
  for (const auto& lang_file : ProjManager()->SimulationFiles()) {
    if (langDirective == false) {
      Design::Language language = (Design::Language)lang_file.first.language;
      std::string directive = LanguageDirective(type, language);
      if (!directive.empty()) {
        langDirective = true;
        fileList += directive + " ";
      }
    }
    if (type == SimulatorType::Verilator) {
      if (lang_file.second.find(".c") != std::string::npos) {
        fileList += "--exe ";
        exeSpecified = true;
      }
    }
    fileList += lang_file.second + " ";
  }
  if (type == SimulatorType::Verilator) {
    if (!exeSpecified) {
      fileList += "--binary ";
    }
  }
  return fileList;
}

int Simulator::SimulationJob(SimulationType simulation, SimulatorType type,
                             const std::string& fileList) {
  /*  // This is depricated.
  if (type == SimulatorType::Verilator) {
    std::string verilator_home = SimulatorExecPath(type).parent_path().string();
    m_compiler->SetEnvironmentVariable("VERILATOR_ROOT", verilator_home);
  }
  */
  ProcessUtilization summaryUtils{};
  auto appendSumUtils = [&summaryUtils](const ProcessUtilization& utils) {
    summaryUtils.duration += utils.duration;
    summaryUtils.utilization =
        std::max(summaryUtils.utilization, utils.utilization);
  };

  std::string log{LogFile(simulation)};
  // Simulator Model compilation step
  std::string execPath =
      (SimulatorExecPath(type) / SimulatorName(type)).string();
  std::string command =
      execPath + " " + SimulatorCompilationOptions(simulation, type);
  if (!GetSimulatorCompileOption(simulation, type).empty())
    command += " " + GetSimulatorCompileOption(simulation, type);
  command += " " + fileList;
  std::string workingDir =
      m_compiler->FilePath(Compiler::ToCompilerAction(simulation)).string();
  FileUtils::WriteToFile(CommandLogFile("comp"), command);
  int status = m_compiler->ExecuteAndMonitorSystemCommand(command, log, false,
                                                          workingDir);
  appendSumUtils(m_compiler->m_utils);
  if (status) {
    ErrorMessage("Design " + ProjManager()->projectName() +
                 " simulation compilation failed!\n");
    return status;
  }

  // Extra Simulator Model compilation step (Elaboration or C++ compilation)
  auto simulationTop{ProjManager()->SimulationTopModule()};
  switch (type) {
    case SimulatorType::Verilator: {
      std::string command =
          "make -j -C obj_dir/ -f V" + simulationTop + ".mk V" + simulationTop;
      if (!GetSimulatorElaborationOption(simulation, type).empty())
        command += " " + GetSimulatorElaborationOption(simulation, type);
      FileUtils::WriteToFile(CommandLogFile("make"), command);
      status = m_compiler->ExecuteAndMonitorSystemCommand(command, log, true,
                                                          workingDir);
      appendSumUtils(m_compiler->m_utils);
      if (status) {
        ErrorMessage("Design " + ProjManager()->projectName() +
                     " simulation compilation failed!\n");
        return status;
      }
      break;
    }
    case SimulatorType::GHDL: {
      std::string command = execPath + " -e -fsynopsys -fexplicit";
      if (!GetSimulatorElaborationOption(simulation, type).empty())
        command += " " + GetSimulatorElaborationOption(simulation, type);
      command += " --workdir=" +
                 m_compiler->FilePath(Compiler::Action::SimulateRTL).string();
      if (!simulationTop.empty()) {
        command += TopModuleCmd(type) + simulationTop;
      }
      FileUtils::WriteToFile(CommandLogFile("make"), command);
      status = m_compiler->ExecuteAndMonitorSystemCommand(command, log, true,
                                                          workingDir);
      appendSumUtils(m_compiler->m_utils);
      if (status) {
        ErrorMessage("Design " + ProjManager()->projectName() +
                     " simulation compilation failed!\n");
        return status;
      }
      break;
    }
    default:
      break;
  }

  // Actual simulation
  command = SimulatorRunCommand(simulation, type);
  FileUtils::WriteToFile(CommandLogFile(std::string{}), command);
  status = m_compiler->ExecuteAndMonitorSystemCommand(command, log, true,
                                                      workingDir);
  appendSumUtils(m_compiler->m_utils);
  m_compiler->m_utils = summaryUtils;
  return status;
}

bool Simulator::SimulateRTL(SimulatorType type) {
  if (!m_compiler->HasTargetDevice()) return false;

  std::string fileList{};
  bool langDirective = false;
  for (const auto& lang_file : ProjManager()->DesignFiles()) {
    if (langDirective == false) {
      std::string directive =
          LanguageDirective(type, (Design::Language)(lang_file.first.language));
      if (!directive.empty()) {
        langDirective = true;
        fileList += directive + " ";
      }
    }
    fileList += lang_file.second + " ";
  }
  fileList = SimulationTypeMacro(SimulationType::RTL, type) + " " + fileList;
  fileList = SimulationFileList(SimulationType::RTL, type, fileList);
  fileList = StringUtils::rtrim(fileList);

  PERF_LOG("RTL Simulation has started");
  Message("##################################################");
  Message("RTL simulation for design: " + ProjManager()->projectName());
  Message("##################################################");

  bool status = SimulationJob(SimulationType::RTL, type, fileList);

  if (status) {
    ErrorMessage("Design " + ProjManager()->projectName() +
                 " simulation failed!\n");
    return false;
  }

  Message("RTL simulation for design: " + ProjManager()->projectName() +
          " had ended");
  return true;
}

bool Simulator::SimulateGate(SimulatorType type) {
  if (!m_compiler->HasTargetDevice()) return false;
  PERF_LOG("Gate Simulation has started");
  Message("##################################################");
  Message("Gate simulation for design: " + ProjManager()->projectName());
  Message("##################################################");

  std::string fileList = SimulationFileList(SimulationType::Gate, type);
  fileList = SimulationTypeMacro(SimulationType::Gate, type) + " " + fileList;

  std::string netlistFile;
  switch (m_compiler->GetNetlistType()) {
    case Compiler::NetlistType::Verilog:
      netlistFile = ProjManager()->projectName() + "_post_synth.v";
      break;
    case Compiler::NetlistType::VHDL:
      netlistFile = ProjManager()->projectName() + "_post_synth.vhd";
      break;
    case Compiler::NetlistType::Edif:
      netlistFile = ProjManager()->projectName() + "_post_synth.v";
      // Simulators do not support edif: netlistFile =
      // ProjManager()->projectName() + "_post_synth.edif";
      break;
    case Compiler::NetlistType::Blif:
      netlistFile = ProjManager()->projectName() + "_post_synth.v";
      // Simulators do not support blif: netlistFile =
      // ProjManager()->projectName() + "_post_synth.blif";
      break;
    case Compiler::NetlistType::EBlif:
      netlistFile = ProjManager()->projectName() + "_post_synth.v";
      // Simulators do not support eblif: netlistFile =
      // ProjManager()->projectName() + "_post_synth.eblif";
      break;
  }
  if (!netlistFile.empty())
    netlistFile =
        m_compiler->FilePath(Compiler::Action::Synthesis, netlistFile).string();

  for (const auto& lang_file : ProjManager()->DesignFiles()) {
    switch (lang_file.first.language) {
      case Design::Language::VERILOG_NETLIST:
      case Design::Language::BLIF:
      case Design::Language::EBLIF: {
        netlistFile = lang_file.second;
        std::filesystem::path the_path = netlistFile;
        if (!the_path.is_absolute()) {
          netlistFile =
              std::filesystem::path(std::filesystem::path("..") / netlistFile)
                  .string();
        }
        break;
      }
      default:
        break;
    }
  }

  fileList += netlistFile + " ";
  for (auto path : m_gateSimulationModels) {
    fileList += LibraryFileDirective(type) + path.string() + " ";
  }
  fileList = StringUtils::rtrim(fileList);

  bool status = SimulationJob(SimulationType::Gate, type, fileList);

  if (status) {
    ErrorMessage("Design " + ProjManager()->projectName() +
                 " simulation failed!\n");
    return false;
  }

  Message("Gate simulation for design: " + ProjManager()->projectName() +
          " had ended");

  return true;
}
bool Simulator::SimulatePNR(SimulatorType type) {
  if (!m_compiler->HasTargetDevice()) return false;
  PERF_LOG("Post-PnR Simulation has started");
  Message("##################################################");
  Message("Post-PnR simulation for design: " + ProjManager()->projectName());
  Message("##################################################");

  std::string fileList = SimulationFileList(SimulationType::PNR, type);

  std::string netlistFile =
      "fabric_" + m_compiler->DesignTopModule() + "_post_route.v";
  std::string sdfFile =
      "fabric_" + m_compiler->DesignTopModule() + "_post_route.sdf";

  std::string wrapperFile =
      m_compiler
          ->FilePath(
              Compiler::Action::Synthesis,
              std::string("post_pnr_wrapper_" + ProjManager()->projectName()) +
                  "_post_synth.v")
          .string();
  fileList += " " + wrapperFile;

  netlistFile =
      m_compiler->FilePath(Compiler::Action::Routing, netlistFile).string();
  sdfFile = m_compiler->FilePath(Compiler::Action::Routing, sdfFile).string();

  if (IsTimedSimulation()) {
    fileList = " -DTIMED_SIM=1 " + fileList;
    if (type == SimulatorType::Icarus) {
      fileList = " -gspecify " + fileList;
      std::filesystem::path sdfFilePath = sdfFile;
      std::string sdfBlastedFile =
          "fabric_" + m_compiler->DesignTopModule() + "_post_route_blasted.sdf";
      sdfBlastedFile =
          m_compiler->FilePath(Compiler::Action::Routing, sdfBlastedFile)
              .string();
      std::filesystem::path sdfBlastedFilePath = sdfBlastedFile;
      // Icarus ignores the timescale present in the SDF file, so we need
      // to scale the content from ps to ns to match the simulation timescale.
      if (!FileUtils::convertPstoNsInSDFFile(sdfFilePath, sdfBlastedFilePath)) {
        ErrorMessage("SDF Unit Conversion for Icarus failed!\n");
        return false;
      }
      // Icarus does not support vectors in SDF files, both the netlist and the
      // sdf have to get bit blasted
      const auto& path = std::filesystem::current_path();
      std::string workingDir =
          std::filesystem::path(path / ProjManager()->projectName()).string();
      std::filesystem::path bitblast_exe =
          m_compiler->GetBinPath() / "bitblast";
      std::filesystem::path datapath = m_compiler->GetDataPath().parent_path();
      std::filesystem::path tech_datapath = datapath / "raptor" / "sim_models" /
                                            "rapidsilicon" /
                                            m_compiler->YosysMapTechnology();
      std::filesystem::path primitives_file = tech_datapath / "primitives.v";
      std::filesystem::path library_path =
          tech_datapath / "FPGA_PRIMITIVES_MODELS" / "sim_models" / "verilog";
      std::filesystem::path dsp_map =
          tech_datapath / "FPGA_PRIMITIVES_MODELS" / "sim_models" /
          "primitives_mapping" / "DSP" / "rs_dsp_multxxx_post_pnr_mapping.v";
      std::filesystem::path ram_map = tech_datapath / "FPGA_PRIMITIVES_MODELS" /
                                      "sim_models" / "primitives_mapping" /
                                      "BRAM" / "rs_tdp36k_post_pnr_mapping.v";
      std::string netlistBlastedFile =
          "fabric_" + m_compiler->DesignTopModule() + "_post_route_blasted.v";
      netlistBlastedFile =
          m_compiler->FilePath(Compiler::Action::Routing, netlistBlastedFile)
              .string();
      std::string command =
          std::string(bitblast_exe.string()) +
          " -nostdout -DSYNTHESIS=1 -top fabric_" +
          m_compiler->DesignTopModule() + " " + netlistFile + " -v " +
          primitives_file.string() + " " + dsp_map.string() + " " +
          ram_map.string() + " -y " + library_path.string() +
          " -bitblast -sdf_in " + sdfBlastedFilePath.string() + " -sdf_out " +
          sdfBlastedFilePath.string() + " -write " + netlistBlastedFile;
      int status = m_compiler->ExecuteAndMonitorSystemCommand(
          command, "bitblast.log", false, workingDir);
      if (status) {
        ErrorMessage("Design " + ProjManager()->projectName() +
                     " Post-PnR simulation failed!\n");
        return false;
      }
      netlistFile = netlistBlastedFile;
    }
  }
  fileList += " " + netlistFile + " ";

  for (auto path : m_gateSimulationModels) {
    fileList += LibraryFileDirective(type) + path.string() + " ";
  }

  fileList = SimulationTypeMacro(SimulationType::PNR, type) + " " + fileList;

  fileList = StringUtils::rtrim(fileList);

  bool status = SimulationJob(SimulationType::PNR, type, fileList);

  if (status) {
    ErrorMessage("Design " + ProjManager()->projectName() +
                 " Post-PnR simulation failed!\n");
    return false;
  }

  Message("Post-PnR simulation for design: " + ProjManager()->projectName() +
          " had ended");

  return true;
}

bool Simulator::SimulateBitstream(SimulationType sim_type, SimulatorType type) {
  if (!m_compiler->HasTargetDevice()) return false;
  PERF_LOG("Bitstream Simulation has started");
  Message("##################################################");
  Message("Bitstream simulation for design: " + ProjManager()->projectName());
  Message("##################################################");
#ifdef PRODUCTION_BUILD
  ErrorMessage("Bitstream simulation is not available in production build");
  return false;
#endif
  std::string fileList =
      LanguageDirective(type, Design::Language::SYSTEMVERILOG_2012);
  auto designTopModule = m_compiler->DesignTopModule();

  fileList = SimulationTypeMacro(sim_type, type) + " " + fileList;

  if (sim_type == SimulationType::BitstreamBackDoor) {
    if (!ProjManager()->SimulationFiles().empty() &&
        type == SimulatorType::Icarus) {
      fileList += " ";
      for (const auto& lang_file : ProjManager()->SimulationFiles()) {
        fileList += lang_file.second + " ";
      }
    } else {
      fileList += std::string(" ") +
                  std::filesystem::path(std::filesystem::path("..") /
                                        "bitstream" / "BIT_SIM" / +"")
                      .string() +
                  "fabric_" + designTopModule + "_formal_random_top_tb.v";
    }
    fileList += std::string(" ") +
                std::filesystem::path(std::filesystem::path("..") /
                                      "bitstream" / "BIT_SIM" / +"")
                    .string() +
                "fabric_" + designTopModule + "_top_formal_verification.v";
  } else {
    fileList += std::string(" ") +
                std::filesystem::path(std::filesystem::path("..") /
                                      "bitstream" / "BIT_SIM" / +"")
                    .string() +
                "fabric_" + designTopModule + "_autocheck_top_tb.v";
  }

  fileList += std::string(" ") +
              std::filesystem::path(std::filesystem::path("..") / "bitstream" /
                                    "BIT_SIM" / "fabric_netlists.v")
                  .string();

  for (auto path : ProjManager()->includePathList()) {
    fileList +=
        std::string(" ") + IncludeDirective(type) +
        FileUtils::AdjustPath(path, ProjManager()->projectPath()).string() +
        " ";
  }

  for (auto path : ProjManager()->libraryPathList()) {
    fileList +=
        std::string(" ") + LibraryPathDirective(type) +
        FileUtils::AdjustPath(path, ProjManager()->projectPath()).string() +
        " ";
  }

  fileList += std::string(" ") + LibraryPathDirective(type) +
              std::filesystem::path(std::filesystem::path("..") / "bitstream" /
                                    "BIT_SIM")
                  .string() +
              " ";

  fileList += std::string(" ") + LibraryPathDirective(type) +
              std::filesystem::path(std::filesystem::path("..") / "bitstream" /
                                    "BIT_SIM" / "lb")
                  .string() +
              " ";

  fileList += std::string(" ") + LibraryPathDirective(type) +
              std::filesystem::path(std::filesystem::path("..") / "bitstream" /
                                    "BIT_SIM" / "routing")
                  .string() +
              " ";

  for (auto ext : ProjManager()->libraryExtensionList()) {
    fileList += std::string(" ") + LibraryExtDirective(type) + ext + " ";
  }

  fileList += std::string(" ") + IncludeDirective(type) +
              std::filesystem::path(std::filesystem::path("..") / "bitstream")
                  .string() +
              " ";

  if (type == SimulatorType::Icarus) {
    if (!ProjManager()->SimulationTopModule().empty())
      fileList += TopModuleCmd(type) + ProjManager()->SimulationTopModule();
  } else {
    fileList += TopModuleCmd(type) + "fabric_" + designTopModule +
                "_top_formal_verification_random_tb";
  }

  for (auto path : ProjManager()->libraryPathList()) {
    std::filesystem::path full_path =
        FileUtils::AdjustPath(path, ProjManager()->projectPath());
    std::filesystem::path user_cells = full_path / "user_cells.v";
    if (FileUtils::FileExists(user_cells))
      fileList += std::string(" ") + user_cells.string() + " ";
  }

  bool status = SimulationJob(sim_type, type, fileList);

  if (status) {
    ErrorMessage("Design " + ProjManager()->projectName() +
                 " Bitstream simulation failed!\n");
    return false;
  }

  Message("Bitstream simulation for design: " + ProjManager()->projectName() +
          " had ended");
  return true;
}
