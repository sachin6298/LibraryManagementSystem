#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QGuiApplication>
#include <QPixmap>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QFont>
#include <QResource>

#include "mainwindow.h"

class WelcomeWindow final : public QWidget
{
public:
    explicit WelcomeWindow(const QIcon &appIcon)
        : QWidget(nullptr)
    {
        setWindowTitle("Welcome — Library Management System");
        setWindowIcon(appIcon);
        resize(820, 680);
        setMinimumSize(640, 560);
        setObjectName("welcomeWindow");
        setStyleSheet(R"(
            QWidget#welcomeWindow {
                background: #101827;
                color: #edf2f7;
                font-family: "Ubuntu", "Noto Sans", sans-serif;
            }
            QFrame#welcomeCard {
                background: #1e293b;
                border: 1px solid #516174;
                border-radius: 22px;
            }
            QLabel#welcomeTitle {
                color: #f4c98b;
                font-size: 27px;
                font-weight: 700;
            }
            QLabel#welcomeSubtitle {
                color: #d3deea;
                font-size: 15px;
            }
            QLabel#welcomeInstitute {
                color: #aab8c9;
                font-size: 12px;
            }
            QPushButton#startButton {
                background: #16a34a;
                color: white;
                border: 1px solid #22c55e;
                border-radius: 9px;
                padding: 13px 34px;
                font-size: 16px;
                font-weight: 700;
            }
            QPushButton#startButton:hover { background: #15803d; }
            QPushButton#startButton:pressed { background: #166534; }
        )");

        auto *outer = new QVBoxLayout(this);
        outer->setContentsMargins(32, 28, 32, 28);
        outer->addStretch(1);

        auto *row = new QHBoxLayout;
        row->addStretch(1);

        auto *card = new QFrame(this);
        card->setObjectName("welcomeCard");
        card->setMinimumWidth(400);
        card->setMaximumWidth(520);
        auto *content = new QVBoxLayout(card);
        content->setContentsMargins(36, 32, 36, 30);
        content->setSpacing(16);

        auto *logo = new QLabel(card);
        logo->setAlignment(Qt::AlignCenter);
        const QPixmap bitLogo(QStringLiteral(":/logos/bit-logo.png"));
        if (!bitLogo.isNull())
        {
            logo->setPixmap(bitLogo.scaled(
                176, 176, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else
        {
            logo->setText("BIT");
            logo->setStyleSheet("font-size: 34px; font-weight: 800; color: #f4c98b;");
        }
        logo->setMinimumHeight(176);
        content->addWidget(logo, 0, Qt::AlignHCenter);

        auto *title = new QLabel("Welcome to Library\nManagement System", card);
        title->setObjectName("welcomeTitle");
        title->setAlignment(Qt::AlignCenter);
        title->setWordWrap(true);
        content->addWidget(title);

        auto *subtitle = new QLabel(
            "Manage books, borrowers, returns, overdue loans\nand reservations.", card);
        subtitle->setObjectName("welcomeSubtitle");
        subtitle->setAlignment(Qt::AlignCenter);
        subtitle->setWordWrap(true);
        content->addWidget(subtitle);

        auto *institute = new QLabel("Library Management System · Qt Widgets", card);
        institute->setObjectName("welcomeInstitute");
        institute->setAlignment(Qt::AlignCenter);
        content->addWidget(institute);

        auto *startButton = new QPushButton("Start", card);
        startButton->setObjectName("startButton");
        startButton->setMinimumWidth(220);
        startButton->setMinimumHeight(50);
        startButton->setCursor(Qt::PointingHandCursor);
        startButton->setDefault(true);
        content->addWidget(startButton, 0, Qt::AlignHCenter);

        row->addWidget(card);
        row->addStretch(1);
        outer->addLayout(row);
        outer->addStretch(1);

        connect(startButton, &QPushButton::clicked, this, [this, appIcon]() {
            auto *mainWindow = new MainWindow;
            mainWindow->setWindowIcon(appIcon);
            mainWindow->setAttribute(Qt::WA_DeleteOnClose);
            mainWindow->showMaximized();
            close();
        });
    }
};

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(library_resources);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("LibraryManagementSystem");
    QGuiApplication::setApplicationDisplayName("Library Management System");
    QCoreApplication::setOrganizationName("LibraryManagementSystem");
    QGuiApplication::setDesktopFileName("library-management-system");

    const QIcon appIcon(QStringLiteral(":/icons/library.png"));
    app.setWindowIcon(appIcon);

    WelcomeWindow welcome(appIcon);
    welcome.show();
    return app.exec();
}
