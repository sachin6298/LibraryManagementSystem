# Manual Test Plan

Mark each test only after checking it on the built application.

1. App starts with the `Start` tab selected and shows the supplied BIT logo.
2. Clicking `Start` switches to Dashboard.
3. Window and executable use the light-brown book-stack app icon where the platform supports it.
4. Toggle light/dark theme; all table text remains readable.
5. Add a valid book; it appears in inventory and is saved.
6. Add a duplicate book ID; the application refuses it.
7. Search existing/non-existing book IDs.
8. Edit title/author/quantity; availability remains consistent with loans.
9. Delete an unused book; it disappears and remains absent after restart.
10. Attempt to delete a book with active loan or waitlisted borrower; deletion is blocked.
11. Issue a book; borrower, issue date and due date appear in Active Loans.
12. Attempt duplicate active issue for same borrower/book; it is refused if supported by the current implementation.
13. Return a selected active loan; it disappears from Active Loans and appears in Return History.
14. Verify the returned date is recorded.
15. Delete a return-history record; it moves to Deleted History.
16. Restore a deleted record; it returns to Return History.
17. Permanently delete a deleted-history record after confirmation.
18. Issue a book when no copies are available; borrower joins waitlist.
19. Return a copy; earliest eligible waiter for that book receives it.
20. Remove a selected waiter; it no longer appears in the queue.
21. Active loan with due date before today appears as overdue and shows overdue days.
22. Close and reopen app; books, active loans, history, deleted history and waitlist persist.
23. Inspect `~/LibraryManagementSystemData` to ensure records are stored separately from sources.
24. Build the project from a clean CMake configure.
25. Test Linux bundle on target Linux versions and test Windows package on a Windows machine without Qt installed.
