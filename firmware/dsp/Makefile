CXX ?= c++
CXXFLAGS = -std=c++14 -Wall -Wextra -Werror
BUILD_DIR = ../build/dsp
TARGET = $(BUILD_DIR)/processor_test

.PHONY: test clean

test: $(TARGET)
	$(TARGET)

$(TARGET): processor_test.cpp processor.h
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) processor_test.cpp -o $(TARGET)

clean:
	rm -f $(TARGET)
