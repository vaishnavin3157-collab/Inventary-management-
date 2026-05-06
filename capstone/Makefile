# ============================================================
#  Makefile  –  Hybrid Inventory Manager
#  C files compiled with CC; C++ files compiled with CXX.
#  All object files linked by CXX to pick up the C++ runtime.
# ============================================================

CC      = gcc
CXX     = g++
CFLAGS  = -Wall -Wextra -pedantic -std=c11   -Iinclude
CXXFLAGS= -Wall -Wextra -pedantic -std=c++17 -Iinclude
LDFLAGS =

TARGET  = inventory_manager
SRCDIR  = src
OBJDIR  = build

# Source files
C_SRCS   = $(SRCDIR)/inventory.c
CPP_SRCS = $(SRCDIR)/InventoryManager.cpp $(SRCDIR)/main.cpp

# Object files (placed in build/)
C_OBJS   = $(patsubst $(SRCDIR)/%.c,   $(OBJDIR)/%.o, $(C_SRCS))
CPP_OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(CPP_SRCS))
ALL_OBJS = $(C_OBJS) $(CPP_OBJS)

# ---- Default target ----------------------------------------
.PHONY: all clean run

all: $(OBJDIR) $(TARGET)

$(TARGET): $(ALL_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^
	@echo "Build complete → $(TARGET)"

# ---- Compile C sources -------------------------------------
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ---- Compile C++ sources -----------------------------------
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---- Create build directory --------------------------------
$(OBJDIR):
	mkdir -p $(OBJDIR)

# ---- Run the program ---------------------------------------
run: all
	./$(TARGET)

# ---- Clean -------------------------------------------------
clean:
	rm -rf $(OBJDIR) $(TARGET)
	@echo "Cleaned."
