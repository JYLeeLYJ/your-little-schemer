
CXX := g++
CPPSTANDARD := c++20
OPT := -O2
LINK:= -ledit
DEFINE := -DFMT_HEADER_ONLY
CXXFLAG := $(OPT) $(LINK) -Wall -std=$(CPPSTANDARD) $(DEFINE)
INCLUDE := ./parser
TEST_SRC := ./test

TARGET := bin/main
SRC_FILE := main.cpp

release: $(SRC_FILE)
	$(CXX) $(SRC_FILE) -o $(TARGET) $(CXXFLAG)

test : $(INCLUDE)/*.h $(TEST_SRC)/*.cpp
	$(CXX) $(TEST_SRC)/*.cpp -o bin/test $(OPT) -Wall -std=$(CPPSTANDARD) -I./ -lgtest -lpthread -fconcepts-diagnostics-depth=10
	./bin/test

clean : 
	rm -rf bin/*