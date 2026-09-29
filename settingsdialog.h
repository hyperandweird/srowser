#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QWidget>
#include "privacyengine.h"

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(PrivacyEngine *privacyEngine, QWidget *parent = nullptr);

    PrivacyLevel selectedPrivacyLevel() const;
    QString homePageUrl() const;
    QString searchEngineUrl() const;
    bool isCssEnabled() const;
    bool isJavaScriptEnabled() const;

private slots:
    void saveSettings();

private:
    PrivacyEngine *m_privacyEngine;

    QRadioButton *m_radioDisabled;
    QRadioButton *m_radioStandard;
    QRadioButton *m_radioStrict;

    QLineEdit *m_homePageEdit;
    QLineEdit *m_searchEngineEdit;
    QCheckBox *m_cssCheckBox;
    QCheckBox *m_jsCheckBox;
    QCheckBox *m_allowedSitesEnabledCheckBox;
    QLineEdit *m_allowedSiteEdit;
    QListWidget *m_allowedSitesList;
    QPushButton *m_addAllowedSiteButton;
    QPushButton *m_removeAllowedSiteButton;

    QLabel *m_blockedCountLabel;
    QPushButton *m_saveButton;

    void setupUI();
    void addAllowedSite();
    void removeAllowedSite();
};

#endif // SETTINGSDIALOG_H