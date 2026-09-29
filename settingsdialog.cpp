#include "settingsdialog.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(PrivacyEngine *privacyEngine, QWidget *parent)
    : QDialog(parent), m_privacyEngine(privacyEngine) {
    setupUI();
    setWindowTitle("Srowser Ayarlar");
    resize(520, 680);
}

void SettingsDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Gizlilik Grubu
    QGroupBox *privacyGroup = new QGroupBox("Gizlilik ve Güvenlik Koruması", this);
    QVBoxLayout *privacyLayout = new QVBoxLayout(privacyGroup);

    m_radioDisabled = new QRadioButton("YOK (Korumalar Kapalı)", privacyGroup);
    m_radioStandard = new QRadioButton("Standart (Reklamlar, bilinen izleyiciler engellenir)", privacyGroup);
    m_radioStrict = new QRadioButton("Strict (Sıkı: Üçüncü taraf tüm izleyiciler, parmak izi koruması)", privacyGroup);

    PrivacyLevel currentLevel = m_privacyEngine->privacyLevel();
    if (currentLevel == PrivacyLevel::Disabled) m_radioDisabled->setChecked(true);
    else if (currentLevel == PrivacyLevel::Standard) m_radioStandard->setChecked(true);
    else if (currentLevel == PrivacyLevel::Strict) m_radioStrict->setChecked(true);

    privacyLayout->addWidget(m_radioDisabled);
    privacyLayout->addWidget(m_radioStandard);
    privacyLayout->addWidget(m_radioStrict);

    m_blockedCountLabel = new QLabel(QString("Şu ana kadar engellenen istek sayısı: %1").arg(m_privacyEngine->blockedCount()), privacyGroup);
    privacyLayout->addWidget(m_blockedCountLabel);

    mainLayout->addWidget(privacyGroup);

    QGroupBox *allowedSitesGroup = new QGroupBox("Allowed websites", this);
    QVBoxLayout *allowedSitesLayout = new QVBoxLayout(allowedSitesGroup);
    m_allowedSitesEnabledCheckBox = new QCheckBox("Enable protection exceptions for listed websites", allowedSitesGroup);
    m_allowedSitesEnabledCheckBox->setChecked(m_privacyEngine->allowedSitesEnabled());
    allowedSitesLayout->addWidget(m_allowedSitesEnabledCheckBox);

    m_allowedSitesList = new QListWidget(allowedSitesGroup);
    m_allowedSitesList->addItems(m_privacyEngine->allowedSites());
    m_allowedSitesList->setMinimumHeight(110);
    allowedSitesLayout->addWidget(m_allowedSitesList);

    QHBoxLayout *allowedSiteInputLayout = new QHBoxLayout();
    m_allowedSiteEdit = new QLineEdit(allowedSitesGroup);
    m_allowedSiteEdit->setPlaceholderText("example.com or https://example.com");
    m_addAllowedSiteButton = new QPushButton("Add", allowedSitesGroup);
    allowedSiteInputLayout->addWidget(m_allowedSiteEdit, 1);
    allowedSiteInputLayout->addWidget(m_addAllowedSiteButton);
    allowedSitesLayout->addLayout(allowedSiteInputLayout);

    m_removeAllowedSiteButton = new QPushButton("Remove selected website", allowedSitesGroup);
    m_removeAllowedSiteButton->setEnabled(false);
    allowedSitesLayout->addWidget(m_removeAllowedSiteButton);
    connect(m_addAllowedSiteButton, &QPushButton::clicked, this, &SettingsDialog::addAllowedSite);
    connect(m_allowedSiteEdit, &QLineEdit::returnPressed, this, &SettingsDialog::addAllowedSite);
    connect(m_removeAllowedSiteButton, &QPushButton::clicked, this, &SettingsDialog::removeAllowedSite);
    connect(m_allowedSitesList, &QListWidget::currentRowChanged, m_removeAllowedSiteButton,
            [this](int row) { m_removeAllowedSiteButton->setEnabled(row >= 0); });

    mainLayout->addWidget(allowedSitesGroup);

    // Genel Ayarlar
    QGroupBox *generalGroup = new QGroupBox("Genel Ayarlar", this);
    QVBoxLayout *generalLayout = new QVBoxLayout(generalGroup);

    QHBoxLayout *homeLayout = new QHBoxLayout();
    homeLayout->addWidget(new QLabel("Ana Sayfa:", this));
    m_homePageEdit = new QLineEdit("https://www.google.com", this);
    homeLayout->addWidget(m_homePageEdit);
    generalLayout->addLayout(homeLayout);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Arama Motoru:", this));
    m_searchEngineEdit = new QLineEdit("https://www.google.com/search?q=", this);
    searchLayout->addWidget(m_searchEngineEdit);
    generalLayout->addLayout(searchLayout);

    QSettings settings;
    m_cssCheckBox = new QCheckBox("CSS Stillerini Etkinleştir", this);
    m_cssCheckBox->setChecked(settings.value("web/cssEnabled", true).toBool());
    generalLayout->addWidget(m_cssCheckBox);

    m_jsCheckBox = new QCheckBox("JavaScript Çalıştırmaya İzin Ver", this);
    m_jsCheckBox->setChecked(settings.value("web/javascriptEnabled", true).toBool());
    generalLayout->addWidget(m_jsCheckBox);

    mainLayout->addWidget(generalGroup);

    // Kaydet Butonu
    m_saveButton = new QPushButton("Kaydet ve Kapat", this);
    connect(m_saveButton, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    mainLayout->addWidget(m_saveButton);
}

