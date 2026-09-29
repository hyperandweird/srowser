#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QString>
#include <QTabWidget>
#include <QUrl>
#include <QWebEngineProfile>
#include <QWebEngineView>

#include "privacyengine.h"
#include "extensionmanager.h"

class BrowserWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);
    ~BrowserWindow();

private slots:
    void addNewTab(const QUrl &url = QUrl());
    void closeTab(int index);
    void currentTabChanged(int index);
    void navigateToAddress();
    void updateAddressBar(const QUrl &url);
    void updateLoadProgress(int progress);
    void translateCurrentWebsite();
    void openSettings();
    void openExtensionStore();

private:
    QTabWidget *m_tabWidget;
    QLineEdit *m_addressBar;
    QPushButton *m_backButton;
    QPushButton *m_forwardButton;
    QPushButton *m_reloadButton;
    QPushButton *m_translateButton;
    QPushButton *m_settingsButton;
    QPushButton *m_extensionsButton;
    QProgressBar *m_progressBar;
    QLabel *m_shieldIcon;

    QWebEngineProfile *m_profile;
    PrivacyEngine *m_privacyEngine;
    ExtensionManager *m_extensionManager;

    QString m_searchEngine;
    bool m_cssEnabled;
    bool m_javascriptEnabled;

    void setupChromeUI();
    void applyChromeStyle();
    QWebEngineView* currentWebView() const;
};

#endif // BROWSERWINDOW_H