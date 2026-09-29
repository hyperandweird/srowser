#include "browserwindow.h"
#include "settingsdialog.h"

#include <QCoreApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFormLayout>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QList>
#include <QMessageBox>
#include <QPair>
#include <QShortcut>
#include <QSettings>
#include <QStringList>
#include <QVBoxLayout>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include <QUrlQuery>

namespace {
const QList<QPair<QString, QString>> &supportedLanguages() {
    static const QList<QPair<QString, QString>> languages = {
        {"Afrikaans", "af"}, {"Albanian", "sq"}, {"Amharic", "am"},
        {"Arabic", "ar"}, {"Armenian", "hy"}, {"Assamese", "as"},
        {"Aymara", "ay"}, {"Azerbaijani", "az"}, {"Bambara", "bm"},
        {"Basque", "eu"}, {"Belarusian", "be"}, {"Bengali", "bn"},
        {"Bhojpuri", "bho"}, {"Bosnian", "bs"}, {"Bulgarian", "bg"},
        {"Catalan", "ca"}, {"Cebuano", "ceb"}, {"Chinese (Simplified)", "zh-CN"},
        {"Chinese (Traditional)", "zh-TW"}, {"Corsican", "co"}, {"Croatian", "hr"},
        {"Czech", "cs"}, {"Danish", "da"}, {"Dhivehi", "dv"},
        {"Dogri", "doi"}, {"Dutch", "nl"}, {"English", "en"},
        {"Esperanto", "eo"}, {"Estonian", "et"}, {"Ewe", "ee"},
        {"Filipino", "fil"}, {"Finnish", "fi"}, {"French", "fr"},
        {"Frisian", "fy"}, {"Galician", "gl"}, {"Georgian", "ka"},
        {"German", "de"}, {"Greek", "el"}, {"Guarani", "gn"},
        {"Gujarati", "gu"}, {"Haitian Creole", "ht"}, {"Hausa", "ha"},
        {"Hawaiian", "haw"}, {"Hebrew", "he"}, {"Hindi", "hi"},
        {"Hmong", "hmn"}, {"Hungarian", "hu"}, {"Icelandic", "is"},
        {"Igbo", "ig"}, {"Iloko", "ilo"}, {"Indonesian", "id"},
        {"Irish", "ga"}, {"Italian", "it"}, {"Japanese", "ja"},
        {"Javanese", "jv"}, {"Kannada", "kn"}, {"Kazakh", "kk"},
        {"Khmer", "km"}, {"Kinyarwanda", "rw"}, {"Konkani", "gom"},
        {"Korean", "ko"}, {"Krio", "kri"}, {"Kurdish", "ku"},
        {"Kyrgyz", "ky"}, {"Lao", "lo"}, {"Latin", "la"},
        {"Latvian", "lv"}, {"Lingala", "ln"}, {"Lithuanian", "lt"},
        {"Luganda", "lg"}, {"Luxembourgish", "lb"}, {"Macedonian", "mk"},
        {"Maithili", "mai"}, {"Malagasy", "mg"}, {"Malay", "ms"},
        {"Malayalam", "ml"}, {"Maltese", "mt"}, {"Maori", "mi"},
        {"Marathi", "mr"}, {"Meiteilon (Manipuri)", "mni-Mtei"}, {"Mizo", "lus"},
        {"Mongolian", "mn"}, {"Myanmar (Burmese)", "my"}, {"Nepali", "ne"},
        {"Norwegian", "no"}, {"Nyanja (Chichewa)", "ny"}, {"Odia", "or"},
        {"Oromo", "om"}, {"Pashto", "ps"}, {"Persian", "fa"},
        {"Polish", "pl"}, {"Portuguese", "pt"}, {"Punjabi", "pa"},
        {"Quechua", "qu"}, {"Romanian", "ro"}, {"Russian", "ru"},
        {"Samoan", "sm"}, {"Sanskrit", "sa"}, {"Scots Gaelic", "gd"},
        {"Serbian", "sr"}, {"Sesotho", "st"}, {"Shona", "sn"},
        {"Sindhi", "sd"}, {"Sinhala", "si"}, {"Slovak", "sk"},
        {"Slovenian", "sl"}, {"Somali", "so"}, {"Spanish", "es"},
        {"Sundanese", "su"}, {"Swahili", "sw"}, {"Swedish", "sv"},
        {"Tajik", "tg"}, {"Tamil", "ta"}, {"Tatar", "tt"},
        {"Telugu", "te"}, {"Thai", "th"}, {"Tigrinya", "ti"},
        {"Tsonga", "ts"}, {"Turkish", "tr"}, {"Turkmen", "tk"},
        {"Twi", "ak"}, {"Ukrainian", "uk"}, {"Urdu", "ur"},
        {"Uyghur", "ug"}, {"Uzbek", "uz"}, {"Vietnamese", "vi"},
        {"Welsh", "cy"}, {"Xhosa", "xh"}, {"Yiddish", "yi"},
        {"Yoruba", "yo"}, {"Zulu", "zu"}
    };
    return languages;
}

QUrl defaultIndexUrl() {
    QDir applicationDirectory(QCoreApplication::applicationDirPath());
    const QStringList searchDirectories = {
        QDir::currentPath(),
        applicationDirectory.absolutePath()
    };
    QStringList directories = searchDirectories;
    applicationDirectory.cdUp();
    directories.append(applicationDirectory.absolutePath());

    for (const QString &directory : directories) {
        const QFileInfo indexFile(QDir(directory).filePath("index.html"));
        if (indexFile.isFile()) {
            return QUrl::fromLocalFile(indexFile.absoluteFilePath());
        }
    }

    return QUrl::fromLocalFile(QDir::current().absoluteFilePath("index.html"));
}

void disablePageStyles(QWebEnginePage *page) {
    page->runJavaScript(QStringLiteral(R"JS(
        (() => {
            const disableElement = (element) => {
                if (!(element instanceof Element)) return;
                if (element.matches("style, link[rel~='stylesheet']")) element.disabled = true;
                if (element.hasAttribute("style")) {
                    if (!Object.prototype.hasOwnProperty.call(element, "__srowserOriginalStyle")) {
                        element.__srowserOriginalStyle = element.getAttribute("style");
                    }
                    element.removeAttribute("style");
                }
            };
            const disableTree = (node) => {
                if (!(node instanceof Element)) return;
                disableElement(node);
                node.querySelectorAll("style, link[rel~='stylesheet'], [style]").forEach(disableElement);
            };

            disableTree(document.documentElement);
            for (const sheet of document.styleSheets) sheet.disabled = true;
            if (window.__srowserCssObserver) window.__srowserCssObserver.disconnect();
            window.__srowserCssObserver = new MutationObserver((records) => {
                for (const record of records) {
                    if (record.type === "attributes") {
                        disableElement(record.target);
                    } else {
                        record.addedNodes.forEach(disableTree);
                    }
                }
            });
            window.__srowserCssObserver.observe(document.documentElement, {
                attributes: true,
                attributeFilter: ["style", "rel"],
                childList: true,
                subtree: true
            });
        })();
    )JS"));
}
}

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent), m_searchEngine("https://www.google.com/search?q=") {

    QSettings settings;
    m_cssEnabled = settings.value("web/cssEnabled", true).toBool();
    m_javascriptEnabled = settings.value("web/javascriptEnabled", true).toBool();

    m_profile = new QWebEngineProfile("SrowserProfile", this);
    m_privacyEngine = new PrivacyEngine(this);
    m_privacyEngine->setCssEnabled(m_cssEnabled);
    m_privacyEngine->setAllowedSites(settings.value("privacy/allowedSites").toStringList());
    m_privacyEngine->setAllowedSitesEnabled(settings.value("privacy/allowedSitesEnabled", false).toBool());
    m_profile->setUrlRequestInterceptor(m_privacyEngine);

    m_extensionManager = new ExtensionManager(m_profile, this);
    m_extensionManager->installUBlockOriginBuiltIn();

    setupChromeUI();
    applyChromeStyle();

    addNewTab();

    resize(1280, 800);
    setWindowTitle("Srowser");
}

