#include "BookList.h"

BookList::BookList()
    : head(nullptr),
      tail(nullptr),
      buckets(new BookNode *[101]()),
      bucketCount(101),
      bookCount(0)
{
}

BookList::~BookList()
{
    BookNode *current = head;
    while (current)
    {
        BookNode *nextNode = current->next;
        delete current;
        current = nextNode;
    }
    delete[] buckets;
}

unsigned long long BookList::hashId(const QString &id, int count)
{
    // djb2-style string hash. Hash collisions are resolved by chaining.
    unsigned long long hash = 5381;
    for (QChar ch : id)
        hash = ((hash << 5) + hash) + ch.unicode();

    return hash % static_cast<unsigned long long>(count);
}

void BookList::rehash(int newBucketCount)
{
    BookNode **newBuckets = new BookNode *[newBucketCount]();

    // Traverse the list once and rebuild each node's hash-chain link.
    for (BookNode *node = head; node; node = node->next)
    {
        int index = static_cast<int>(hashId(node->data.id, newBucketCount));
        node->hashNext = newBuckets[index];
        newBuckets[index] = node;
    }

    delete[] buckets;
    buckets = newBuckets;
    bucketCount = newBucketCount;
}

bool BookList::addBook(const Book &book)
{
    if (book.id.trimmed().isEmpty() || findById(book.id))
        return false;

    // Keep the hash table's load factor at approximately 0.75 or less.
    if ((bookCount + 1) * 4 > bucketCount * 3)
        rehash(bucketCount * 2 + 1);

    BookNode *node = new BookNode(book);

    if (!head)
        head = tail = node;
    else
    {
        tail->next = node;
        tail = node;
    }

    int index = static_cast<int>(hashId(book.id, bucketCount));
    node->hashNext = buckets[index];
    buckets[index] = node;
    ++bookCount;
    return true;
}

bool BookList::removeBook(const QString &id)
{
    BookNode *target = findById(id);
    if (!target)
        return false;

    // Remove target from its hash bucket chain.
    int index = static_cast<int>(hashId(id, bucketCount));
    BookNode *hashPrevious = nullptr;
    BookNode *hashCurrent = buckets[index];

    while (hashCurrent && hashCurrent != target)
    {
        hashPrevious = hashCurrent;
        hashCurrent = hashCurrent->hashNext;
    }

    if (hashCurrent)
    {
        if (hashPrevious)
            hashPrevious->hashNext = hashCurrent->hashNext;
        else
            buckets[index] = hashCurrent->hashNext;
    }

    // A singly linked list needs a traversal to find the predecessor.
    BookNode *previous = nullptr;
    BookNode *current = head;
    while (current && current != target)
    {
        previous = current;
        current = current->next;
    }

    if (!current)
        return false;

    if (previous)
        previous->next = current->next;
    else
        head = current->next;

    if (tail == current)
        tail = previous;

    delete current;
    --bookCount;
    return true;
}

BookNode *BookList::findById(const QString &id) const
{
    if (bucketCount <= 0)
        return nullptr;

    int index = static_cast<int>(hashId(id, bucketCount));
    for (BookNode *node = buckets[index]; node; node = node->hashNext)
    {
        if (node->data.id == id)
            return node;
    }
    return nullptr;
}

BookNode *BookList::headNode() const
{
    return head;
}

int BookList::size() const
{
    return bookCount;
}
