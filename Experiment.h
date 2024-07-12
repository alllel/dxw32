//
// Created by sasha on 06.07.24.
//

#ifndef DXW32_EXPERIMENT_H
#define DXW32_EXPERIMENT_H
#include <memory>
#include <string>
#include <filesystem>

struct Experiment : std::enable_shared_from_this<Experiment> {
  static int nExp;
  std::filesystem::path path;
  explicit Experiment(std::filesystem::path p) noexcept : path(std::move(p)) { ++nExp; };
  ~Experiment() noexcept { --nExp; };

  std::filesystem::path const& IXC() const { return path; }
  std::filesystem::path File(std::string_view ext) const {
    auto tmp = path;
    return tmp.replace_extension(ext);
  }
  std::filesystem::path DAT() const { return File(".dat"); }
  bool operator==(Experiment const& rhs) const { return path == rhs.path; }
  auto count() const {
    return weak_from_this().use_count();
  }
  static std::shared_ptr<Experiment> SetupExp(std::string_view p) {
    return std::make_shared<Experiment>(std::filesystem::path { p });
  }
};
// Creates Experiment after Get[Open/Save]FileName

#endif //DXW32_EXPERIMENT_H
