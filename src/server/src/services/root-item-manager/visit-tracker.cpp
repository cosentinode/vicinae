#include "services/root-item-manager/visit-tracker.hpp"
#include <glaze/core/reflect.hpp>
#include <glaze/json/read.hpp>
#include <glaze/json/write.hpp>
#include <glaze/util/key_transformers.hpp>
#include <qlogging.h>
#include <QDateTime>

namespace fs = std::filesystem;

namespace {
constexpr auto MAX_HISTORY_SIZE = 1000;
};

VisitTracker::VisitTracker(const fs::path &path) : m_path(path) { loadFromDisk(); }

void VisitTracker::registerVisit(const EntrypointId &id, std::string_view q) {
  auto ts = QDateTime::currentSecsSinceEpoch();
  auto &front = m_data.history.front();
  const bool isImmediateDupe = std::string{id} == front.id && q == front.q;

  if (!isImmediateDupe) {
    m_data.history.emplace_front(HistoryEntry{.id = id, .q = std::string{q}, .ts = ts});

    if (m_data.history.size() > MAX_HISTORY_SIZE) {
      for (int i = m_data.history.size() - MAX_HISTORY_SIZE; i > 0; --i) {
        m_data.history.pop_back();
      }
    }
  }

  VisitInfo &data = m_data.visited[id];

  data.lastVisitedAt = ts;
  ++data.visitCount;
  saveToDisk();
}

std::optional<VisitTracker::HistoryEntry> VisitTracker::history(int offset) const {
  if (offset >= static_cast<int>(m_data.history.size())) return std::nullopt;
  return m_data.history[offset];
}

VisitTracker::VisitInfo VisitTracker::getVisit(const EntrypointId &id) const {
  if (auto it = m_data.visited.find(id); it != m_data.visited.end()) { return it->second; }
  return {};
}

void VisitTracker::forget(const EntrypointId &id) {
  m_data.visited.erase(id);
  saveToDisk();
}

void VisitTracker::loadFromDisk() {
  if (auto error = glz::read_file_jsonc(m_data, m_path.string(), m_buf)) {
    qWarning() << "Failed to load visits file from" << m_path.string().c_str() << glz::format_error(error);
  }
}

void VisitTracker::saveToDisk() {
  if (auto error = glz::write_file_json(m_data, m_path.string(), m_buf)) {
    qWarning() << "Failed to save visit to" << m_path.string().c_str() << glz::format_error(error);
  }
}

template <> struct glz::meta<VisitTracker::VisitInfo> : glz::snake_case {};
