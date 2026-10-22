# Library Management System — Development Log

## Constraints from assignment and user
- C++17 and Qt 6 Widgets GUI.
- File-based storage (no database required).
- Required custom singly linked list and custom linked FIFO waitlist; do not replace the required structures with `std::list`/`std::queue`.
- Required issue/return tracking, overdue detection, edge-case tests, time-complexity analysis, 5–8 page report and 5–10 minute demo.
- Explain implementation so a student can present the project from scratch.

## Current design
- `Book` stores ID/title/author/quantity/available.
- `BookList` uses a custom singly linked list with a tail pointer and custom hash index for expected O(1) ID lookup.
- Loan, return history, deleted history and waitlist use custom linked nodes.
- Qt Widgets provides tabbed dashboard/inventory/active loans/history/deleted history/overdue-waitlist UI.
- Books/loans/history/deleted-history/waitlist persist under `~/LibraryManagementSystemData`.
- App icon and BIT logo are embedded from Qt resources. The first tab is a Start page showing the supplied BIT logo; clicking Start opens Dashboard.
- Linux and Windows are built separately using GitHub Actions on native OS runners.

## Icon/start-page update
- Created a light-brown book-stack transparent PNG from the requested icon visual.
- Generated a multiple-size Windows ICO.
- Applied a circular alpha mask to the outside of the supplied BIT seal; the seal itself still says “RANCHI.”
- Added Qt resource references for both images and a Start tab in `mainwindow.cpp`.
- Retained the green-accent light/dark theme and the existing history delete/restore UI.

## Verification history
- Previous application builds were successful on the user's Ubuntu machine before this kit was requested.
- The current icon/start-page kit archive and Bash scripts are checked in the packaging environment.
- The current coordinated Qt source has not been compiled in the packaging environment (Qt 6 dev packages are unavailable there). After applying, run the provided build and manual test plan; record actual pass/fail results before claiming release readiness.
