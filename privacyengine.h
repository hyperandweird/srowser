#ifndef PRIVACYENGINE_H
#define PRIVACYENGINE_H

#include <QSet>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QWebEngineUrlRequestInterceptor>

enum class PrivacyLevel {
    Disabled,
    Standard,
    Strict
};

class PrivacyEngine : public QWebEngineUrlRequestInterceptor {
    Q_OBJECT

public:
    explicit PrivacyEngine(QObject *parent = nullptr);
    void setPrivacyLevel(PrivacyLevel level);
    PrivacyLevel privacyLevel() const;
    quint64 blockedCount() const;
    void setCssEnabled(bool enabled);
    static QString normalizeAllowedSite(const QString &address);
    QStringList allowedSites() const;
    void setAllowedSites(const QStringList &sites);
    bool allowedSitesEnabled() const;
    void setAllowedSitesEnabled(bool enabled);

    void interceptRequest(QWebEngineUrlRequestInfo &info) override;

signals:
    void requestBlocked(const QUrl &url);

private:
    PrivacyLevel m_level;
    quint64 m_blockedCount;
    bool m_cssEnabled;
    bool m_allowedSitesEnabled;
    QSet<QString> m_blockRules;
    QSet<QString> m_strictRules;
    QSet<QString> m_allowedSites;

    void loadDefaultBlockLists();
    bool isAllowedUrl(const QUrl &url) const;
    bool shouldBlock(const QUrl &url, const QUrl &firstPartyUrl) const;
};

#endif // PRIVACYENGINE_H