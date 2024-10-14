CC			 := g++
CFLAGS 		 := -D _DEBUG -lm -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -pie -fPIE -Werror=vla
LOGGER_FLAGS :=
CFLAGS = -D _DEBUG
# CFLAGS += -D NDEBUG
# CFLAGS += -fsanitize=address

SOURCE_DIR           := source
LIB_RUN_NAME         := stackStruct
BUILD_DIR            := building
MY_LOG_LIB_NAME      := my_loglib
LOGGER_HEADER_FOLDER := LoggerLib/include/

ifeq ($(DEBUG), 0)
	ASSERT_DEFINE = -DNDEBUG
endif

.PHONY: $(LIB_RUN_NAME) run $(BUILD_DIR) clean

# -------------------------   LIB RUN   -----------------------------

SRC 	   			:= $(wildcard ./$(SOURCE_DIR)/*.cpp)
OBJ 	   			:= $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(notdir ${SRC}))

# running all commands without output (@ at the beginning)
$(LIB_RUN_NAME): $(OBJ)
	$(CC) $^ -o $(BUILD_DIR)/$(LIB_RUN_NAME) -l$(MY_LOG_LIB_NAME) $(CFLAGS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(BUILD_DIR)
	@$(CC) -c $< $(CFLAGS) -o $@ $(ASSERT_DEFINE) -I$(LOGGER_HEADER_FOLDER)

run: $(LIB_RUN_NAME)
	$(BUILD_DIR)/$(LIB_RUN_NAME)


HELP_MESSAGE := installing stack structure \(just creating static library for it\)

install: $(LIB_RUN_NAME)
	@echo installing stack structure $(HELP_MESSAGE)
	@ar rcs stackLib.a $(OBJ)

# -------------------------   HELPER TARGETS   ---------------------------

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/$(LIB_RUN_NAME)
