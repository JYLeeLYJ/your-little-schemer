# compiling vars
CXX := g++
CPPSTANDARD := c++20
OPT := -O3
LINK:= -ledit
DEFINE := -DFMT_HEADER_ONLY
INCLUDE := ./include
CXXFLAG := $(OPT) -Wall -std=$(CPPSTANDARD) $(DEFINE) -I$(INCLUDE) -fconcepts-diagnostics-depth=10

# main building vars
TEMP_OBJ_DIR := ./tmp
TARGET := bin/main
SRC_DIR := src
SRC_FILES := $(shell ls $(SRC_DIR)/*.cpp)
OBJS := $(patsubst %.cpp,$(TEMP_OBJ_DIR)/%.o,$(notdir $(SRC_FILES)))

# test building vars
TEST_SRC_DIR := ./test
TEST_FILES := $(shell ls $(TEST_SRC_DIR)/*.cpp)
TEST_OBJS:= $(patsubst %.cpp,$(TEMP_OBJ_DIR)/test/%.o,$(notdir $(TEST_FILES)))

release: bin $(TEMP_OBJ_DIR) $(OBJS) main.cpp
	$(CXX) $(OBJS) main.cpp -o $(TARGET) $(CXXFLAG)  $(LINK)
	$(TARGET)

$(TEMP_OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp $(INCLUDE)/*.h
	$(CXX) $< -o $@ -c $(CXXFLAG)

test : bin $(TEMP_OBJ_DIR) $(TEST_OBJS) $(OBJS)
	$(CXX) $(TEST_OBJS) $(OBJS) -o bin/test $(CXXFLAG) $(LINK) -lgtest -lpthread -lgtest_main 
	./bin/test

$(TEMP_OBJ_DIR)/test/%.o: $(TEST_SRC_DIR)/%.cpp $(INCLUDE)/*.h 
	$(CXX) $< -o $@ -c $(CXXFLAG)

bin:
	@mkdir -p bin

$(TEMP_OBJ_DIR):
	@mkdir -p $(TEMP_OBJ_DIR)
	@mkdir -p $(TEMP_OBJ_DIR)/test

clean : 
	rm -rf bin/*
	rm -rf tmp/*.o
	rm -rf tmp/test/*.o