# compiling vars
CXX := g++
CPPSTANDARD := c++20
OPT := -O3
LINK:= -ledit
DEFINE := -DFMT_HEADER_ONLY
INCLUDE := ./include
CXXFLAG := $(OPT) $(LINK) -Wall -std=$(CPPSTANDARD) $(DEFINE) -I$(INCLUDE)

# main building vars
TEMP_OBJ_DIR := ./tmp
TARGET := bin/main
SRC_DIR := src
SRC_FILE := $(shell ls $(SRC_DIR)/*.cpp)

# test building vars
TEST_SRC_DIR := ./test
TEST_FILES := $(shell ls $(TEST_SRC_DIR)/*.cpp)
TEST_OBJS:= $(patsubst %.cpp,$(TEMP_OBJ_DIR)/%.o,$(notdir $(TEST_FILES)))
TEST_DEF := -DNOTEST_EXPERIMENTAL #-DNOTEST_V1

release: $(SRC_FILE)
	$(CXX) $(SRC_FILE) -o $(TARGET) $(CXXFLAG)
	$(TARGET)

test : bin $(TEMP_OBJ_DIR) $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o bin/test $(OPT) -Wall -std=$(CPPSTANDARD) -I$(INCLUDE) -lgtest -lpthread -lgtest_main 
	./bin/test

$(TEMP_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp $(INCLUDE)/*.h 
	$(CXX) $< -o $@ -c $(OPT) -Wall -std=$(CPPSTANDARD) -I$(INCLUDE) $(TEST_DEF) -fconcepts-diagnostics-depth=10

bin:
	@mkdir -p bin

$(TEMP_OBJ_DIR):
	@mkdir -p $(TEMP_OBJ_DIR)

clean : 
	rm -rf bin/*
	rm -rf tmp/*