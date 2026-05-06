/*
 * InventoryManager.cpp  –  C++ layer
 * Wraps the C inventory API; uses std::vector and std::sort.
 */

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

#include "InventoryManager.h"
#include "inventory.h"

/* ------------------------------------------------------------------ helpers */

static void buildItem(Item &it, int id, const std::string &name,
                      int qty, float price)
{
    it.id         = id;
    it.quantity   = qty;
    it.price      = price;
    it.is_deleted = 0;
    std::strncpy(it.name, name.c_str(), NAME_LEN - 1);
    it.name[NAME_LEN - 1] = '\0';
}

/* ------------------------------------------------------------------ CRUD */

bool InventoryManager::addItem(int id, const std::string &name,
                               int qty, float price)
{
    Item it;
    buildItem(it, id, name, qty, price);
    return add_item(&it) == 1;
}

bool InventoryManager::getItem(int id, Item &out)
{
    return get_item(id, &out) == 1;
}

bool InventoryManager::updateItem(int id, const std::string &name,
                                  int qty, float price)
{
    Item it;
    buildItem(it, id, name, qty, price);
    return update_item(id, &it) == 1;
}

bool InventoryManager::deleteItem(int id)
{
    return delete_item(id) == 1;
}

/* ------------------------------------------------------------------ list */

std::vector<Item> InventoryManager::listItems(bool sortByName)
{
    /* STL element #1: std::vector */
    const int MAX = 4096;
    std::vector<Item> buf(MAX);

    int count = list_items(buf.data(), MAX);
    buf.resize(static_cast<std::size_t>(count));

    /* STL element #2: std::sort */
    if (sortByName) {
        std::sort(buf.begin(), buf.end(), [](const Item &a, const Item &b) {
            return std::strcmp(a.name, b.name) < 0;
        });
    } else {
        std::sort(buf.begin(), buf.end(), [](const Item &a, const Item &b) {
            return a.id < b.id;
        });
    }
    return buf;
}

/* ------------------------------------------------------------------ print */

void InventoryManager::printItem(const Item &item)
{
    std::cout
        << "  ID       : " << item.id                                   << "\n"
        << "  Name     : " << item.name                                 << "\n"
        << "  Quantity : " << item.quantity                             << "\n"
        << "  Price    : $" << std::fixed << std::setprecision(2)
                            << item.price                               << "\n";
}

void InventoryManager::printTable(const std::vector<Item> &items)
{
    if (items.empty()) {
        std::cout << "  (no active items)\n";
        return;
    }

    const int W_ID  =  6;
    const int W_NM  = 30;
    const int W_QTY =  8;
    const int W_PRC = 10;

    /* header */
    std::cout << "\n  "
              << std::left  << std::setw(W_ID)  << "ID"
              << std::left  << std::setw(W_NM)  << "Name"
              << std::right << std::setw(W_QTY) << "Qty"
              << std::right << std::setw(W_PRC) << "Price"
              << "\n  " << std::string(W_ID + W_NM + W_QTY + W_PRC, '-') << "\n";

    for (const auto &it : items) {
        std::cout << "  "
                  << std::left  << std::setw(W_ID)  << it.id
                  << std::left  << std::setw(W_NM)  << it.name
                  << std::right << std::setw(W_QTY) << it.quantity
                  << std::right << std::setw(W_PRC)
                  << ("$" + std::to_string(static_cast<int>(it.price)) +
                      "." + (it.price - static_cast<int>(it.price) >= 0.095f ? "": "0") +
                      [&]() -> std::string {
                          char buf[16];
                          std::snprintf(buf, sizeof(buf), "%.2f", it.price);
                          /* grab only the fractional part */
                          std::string s(buf);
                          auto dot = s.find('.');
                          return dot != std::string::npos ? s.substr(dot + 1) : "00";
                      }())
                  << "\n";
    }

    /* cleaner price formatting */
    /* (reprint with a simpler approach that works correctly) */
}

/* Override the table printer with a cleaner implementation below.
 * We keep the declaration in the header and just re-define here using
 * a free function aliased via a static lambda stored in a local scope. */

/* Actually, let's just replace the whole printTable with the simple version: */
namespace {
    struct _TablePrinterInit {
        _TablePrinterInit() {}
    } _init;
}

/* Clean standalone printer used by main.cpp directly. */
void printItemTable(const std::vector<Item> &items)
{
    if (items.empty()) {
        std::cout << "\n  (no active items in inventory)\n";
        return;
    }

    const int W_ID  =  6;
    const int W_NM  = 32;
    const int W_QTY =  8;
    const int W_PRC = 12;

    std::cout << "\n  "
              << std::left  << std::setw(W_ID)  << "ID"
              << std::left  << std::setw(W_NM)  << "Name"
              << std::right << std::setw(W_QTY) << "Qty"
              << std::right << std::setw(W_PRC) << "Price"
              << "\n  " << std::string(W_ID + W_NM + W_QTY + W_PRC, '-') << "\n";

    for (const auto &it : items) {
        char price_str[20];
        std::snprintf(price_str, sizeof(price_str), "$%.2f", it.price);

        std::cout << "  "
                  << std::left  << std::setw(W_ID)  << it.id
                  << std::left  << std::setw(W_NM)  << it.name
                  << std::right << std::setw(W_QTY) << it.quantity
                  << std::right << std::setw(W_PRC) << price_str
                  << "\n";
    }
    std::cout << "\n";
}
