/*
Copyright 2021 The Foedag team

GPL License

Copyright (c) 2021 The Open-Source FPGA Foundation

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
#ifndef COMPILER_H
#define COMPILER_H

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || \
    defined(_MSC_VER) || defined(__CYGWIN__)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#ifndef __GNUC__
#include <direct.h>
#endif
#include <process.h>
#ifndef __SIZEOF_INT__
#define __SIZEOF_INT__ sizeof(int)
#endif
#else
#include <stdlib.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Command/Command.h"
#include "Command/CommandStack.h"
#include "IPGenerate/IPGenerator.h"
#include "Main/CommandLine.h"
#include "NetlistEditData.h"
#include "Simulation/Simulator.h"
#include "Task.h"
#include "Tcl/TclInterpreter.h"

class QProcess;
namespace fs = std::filesystem;

namespace FOEDAG {

class TaskManager;
class TclInterpreterHandler;
class Session;
class DesignManager;
class TclCommandIntegration;
class Constraints;
class CFGCompiler;
class ToolContext;
class DeviceModeling;

struct DeviceData {
  std::string family;
  std::string series;
  std::string package;
};

enum class ClbPacking { Auto, Dense, Timing_driven };
enum class SynthesisOptimization { Area, Delay, Mixed };
enum class BitstreamFlags { DefaultBitsOpt, Force, EnableSimulation };

class Compiler {
  friend Simulator;

 public:
  enum class NetlistType { Blif, EBlif, Edif, Verilog, VHDL };
  enum class ParserType { Default, Verific, Surelog, GHDL };
  enum class Action {
    NoAction,
    IPGen,
    Analyze,
    Synthesis,
    Pack,
    Global,
    Placement,
    Routing,
    STA,
    Power,
    Bitstream,
    Batch,
    SimulateRTL,
    SimulateGate,
    SimulatePNR,
    SimulateBitstream,
    Configuration
  };
  enum class State {
    None,
    IPGenerated,
    Analyzed,
    Synthesized,
    Packed,
    GloballyPlaced,
    Placed,
    Routed,
    TimingAnalyzed,
    PowerAnalyzed,
    BistreamGenerated
  };
  enum MsgSeverity { Ignore, Info, Warning, Error };
  enum class IPGenerateOpt { None, Clean, List };
  enum class DesignAnalysisOpt { None, Clean };
  enum class SynthesisOpt { None, Clean };
  enum class PackingOpt { None, Clean, Debug };
  enum class GlobalPlacementOpt { None, Clean };
  enum class PlacementOpt { None, Clean };
  enum class PinAssignOpt { Random, In_Define_Order, Pin_constraint_disabled };
  enum class RoutingOpt { None, Clean };
  enum class PowerOpt { None, Clean };
  enum class STAOpt { None, Clean, View };
  enum class BitstreamOpt { None, Clean };
  enum class STAEngineOpt { Tatum, Opensta };
  static Action ToCompilerAction(Simulator::SimulationType type);

  // Most common use case, create the compiler in your main
  Compiler() {
    m_name = "dummy";
    m_netlistEditData = new NetlistEditData();
  };
  Compiler(TclInterpreter* interp, std::ostream* out,
           TclInterpreterHandler* tclInterpreterHandler = nullptr);
  void SetInterpreter(TclInterpreter* interp) { m_interp = interp; }
  void SetOutStream(std::ostream* out) { m_out = out; };
  void SetErrStream(std::ostream* err) { m_err = err; };
  std::ostream* GetOutStream() { return m_out; }
  std::ostream* GetErrStream() { return m_err; }
  void SetTclInterpreterHandler(TclInterpreterHandler* tclInterpreterHandler);
  void SetSession(Session* session) { m_session = session; }
  Session* GetSession() const { return m_session; }
  virtual ~Compiler();

  void BatchScript(const std::string& script) { m_batchScript = script; }
  State CompilerState() const { return m_state; }
  void CompilerState(State st) { m_state = st; }
  bool Compile(Action action);
  void GenerateReport(int action);
  void Stop();
  void ResetStopFlag();
  TclInterpreter* TclInterp() { return m_interp; }
  virtual bool RegisterCommands(TclInterpreter* interp, bool batchMode);
  void start();
  void finish();
  class ProjectManager* ProjManager() const {
    return m_projManager;
  }
  std::string& getResult() { return m_result; }

  void setTaskManager(TaskManager* newTaskManager);
  TaskManager* GetTaskManager() const;
  Constraints* getConstraints() { return m_constraints; }
  NetlistEditData* getNetlistEditData() { return m_netlistEditData; }
  void setGuiTclSync(TclCommandIntegration* tclCommands);
  virtual std::vector<std::string> helpTags() const;
  virtual void Help(ToolContext* context, std::ostream* out);
  TclCommandIntegration* GuiTclSync() const;
  virtual void Version(std::ostream* out);
  virtual void Message(const std::string& message,
                       const std::string& messagePrefix = "",
                       bool raw = false) const;
  virtual void ErrorMessage(const std::string& message, bool append = true,
                            const std::string& messagePrefix = "",
                            bool raw = false) const;
  virtual void reloadSettings() {}
  void CleanFiles(Action action);
  std::string GetMessagePrefix() const;
  void SetParserType(ParserType type) { m_parserType = type; }
  ParserType GetParserType() { return m_parserType; }
  void SetIPGenerator(IPGenerator* generator);
  IPGenerator* GetIPGenerator() { return m_IPGenerator; }
  void SetSimulator(Simulator* simulator) { m_simulator = simulator; }
  Simulator* GetSimulator();

  bool BuildLiteXIPCatalog(std::filesystem::path litexPath,
                           bool namesOnly = false);
  bool HasIPInstances();
  bool HasIPDefinitions();

  // VPR, Yosys generic opt
  void ChannelWidth(uint32_t width) { m_channel_width = width; }
  void LutSize(uint32_t size) { m_lut_size = size; }
  const std::string& YosysPluginLibName() { return m_yosysPluginLib; }
  const std::string& YosysPluginName() { return m_yosysPlugin; }
  const std::string& YosysMapTechnology() { return m_mapToTechnology; }
  void YosysPluginLibName(const std::string& libname) {
    m_yosysPluginLib = libname;
  }
  void YosysPluginName(const std::string& name) { m_yosysPlugin = name; }
  void YosysMapTechnology(const std::string& tech) { m_mapToTechnology = tech; }

  IPGenerateOpt IPGenOpt() const { return m_ipGenerateOpt; }
  void IPGenOpt(IPGenerateOpt opt) { m_ipGenerateOpt = opt; }
  DesignAnalysisOpt AnalyzeOpt() const { return m_analysisOpt; }
  void AnalyzeOpt(DesignAnalysisOpt opt) { m_analysisOpt = opt; }
  PackingOpt PackOpt() const { return m_packingOpt; }
  void PackOpt(PackingOpt opt) { m_packingOpt = opt; }
  SynthesisOpt SynthOpt() const { return m_synthOpt; }
  void SynthOpt(SynthesisOpt opt) { m_synthOpt = opt; }
  SynthesisOptimization SynthOptimization() const {
    return m_synthOptimization;
  }
  void SynthOptimization(SynthesisOptimization opt) {
    m_synthOptimization = opt;
  }
  GlobalPlacementOpt GlobPlacementOpt() const { return m_globalPlacementOpt; }
  void GlobPlacementOpt(GlobalPlacementOpt opt) { m_globalPlacementOpt = opt; }
  PlacementOpt PlaceOpt() const { return m_placementOpt; }
  void PlaceOpt(PlacementOpt opt) { m_placementOpt = opt; }
  PinAssignOpt PinAssignOpts() { return m_pinAssignOpt; }
  void PinAssignOpts(PinAssignOpt opt) { m_pinAssignOpt = opt; }
  RoutingOpt RouteOpt() const { return m_routingOpt; }
  void RouteOpt(RoutingOpt opt) { m_routingOpt = opt; }
  STAOpt TimingAnalysisOpt() const { return m_staOpt; }
  void TimingAnalysisOpt(STAOpt opt) { m_staOpt = opt; }
  PowerOpt PowerAnalysisOpt() const { return m_powerOpt; }
  void PowerAnalysisOpt(PowerOpt opt) { m_powerOpt = opt; }
  STAEngineOpt TimingAnalysisEngineOpt() const { return m_staEngineOpt; }
  void TimingAnalysisEngineOpt(STAEngineOpt opt);
  BitstreamOpt BitsOpt() const { return m_bitstreamOpt; }
  void BitsOpt(BitstreamOpt opt) { m_bitstreamOpt = opt; }

  BitstreamFlags BitsFlags() const { return m_bitstreamFlags; }
  void BitsFlags(BitstreamFlags flags) { m_bitstreamFlags = flags; }

  // Compiler specific opt
  const std::string& BistreamMoreOpt() { return m_bitstreamMoreOpt; }
  void BitstreamMoreOpt(const std::string& opt) {
    m_bitstreamMoreOpt += opt + " ";
  }

  const std::string& SynthMoreOpt() { return m_synthMoreOpt; }
  void SynthMoreOpt(const std::string& opt) { m_synthMoreOpt = opt; }

  const std::string& PlaceMoreOpt() { return m_placeMoreOpt; }
  void PlaceMoreOpt(const std::string& opt) { m_placeMoreOpt = opt; }

  const std::string& IPGenMoreOpt() { return m_ipGenMoreOpt; }
  void IPGenMoreOpt(const std::string& opt) { m_ipGenMoreOpt = opt; }

  void PnROpt(const std::string& opt) { m_pnrOpt = opt; }
  const std::string& PnROpt() { return m_pnrOpt; }

  bool BitstreamEnabled() { return m_bitstreamEnabled; }
  void BitstreamEnabled(bool enabled) { m_bitstreamEnabled = enabled; }

  bool PinConstraintEnabled() { return m_pin_constraintEnabled; }
  void PinConstraintEnabled(bool enabled) { m_pin_constraintEnabled = enabled; }

  virtual std::string GetNetlistPath() const;

  void AddMsgSeverity(std::string msg, MsgSeverity severity) {
    m_severityMap.insert(std::make_pair(msg, severity));
  }
  const std::map<std::string, MsgSeverity>& MsgSeverityMap() {
    return m_severityMap;
  }

  void SetConstraints(Constraints* c);

  void SetNetlistType(NetlistType type) { m_netlistType = type; }
  NetlistType GetNetlistType() const { return m_netlistType; }

  void SetConfiguration(CFGCompiler* c) { m_configuration = c; }
  CFGCompiler* GetConfiguration() { return m_configuration; }

  void virtual CustomSimulatorSetup(Simulator::SimulationType action);
  void SetWaveformFile(const std::string& wave) { m_waveformFile = wave; }
  const std::string& GetWavefromFile() { return m_waveformFile; }

  QProcess* GetGTKWaveProcess();
  void GTKWaveSendCmd(const std::string& gtkWaveCmd,
                      bool raiseGtkWindow = true);

  void PinmapCSVFile(const std::filesystem::path& path);
  const std::filesystem::path& PinmapCSVFile() const;

  DeviceData deviceData() const;
  void setDeviceData(const DeviceData& newDeviceData);

  void ClbPackingOption(ClbPacking clbPacking);
  ClbPacking ClbPackingOption() const;

  virtual int ExecuteAndMonitorSystemCommand(
      const std::string& command, const std::string logFile = std::string{},
      bool appendLog = false, const fs::path& workingDir = {});

  void ProgrammerToolExecPath(const std::filesystem::path& path) {
    m_programmerToolExecutablePath = path;
  }

  std::filesystem::path GetProgrammerToolExecPath() const {
    return m_programmerToolExecutablePath;
  }

  void SetConfigFileSearchDirectory(const std::filesystem::path& path) {
    m_configFileSearchDir = path;
  }

  std::filesystem::path GetConfigFileSearchDirectory() const {
    return m_configFileSearchDir;
  }

  std::filesystem::path GetBinPath() const;

  std::filesystem::path GetDataPath() const;

  std::string Name() const { return m_name; }

  static constexpr SynthesisOptimization SYNTH_OPT_DEFAULT{
      SynthesisOptimization::Mixed};
  std::filesystem::path FilePath(Action action) const;
  std::filesystem::path FilePath(Action action, const std::string& file) const;
  virtual std::pair<bool, std::string> isRtlClock(const std::string& str,
                                                  bool regex, bool input_only) {
    return std::make_pair(false, std::string{});
  }
  std::vector<std::string> TopModules(
      const std::filesystem::path& ports_info) const;

  class DesignQuery* GetDesignQuery();
  void Compile2bits(bool compile2bits);

  std::filesystem::path DeviceFile() const;
  void DeviceFile(const std::filesystem::path& file);

  virtual void adjustTargetDeviceDefaults() {}
  std::string DesignTopModule() const;

  bool DeviceFileLocal() const;

 protected:
  /* Methods that can be customized for each new compiler flow */
  virtual bool IPGenerate();
  virtual bool Analyze();
  virtual bool Synthesize();
  virtual bool Packing();
  virtual bool GlobalPlacement();
  virtual bool Placement();
  virtual bool Route();
  virtual bool TimingAnalysis();
  virtual bool PowerAnalysis();
  virtual bool GenerateBitstream();

  /*!
   * \brief CheckTargetDevice
   * \return true if target device is set otherwise return false
   */
  virtual bool VerifyTargetDevice() const;
  bool HasTargetDevice();

  std::pair<bool, std::string> CreateDesign(
      const std::string& name, const std::string& type = std::string{},
      bool cleanup = false);

  /* Compiler class utilities */
  bool RunBatch();
  bool RunCompileTask(Action action);
  bool SwitchCompileContext(Action action, const std::function<bool(void)>& fn);

  void SetEnvironmentVariable(const std::string variable,
                              const std::string value);
  std::string ReplaceAll(std::string_view str, std::string_view from,
                         std::string_view to);
  virtual std::pair<bool, std::string> IsDeviceSizeCorrect(
      const std::string& size) const;
  virtual void SetDeviceResources() {}

  enum AddFilesType { Design, Simulation };
  int add_files(Compiler* compiler, Tcl_Interp* interp, int argc,
                const char* argv[], AddFilesType filesType);
  int verifySynthPorts(Compiler* compiler, Tcl_Interp* interp, int argc,
                       const char* argv[]);

  void installGTKWaveHelpers();
  void writeHelp(
      std::ostream* out,
      const std::vector<std::pair<std::string, std::string>>& cmdDescPairs,
      int frontSpacePadCount, int descColumn);
  void writeWaveHelp(std::ostream* out, int frontSpacePadCount, int descColumn);
  void AddHeadersToLogs(Action action);
  void AddErrorLink(const class Task* const current);
  bool HasInternalError() const;
  void SetError(const std::string& message);
  void ResetError();

  /* Propected members */
  TclInterpreter* m_interp = nullptr;
  Session* m_session = nullptr;
  class ProjectManager* m_projManager = nullptr;
  bool m_stop = false;
  State m_state = State::None;
  std::ostream* m_out = &std::cout;
  std::ostream* m_err = &std::cerr;
  std::string m_batchScript;
  std::string m_result;
  TclInterpreterHandler* m_tclInterpreterHandler{nullptr};
  TaskManager* m_taskManager{nullptr};
  TclCommandIntegration* m_tclCmdIntegration{nullptr};
  Constraints* m_constraints = nullptr;
  NetlistEditData* m_netlistEditData = nullptr;
  ParserType m_parserType{ParserType::Default};

  // Tasks generic options
  IPGenerateOpt m_ipGenerateOpt = IPGenerateOpt::None;
  DesignAnalysisOpt m_analysisOpt = DesignAnalysisOpt::None;
  SynthesisOpt m_synthOpt = SynthesisOpt::None;
  SynthesisOptimization m_synthOptimization{SYNTH_OPT_DEFAULT};
  PackingOpt m_packingOpt = PackingOpt::None;
  GlobalPlacementOpt m_globalPlacementOpt = GlobalPlacementOpt::None;
  PlacementOpt m_placementOpt = PlacementOpt::None;
  PinAssignOpt m_pinAssignOpt = PinAssignOpt::In_Define_Order;
  RoutingOpt m_routingOpt = RoutingOpt::None;
  PowerOpt m_powerOpt = PowerOpt::None;
  STAOpt m_staOpt = STAOpt::None;
  STAEngineOpt m_staEngineOpt = STAEngineOpt::Tatum;
  BitstreamOpt m_bitstreamOpt = BitstreamOpt::None;
  BitstreamFlags m_bitstreamFlags = BitstreamFlags::DefaultBitsOpt;
  std::filesystem::path m_PinMapCSV{};
  DeviceData m_deviceData;
  ClbPacking m_clbPacking{ClbPacking::Auto};

  // Compiler specific options
  std::string m_pnrOpt;
  std::string m_bitstreamMoreOpt;
  std::string m_synthMoreOpt;
  std::string m_placeMoreOpt;
  std::string m_ipGenMoreOpt;

  // VPR, Yosys options
  uint32_t m_channel_width = 100;
  uint32_t m_lut_size = 6;
  std::string m_yosysPluginLib;
  std::string m_yosysPlugin;
  std::string m_mapToTechnology;
  bool m_bitstreamEnabled = true;
  bool m_pin_constraintEnabled = true;
  class QProcess* m_process = nullptr;
  class DeviceModeling* m_DeviceModeling = nullptr;
  // Sub engines
  IPGenerator* m_IPGenerator = nullptr;
  Simulator* m_simulator = nullptr;
  class DesignQuery* m_DesignQuery = nullptr;
  CFGCompiler* m_configuration = nullptr;
  // Error message severity
  std::map<std::string, MsgSeverity> m_severityMap;

  std::map<std::string, std::string> m_environmentVariableMap;

  NetlistType m_netlistType = NetlistType::Verilog;

  std::string m_waveformFile;

  // GTKWave
  QProcess* m_gtkwave_process = nullptr;

  std::filesystem::path m_programmerToolExecutablePath{};
  std::filesystem::path m_configFileSearchDir{};
  std::string m_name;
  ProcessUtilization m_utils;
  struct ErrorState m_errorState;
  bool m_compile2bits{false};
  std::filesystem::path m_deviceFile{};
  bool m_deviceFileLocal{false};
};

}  // namespace FOEDAG

#endif