BrowserWindow::~BrowserWindow() {}

void BrowserWindow::setupChromeUI() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Chrome Tarzı Üst Bar
    QHBoxLayout *navLayout = new QHBoxLayout();
    navLayout->setContentsMargins(8, 6, 8, 6);
    navLayout->setSpacing(6);

    m_backButton = new QPushButton("←", this);
    m_forwardButton = new QPushButton("→", this);
    m_reloadButton = new QPushButton("↻", this);
    m_translateButton = new QPushButton("Translate", this);
    m_translateButton->setToolTip("Translate this website");

    m_shieldIcon = new QLabel("🛡️", this);
    m_shieldIcon->setToolTip("Srowser Gizlilik Koruması Aktif");

    m_addressBar = new QLineEdit(this);
    m_addressBar->setPlaceholderText("Arama yapın veya bir web adresi yazın");

    m_extensionsButton = new QPushButton("🧩 Eklentiler", this);
    m_settingsButton = new QPushButton("⋮", this);

    navLayout->addWidget(m_backButton);
    navLayout->addWidget(m_forwardButton);
    navLayout->addWidget(m_reloadButton);
    navLayout->addWidget(m_shieldIcon);
    navLayout->addWidget(m_addressBar, 1);
    navLayout->addWidget(m_translateButton);
    navLayout->addWidget(m_extensionsButton);
    navLayout->addWidget(m_settingsButton);

    mainLayout->addLayout(navLayout);

    // İlerleme Çubuğu
    m_progressBar = new QProgressBar(this);
    m_progressBar->setFixedHeight(2);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet("QProgressBar::chunk { background-color: #1a73e8; } QProgressBar { border: none; background: transparent; }");
    mainLayout->addWidget(m_progressBar);

    // Sekmeler
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    auto *newTabButton = new QPushButton("+", m_tabWidget);
    newTabButton->setToolTip("New tab");
    newTabButton->setAccessibleName("New tab");
    newTabButton->setFixedSize(28, 28);
    m_tabWidget->setCornerWidget(newTabButton, Qt::TopRightCorner);
    mainLayout->addWidget(m_tabWidget);

    setCentralWidget(centralWidget);

    // Bağlantılar
    connect(m_addressBar, &QLineEdit::returnPressed, this, &BrowserWindow::navigateToAddress);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &BrowserWindow::closeTab);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &BrowserWindow::currentTabChanged);
    connect(newTabButton, &QPushButton::clicked, this, [this]() { addNewTab(); });

    connect(m_backButton, &QPushButton::clicked, [this]() {
        if (currentWebView()) currentWebView()->back();
    });
    connect(m_forwardButton, &QPushButton::clicked, [this]() {
        if (currentWebView()) currentWebView()->forward();
    });
    connect(m_reloadButton, &QPushButton::clicked, [this]() {
        if (currentWebView()) currentWebView()->reload();
    });

    connect(m_settingsButton, &QPushButton::clicked, this, &BrowserWindow::openSettings);
    connect(m_extensionsButton, &QPushButton::clicked, this, &BrowserWindow::openExtensionStore);
    connect(m_translateButton, &QPushButton::clicked, this, &BrowserWindow::translateCurrentWebsite);

    // Kısayollar
    new QShortcut(QKeySequence("Ctrl+T"), this, [this]() { addNewTab(); });
    new QShortcut(QKeySequence("Ctrl+W"), this, [this]() { closeTab(m_tabWidget->currentIndex()); });
}

