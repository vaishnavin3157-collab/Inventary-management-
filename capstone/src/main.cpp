/*
 * main.cpp  –  C++ menu / UI layer
 * Handles all user interaction; delegates storage to InventoryManager (C backend).
 */

#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "InventoryManager.h"
#include "inventory.h"

/* forward declaration of the cleaner table printer in InventoryManager.cpp */
void printItemTable(const std::vector<Item> &items);

/* ========================================================= terminal helpers */

static void clearInput()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static void pause()
{
    std::cout << "\n  Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static void printBanner()
{
    std::cout
        << "\n"
        << "  ╔══════════════════════════════════════╗\n"
        << "  ║     HYBRID INVENTORY MANAGER  v1.0   ║\n"
        << "  ║     C backend  ·  C++ frontend        ║\n"
        << "  ╚══════════════════════════════════════╝\n";
}

static void printMenu()
{
    std::cout
        << "\n"
        << "  ┌────────────────────────┐\n"
        << "  │  1  Add Item           │\n"
        << "  │  2  View Item by ID    │\n"
        << "  │  3  Update Item        │\n"
        << "  │  4  Delete Item        │\n"
        << "  │  5  List All Items     │\n"
        << "  │  6  Exit               │\n"
        << "  └────────────────────────┘\n"
        << "  Choice: ";
}

static void printSeparator()
{
    std::cout << "  " << std::string(54, '-') << "\n";
}

/* ======================================================== validated input */

static int readPositiveInt(const char *prompt)
{
    int v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v > 0) {
            clearInput();
            return v;
        }
        clearInput();
        std::cout << "  [!] Must be a positive integer. Try again.\n";
    }
}

static int readNonNegInt(const char *prompt)
{
    int v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v >= 0) {
            clearInput();
            return v;
        }
        clearInput();
        std::cout << "  [!] Must be 0 or greater. Try again.\n";
    }
}

static float readNonNegFloat(const char *prompt)
{
    float v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v >= 0.0f) {
            clearInput();
            return v;
        }
        clearInput();
        std::cout << "  [!] Must be 0.00 or greater. Try again.\n";
    }
}

static std::string readNonEmptyString(const char *prompt)
{
    std::string s;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, s);
        /* trim leading/trailing spaces */
        std::size_t start = s.find_first_not_of(" \t");
        if (start == std::string::npos) {
            std::cout << "  [!] Name must not be empty. Try again.\n";
            continue;
        }
        std::size_t end = s.find_last_not_of(" \t");
        s = s.substr(start, end - start + 1);
        if (s.empty()) {
            std::cout << "  [!] Name must not be empty. Try again.\n";
            continue;
        }
        if (s.length() >= NAME_LEN) {
            std::cout << "  [!] Name too long (max " << NAME_LEN - 1
                      << " chars). Try again.\n";
            continue;
        }
        return s;
    }
}

/* =========================================================== menu actions */

static void doAdd(InventoryManager &mgr)
{
    printSeparator();
    std::cout << "  ADD NEW ITEM\n";
    printSeparator();

    int         id  = readPositiveInt("  Item ID    : ");
    std::string nm  = readNonEmptyString("  Name       : ");
    int         qty = readNonNegInt    ("  Quantity   : ");
    float       prc = readNonNegFloat  ("  Price ($)  : ");

    if (mgr.addItem(id, nm, qty, prc)) {
        std::cout << "  [✓] Item " << id << " added successfully.\n";
    } else {
        std::cout << "  [✗] Failed to add item. ID " << id
                  << " may already exist.\n";
    }
}

static void doView(InventoryManager &mgr)
{
    printSeparator();
    std::cout << "  VIEW ITEM\n";
    printSeparator();

    int  id = readPositiveInt("  Item ID : ");
    Item it;
    if (mgr.getItem(id, it)) {
        std::cout << "\n";
        InventoryManager::printItem(it);
    } else {
        std::cout << "  [✗] Item " << id << " not found or has been deleted.\n";
    }
}

