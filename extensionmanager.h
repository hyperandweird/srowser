#ifndef EXTENSIONMANAGER_H
#define EXTENSIONMANAGER_H

#include <QObject>
#include <QString>
#include <QWebEngineProfile>

class ExtensionManager : public QObject {
    Q_OBJECT

public:
    explicit ExtensionManager(QWebEngineProfile *profile, QObject *parent = nullptr);
    
    void loadExtensionFromPath(const QString &path);
    void installUBlockOriginBuiltIn();
    void setExtensionStoreUrl();

signals:
    void extensionLoaded(const QString &name);
    void extensionError(const QString &error);

private:
    QWebEngineProfile *m_profile;
    void ensureExtensionDirExists();
    QString m_extensionsPath;
};

#endif // EXTENSIONMANAGER_H