void BrowserWindow::applyChromeStyle() {
    QString style = R"(
        QMainWindow {
            background-color: #dee1e6;
        }
        QLineEdit {
            background-color: #f1f3f4;
            border: 1px solid transparent;
            border-radius: 14px;
            padding: 5px 12px;
            font-size: 13px;
            color: #202124;
        }
        QLineEdit:focus {
            background-color: #ffffff;
            border: 1px solid #1a73e8;
        }
        QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 12px;
            min-width: 28px;
            min-height: 28px;
            font-size: 14px;
            color: #5f6368;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
        QTabWidget::pane {
            border: none;
        }
        QTabBar::tab {
            background-color: #dee1e6;
            color: #5f6368;
            padding: 8px 16px;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #ffffff;
            color: #202124;
        }
    )";
    this->setStyleSheet(style);
}

void BrowserWindow::addNewTab(const QUrl &url) {
    QWebEnginePage *page = new QWebEnginePage(m_profile, this);
    QWebEngineView *view = new QWebEngineView(this);
    view->setPage(page);

    view->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    view->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    view->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    view->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, m_javascriptEnabled);
    view->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    int index = m_tabWidget->addTab(view, "Yeni Sekme");
    m_tabWidget->setCurrentIndex(index);

    connect(view, &QWebEngineView::urlChanged, this, [this, view](const QUrl &newUrl) {
        if (currentWebView() == view) {
            updateAddressBar(newUrl);
        }
    });

    connect(view, &QWebEngineView::titleChanged, this, [this, view](const QString &title) {
        int idx = m_tabWidget->indexOf(view);
        if (idx != -1) {
            m_tabWidget->setTabText(idx, title.left(15) + (title.length() > 15 ? "..." : ""));
        }
    });

    connect(view, &QWebEngineView::loadProgress, this, [this, view](int progress) {
        if (currentWebView() == view) {
            updateLoadProgress(progress);
        }
    });
    connect(view, &QWebEngineView::loadFinished, this, [this, view](bool) {
        if (!m_cssEnabled) {
            disablePageStyles(view->page());
        }
    });

    view->load(url.isEmpty() ? defaultIndexUrl() : url);
}

