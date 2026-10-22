#include "mainwindow.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDate>
#include <QDataStream>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QStringList>
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
constexpr quint32 kMaximumRecords = 100000;

QTableWidget *makeTable(QWidget *parent, const QStringList &headers)
{
    auto *table = new QTableWidget(0, headers.size(), parent);
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    table->setSortingEnabled(false);
    table->verticalHeader()->setVisible(false);
    return table;
}

void setCell(QTableWidget *table, int row, int column, const QString &value)
{
    table->setItem(row, column, new QTableWidgetItem(value));
}

bool finishWrite(QSaveFile &file, const QDataStream &stream)
{
    if (stream.status() != QDataStream::Ok)
        return false;
    return file.commit();
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Library Management System");
    resize(1180, 760);
    setMinimumSize(920, 620);


    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(22, 18, 22, 18);
    root->setSpacing(14);
    setCentralWidget(central);

    auto *headerRow = new QHBoxLayout();
    auto *headingColumn = new QVBoxLayout();
    auto *heading = new QLabel("Library Management System");
    heading->setObjectName("pageHeading");
    auto *subHeading = new QLabel(
        "Manage inventory, circulation, borrowers, overdue books and reservations.");
    subHeading->setObjectName("subHeading");
    headingColumn->addWidget(heading);
    headingColumn->addWidget(subHeading);
    headerRow->addLayout(headingColumn);
    headerRow->addStretch();
    themeButton = new QPushButton("Switch to Dark Mode", central);
    themeButton->setObjectName("themeButton");
    themeButton->setToolTip("Switch between light and dark appearance");
    headerRow->addWidget(themeButton);
    connect(themeButton, &QPushButton::clicked, this, [this]() {
        darkMode = !darkMode;
        applyTheme(darkMode);
    });
    root->addLayout(headerRow);

    auto *statsRow = new QHBoxLayout();
    auto makeStatCard = [central, statsRow](const QString &caption) -> QLabel * {
        auto *card = new QFrame(central);
        card->setObjectName("statCard");
        auto *cardLayout = new QVBoxLayout(card);
        auto *title = new QLabel(caption, card);
        title->setObjectName("statCaption");
        auto *value = new QLabel("0", card);
        value->setObjectName("statValue");
        cardLayout->addWidget(title);
        cardLayout->addWidget(value);
        cardLayout->setContentsMargins(14, 10, 14, 10);
        statsRow->addWidget(card, 1);
        return value;
    };

    titlesStat = makeStatCard("Book titles");
    copiesStat = makeStatCard("Available copies");
    loansStat = makeStatCard("Active loans");
    overdueStat = makeStatCard("Overdue loans");
    waitlistStat = makeStatCard("Waiting borrowers");
    root->addLayout(statsRow);

    tabs = new QTabWidget(central);
    root->addWidget(tabs, 1);

    // Dashboard page
    auto *dashboardPage = new QWidget(tabs);
    auto *dashboardLayout = new QVBoxLayout(dashboardPage);
    auto *dashboardTitle = new QLabel("Overview");
    dashboardTitle->setObjectName("sectionHeading");
    auto *dashboardText = new QLabel(
        "Use Book Inventory to add, find, edit or remove titles. "
        "Use Active Loans to issue books and return selected loans. "
        "Return history is retained separately from active loans.");
    dashboardText->setWordWrap(true);
    dashboardText->setObjectName("bodyText");
    dashboardLayout->addWidget(dashboardTitle);
    dashboardLayout->addWidget(dashboardText);
    dashboardLayout->addStretch();
    tabs->addTab(dashboardPage, "Dashboard");

    // Book inventory page
    auto *booksPage = new QWidget(tabs);
    auto *booksLayout = new QVBoxLayout(booksPage);
    auto *searchRow = new QHBoxLayout();
    searchEdit = new QLineEdit(booksPage);
    searchEdit->setPlaceholderText("Filter by book ID, title or author...");
    auto *searchButton = new QPushButton("Find ID", booksPage);
    auto *addButton = new QPushButton("Add Book", booksPage);
    auto *editButton = new QPushButton("Edit Selected", booksPage);
    auto *deleteButton = new QPushButton("Delete Book", booksPage);
    deleteButton->setObjectName("dangerButton");
    deleteButton->setToolTip("Select a book row, then delete it. Books with active loans or waiters cannot be deleted.");
    searchRow->addWidget(searchEdit, 1);
    searchRow->addWidget(searchButton);
    searchRow->addWidget(addButton);
    searchRow->addWidget(editButton);
    searchRow->addWidget(deleteButton);
    booksLayout->addLayout(searchRow);
    bookTable = makeTable(booksPage,
        {"Book ID", "Title", "Author", "Total copies", "Available"});
    booksLayout->addWidget(bookTable, 1);
    tabs->addTab(booksPage, "Book Inventory");

    // Active loans page
    auto *loansPage = new QWidget(tabs);
    auto *loansLayout = new QVBoxLayout(loansPage);
    auto *loanActions = new QHBoxLayout();
    auto *issueButton = new QPushButton("Issue Book", loansPage);
    auto *returnButton = new QPushButton("Return Selected Loan", loansPage);
    loanActions->addWidget(issueButton);
    loanActions->addWidget(returnButton);
    loanActions->addStretch();
    loansLayout->addLayout(loanActions);
    loansTable = makeTable(loansPage,
        {"Book ID", "Title", "Borrower", "Issue date", "Due date", "Status"});
    loansLayout->addWidget(loansTable, 1);
    tabs->addTab(loansPage, "Active Loans");

    // Return history page. Deleting a record moves it to Deleted History,
    // so mistakes can be reversed without losing the record immediately.
    auto *historyPage = new QWidget(tabs);
    auto *historyLayout = new QVBoxLayout(historyPage);
    auto *historyDescription = new QLabel(
        "Completed loans are retained here. Delete moves a selected record to Deleted History.", historyPage);
    historyDescription->setObjectName("mutedText");
    historyLayout->addWidget(historyDescription);
    auto *historyActions = new QHBoxLayout();
    auto *deleteHistoryButton = new QPushButton("Delete Selected History", historyPage);
    deleteHistoryButton->setObjectName("dangerButton");
    deleteHistoryButton->setToolTip("Move the selected return record to Deleted History; it can be restored.");
    historyActions->addWidget(deleteHistoryButton);
    historyActions->addStretch();
    historyLayout->addLayout(historyActions);
    historyTable = makeTable(historyPage,
        {"Book ID", "Title", "Borrower", "Issue date", "Due date", "Returned date"});
    historyLayout->addWidget(historyTable, 1);
    tabs->addTab(historyPage, "Return History");

    // Deleted history recycle bin with restore and permanent-delete actions.
    auto *deletedHistoryPage = new QWidget(tabs);
    auto *deletedHistoryLayout = new QVBoxLayout(deletedHistoryPage);
    auto *deletedHistoryDescription = new QLabel(
        "Deleted records are kept here until restored or permanently deleted.", deletedHistoryPage);
    deletedHistoryDescription->setObjectName("mutedText");
    deletedHistoryLayout->addWidget(deletedHistoryDescription);
    auto *deletedHistoryActions = new QHBoxLayout();
    auto *restoreHistoryButton = new QPushButton("Restore Selected", deletedHistoryPage);
    auto *purgeHistoryButton = new QPushButton("Delete Permanently", deletedHistoryPage);
    purgeHistoryButton->setObjectName("dangerButton");
    purgeHistoryButton->setToolTip("Permanently remove the selected deleted-history record. This cannot be undone.");
    deletedHistoryActions->addWidget(restoreHistoryButton);
    deletedHistoryActions->addWidget(purgeHistoryButton);
    deletedHistoryActions->addStretch();
    deletedHistoryLayout->addLayout(deletedHistoryActions);
    deletedHistoryTable = makeTable(deletedHistoryPage,
        {"Book ID", "Title", "Borrower", "Issue date", "Due date", "Returned date"});
    deletedHistoryLayout->addWidget(deletedHistoryTable, 1);
    tabs->addTab(deletedHistoryPage, "Deleted History");

    // Overdue and FIFO waitlist page
    auto *queuePage = new QWidget(tabs);
    auto *queueLayout = new QVBoxLayout(queuePage);
    auto *queueActions = new QHBoxLayout();
    auto *cancelWaiterButton = new QPushButton("Remove Selected Waiter", queuePage);
    queueActions->addWidget(cancelWaiterButton);
    queueActions->addStretch();
    queueLayout->addLayout(queueActions);
    auto *splitter = new QSplitter(Qt::Vertical, queuePage);
    auto *overdueGroup = new QWidget();
    auto *overdueLayout = new QVBoxLayout(overdueGroup);
    auto *overdueHeading = new QLabel("Overdue loans", overdueGroup);
    overdueHeading->setObjectName("dangerHeading");
    overdueTable = makeTable(overdueGroup,
        {"Book ID", "Title", "Borrower", "Issue date", "Due date", "Days overdue"});
    overdueLayout->addWidget(overdueHeading);
    overdueLayout->addWidget(overdueTable, 1);

    auto *waitGroup = new QWidget();
    auto *waitLayout = new QVBoxLayout(waitGroup);
    auto *waitHeading = new QLabel("FIFO waitlist — top row is oldest", waitGroup);
    waitHeading->setObjectName("accentHeading");
    waitlistTable = makeTable(waitGroup,
        {"Queue position", "Book ID", "Title", "Borrower"});
    waitLayout->addWidget(waitHeading);
    waitLayout->addWidget(waitlistTable, 1);

    splitter->addWidget(overdueGroup);
    splitter->addWidget(waitGroup);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    queueLayout->addWidget(splitter);
    tabs->addTab(queuePage, "Overdue & Waitlist");
    tabs->setCurrentIndex(0); // Open directly on Dashboard; there is no Start page.

    connect(searchEdit, &QLineEdit::textChanged,
            this, &MainWindow::applyBookFilter);
    connect(searchButton, &QPushButton::clicked,
            this, &MainWindow::searchBook);
    connect(addButton, &QPushButton::clicked,
            this, &MainWindow::addBook);
    connect(editButton, &QPushButton::clicked,
            this, &MainWindow::editBook);
    connect(deleteButton, &QPushButton::clicked,
            this, &MainWindow::deleteBook);
    connect(issueButton, &QPushButton::clicked,
            this, &MainWindow::issueBook);
    connect(returnButton, &QPushButton::clicked,
            this, &MainWindow::returnSelectedLoan);
    connect(cancelWaiterButton, &QPushButton::clicked,
            this, &MainWindow::cancelSelectedWaiter);
    connect(deleteHistoryButton, &QPushButton::clicked,
            this, &MainWindow::deleteSelectedHistory);
    connect(restoreHistoryButton, &QPushButton::clicked,
            this, &MainWindow::restoreSelectedHistory);
    connect(purgeHistoryButton, &QPushButton::clicked,
            this, &MainWindow::permanentlyDeleteSelectedHistory);

    loadData();
    refreshAllViews();
    applyTheme(darkMode);
}

