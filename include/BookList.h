#ifndef BOOKLIST_H
#define BOOKLIST_H

#include "Book.h"

// A node in the required custom singly linked book list.
// hashNext is a separate chain link used by the custom hash index.
struct BookNode
{
    Book data;
    BookNode *next;
    BookNode *hashNext;

    explicit BookNode(const Book &book)
        : data(book), next(nullptr), hashNext(nullptr)
    {
    }
};

class BookList
{
private:
    BookNode *head;
    BookNode *tail;
    BookNode **buckets;
    int bucketCount;
    int bookCount;

    static unsigned long long hashId(const QString &id, int bucketCount);
    void rehash(int newBucketCount);

public:
    BookList();
    ~BookList();

    BookList(const BookList &) = delete;
    BookList &operator=(const BookList &) = delete;

    bool addBook(const Book &book);
    bool removeBook(const QString &id);
    BookNode *findById(const QString &id) const;
    BookNode *headNode() const;
    int size() const;
};

#endif // BOOKLIST_H