void BrowserWindow::closeTab(int index) {
    if (m_tabWidget->count() > 1) {
        QWidget *widget = m_tabWidget->widget(index);
        m_tabWidget->removeTab(index);
        delete widget;
    } else {
        close();
    }
}

void BrowserWindow::currentTabChanged(int index) {
    Q_UNUSED(index);
    if (currentWebView()) {
        updateAddressBar(currentWebView()->url());
    }
}

void BrowserWindow::navigateToAddress() {
    if (!currentWebView()) return;

    QString input = m_addressBar->text().trimmed();
    QUrl url = QUrl::fromUserInput(input);

    if (!input.contains(".") && !input.contains("://")) {
        url = QUrl(m_searchEngine + QUrl::toPercentEncoding(input));
    }

    currentWebView()->load(url);
}

void BrowserWindow::updateAddressBar(const QUrl &url) {
    m_addressBar->setText(url.toString());
}

void BrowserWindow::updateLoadProgress(int progress) {
    if (progress == 100) {
        m_progressBar->setValue(0);
    } else {
        m_progressBar->setValue(progress);
    }
}

void BrowserWindow::translateCurrentWebsite() {
    QWebEngineView *view = currentWebView();
    if (!view) return;

    const QUrl pageUrl = view->url();
    if (pageUrl.scheme() != "http" && pageUrl.scheme() != "https") {
        QMessageBox::information(this, "Translate website",
                                 "Website translation requires an HTTP or HTTPS page.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Translate website");
    QFormLayout *form = new QFormLayout(&dialog);
    QComboBox *sourceLanguage = new QComboBox(&dialog);
    QComboBox *targetLanguage = new QComboBox(&dialog);
    sourceLanguage->addItem("Detect automatically", "auto");

    for (const auto &language : supportedLanguages()) {
        sourceLanguage->addItem(language.first, language.second);
        targetLanguage->addItem(language.first, language.second);
    }

    form->addRow("Translate from:", sourceLanguage);
    form->addRow("Translate to:", targetLanguage);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Cancel | QDialogButtonBox::Ok, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText("Translate");
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    QUrl translationUrl("https://translate.google.com/translate");
    QUrlQuery query;
    query.addQueryItem("sl", sourceLanguage->currentData().toString());
    query.addQueryItem("tl", targetLanguage->currentData().toString());
    query.addQueryItem("u", pageUrl.toString());
    translationUrl.setQuery(query);
    view->load(translationUrl);
}

void BrowserWindow::openSettings() {
    SettingsDialog dialog(m_privacyEngine, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_searchEngine = dialog.searchEngineUrl();
        const bool cssChanged = m_cssEnabled != dialog.isCssEnabled();
        m_cssEnabled = dialog.isCssEnabled();
        m_javascriptEnabled = dialog.isJavaScriptEnabled();
        m_privacyEngine->setCssEnabled(m_cssEnabled);

        for (int index = 0; index < m_tabWidget->count(); ++index) {
            auto *view = qobject_cast<QWebEngineView *>(m_tabWidget->widget(index));
            if (!view) continue;

            if (cssChanged && !m_cssEnabled) {
                disablePageStyles(view->page());
            }
            view->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, m_javascriptEnabled);
            if (cssChanged && m_cssEnabled) {
                view->reload();
            }
        }
    }
}

void BrowserWindow::openExtensionStore() {
    addNewTab(QUrl("https://chromewebstore.google.com/"));
}

QWebEngineView* BrowserWindow::currentWebView() const {
    return qobject_cast<QWebEngineView *>(m_tabWidget->currentWidget());
}