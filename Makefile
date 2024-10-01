CC			 := g++
# CFLAGS 		 := -O3 -D _DEBUG -lm -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -pie -fPIE -Werror=vla
LOGGER_FLAGS :=
CFLAGS = -D _DEBUG
# CFLAGS += -fsanitize=address

# [[fallthrough]]

# 1) libRun -> variable; testRun -> variable
# 2) build directory -> variable, -o flags for each compilation
# 3) @> @^ @< $<
# 4) PHONY - что делает

# ASK: How to make multiple makefiles, so loggerLib can be run indivually without warnings
SOURCE_DIR        := source
LIB_RUN_NAME      := stackStruct
BUILD_DIR         := building
LOG_LIB_DIR       := LoggerLib/source

ifeq ($(DEBUG), 0)
	ASSERT_DEFINE = -DNDEBUG
endif

.PHONY: $(LIB_RUN_NAME) run $(BUILD_DIR) clean

# -------------------------   LIB RUN   -----------------------------

LOGGER_SRC 			:= $(LOG_LIB_DIR)/colourfullPrint.cpp $(LOG_LIB_DIR)/debugMacros.cpp $(LOG_LIB_DIR)/logLib.cpp
LOGGER_OBJ 			:= $(patsubst %.cpp, $(BUILD_DIR)/LOGGER_%.o, $(notdir ${LOGGER_SRC}))
SRC 	   			:= $(wildcard ./$(SOURCE_DIR)/*.cpp)
OBJ 	   			:= $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(notdir ${SRC}))

# running all commands without output (@ at the beginning)
$(LIB_RUN_NAME): $(OBJ) $(LOGGER_OBJ) $(SORTING_ALGOS_OBJ)
	@$(CC) $^ -o $(BUILD_DIR)/$(LIB_RUN_NAME) $(CFLAGS)

$(BUILD_DIR)/LOGGER_%.o: $(LOG_LIB_DIR)/%.cpp $(BUILD_DIR)
	@$(CC) -c $< $(LOGGER_FLAGS) -o $@ $(ASSERT_DEFINE)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(BUILD_DIR)
	@$(CC) -c $< $(CFLAGS) -o $@ $(ASSERT_DEFINE)

run: $(LIB_RUN_NAME)
	@$(BUILD_DIR)/$(LIB_RUN_NAME)




# -------------------------   HELPER TARGETS   ---------------------------

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/$(LIB_RUN_NAME)

# g++ -o main.exe main.cpp quadraticEquationLib/quadraticEquation.cpp testsGeneratorLib/testsGenerator.cpp colourfullPrintLib/colourfullPrint.cpp -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -pie -fPIE -Werror=vla