PrivacyLevel SettingsDialog::selectedPrivacyLevel() const {
    if (m_radioDisabled->isChecked()) return PrivacyLevel::Disabled;
    if (m_radioStrict->isChecked()) return PrivacyLevel::Strict;
    return PrivacyLevel::Standard;
}

QString SettingsDialog::homePageUrl() const {
    return m_homePageEdit->text();
}

QString SettingsDialog::searchEngineUrl() const {
    return m_searchEngineEdit->text();
}

bool SettingsDialog::isCssEnabled() const {
    return m_cssCheckBox->isChecked();
}

bool SettingsDialog::isJavaScriptEnabled() const {
    return m_jsCheckBox->isChecked();
}

void SettingsDialog::saveSettings() {
    m_privacyEngine->setPrivacyLevel(selectedPrivacyLevel());
    QStringList allowedSites;
    for (int row = 0; row < m_allowedSitesList->count(); ++row) {
        allowedSites.append(m_allowedSitesList->item(row)->text());
    }
    m_privacyEngine->setAllowedSites(allowedSites);
    m_privacyEngine->setAllowedSitesEnabled(m_allowedSitesEnabledCheckBox->isChecked());

    QSettings settings;
    settings.setValue("web/cssEnabled", isCssEnabled());
    settings.setValue("web/javascriptEnabled", isJavaScriptEnabled());
    settings.setValue("privacy/allowedSites", m_privacyEngine->allowedSites());
    settings.setValue("privacy/allowedSitesEnabled", m_privacyEngine->allowedSitesEnabled());
    accept();
}

void SettingsDialog::addAllowedSite() {
    const QString host = PrivacyEngine::normalizeAllowedSite(m_allowedSiteEdit->text());
    if (host.isEmpty()) {
        QMessageBox::warning(this, "Invalid website", "Enter a valid website address.");
        return;
    }

    const QList<QListWidgetItem *> matches = m_allowedSitesList->findItems(host, Qt::MatchFixedString);
    if (matches.isEmpty()) m_allowedSitesList->addItem(host);
    m_allowedSiteEdit->clear();
}

void SettingsDialog::removeAllowedSite() {
    delete m_allowedSitesList->takeItem(m_allowedSitesList->currentRow());
}