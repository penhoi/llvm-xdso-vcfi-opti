# This Makefile expects PROJECT_CXX and CXXFLAGS to be set from the environment.
# It provides basic defaults as a fallback.
PROJECT_CXX	?= clang++
CXXFLAGS 	?= -O2 -g

# Target directories (in build order) 
TARGET_DIRS := calculator-50 calculator-40 calculator-30 calculator-20 calculator-10 calculator-8 calculator-4 calculator-3 calculator-2 calculator-1

# Ensure variables are passed to sub-makes [cite: 3]
.EXPORT_ALL_VARIABLES:

# Default target - build all directories 
.PHONY: all $(TARGET_DIRS)
all: $(TARGET_DIRS) 

# Rule to build each directory, explicitly calling the 'all' target 
$(TARGET_DIRS):
	@echo "Building $@ with CXXFLAGS=$(CXXFLAGS)"
	@$(MAKE) -C $@ all

# Clean all directories [cite: 2, 3]
.PHONY: clean
clean:
	@for dir in $(TARGET_DIRS); do \
		echo "Cleaning $$dir..."; \
		$(MAKE) -C $$dir clean || true; \
	done
