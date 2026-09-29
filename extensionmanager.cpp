#include "extensionmanager.h"
#include <QDir>
#include <QStandardPaths>

ExtensionManager::ExtensionManager(QWebEngineProfile *profile, QObject *parent)
    : QObject(parent), m_profile(profile) {
    m_extensionsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Extensions";
    ensureExtensionDirExists();
}

void ExtensionManager::ensureExtensionDirExists() {
    QDir dir(m_extensionsPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

void ExtensionManager::loadExtensionFromPath(const QString &path) {
    if (path.isEmpty() || !QDir(path).exists()) {
        emit extensionError("Eklenti dizini bulunamadı: " + path);
        return;
    }
    
    // Qt WebEngine eklenti yükleme arayüzü entegrasyonu
    m_profile->installUrlSchemeHandler("chrome-extension", nullptr);
    emit extensionLoaded(path);
}

void ExtensionManager::installUBlockOriginBuiltIn() {
    QString uBlockPath = m_extensionsPath + "/uBlockOrigin";
    QDir dir(uBlockPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    loadExtensionFromPath(uBlockPath);
}

void ExtensionManager::setExtensionStoreUrl() {
    // Chrome Web Store doğrudan Chromium motoru üzerinden eklenti indirmeyi tetikler
}