#ifndef INVENTORY_MANAGER_H
#define INVENTORY_MANAGER_H

#include <vector>
#include <string>
#include "inventory.h"

class InventoryManager {
public:
    /* CRUD wrappers ----------------------------------------------------- */
    bool addItem   (int id, const std::string &name, int qty, float price);
    bool getItem   (int id, Item &out);
    bool updateItem(int id, const std::string &name, int qty, float price);
    bool deleteItem(int id);

    /* Returns sorted vector of all active items.
     * sortByName=false → sort by id; sortByName=true → sort by name. */
    std::vector<Item> listItems(bool sortByName = false);

    /* Pretty-print a single item to stdout. */
    static void printItem (const Item &item);

    /* Pretty-print a table of items. */
    static void printTable(const std::vector<Item> &items);
};

#endif /* INVENTORY_MANAGER_H */
