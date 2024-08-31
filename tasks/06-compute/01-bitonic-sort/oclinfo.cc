/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "opencl_include.hpp"
#include "selector.hpp"

#include <exception>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>

static const auto print_delim = [](auto &os) { os << " -------- \n"; };

std::string get_device_type_string(int type) {
  static const std::unordered_map<int, std::string> type_map = {
      {CL_DEVICE_TYPE_GPU, "CL_DEVICE_TYPE_GPU"},
      {CL_DEVICE_TYPE_CPU, "CL_DEVICE_TYPE_CPU"},
      {CL_DEVICE_TYPE_ACCELERATOR, "CL_DEVICE_TYPE_ACCELERATOR"}};

  auto found = type_map.find(type);
  if (found == type_map.end())
    return "UNKNOWN";
  return found->second;
}

void display_device_info(cl::Device &dev, std::ostream &os) {
  os << "  Name: " << dev.getInfo<CL_DEVICE_NAME>() << "\n";
  os << "  Type: " << get_device_type_string(dev.getInfo<CL_DEVICE_TYPE>())
     << "\n";
  os << "  Extensions: " << dev.getInfo<CL_DEVICE_EXTENSIONS>() << "\n";
  os << "  Compute units: " << dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()
     << "\n";

  os << "  Global memory size: "
     << dev.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() /
            static_cast<float>(1073741824)
     << " GiB\n";
  os << "  Global memory cacheline size: "
     << dev.getInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>() << " Bytes\n";
  os << "  Global memory cache size: "
     << dev.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>() /
            static_cast<float>(1048576)
     << "MiB\n";

  os << "  Local memory size: "
     << dev.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() / static_cast<float>(1024)
     << " KiB\n";

  const auto wi_sizes = dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
  os << "  Max work item sizes: {" << wi_sizes[0] << ", " << wi_sizes[1] << ", "
     << wi_sizes[2] << "}\n";
  os << "  Max work group size: "
     << dev.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << "\n";
  os << "  Images supported: " << std::boolalpha
     << static_cast<bool>(dev.getInfo<CL_DEVICE_IMAGE_SUPPORT>()) << "\n";

  os << "  Native vector width of char: "
     << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR>() << "\n";
  os << "  Native vector width of short: "
     << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT>() << "\n";
  os << "  Native vector width of int: "
     << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_INT>() << "\n";
  os << "  Native vector width of long: "
     << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG>() << "\n";
  os << "  Native vector width of float: "
     << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT>() << "\n";
  os << "  Native vector width of half: "
     << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF>() << "\n";
  os << "  Native vector width of double: "
     << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE>() << "\n";
}

void display_platform_info(cl::Platform &plat, std::ostream &os) {
  os << "Name: " << plat.getInfo<CL_PLATFORM_NAME>() << "\n";
  os << "Vendor: " << plat.getInfo<CL_PLATFORM_VENDOR>() << "\n";

  auto raw_version = plat.getInfo<CL_PLATFORM_VERSION>();
  os << "Raw Version: " << raw_version << "\n";
  auto decoded_version = clutils::decode_platform_version(raw_version);
  os << "Decoded Version: " << decoded_version.ver.major << "."
     << decoded_version.ver.minor << " + " << decoded_version.platform_specific
     << "\n";

  os << "Extensions: " << plat.getInfo<CL_PLATFORM_EXTENSIONS>() << "\n";

  std::vector<cl::Device> devices;
  plat.getDevices(CL_DEVICE_TYPE_ALL, &devices);

  os << "\nNumber of devices = " << devices.size() << "\n";

  for (unsigned i = 0; auto &dev : devices) {
    os << "Device [" << i++ << "] properties:\n";
    display_device_info(dev, os);
  }
}

void display_info(std::ostream &os) {
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);

  print_delim(os);

  for (auto &p : platforms) {
    display_platform_info(p, os);
    print_delim(os);
  }
}

int main() try { display_info(std::cout); } catch (cl::Error &e) {
  std::cerr << "OpenCL error: " << e.what() << "\n";
} catch (std::exception &e) {
  std::cerr << "Encountered error: " << e.what() << "\n";
} catch (...) {
  std::cerr << "Unknown error\n";
}
