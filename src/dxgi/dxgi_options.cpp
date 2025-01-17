#include "dxgi_options.h"

#include <unordered_map>

namespace dxvk {

  static int32_t parsePciId(const std::string& str) {
    if (str.size() != 4)
      return -1;
    
    int32_t id = 0;

    for (size_t i = 0; i < str.size(); i++) {
      id *= 16;

      if (str[i] >= '0' && str[i] <= '9')
        id += str[i] - '0';
      else if (str[i] >= 'A' && str[i] <= 'F')
        id += str[i] - 'A' + 10;
      else if (str[i] >= 'a' && str[i] <= 'f')
        id += str[i] - 'a' + 10;
      else
        return -1;
    }

    return id;
  }

  
  DxgiOptions::DxgiOptions(const Config& config) {
    // Fetch these as a string representing a hexadecimal number and parse it.
    this->customVendorId = parsePciId(config.getOption<std::string>("dxgi.customVendorId"));
    this->customDeviceId = parsePciId(config.getOption<std::string>("dxgi.customDeviceId"));
    this->customDeviceDesc = config.getOption<std::string>("dxgi.customDeviceDesc", "");

    // Emulate a UMA device
    this->emulateUMA = config.getOption<bool>("dxgi.emulateUMA", false);
    
    // Interpret the memory limits as Megabytes
    this->maxDeviceMemory = VkDeviceSize(config.getOption<int32_t>("dxgi.maxDeviceMemory", 0)) << 20;
    this->maxSharedMemory = VkDeviceSize(config.getOption<int32_t>("dxgi.maxSharedMemory", 0)) << 20;

    // Expose Nvidia GPUs properly if NvAPI is enabled in environment
    this->hideNvidiaGpu = env::getEnvVar("DXVK_ENABLE_NVAPI") != "1";
    Tristate hideNvidiaGpuOption = config.getOption<Tristate>("dxgi.hideNvidiaGpu", Tristate::Auto);
    if (hideNvidiaGpuOption == Tristate::Auto && !config.getOption<bool>("dxgi.nvapiHack", true)) {
      Logger::warn("dxgi.nvapiHack is deprecated, please set dxgi.hideNvidiaGpu instead.");
      hideNvidiaGpuOption = Tristate::False;
    }
    applyTristate(this->hideNvidiaGpu, hideNvidiaGpuOption);

    // Expose AMD and Intel GPU by default, unless a config override is active.
    // Implement as a tristate so that we have the option to introduce similar
    // logic to Nvidia later, if necessary.
    this->hideAmdGpu = config.getOption<Tristate>("dxgi.hideAmdGpu", Tristate::Auto) == Tristate::True;
    this->hideIntelGpu = config.getOption<Tristate>("dxgi.hideIntelGpu", Tristate::Auto) == Tristate::True;    
  }
  
}
