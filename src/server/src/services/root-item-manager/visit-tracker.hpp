#pragma once
#include <cstdint>
#include <deque>
#include <unordered_map>
#include <filesystem>
#include "common/entrypoint.hpp"

class VisitTracker {
public:
  struct VisitInfo {
    std::optional<std::uint64_t> lastVisitedAt;
    int visitCount = 0;
  };

  struct HistoryEntry {
    std::string id;
    std::string q;
    std::optional<std::uint64_t> ts;
  };

  struct Data {
    std::unordered_map<std::string, VisitInfo> visited;
    std::deque<HistoryEntry> history;
  };

  VisitTracker(const std::filesystem::path &path);

  std::optional<HistoryEntry> history(int offset = 0) const;
  void registerVisit(const EntrypointId &id, std::string_view q);
  void forget(const EntrypointId &id);
  VisitInfo getVisit(const EntrypointId &id) const;

private:
  void loadFromDisk();
  void saveToDisk();

  std::filesystem::path m_path;
  std::string m_buf;
  Data m_data;
};