MainWindow::~MainWindow()
{
    // Best-effort final save; normal operations also save after each change.
    saveAll();

    while (loanHead)
    {
        LoanNode *next = loanHead->next;
        delete loanHead;
        loanHead = next;
    }
    while (historyHead)
    {
        HistoryNode *next = historyHead->next;
        delete historyHead;
        historyHead = next;
    }
    while (deletedHistoryHead)
    {
        HistoryNode *next = deletedHistoryHead->next;
        delete deletedHistoryHead;
        deletedHistoryHead = next;
    }
    while (waitHead)
    {
        WaitNode *next = waitHead->next;
        delete waitHead;
        waitHead = next;
    }
    historyTail = nullptr;
    deletedHistoryTail = nullptr;
    waitTail = nullptr;
}

void MainWindow::applyTheme(bool dark)
{
    const QString lightStyle = R"STYLE(
        QWidget { background-color: #f3f6fb; color: #1d2939; font-size: 13px; }
        QMainWindow, QDialog, QMessageBox, QInputDialog { background-color: #f3f6fb; color: #1d2939; }
        QLabel { color: #1d2939; background: transparent; }
        QLabel#pageHeading { color: #172b4d; font-size: 25px; font-weight: 700; }
        QLabel#subHeading { color: #667085; font-size: 13px; }
        QLabel#sectionHeading { color: #172b4d; font-size: 20px; font-weight: 700; }
        QLabel#bodyText { color: #475467; font-size: 14px; padding: 4px; }
        QLabel#mutedText, QLabel#statCaption { color: #667085; }
        QLabel#statCaption { font-size: 12px; }
        QLabel#statValue { color: #15803d; font-size: 24px; font-weight: 700; }
        QLabel#dangerHeading { color: #b42318; font-size: 16px; font-weight: 700; }
        QLabel#accentHeading { color: #15803d; font-size: 16px; font-weight: 700; }
        QFrame#statCard { background-color: #ffffff; border: 1px solid #e4e7ec; border-radius: 8px; }
        QTabWidget::pane { border: 1px solid #d9e2ef; border-radius: 8px; background: #ffffff; }
        QTabBar::tab { background: #e7edf6; color: #344054; padding: 10px 18px; margin-right: 3px; border-top-left-radius: 6px; border-top-right-radius: 6px; }
        QTabBar::tab:selected { background: #ffffff; color: #15803d; font-weight: 700; }
        QPushButton { background: #15803d; color: #ffffff; border: none; border-radius: 6px; padding: 9px 14px; font-weight: 600; }
        QPushButton:hover { background: #166534; }
        QPushButton:disabled { background: #b9c4d4; color: #f5f5f5; }
        QPushButton#dangerButton { background: #b42318; color: #ffffff; }
        QPushButton#dangerButton:hover { background: #912018; }
        QPushButton#themeButton { background: #344054; color: #ffffff; }
        QPushButton#themeButton:hover { background: #1d2939; }
        QLineEdit, QSpinBox, QComboBox, QDateEdit, QTextEdit { color: #1d2939; background: #ffffff; border: 1px solid #cbd5e1; border-radius: 5px; padding: 7px; selection-background-color: #15803d; selection-color: #ffffff; }
        QTableWidget { color: #1d2939; background: #ffffff; alternate-background-color: #f8fafc; gridline-color: #e4e7ec; border: 1px solid #e4e7ec; selection-background-color: #dcfce7; selection-color: #172b4d; }
        QTableWidget::item { color: #1d2939; padding: 6px; }
        QTableWidget::item:selected { color: #172b4d; background: #dcfce7; }
        QHeaderView::section { background: #eaf0f8; color: #344054; padding: 8px; border: none; border-bottom: 1px solid #d0d5dd; font-weight: 700; }
        QScrollBar:vertical { background: #eef2f7; width: 13px; margin: 0; }
        QScrollBar::handle:vertical { background: #c1cad7; min-height: 24px; border-radius: 5px; }
        QScrollBar:horizontal { background: #eef2f7; height: 13px; margin: 0; }
        QScrollBar::handle:horizontal { background: #c1cad7; min-width: 24px; border-radius: 5px; }
    )STYLE";

    const QString darkStyle = R"STYLE(
        QWidget { background-color: #111827; color: #e5e7eb; font-size: 13px; }
        QMainWindow, QDialog, QMessageBox, QInputDialog { background-color: #111827; color: #e5e7eb; }
        QLabel { color: #e5e7eb; background: transparent; }
        QLabel#pageHeading { color: #f9fafb; font-size: 25px; font-weight: 700; }
        QLabel#subHeading { color: #9ca3af; font-size: 13px; }
        QLabel#sectionHeading { color: #f9fafb; font-size: 20px; font-weight: 700; }
        QLabel#bodyText { color: #d1d5db; font-size: 14px; padding: 4px; }
        QLabel#mutedText, QLabel#statCaption { color: #9ca3af; }
        QLabel#statCaption { font-size: 12px; }
        QLabel#statValue { color: #4ade80; font-size: 24px; font-weight: 700; }
        QLabel#dangerHeading { color: #fca5a5; font-size: 16px; font-weight: 700; }
        QLabel#accentHeading { color: #86efac; font-size: 16px; font-weight: 700; }
        QFrame#statCard { background-color: #1f2937; border: 1px solid #374151; border-radius: 8px; }
        QTabWidget::pane { border: 1px solid #374151; border-radius: 8px; background: #1f2937; }
        QTabBar::tab { background: #1f2937; color: #d1d5db; padding: 10px 18px; margin-right: 3px; border-top-left-radius: 6px; border-top-right-radius: 6px; }
        QTabBar::tab:selected { background: #374151; color: #86efac; font-weight: 700; }
        QPushButton { background: #16a34a; color: #ffffff; border: 1px solid #22c55e; border-radius: 6px; padding: 9px 14px; font-weight: 600; }
        QPushButton:hover { background: #15803d; }
        QPushButton:disabled { background: #374151; color: #9ca3af; }
        QPushButton#dangerButton { background: #b91c1c; color: #ffffff; border-color: #ef4444; }
        QPushButton#dangerButton:hover { background: #991b1b; }
        QPushButton#themeButton { background: #374151; color: #f9fafb; border-color: #4b5563; }
        QPushButton#themeButton:hover { background: #4b5563; }
        QLineEdit, QSpinBox, QComboBox, QDateEdit, QTextEdit { color: #f9fafb; background: #111827; border: 1px solid #4b5563; border-radius: 5px; padding: 7px; selection-background-color: #16a34a; selection-color: #ffffff; }
        QTableWidget { color: #f3f4f6; background: #111827; alternate-background-color: #1f2937; gridline-color: #374151; border: 1px solid #374151; selection-background-color: #15803d; selection-color: #ffffff; }
        QTableWidget::item { color: #f3f4f6; padding: 6px; }
        QTableWidget::item:selected { color: #ffffff; background: #15803d; }
        QHeaderView::section { background: #273449; color: #f3f4f6; padding: 8px; border: none; border-bottom: 1px solid #4b5563; font-weight: 700; }
        QScrollBar:vertical { background: #1f2937; width: 13px; margin: 0; }
        QScrollBar::handle:vertical { background: #4b5563; min-height: 24px; border-radius: 5px; }
        QScrollBar:horizontal { background: #1f2937; height: 13px; margin: 0; }
        QScrollBar::handle:horizontal { background: #4b5563; min-width: 24px; border-radius: 5px; }
    )STYLE";

    qApp->setStyleSheet(dark ? darkStyle : lightStyle);
    if (themeButton)
        themeButton->setText(dark ? "Switch to Light Mode" : "Switch to Dark Mode");
}

QString MainWindow::storageDirectory() const
{
    return QDir::homePath() + "/LibraryManagementSystemData";
}

void MainWindow::loadData()
{
    // Read books first. Availability is reconstructed from active loan records below.
    QFile booksFile(QDir(storageDirectory()).filePath("books.dat"));
    if (booksFile.open(QIODevice::ReadOnly))
    {
        QDataStream in(&booksFile);
        quint32 count = 0;
        in >> count;
        if (in.status() == QDataStream::Ok && count <= kMaximumRecords)
        {
            for (quint32 i = 0; i < count; ++i)
            {
                Book book;
                int savedAvailable = 0;
                in >> book.id >> book.title >> book.author
                   >> book.quantity >> savedAvailable;
                if (in.status() != QDataStream::Ok)
                    break;
                Q_UNUSED(savedAvailable);

                if (book.id.trimmed().isEmpty() || book.title.trimmed().isEmpty()
                    || book.author.trimmed().isEmpty()
                    || book.quantity < 1 || book.quantity > 10000)
                    continue;

                book.available = book.quantity;
                bookList.addBook(book); // Duplicate IDs in a damaged file are skipped.
            }
        }
    }

    // v2 filename distinguishes the new loan schema (which includes issue date).
    QFile loansFile(QDir(storageDirectory()).filePath("loans_v2.dat"));
    if (loansFile.open(QIODevice::ReadOnly))
    {
        QDataStream in(&loansFile);
        quint32 count = 0;
        in >> count;
        if (in.status() == QDataStream::Ok && count <= kMaximumRecords)
        {
            LoanNode *tail = nullptr;
            for (quint32 i = 0; i < count; ++i)
            {
                QString id, borrower;
                QDate issued, due;
                in >> id >> borrower >> issued >> due;
                if (in.status() != QDataStream::Ok)
                    break;

                BookNode *bookNode = bookList.findById(id);
                if (!bookNode || borrower.trimmed().isEmpty()
                    || !issued.isValid() || !due.isValid()
                    || bookNode->data.available <= 0)
                    continue;

                auto *loan = new LoanNode(id, borrower, issued, due);
                if (!loanHead)
                    loanHead = loan;
                else
                    tail->next = loan;
                tail = loan;
                --bookNode->data.available;
            }
        }
    }

    QFile historyFile(QDir(storageDirectory()).filePath("return_history.dat"));
    if (historyFile.open(QIODevice::ReadOnly))
    {
        QDataStream in(&historyFile);
        quint32 count = 0;
        in >> count;
        if (in.status() == QDataStream::Ok && count <= kMaximumRecords)
        {
            for (quint32 i = 0; i < count; ++i)
            {
                QString id, title, borrower;
                QDate issued, due, returned;
                in >> id >> title >> borrower >> issued >> due >> returned;
                if (in.status() != QDataStream::Ok)
                    break;
                if (id.trimmed().isEmpty() || borrower.trimmed().isEmpty()
                    || !issued.isValid() || !due.isValid() || !returned.isValid())
                    continue;
                appendHistory(id, title, borrower, issued, due, returned);
            }
        }
    }

    // Load records in the recycle bin. This is a separate file so moving a
    // record out of Return History never destroys it; records survive restarts.
    QFile deletedHistoryFile(QDir(storageDirectory()).filePath("deleted_return_history.dat"));
    if (deletedHistoryFile.open(QIODevice::ReadOnly))
    {
        QDataStream in(&deletedHistoryFile);
        quint32 count = 0;
        in >> count;
        if (in.status() == QDataStream::Ok && count <= kMaximumRecords)
        {
            for (quint32 i = 0; i < count; ++i)
            {
                QString id, title, borrower;
                QDate issued, due, returned;
                in >> id >> title >> borrower >> issued >> due >> returned;
                if (in.status() != QDataStream::Ok)
                    break;
                if (id.trimmed().isEmpty() || borrower.trimmed().isEmpty()
                    || !issued.isValid() || !due.isValid() || !returned.isValid())
                    continue;
                appendDeletedHistory(id, title, borrower, issued, due, returned);
            }
        }
    }

    // v2 filename ensures FIFO records use the expected current format.
    QFile waitFile(QDir(storageDirectory()).filePath("waitlist_v2.dat"));
    if (waitFile.open(QIODevice::ReadOnly))
    {
        QDataStream in(&waitFile);
        quint32 count = 0;
        in >> count;
        if (in.status() == QDataStream::Ok && count <= kMaximumRecords)
        {
            for (quint32 i = 0; i < count; ++i)
            {
                QString id, borrower;
                in >> id >> borrower;
                if (in.status() != QDataStream::Ok)
                    break;
                if (bookList.findById(id) && !borrower.trimmed().isEmpty())
                    enqueueWaiter(id, borrower);
            }
        }
    }

    // If persisted stock and queue state differ, older queued borrowers get priority.
    for (BookNode *node = bookList.headNode(); node; node = node->next)
        assignAvailableCopiesToWaiters(node);
}

bool MainWindow::saveBooks()
{
    if (!QDir().mkpath(storageDirectory()))
        return false;
    QSaveFile file(QDir(storageDirectory()).filePath("books.dat"));
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream out(&file);
    out << quint32(bookList.size());
    for (BookNode *node = bookList.headNode(); node; node = node->next)
    {
        const Book &book = node->data;
        out << book.id << book.title << book.author
            << book.quantity << book.available;
    }
    return finishWrite(file, out);
}

bool MainWindow::saveLoans()
{
    if (!QDir().mkpath(storageDirectory()))
        return false;
    QSaveFile file(QDir(storageDirectory()).filePath("loans_v2.dat"));
    if (!file.open(QIODevice::WriteOnly))
        return false;

    quint32 count = 0;
    for (LoanNode *node = loanHead; node; node = node->next)
        ++count;

    QDataStream out(&file);
    out << count;
    for (LoanNode *node = loanHead; node; node = node->next)
        out << node->bookId << node->borrower << node->issueDate << node->dueDate;
    return finishWrite(file, out);
}

bool MainWindow::saveHistory()
{
    if (!QDir().mkpath(storageDirectory()))
        return false;
    QSaveFile file(QDir(storageDirectory()).filePath("return_history.dat"));
    if (!file.open(QIODevice::WriteOnly))
        return false;

    quint32 count = 0;
    for (HistoryNode *node = historyHead; node; node = node->next)
        ++count;

    QDataStream out(&file);
    out << count;
    for (HistoryNode *node = historyHead; node; node = node->next)
        out << node->bookId << node->bookTitle << node->borrower
            << node->issueDate << node->dueDate << node->returnedDate;
    return finishWrite(file, out);
}

bool MainWindow::saveDeletedHistory()
{
    if (!QDir().mkpath(storageDirectory()))
        return false;
    QSaveFile file(QDir(storageDirectory()).filePath("deleted_return_history.dat"));
    if (!file.open(QIODevice::WriteOnly))
        return false;

    quint32 count = 0;
    for (HistoryNode *node = deletedHistoryHead; node; node = node->next)
        ++count;

    QDataStream out(&file);
    out << count;
    for (HistoryNode *node = deletedHistoryHead; node; node = node->next)
        out << node->bookId << node->bookTitle << node->borrower
            << node->issueDate << node->dueDate << node->returnedDate;
    return finishWrite(file, out);
}

bool MainWindow::saveWaitlist()
{
    if (!QDir().mkpath(storageDirectory()))
        return false;
    QSaveFile file(QDir(storageDirectory()).filePath("waitlist_v2.dat"));
    if (!file.open(QIODevice::WriteOnly))
        return false;

    quint32 count = 0;
    for (WaitNode *node = waitHead; node; node = node->next)
        ++count;

    QDataStream out(&file);
    out << count;
    for (WaitNode *node = waitHead; node; node = node->next)
        out << node->bookId << node->borrower;
    return finishWrite(file, out);
}

bool MainWindow::saveAll()
{
    // Evaluate every save call; don't let one failure skip the later files.
    const bool booksOk = saveBooks();
    const bool loansOk = saveLoans();
    const bool historyOk = saveHistory();
    const bool deletedHistoryOk = saveDeletedHistory();
    const bool waitlistOk = saveWaitlist();
    return booksOk && loansOk && historyOk && deletedHistoryOk && waitlistOk;
}

void MainWindow::refreshAllViews()
{
    refreshBookTable();
    refreshLoanTable();
    refreshHistoryTable();
    refreshDeletedHistoryTable();
    refreshOverdueTable();
    refreshWaitlistTable();
    refreshDashboard();
}

void MainWindow::refreshBookTable()
{
    bookTable->setRowCount(0);
    for (BookNode *node = bookList.headNode(); node; node = node->next)
    {
        const Book &book = node->data;
        const int row = bookTable->rowCount();
        bookTable->insertRow(row);
        setCell(bookTable, row, 0, book.id);
        setCell(bookTable, row, 1, book.title);
        setCell(bookTable, row, 2, book.author);
        setCell(bookTable, row, 3, QString::number(book.quantity));
        setCell(bookTable, row, 4, QString::number(book.available));
    }
    applyBookFilter(searchEdit ? searchEdit->text() : QString());
}

void MainWindow::refreshLoanTable()
{
    loansTable->setRowCount(0);
    const QDate today = QDate::currentDate();
    for (LoanNode *loan = loanHead; loan; loan = loan->next)
    {
        BookNode *bookNode = bookList.findById(loan->bookId);
        const QString title = bookNode ? bookNode->data.title : QString("(deleted book)");
        const QString status = loan->dueDate < today ? "OVERDUE"
            : (loan->dueDate == today ? "DUE TODAY" : "Active");
        const int row = loansTable->rowCount();
        loansTable->insertRow(row);
        setCell(loansTable, row, 0, loan->bookId);
        setCell(loansTable, row, 1, title);
        setCell(loansTable, row, 2, loan->borrower);
        setCell(loansTable, row, 3, loan->issueDate.toString(Qt::ISODate));
        setCell(loansTable, row, 4, loan->dueDate.toString(Qt::ISODate));
        setCell(loansTable, row, 5, status);
    }
}

void MainWindow::refreshHistoryTable()
{
    historyTable->setRowCount(0);
    for (HistoryNode *history = historyHead; history; history = history->next)
    {
        const int row = historyTable->rowCount();
        historyTable->insertRow(row);
        setCell(historyTable, row, 0, history->bookId);
        setCell(historyTable, row, 1, history->bookTitle);
        setCell(historyTable, row, 2, history->borrower);
        setCell(historyTable, row, 3, history->issueDate.toString(Qt::ISODate));
        setCell(historyTable, row, 4, history->dueDate.toString(Qt::ISODate));
        setCell(historyTable, row, 5, history->returnedDate.toString(Qt::ISODate));
    }
}

void MainWindow::refreshDeletedHistoryTable()
{
    deletedHistoryTable->setRowCount(0);
    for (HistoryNode *history = deletedHistoryHead;
         history; history = history->next)
    {
        const int row = deletedHistoryTable->rowCount();
        deletedHistoryTable->insertRow(row);
        setCell(deletedHistoryTable, row, 0, history->bookId);
        setCell(deletedHistoryTable, row, 1, history->bookTitle);
        setCell(deletedHistoryTable, row, 2, history->borrower);
        setCell(deletedHistoryTable, row, 3, history->issueDate.toString(Qt::ISODate));
        setCell(deletedHistoryTable, row, 4, history->dueDate.toString(Qt::ISODate));
        setCell(deletedHistoryTable, row, 5, history->returnedDate.toString(Qt::ISODate));
    }
}

void MainWindow::refreshOverdueTable()
{
    overdueTable->setRowCount(0);
    const QDate today = QDate::currentDate();
    for (LoanNode *loan = loanHead; loan; loan = loan->next)
    {
        if (loan->dueDate >= today)
            continue;
        BookNode *bookNode = bookList.findById(loan->bookId);
        const QString title = bookNode ? bookNode->data.title : QString("(deleted book)");
        const int row = overdueTable->rowCount();
        overdueTable->insertRow(row);
        setCell(overdueTable, row, 0, loan->bookId);
        setCell(overdueTable, row, 1, title);
        setCell(overdueTable, row, 2, loan->borrower);
        setCell(overdueTable, row, 3, loan->issueDate.toString(Qt::ISODate));
        setCell(overdueTable, row, 4, loan->dueDate.toString(Qt::ISODate));
        setCell(overdueTable, row, 5,
                QString::number(loan->dueDate.daysTo(today)));
    }
}

void MainWindow::refreshWaitlistTable()
{
    waitlistTable->setRowCount(0);
    int position = 1;
    for (WaitNode *waiter = waitHead; waiter; waiter = waiter->next)
    {
        BookNode *bookNode = bookList.findById(waiter->bookId);
        const QString title = bookNode ? bookNode->data.title : QString("(unknown book)");
        const int row = waitlistTable->rowCount();
        waitlistTable->insertRow(row);
        setCell(waitlistTable, row, 0, QString::number(position++));
        setCell(waitlistTable, row, 1, waiter->bookId);
        setCell(waitlistTable, row, 2, title);
        setCell(waitlistTable, row, 3, waiter->borrower);
    }
}

void MainWindow::refreshDashboard()
{
    int totalAvailable = 0;
    for (BookNode *node = bookList.headNode(); node; node = node->next)
        totalAvailable += node->data.available;

    int loans = 0;
    int overdue = 0;
    const QDate today = QDate::currentDate();
    for (LoanNode *node = loanHead; node; node = node->next)
    {
        ++loans;
        if (node->dueDate < today)
            ++overdue;
    }

    int waiting = 0;
    for (WaitNode *node = waitHead; node; node = node->next)
        ++waiting;

    titlesStat->setText(QString::number(bookList.size()));
    copiesStat->setText(QString::number(totalAvailable));
    loansStat->setText(QString::number(loans));
    overdueStat->setText(QString::number(overdue));
    waitlistStat->setText(QString::number(waiting));
}

void MainWindow::applyBookFilter(const QString &filter)
{
    const QString term = filter.trimmed();
    for (int row = 0; row < bookTable->rowCount(); ++row)
    {
        bool match = term.isEmpty();
        for (int column = 0; column < 3 && !match; ++column)
        {
            QTableWidgetItem *item = bookTable->item(row, column);
            match = item && item->text().contains(term, Qt::CaseInsensitive);
        }
        bookTable->setRowHidden(row, !match);
    }
}

void MainWindow::appendHistory(const QString &id, const QString &title,
                               const QString &borrower, const QDate &issued,
                               const QDate &due, const QDate &returned)
{
    auto *node = new HistoryNode(id, title, borrower, issued, due, returned);
    if (!historyHead)
        historyHead = historyTail = node;
    else
    {
        historyTail->next = node;
        historyTail = node;
    }
}

void MainWindow::appendDeletedHistory(const QString &id, const QString &title,
                                      const QString &borrower, const QDate &issued,
                                      const QDate &due, const QDate &returned)
{
    auto *node = new HistoryNode(id, title, borrower, issued, due, returned);
    if (!deletedHistoryHead)
        deletedHistoryHead = deletedHistoryTail = node;
    else
    {
        deletedHistoryTail->next = node;
        deletedHistoryTail = node;
    }
}

void MainWindow::enqueueWaiter(const QString &id, const QString &borrower)
{
    auto *node = new WaitNode(id, borrower);
    if (!waitTail)
        waitHead = waitTail = node;
    else
    {
        waitTail->next = node;
        waitTail = node;
    }
}

bool MainWindow::removeFirstWaiterForBook(const QString &id, QString &borrower)
{
    WaitNode *previous = nullptr;
    WaitNode *current = waitHead;
    while (current)
    {
        if (current->bookId == id)
        {
            borrower = current->borrower;
            if (previous)
                previous->next = current->next;
            else
                waitHead = current->next;
            if (waitTail == current)
                waitTail = previous;
            delete current;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

void MainWindow::assignAvailableCopiesToWaiters(BookNode *bookNode)
{
    // Older waiters for this title receive newly available copies first.
    while (bookNode && bookNode->data.available > 0)
    {
        QString borrower;
        if (!removeFirstWaiterForBook(bookNode->data.id, borrower))
            break;

        auto *loan = new LoanNode(
            bookNode->data.id, borrower, QDate::currentDate(),
            QDate::currentDate().addDays(14));
        loan->next = loanHead;
        loanHead = loan;
        --bookNode->data.available;
    }
}

int MainWindow::activeLoanCountForBook(const QString &id) const
{
    int count = 0;
    for (LoanNode *node = loanHead; node; node = node->next)
        if (node->bookId == id)
            ++count;
    return count;
}

int MainWindow::waitingCountForBook(const QString &id) const
{
    int count = 0;
    for (WaitNode *node = waitHead; node; node = node->next)
        if (node->bookId == id)
            ++count;
    return count;
}

bool MainWindow::borrowerHasActiveLoan(const QString &id, const QString &borrower) const
{
    for (LoanNode *node = loanHead; node; node = node->next)
    {
        if (node->bookId == id && node->borrower.compare(
                borrower, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

bool MainWindow::borrowerIsWaiting(const QString &id, const QString &borrower) const
{
    for (WaitNode *node = waitHead; node; node = node->next)
    {
        if (node->bookId == id && node->borrower.compare(
                borrower, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

QString MainWindow::selectedBookId() const
{
    const int row = bookTable ? bookTable->currentRow() : -1;
    if (row < 0 || !bookTable->item(row, 0))
        return QString();
    return bookTable->item(row, 0)->text();
}

void MainWindow::addBook()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add Book");
    QLineEdit idInput, titleInput, authorInput;
    QSpinBox quantityInput;
    quantityInput.setRange(1, 10000);
    quantityInput.setValue(1);

    QFormLayout form(&dialog);
    form.addRow("Book ID:", &idInput);
    form.addRow("Title:", &titleInput);
    form.addRow("Author:", &authorInput);
    form.addRow("Total copies:", &quantityInput);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    Book book;
    book.id = idInput.text().trimmed();
    book.title = titleInput.text().trimmed();
    book.author = authorInput.text().trimmed();
    book.quantity = quantityInput.value();
    book.available = book.quantity;

    if (book.id.isEmpty() || book.title.isEmpty() || book.author.isEmpty())
    {
        QMessageBox::warning(this, "Invalid Input", "Enter the ID, title and author.");
        return;
    }
    if (!bookList.addBook(book))
    {
        QMessageBox::warning(this, "Duplicate ID", "That book ID already exists.");
        return;
    }

    refreshAllViews();
    if (!saveAll())
        QMessageBox::warning(this, "Save Error", "The book was added, but one or more data files could not be saved.");
}

void MainWindow::editBook()
{
    const QString id = selectedBookId();
    if (id.isEmpty())
    {
        QMessageBox::information(this, "Select a book", "Select a row in Book Inventory first.");
        return;
    }

    BookNode *node = bookList.findById(id);
    if (!node)
        return;

    const int checkedOut = activeLoanCountForBook(id);
    QDialog dialog(this);
    dialog.setWindowTitle("Edit Book");
    QLineEdit idInput, titleInput, authorInput;
    idInput.setText(node->data.id);
    idInput.setReadOnly(true); // IDs are immutable so the custom hash index remains valid.
    titleInput.setText(node->data.title);
    authorInput.setText(node->data.author);

    QSpinBox quantityInput;
    quantityInput.setRange(checkedOut > 1 ? checkedOut : 1, 10000);
    quantityInput.setValue(node->data.quantity);

    QFormLayout form(&dialog);
    form.addRow("Book ID (cannot change):", &idInput);
    form.addRow("Title:", &titleInput);
    form.addRow("Author:", &authorInput);
    form.addRow("Total copies:", &quantityInput);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString title = titleInput.text().trimmed();
    const QString author = authorInput.text().trimmed();
    if (title.isEmpty() || author.isEmpty())
    {
        QMessageBox::warning(this, "Invalid Input", "Title and author cannot be empty.");
        return;
    }

    node->data.title = title;
    node->data.author = author;
    node->data.quantity = quantityInput.value();
    node->data.available = node->data.quantity - checkedOut;
    assignAvailableCopiesToWaiters(node);

    refreshAllViews();
    if (!saveAll())
        QMessageBox::warning(this, "Save Error", "The edit was applied, but one or more data files could not be saved.");
}

void MainWindow::deleteBook()
{
    const QString id = selectedBookId();
    if (id.isEmpty())
    {
        QMessageBox::information(this, "Select a book", "Select a row in Book Inventory first.");
        return;
    }

    if (activeLoanCountForBook(id) > 0)
    {
        QMessageBox::warning(this, "Cannot Delete", "This book has active loans. Return all copies before deleting it.");
        return;
    }
    if (waitingCountForBook(id) > 0)
    {
        QMessageBox::warning(this, "Cannot Delete", "This book has people on its waitlist. Clear the waitlist before deleting it.");
        return;
    }

    const auto answer = QMessageBox::question(
        this, "Confirm Delete", "Delete book " + id + "?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    if (!bookList.removeBook(id))
    {
        QMessageBox::warning(this, "Delete Failed", "The selected book could not be found.");
        return;
    }

    refreshAllViews();
    if (!saveAll())
        QMessageBox::warning(this, "Save Error", "The book was deleted, but one or more data files could not be saved.");
}

void MainWindow::searchBook()
{
    const QString id = searchEdit->text().trimmed();
    if (id.isEmpty())
    {
        QMessageBox::information(this, "Find Book", "Enter a book ID or use the filter field to search titles and authors.");
        return;
    }

    BookNode *node = bookList.findById(id);
    if (!node)
    {
        QMessageBox::information(this, "Not Found", "No book has ID: " + id);
        return;
    }

    tabs->setCurrentIndex(1);
    applyBookFilter(id);
    for (int row = 0; row < bookTable->rowCount(); ++row)
    {
        if (bookTable->item(row, 0) && bookTable->item(row, 0)->text() == id)
        {
            bookTable->setCurrentCell(row, 0);
            bookTable->scrollToItem(bookTable->item(row, 0));
            break;
        }
    }
    QMessageBox::information(this, "Book Found",
        QString("ID: %1\nTitle: %2\nAuthor: %3\nTotal copies: %4\nAvailable: %5")
            .arg(node->data.id).arg(node->data.title).arg(node->data.author)
            .arg(node->data.quantity).arg(node->data.available));
}

void MainWindow::issueBook()
{
    bool ok = false;
    const QString id = QInputDialog::getText(
        this, "Issue Book", "Enter Book ID:", QLineEdit::Normal, QString(), &ok).trimmed();
    if (!ok || id.isEmpty())
        return;

    BookNode *node = bookList.findById(id);
    if (!node)
    {
        QMessageBox::warning(this, "Not Found", "That book ID does not exist.");
        return;
    }

    const QString borrower = QInputDialog::getText(
        this, "Issue Book", "Borrower's full name:", QLineEdit::Normal, QString(), &ok).trimmed();
    if (!ok || borrower.isEmpty())
        return;

    if (borrowerHasActiveLoan(id, borrower))
    {
        QMessageBox::warning(this, "Already Issued", "This borrower already has an active loan for this title.");
        return;
    }
    if (borrowerIsWaiting(id, borrower))
    {
        QMessageBox::warning(this, "Already Waiting", "This borrower is already on this book's waitlist.");
        return;
    }

    // Any earlier waiters retain priority over a new borrower.
    if (node->data.available <= 0 || waitingCountForBook(id) > 0)
    {
        enqueueWaiter(id, borrower);
        assignAvailableCopiesToWaiters(node);
        const bool issuedToBorrower = borrowerHasActiveLoan(id, borrower);
        refreshAllViews();
        if (!saveAll())
            QMessageBox::warning(this, "Save Error", "The issue/waitlist change happened, but saving failed.");

        if (issuedToBorrower)
            QMessageBox::information(this, "Book Issued", borrower + " received the copy after earlier waiters.");
        else
            QMessageBox::information(this, "Added to Waitlist", borrower + " was added to the FIFO waitlist for " + id + ".");
        return;
    }

    auto *loan = new LoanNode(id, borrower, QDate::currentDate(),
                              QDate::currentDate().addDays(14));
    loan->next = loanHead;
    loanHead = loan;
    --node->data.available;

    const QDate dueDate = loan->dueDate;
    refreshAllViews();
    if (!saveAll())
        QMessageBox::warning(this, "Save Error", "The book was issued, but one or more data files could not be saved.");
    QMessageBox::information(this, "Book Issued",
        "Issued to: " + borrower + "\nDue date: " + dueDate.toString(Qt::ISODate));
}

void MainWindow::returnSelectedLoan()
{
    const int row = loansTable->currentRow();
    if (row < 0 || !loansTable->item(row, 0) || !loansTable->item(row, 2))
    {
        QMessageBox::information(this, "Select a loan", "Select the borrower's loan in the Active Loans tab first.");
        return;
    }

    const QString id = loansTable->item(row, 0)->text();
    const QString borrower = loansTable->item(row, 2)->text();
    LoanNode *previous = nullptr;
    LoanNode *current = loanHead;
    while (current && !(current->bookId == id && current->borrower == borrower))
    {
        previous = current;
        current = current->next;
    }
    if (!current)
    {
        QMessageBox::warning(this, "Loan Not Found", "The selected active loan no longer exists.");
        refreshAllViews();
        return;
    }

    const QDate issueDate = current->issueDate;
    const QDate dueDate = current->dueDate;
    BookNode *bookNode = bookList.findById(id);
    const QString title = bookNode ? bookNode->data.title : QString("(deleted book)");

    const auto answer = QMessageBox::question(
        this, "Confirm Return",
        "Record the return of " + title + " from " + borrower + "?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (answer != QMessageBox::Yes)
        return;

    if (previous)
        previous->next = current->next;
    else
        loanHead = current->next;
    delete current;

    appendHistory(id, title, borrower, issueDate, dueDate, QDate::currentDate());

    if (bookNode)
    {
        ++bookNode->data.available;
        if (bookNode->data.available > bookNode->data.quantity)
            bookNode->data.available = bookNode->data.quantity;
        assignAvailableCopiesToWaiters(bookNode);
    }

    refreshAllViews();
    if (!saveAll())
        QMessageBox::warning(this, "Save Error", "The return was recorded, but one or more data files could not be saved.");

    QMessageBox::information(this, "Return Recorded",
        "Return recorded for " + borrower + ". Return history has been updated.\n"
        "Any available copy was allocated to the earliest eligible waitlist borrower.");
}


void MainWindow::cancelSelectedWaiter()
{
    const int row = waitlistTable->currentRow();
    if (row < 0 || !waitlistTable->item(row, 1) || !waitlistTable->item(row, 3))
    {
        QMessageBox::information(this, "Select a waiter",
            "Select a row in the FIFO waitlist first.");
        return;
    }

    const QString id = waitlistTable->item(row, 1)->text();
    const QString borrower = waitlistTable->item(row, 3)->text();
    WaitNode *previous = nullptr;
    WaitNode *current = waitHead;

    while (current && !(current->bookId == id && current->borrower == borrower))
    {
        previous = current;
        current = current->next;
    }

    if (!current)
    {
        QMessageBox::warning(this, "Waiter Not Found",
            "The selected waitlist entry no longer exists.");
        refreshAllViews();
        return;
    }

    const auto answer = QMessageBox::question(
        this, "Remove Waiter",
        "Remove " + borrower + " from the waitlist for " + id + "?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    if (previous)
        previous->next = current->next;
    else
        waitHead = current->next;

    if (waitTail == current)
        waitTail = previous;

    delete current;
    refreshAllViews();
    if (!saveAll())
        QMessageBox::warning(this, "Save Error",
            "The waitlist entry was removed, but saving failed.");
}

void MainWindow::deleteSelectedHistory()
{
    const int row = historyTable->currentRow();
    if (row < 0)
    {
        QMessageBox::information(this, "Select a record",
            "Select a return-history row before deleting it.");
        return;
    }

    const auto answer = QMessageBox::question(
        this, "Move to Deleted History",
        "Move the selected return-history record to Deleted History? You can restore it later.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    HistoryNode *previous = nullptr;
    HistoryNode *current = historyHead;
    for (int index = 0; current && index < row; ++index)
    {
        previous = current;
        current = current->next;
    }

    if (!current)
    {
        QMessageBox::warning(this, "Record Not Found",
            "The selected history record no longer exists.");
        refreshAllViews();
        return;
    }

    if (previous)
        previous->next = current->next;
    else
        historyHead = current->next;

    if (historyTail == current)
        historyTail = previous;

    current->next = nullptr;
    if (!deletedHistoryHead)
        deletedHistoryHead = deletedHistoryTail = current;
    else
    {
        deletedHistoryTail->next = current;
        deletedHistoryTail = current;
    }

    refreshAllViews();
    if (!saveAll())
        QMessageBox::warning(this, "Save Error",
            "The history record was moved, but not all data files could be saved.");
}

void MainWindow::restoreSelectedHistory()
{
    const int row = deletedHistoryTable->currentRow();
    if (row < 0)
    {
        QMessageBox::information(this, "Select a record",
            "Select a record in Deleted History to restore it.");
        return;
    }

    HistoryNode *previous = nullptr;
    HistoryNode *current = deletedHistoryHead;
    for (int index = 0; current && index < row; ++index)
    {
        previous = current;
        current = current->next;
    }

    if (!current)
    {
        QMessageBox::warning(this, "Record Not Found",
            "The selected deleted-history record no longer exists.");
        refreshAllViews();
        return;
    }

    if (previous)
        previous->next = current->next;
    else
        deletedHistoryHead = current->next;

    if (deletedHistoryTail == current)
        deletedHistoryTail = previous;

    current->next = nullptr;
    if (!historyHead)
        historyHead = historyTail = current;
    else
    {
        historyTail->next = current;
        historyTail = current;
    }

    refreshAllViews();
    if (!saveAll())
        QMessageBox::warning(this, "Save Error",
            "The record was restored, but not all data files could be saved.");
    else
        QMessageBox::information(this, "Restored",
            "The record was restored to Return History.");
}

void MainWindow::permanentlyDeleteSelectedHistory()
{
    const int row = deletedHistoryTable->currentRow();
    if (row < 0)
    {
        QMessageBox::information(this, "Select a record",
            "Select a record in Deleted History first.");
        return;
    }

    const auto answer = QMessageBox::warning(
        this, "Delete Permanently",
        "Permanently delete this record? This action cannot be undone.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    HistoryNode *previous = nullptr;
    HistoryNode *current = deletedHistoryHead;
    for (int index = 0; current && index < row; ++index)
    {
        previous = current;
        current = current->next;
    }

    if (!current)
    {
        QMessageBox::warning(this, "Record Not Found",
            "The selected record no longer exists.");
        refreshAllViews();
        return;
    }

    if (previous)
        previous->next = current->next;
    else
        deletedHistoryHead = current->next;

    if (deletedHistoryTail == current)
        deletedHistoryTail = previous;

    delete current;
    refreshAllViews();
    if (!saveAll())
        QMessageBox::warning(this, "Save Error",
            "The record was removed, but not all data files could be saved.");
}