static void doUpdate(InventoryManager &mgr)
{
    printSeparator();
    std::cout << "  UPDATE ITEM\n";
    printSeparator();

    int id = readPositiveInt("  Item ID to update : ");

    /* Show current values first. */
    Item existing;
    if (!mgr.getItem(id, existing)) {
        std::cout << "  [✗] Item " << id << " not found or has been deleted.\n";
        return;
    }
    std::cout << "  Current values:\n";
    InventoryManager::printItem(existing);
    std::cout << "\n  Enter new values (leave blank to reuse current values):\n";

    /* Name */
    std::cout << "  Name [" << existing.name << "] : ";
    std::string nm;
    std::getline(std::cin, nm);
    if (nm.empty()) nm = existing.name;

    /* Quantity */
    int qty = existing.quantity;
    while (true) {
        std::cout << "  Quantity [" << existing.quantity << "] : ";
        std::string qStr;
        std::getline(std::cin, qStr);
        if (qStr.empty()) { break; }
        try {
            int parsed = std::stoi(qStr);
            if (parsed < 0) { std::cout << "  [!] Must be 0 or greater.\n"; continue; }
            qty = parsed; break;
        } catch (...) { std::cout << "  [!] Invalid number. Try again.\n"; }
    }

    /* Price */
    float prc = existing.price;
    while (true) {
        std::cout << "  Price [$" << std::fixed << std::setprecision(2)
                  << existing.price << "] : ";
        std::string pStr;
        std::getline(std::cin, pStr);
        if (pStr.empty()) { break; }
        try {
            float parsed = std::stof(pStr);
            if (parsed < 0.0f) { std::cout << "  [!] Must be >= 0.00.\n"; continue; }
            prc = parsed; break;
        } catch (...) { std::cout << "  [!] Invalid number. Try again.\n"; }
    }

    if (mgr.updateItem(id, nm, qty, prc)) {
        std::cout << "  [✓] Item " << id << " updated successfully.\n";
    } else {
        std::cout << "  [✗] Update failed.\n";
    }
}

static void doDelete(InventoryManager &mgr)
{
    printSeparator();
    std::cout << "  DELETE ITEM (soft delete)\n";
    printSeparator();

    int id = readPositiveInt("  Item ID to delete : ");

    /* Confirm the item exists first. */
    Item it;
    if (!mgr.getItem(id, it)) {
        std::cout << "  [✗] Item " << id << " not found or already deleted.\n";
        return;
    }

    std::cout << "  Deleting: " << it.name << " (ID " << id << ")\n";
    std::cout << "  Confirm? [y/N] : ";
    std::string confirm;
    std::getline(std::cin, confirm);
    if (confirm != "y" && confirm != "Y") {
        std::cout << "  Cancelled.\n";
        return;
    }

    if (mgr.deleteItem(id)) {
        std::cout << "  [✓] Item " << id << " deleted.\n";
    } else {
        std::cout << "  [✗] Delete failed.\n";
    }
}

static void doList(InventoryManager &mgr)
{
    printSeparator();
    std::cout << "  LIST ALL ACTIVE ITEMS\n";
    printSeparator();

    std::cout << "  Sort by  [1] ID (default)  [2] Name : ";
    std::string choice;
    std::getline(std::cin, choice);
    bool byName = (choice == "2");

    std::vector<Item> items = mgr.listItems(byName);
    printItemTable(items);
    std::cout << "  Total: " << items.size() << " item(s).\n";
}

/* ================================================================== main */

int main()
{
    InventoryManager mgr;
    printBanner();

    while (true) {
        printMenu();

        int choice;
        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << "  [!] Invalid input.\n";
            continue;
        }
        clearInput();

        switch (choice) {
        case 1: doAdd   (mgr); break;
        case 2: doView  (mgr); break;
        case 3: doUpdate(mgr); break;
        case 4: doDelete(mgr); break;
        case 5: doList  (mgr); break;
        case 6:
            std::cout << "\n  Goodbye.\n\n";
            return 0;
        default:
            std::cout << "  [!] Invalid choice. Enter 1-6.\n";
            continue;
        }

        pause();
    }
}
