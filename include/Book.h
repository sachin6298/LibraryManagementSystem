#ifndef BOOK_H
#define BOOK_H

#include <QString>

struct Book
{
    QString id;
    QString title;
    QString author;
    int quantity = 0;
    int available = 0;
};

#endif // BOOK_H
