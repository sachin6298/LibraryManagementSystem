# Library Management System

A desktop-based Library Management System built using **C++ and Qt
Widgets**. It is designed to handle common library operations such as
managing books, issuing and returning books, checking due dates, and
maintaining a waiting list for books that are currently unavailable.

The application uses local file storage to keep library records between
sessions. It also includes custom implementations of a singly linked
list and a FIFO queue as part of its data-structure functionality.

## Features

-   **Book management** -- Add, edit, delete, and view book records.
-   **Book issuing** -- Issue available books and keep track of issue
    details.
-   **Book returns** -- Record returned books and update book
    availability.
-   **Issue and return history** -- Review recorded borrowing and return
    activity.
-   **Overdue detection** -- Identify issued books that have passed
    their due dates.
-   **Waiting list** -- Keep a FIFO waiting list for books that are not
    currently available.
-   **Search** -- Find books using the search options provided in the
    application.
-   **Local data storage** -- Store library records in files so they can
    be loaded again when the application is reopened.
-   **Desktop GUI** -- Use the application through a graphical interface
    built with Qt Widgets.

## Technologies Used

  Technology           Purpose
  -------------------- ---------------------------------
  C++                  Application logic
  Qt 6 Widgets         Graphical user interface
  CMake                Project configuration and build
  File handling        Local data persistence
  Singly linked list   Book record management
  FIFO queue           Waiting-list management

## Data Structures

### Singly Linked List

A custom singly linked list is used to manage book records. Each node
stores book information and a pointer to the next node. This provides
practice with dynamic memory allocation, node traversal, insertion,
deletion, and searching.

### FIFO Queue

A custom first-in, first-out (FIFO) queue is used for the waiting list.
Students are added at the rear and processed from the front, preserving
the order in which they joined the queue.

## Requirements

-   C++17-compatible compiler
-   Qt 6 with Qt Widgets
-   CMake 3.16 or later
-   Qt Creator (recommended)

The exact Qt kit and compiler must be compatible with the environment
used to build the project.

## Getting Started

### 1. Clone the repository

``` bash
git clone YOUR_REPOSITORY_URL
cd YOUR_PROJECT_FOLDER
```

Replace the placeholder values with the repository URL and folder name.

### 2. Open the project in Qt Creator

1.  Start Qt Creator.
2.  Select **File → Open File or Project**.
3.  Choose the project's `CMakeLists.txt`.
4.  Select a compatible Qt 6 kit.
5.  Allow Qt Creator to configure the project.

### 3. Build and run

Use Qt Creator's **Build** and **Run** controls.

Alternatively, build from a terminal in the project root:

``` bash
cmake -S . -B build
cmake --build build -j2
```

Run the executable generated in the build directory. Its exact location
depends on the operating system, generator, and build configuration.

## Using the Application

A typical workflow is:

1.  Add the books available in the library.
2.  Search for a book or view the available records.
3.  Edit or delete a record when necessary.
4.  Issue an available book to a student.
5.  Record the return when the book is brought back.
6.  Review issue and return history.
7.  Check overdue records.
8.  Add a student to the waiting list when the requested book is
    unavailable.

The exact screens and available actions depend on the version of the
application.

## Data Storage

The application uses local files rather than requiring a database
server. Records are saved locally and loaded when the application
starts.

Keep the application's data files in the locations expected by the
program. If the project is moved to another computer, copy the required
data files as well.

## Testing Checklist

The following cases are useful when checking the application:

-   Add a new book and verify that it appears in the book list.
-   Edit a book and confirm that the updated details are displayed.
-   Delete a book and verify that it is removed.
-   Issue an available book and check its availability status.
-   Try to issue a book that is already unavailable.
-   Return an issued book and verify that its status changes.
-   Check that issue and return history is recorded correctly.
-   Verify overdue detection using a due date in the past.
-   Add multiple students to a waiting list and confirm FIFO order.
-   Test empty lists, invalid input, and missing or unavailable data
    files.

## Future Improvements

Possible additions for later versions include:

-   Student and librarian login
-   Database support
-   Automatic fine calculation
-   Book categories and advanced filters
-   Report generation and export
-   Multi-user access over a network

## Project Purpose

This project provides practical experience with C++, object-oriented
programming, Qt-based desktop development, file handling, and
implementing data structures from scratch. It brings these concepts
together in a library application with common book-management workflows.
