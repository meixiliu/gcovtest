
vpath %.cpp tinyxml2/src .
vpath %.h   src/include

GTEST_DIR = googletest
TINYXML_DIR = tinyxml2

TEST_OBJ = test.o
TINYXML_OBJ = tinyxml2.o

BUILD_DIR = build
BUILD_TEST_OBJ = $(addprefix $(BUILD_DIR)/, $(TEST_OBJ))
BUILD_TINYXML_OBJ = $(addprefix $(BUILD_DIR)/$(TINYXML_DIR)/, $(TINYXML_OBJ))


INCLUDE = -I./$(TINYXML_DIR)/include -I./$(GTEST_DIR)/include
LIBS = -L$(GTEST_DIR)/lib -lgmock -lgtest -lpthread
CXXFLAGS = -fprofile-arcs -ftest-coverage -g -O0 -fno-exceptions -fno-inline -pg

all: test

gmon.out BUILD_DIR/test.gcda BUILD_DIR/TINYXML_DIR/tinyxml2.gcda: test
	@./test

gprof: test gmon.out
	@gprof ./test gmon.out > gprof_report/report.txt

html: test BUILD_DIR/test.gcda BUILD_DIR/TINYXML_DIR/tinyxml2.gcda
	@lcov -c -d . -o test.info --rc lcov_branch_coverage=1
	@lcov --remove test.info '/usr/include/*' '*/googletest/*' '*test.cpp*' '*src/tinyxml2.h' -o test.info --rc lcov_branch_coverage=1
	@echo Gen test.info
	@genhtml --branch-coverage test.info -o ./output
	@echo Gen html

test: $(BUILD_TEST_OBJ) $(BUILD_TINYXML_OBJ)
	@echo CXX + $@
	@$(CXX) $(CXXFLAGS) $^  -o $@ $(LIBS)

$(BUILD_DIR)/%.o: %.cpp
	@echo CXX + $@
	@$(CXX) -c $(CXXFLAGS) $(INCLUDE) $< -o $@


$(BUILD_DIR)/$(TINYXML_DIR)/%.o: %.cpp
	@echo CXX + $@
	@$(CXX) -c $(CXXFLAGS) $(INCLUDE) $< -o $@


clean:
	@rm -rf $(BUILD_DIR)/*.gcda $(BUILD_DIR)/$(TINYXML_DIR)/*.gcda
	@rm -rf *.gcov
	@rm -rf *.info
	@rm -rf ./html
	@rm -rf ./*.out
	@rm -rf ./gprof_report/*
	@rm -rf test
	@rm -rf $(BUILD_TEST_OBJ)  $(BUILD_TINYXML_OBJ)
	@rm -rf $(BUILD_DIR)/*.gcno $(BUILD_DIR)/$(TINYXML_DIR)/*.gcno
