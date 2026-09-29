#include "privacyengine.h"
#include <QString>

PrivacyEngine::PrivacyEngine(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent),
      m_level(PrivacyLevel::Standard),
            m_blockedCount(0),
            m_cssEnabled(true),
            m_allowedSitesEnabled(false) {
    loadDefaultBlockLists();
}

void PrivacyEngine::setPrivacyLevel(PrivacyLevel level) {
    m_level = level;
}

PrivacyLevel PrivacyEngine::privacyLevel() const {
    return m_level;
}

quint64 PrivacyEngine::blockedCount() const {
    return m_blockedCount;
}

void PrivacyEngine::setCssEnabled(bool enabled) {
    m_cssEnabled = enabled;
}

QString PrivacyEngine::normalizeAllowedSite(const QString &address) {
    QString candidate = address.trimmed();
    if (candidate.isEmpty()) return {};
    if (!candidate.contains("://")) candidate.prepend("https://");

    const QUrl url(candidate, QUrl::TolerantMode);
    QString host = url.isValid() ? url.host().toLower() : QString();
    while (host.endsWith('.')) host.chop(1);
    if (host.startsWith("www.")) host.remove(0, 4);
    return host;
}

QStringList PrivacyEngine::allowedSites() const {
    QStringList sites = m_allowedSites.values();
    sites.sort(Qt::CaseInsensitive);
    return sites;
}

void PrivacyEngine::setAllowedSites(const QStringList &sites) {
    m_allowedSites.clear();
    for (const QString &site : sites) {
        const QString host = normalizeAllowedSite(site);
        if (!host.isEmpty()) m_allowedSites.insert(host);
    }
}

bool PrivacyEngine::allowedSitesEnabled() const {
    return m_allowedSitesEnabled;
}

void PrivacyEngine::setAllowedSitesEnabled(bool enabled) {
    m_allowedSitesEnabled = enabled;
}

void PrivacyEngine::loadDefaultBlockLists() {
    m_blockRules = {
        "doubleclick.net",
        "google-analytics.com",
        "googlesyndication.com",
        "adservice.google.com",
        "facebook.com/tr",
        "connect.facebook.net",
        "adnxs.com",
        "criteo.com",
        "outbrain.com",
        "taboola.com",
        "scorecardresearch.com",
        "amazon-adsystem.com",
        "analytics.yahoo.com",
        "hotjar.com",
        "clarity.ms",
        "mixpanel.com"
    };

    m_strictRules = {
        "fingerprint.com",
        "fonts.googleapis.com",
        "gstatic.com",
        "mc.yandex.ru",
        "disqus.com"
    };
}

bool PrivacyEngine::shouldBlock(const QUrl &url, const QUrl &firstPartyUrl) const {
    if (m_level == PrivacyLevel::Disabled) {
        return false;
    }

    QString host = url.host().toLower();

    for (const QString &rule : m_blockRules) {
        if (host.contains(rule)) {
            return true;
        }
    }

    if (m_level == PrivacyLevel::Strict) {
        for (const QString &strictRule : m_strictRules) {
            if (host.contains(strictRule)) {
                return true;
            }
        }
        const QString firstPartyHost = firstPartyUrl.host().toLower();
        if (!firstPartyHost.isEmpty() && host != firstPartyHost && url.scheme() == "http") {
            return true;
        }
    }

    return false;
}

bool PrivacyEngine::isAllowedUrl(const QUrl &url) const {
    if (!m_allowedSitesEnabled) return false;

    const QString host = normalizeAllowedSite(url.host());
    if (host.isEmpty()) return false;

    for (const QString &site : m_allowedSites) {
        if (host == site || host.endsWith('.' + site)) return true;
    }
    return false;
}

void PrivacyEngine::interceptRequest(QWebEngineUrlRequestInfo &info) {
    if (!m_cssEnabled && info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeStylesheet) {
        info.block(true);
        return;
    }

    const bool allowedSiteRequest = isAllowedUrl(info.requestUrl()) || isAllowedUrl(info.firstPartyUrl());
    if (m_level == PrivacyLevel::Strict && !allowedSiteRequest) {
        info.setHttpHeader("DNT", "1");
        info.setHttpHeader("Sec-GPC", "1");
    }

    if (allowedSiteRequest) return;

    if (shouldBlock(info.requestUrl(), info.firstPartyUrl())) {
        info.block(true);
        m_blockedCount++;
        emit requestBlocked(info.requestUrl());
    }
}