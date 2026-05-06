# Hybrid Inventory Manager

A console-based inventory management system that demonstrates hybrid C / C++ architecture:

| Layer | Language | Responsibility |
|-------|----------|----------------|
| Data (backend) | **C** | Binary file I/O with `fread` / `fwrite` / `fseek` |
| UI (frontend) | **C++** | Menu, input validation, STL sorting / printing |

---

## Project Structure

```
capstone/
├── include/
│   ├── inventory.h          # C struct + C-linkage function declarations
│   └── InventoryManager.h   # C++ class declaration
├── src/
│   ├── inventory.c          # C backend (file storage)
│   ├── InventoryManager.cpp # C++ wrapper class (STL usage)
│   └── main.cpp             # C++ menu / UI driver
├── Makefile                 # Primary build file
├── CMakeLists.txt           # Alternate CMake build
└── README.md
```

---

## Build & Run

### Using Make (recommended)

```bash
# Build
make

# Build and run immediately
make run

# Clean build artifacts
make clean
```

### Using CMake

```bash
mkdir -p build_cmake && cd build_cmake
cmake ..
cmake --build .
./inventory_manager
```

### Requirements

- GCC ≥ 9 (or any C11/C++17-compliant toolchain)
- GNU Make ≥ 3.81  *(or CMake ≥ 3.14)*
- POSIX-compatible OS (Linux, macOS, WSL)

---

## How It Works

### Data Persistence

All inventory records are stored in `inventory.dat` (created on first run) as a flat binary file of fixed-size `Item` structs:

```c
typedef struct {
    int   id;            // unique positive integer
    char  name[40];      // item name
    int   quantity;      // units in stock (≥ 0)
    float price;         // unit price (≥ 0.00)
    int   is_deleted;    // 0 = active, 1 = soft-deleted
} Item;
```

`fseek` is used to locate and overwrite individual records in-place, so updates and soft-deletes are O(n) in the number of records.

### C Backend API

```c
int add_item   (const Item *item);            // 1 = success, 0 = fail / dup ID
int get_item   (int id, Item *out);           // 1 = found active, 0 = not found
int update_item(int id, const Item *updated); // 1 = success, 0 = fail
int delete_item(int id);                      // soft-delete; 1 = success
int list_items (Item *buffer, int max_items); // returns count copied
```

### C++ Layer

`InventoryManager` wraps the C API and exposes:

- `std::vector<Item> listItems(bool sortByName)` — STL `vector` + `std::sort`
- `static void printItem(const Item&)` — single-record display
- `static void printTable(const std::vector<Item>&)` — formatted table

---

## Menu

```
1  Add Item
2  View Item by ID
3  Update Item
4  Delete Item
5  List All Items
6  Exit
```

---

## Test Cases

The following scenarios were manually verified:

- **Persistence after restart**
  Added items with IDs 1, 2, 3; exited the program; relaunched; chose *List All* → all three items appeared correctly with their original data.

- **Update persists after restart**
  Updated item ID 2 (changed name and price); exited; relaunched; chose *View Item* → ID 2 showed the new values, not the originals.

- **Soft-delete hides records**
  Deleted item ID 3; chose *List All* → only IDs 1 and 2 appeared. Chose *View Item* → ID 3 returned "not found". After restart, ID 3 still did not appear.

- **Duplicate ID rejection**
  Attempted to add a second item with ID 1 → program printed a failure message and left the original record untouched.

- **Input validation**
  Entered `-5` for ID, `-10` for quantity, and an empty string for name → the program re-prompted for each invalid field without crashing, then accepted valid values.

---

## Notes

- The database file `inventory.dat` is created in the working directory.  
  Delete it to start fresh: `rm inventory.dat`
- Soft-deleted records remain in the file (they are never physically removed).  
  This keeps the implementation simple and matches real-world "archive" patterns.
- IDs are globally unique, including across soft-deleted records — a deleted ID cannot be reused.
