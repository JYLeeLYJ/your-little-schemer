# compiling vars
CXX := g++
CPPSTANDARD := c++20
OPT := -O3
LINK:= -ledit
DEFINE := -DFMT_HEADER_ONLY
INCLUDE := ./include
CXXFLAG := $(OPT) -Wall -std=$(CPPSTANDARD) $(DEFINE) -I$(INCLUDE) -ftemplate-backtrace-limit=0 #-fconcepts-diagnostics-depth=10

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
LINK_TEST:= -lgtest -lpthread -lgtest_main 

$(shell if [ ! -e bin ]; then mkdir -p bin ; fi)
$(shell if [ ! -e $(TEMP_OBJ_DIR) ];then mkdir -p $(TEMP_OBJ_DIR); fi)
$(shell if [ ! -e $(TEMP_OBJ_DIR)/test ]; then mkdir -p $(TEMP_OBJ_DIR)/test ; fi)

-include $(OBJS:.o=.o.d)
-include $(TEST_OBJS:.o=.o.d)

release: $(OBJS) main.cpp
	$(CXX) $(OBJS) main.cpp -o $(TARGET) $(CXXFLAG) $(LINK)
	$(TARGET)

test : $(TEST_OBJS) $(OBJS)
	$(CXX) $(TEST_OBJS) $(OBJS) -o bin/test $(CXXFLAG) $(LINK_TEST) 
	./bin/test

$(TEMP_OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp 
	$(CXX) $< -o $@ -c $(CXXFLAG) -MMD -MF $@.d

$(TEMP_OBJ_DIR)/test/%.o: $(TEST_SRC_DIR)/%.cpp 
	$(CXX) $< -o $@ -c $(CXXFLAG) -MMD -MF $@.d

clean : 
	rm -rf bin/*
	rm -rf tmp/*.o
	rm -rf tmp/test/*.o
	rm -rf tmp/*.d