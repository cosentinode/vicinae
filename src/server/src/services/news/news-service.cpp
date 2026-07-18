#include "news-service.hpp"
#include "builtin_icon.hpp"
#include "service-registry.hpp"
#include "vicinae.hpp"
#include <algorithm>
#include <fstream>
#include <glaze/core/common.hpp>
#include <glaze/glaze.hpp>
#include <ranges>

struct NewsState {
  std::vector<std::string> dismissed;
};

DismissNewsAction::DismissNewsAction(std::string newsId)
    : AbstractAction(QStringLiteral("Dismiss"), BuiltinIcon::Xmark), m_newsId(std::move(newsId)) {}

void DismissNewsAction::execute(ApplicationContext *ctx) { ctx->services->newsService()->dismiss(m_newsId); }

NewsService::NewsService() : m_stateFile(Omnicast::stateDir() / "news.json") {
  m_items = allItems();
  loadState();
}

void NewsService::dismiss(const std::string &id) {
  if (isDismissed(id)) return;
  m_dismissed.emplace_back(id);
  saveState();
  emit itemsChanged();
}

bool NewsService::isDismissed(const std::string &id) const {
  return std::ranges::find(m_dismissed, id) != m_dismissed.end();
}

bool NewsService::hasUnreadNews() const { return !activeItems().empty(); }

std::vector<const NewsItem *> NewsService::activeItems() const {
  std::vector<const NewsItem *> result;
  result.reserve(m_items.size());
  for (const auto &item : m_items) {
    if (isDismissed(item.id)) continue;
    result.emplace_back(&item);
  }
  return result;
}

void NewsService::loadState() {
  std::string buf;
  NewsState state;

  if (const auto err = glz::read_file_json(state, m_stateFile.string(), buf)) {
    qWarning() << "Failed to parse news state:" << glz::format_error(err);
    return;
  }

  m_dismissed = std::move(state.dismissed);
}

void NewsService::saveState() const {
  NewsState state{.dismissed = m_dismissed};
  std::string buf;
  if (const auto err = glz::write_json(state, buf)) {
    qWarning() << "Failed to serialize news state:" << glz::format_error(err);
    return;
  }

  std::ofstream file(m_stateFile);
  if (!file.is_open()) {
    qWarning() << "Failed to open news state file for writing:" << m_stateFile;
    return;
  }
  file << buf;
}

std::vector<NewsItem> NewsService::allItems() { return {}; }
