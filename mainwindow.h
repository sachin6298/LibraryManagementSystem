#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include <QString>

#include "include/BookList.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QTableWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    struct LoanNode
    {
        QString bookId;
        QString borrower;
        QDate issueDate;
        QDate dueDate;
        LoanNode *next;

        LoanNode(const QString &id, const QString &name,
                 const QDate &issued, const QDate &due)
            : bookId(id), borrower(name), issueDate(issued),
              dueDate(due), next(nullptr)
        {
        }
    };

    struct HistoryNode
    {
        QString bookId;
        QString bookTitle;
        QString borrower;
        QDate issueDate;
        QDate dueDate;
        QDate returnedDate;
        HistoryNode *next;

        HistoryNode(const QString &id, const QString &title,
                    const QString &name, const QDate &issued,
                    const QDate &due, const QDate &returned)
            : bookId(id), bookTitle(title), borrower(name),
              issueDate(issued), dueDate(due),
              returnedDate(returned), next(nullptr)
        {
        }
    };

    struct WaitNode
    {
        QString bookId;
        QString borrower;
        WaitNode *next;

        WaitNode(const QString &id, const QString &name)
            : bookId(id), borrower(name), next(nullptr)
        {
        }
    };

    BookList bookList;
    QTabWidget *tabs = nullptr;
    QPushButton *themeButton = nullptr;
    bool darkMode = false;
    QLineEdit *searchEdit = nullptr;
    QTableWidget *bookTable = nullptr;
    QTableWidget *loansTable = nullptr;
    QTableWidget *historyTable = nullptr;
    QTableWidget *deletedHistoryTable = nullptr;
    QTableWidget *overdueTable = nullptr;
    QTableWidget *waitlistTable = nullptr;

    QLabel *titlesStat = nullptr;
    QLabel *copiesStat = nullptr;
    QLabel *loansStat = nullptr;
    QLabel *overdueStat = nullptr;
    QLabel *waitlistStat = nullptr;

    LoanNode *loanHead = nullptr;
    HistoryNode *historyHead = nullptr;
    HistoryNode *historyTail = nullptr;
    HistoryNode *deletedHistoryHead = nullptr;
    HistoryNode *deletedHistoryTail = nullptr;
    WaitNode *waitHead = nullptr;
    WaitNode *waitTail = nullptr;

    QString storageDirectory() const;
    void applyTheme(bool dark);
    void loadData();
    bool saveAll();
    bool saveBooks();
    bool saveLoans();
    bool saveHistory();
    bool saveDeletedHistory();
    bool saveWaitlist();

    void refreshAllViews();
    void refreshBookTable();
    void refreshLoanTable();
    void refreshHistoryTable();
    void refreshDeletedHistoryTable();
    void refreshOverdueTable();
    void refreshWaitlistTable();
    void refreshDashboard();
    void applyBookFilter(const QString &filter);
    void appendHistory(const QString &id, const QString &title,
                       const QString &borrower, const QDate &issued,
                       const QDate &due, const QDate &returned);
    void appendDeletedHistory(const QString &id, const QString &title,
                              const QString &borrower, const QDate &issued,
                              const QDate &due, const QDate &returned);
    void enqueueWaiter(const QString &id, const QString &borrower);
    void assignAvailableCopiesToWaiters(BookNode *bookNode);
    bool removeFirstWaiterForBook(const QString &id, QString &borrower);
    int activeLoanCountForBook(const QString &id) const;
    int waitingCountForBook(const QString &id) const;
    bool borrowerHasActiveLoan(const QString &id, const QString &borrower) const;
    bool borrowerIsWaiting(const QString &id, const QString &borrower) const;
    QString selectedBookId() const;

private slots:
    void addBook();
    void editBook();
    void deleteBook();
    void searchBook();
    void issueBook();
    void returnSelectedLoan();
    void cancelSelectedWaiter();
    void deleteSelectedHistory();
    void restoreSelectedHistory();
    void permanentlyDeleteSelectedHistory();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
};

#endif // MAINWINDOW_H
