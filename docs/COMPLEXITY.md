# Time Complexity Notes

Let B be the number of books, L active loans, H return-history records, D deleted-history records, and W waitlist nodes.

- `BookList::findById`: expected O(1) with the custom hash index; worst case O(B) with collisions.
- `BookList::addBook`: expected O(1) for duplicate detection and tail append; rehash is O(B), amortized across insertions. Worst case for an individual insertion can be O(B) due to rehash/collisions.
- `BookList::removeBook`: O(B) because a singly linked list must find the predecessor, even though the hash index locates the target node quickly.
- Waitlist enqueue: O(1) due to a tail pointer.
- Find/remove first eligible waiter for a particular book: O(W) in the current single global FIFO queue, because earlier entries for other book IDs are skipped. This preserves per-book arrival order.
- Active loan lookup/return: O(L) by linear traversal.
- Append to history/deleted-history lists: O(1) where tail pointers are maintained. Selecting a history row and finding the linked-list node takes O(H) or O(D).
- Table refresh and inventory filter: O(B); loan and overdue refresh: O(L); history refresh: O(H+D); waitlist refresh: O(W).
- Save/load all file records: O(B+L+H+D+W) time and proportional storage.

This is an appropriate trade-off for the teacher's requirement to use a custom singly linked list and custom linked FIFO queue. Expected hash lookup is not a worst-case O(1) guarantee.
