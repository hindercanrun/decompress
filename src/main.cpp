#include "dependencies/zlib/zlib.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdint>
#include <array>
#include <string>
#include <format>
#include <vector>

// console colour codes
constexpr auto red = "\033[31m";
constexpr auto yellow = "\033[33m";
constexpr auto clear = "\033[0m";

static
int decompress_ff(const std::string& file_name) {
  std::cout << std::format("decompressing: {}...\n", file_name);
  std::ifstream file(file_name, std::ios::binary);
  if (!file) {
    std::cerr << yellow << "failed to open file." << clear;
    return 1;
  }
  // skip the header
  file.seekg(12, std::ios::beg); // adjust to your needs

  std::vector<uint8_t> compressed(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));
  std::vector<uint8_t> decompressed(85 * 1024 * 1024); // 85 MB buffer, adjust to your needs

  z_stream strm{};
  strm.next_in = compressed.data();
  strm.avail_in = static_cast<uInt>(compressed.size());
  strm.next_out = decompressed.data();
  strm.avail_out = static_cast<uInt>(decompressed.size());
  if (inflateInit(&strm) != Z_OK) {
    throw std::runtime_error("inflateInit failed");
    return 1;
  }
  int ret = inflate(&strm, Z_FINISH);
  if (ret != Z_OK && ret != Z_STREAM_END) {
    inflateEnd(&strm);
    throw std::runtime_error("inflate failed");
    return 1;
  }
  inflateEnd(&strm);

  std::filesystem::path p = file_name;
  // remove '.ff' if it exists
  if (p.extension() == ".ff") {
    p = p.stem();
  }
  std::ofstream out(std::format("{}_decompressed.ff", p.string()), std::ios::binary);
  out.write(reinterpret_cast<const char*>(decompressed.data()), strm.total_out);

  // tell the user what we wrote
  std::cout << std::format("writing: {}_decompressed.ff", p.string());
  std::cout << "\ndone";
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << yellow << "usage: decompress.exe <input.ff>\n" << clear;
    return 1;
  }
  decompress_ff(argv[1]);
  return 0;
}