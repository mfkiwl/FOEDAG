#ifndef __PROGRAMMINGADAPTER_H__
#define __PROGRAMMINGADAPTER_H__

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

#include "Device.h"
using progress_func_type = std::function<void(double)>;

namespace FOEDAG {
class ProgramingAdapter {
 public:
  virtual ~ProgramingAdapter() = default;
  virtual int pld(const Device& device, std::string bitfiles,
                  std::atomic<bool>& stop,
                  progress_func_type progress_callback) = 0;
  virtual int otp(const Device& device, std::string bitfile,
                  std::atomic<bool>& stop,
                  progress_func_type progress_callback) = 0;
  virtual int flash(const Device& device, std::string bitfile,
                    std::atomic<bool>& stop,
                    progress_func_type progress_callback) = 0;
};

}  // namespace FOEDAG
#endif  //__PROGRAMMINGADAPTER_H__