# compiling vars
CXX := g++
CPPSTANDARD := c++20
OPT := -O2
LINK:= -ledit
DEFINE := -DFMT_HEADER_ONLY
CXXFLAG := $(OPT) $(LINK) -Wall -std=$(CPPSTANDARD) $(DEFINE)
INCLUDE := ./parser

# main building vars
TEMP_OBJ_DIR := ./tmp
TARGET := bin/main
SRC_FILE := main.cpp

# test building vars
TEST_SRC_DIR := ./test
TEST_FILES := $(shell ls $(TEST_SRC_DIR)/*.cpp)
TEST_OBJS:= $(patsubst %.cpp,$(TEMP_OBJ_DIR)/%.o,$(notdir $(TEST_FILES)))

release: $(SRC_FILE)
	$(CXX) $(SRC_FILE) -o $(TARGET) $(CXXFLAG)

test : $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o bin/test $(OPT) -Wall -std=$(CPPSTANDARD) -I./ -lgtest -lpthread -lgtest_main 
	./bin/test

$(TEMP_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp $(INCLUDE)/*.h 
	$(CXX) $< -o $@ -c $(OPT) -Wall -std=$(CPPSTANDARD) -I./ -fconcepts-diagnostics-depth=10 

clean : 
	rm -rf bin/*