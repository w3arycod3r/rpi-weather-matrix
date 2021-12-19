# Makefile for custom c++ game code using the rgbmatrix library

# List src files needed for each exe, only one .cpp can contain a main()
EXE_LIST = input-test pong-main
IDtest_SRC = InputDeviceTest.cpp GameObjects.cpp InputDevices.cpp
Pong_SRC = PongMain.cpp GameObjects.cpp InputDevices.cpp PongObjects.cpp

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
DEP_DIR = dep

# rgbmatrix library directory
RGB_DIR = ../rpi-rgb-led-matrix

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
EXE = $(addprefix $(BIN_DIR)/, $(EXE_LIST))
# Look for the .d files that have been made previously
DEP = $(wildcard $(DEP_DIR)/*.d)

# Present Files to be removed on make clean
OBJ_PRES = $(wildcard $(OBJ_DIR)/*.o)
EXE_PRES = $(wildcard $(BIN_DIR)/*)

## Preprocessor Flags
# -MD = Create dep files in addition to normal preprocessing
# -MF = Use following output file for dep file
# -MP = Create phony targets for each dep in the .d file, to avoid errors when headers are removed
##
CPPFLAGS += -Iinclude -I$(RGB_DIR)/include -MD -MF $(@:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d) -MP
# C++ Compiler Flags
CXXFLAGS += -O3 -g -Wall
#CXXFLAGS += -O3 -Wall
# Linker Flags
LDFLAGS += -L$(RGB_DIR)/lib
# Library Flags
LDLIBS += -lrgbmatrix -pthread

# Phony targets produce no output by its name
.PHONY: all clean
all: $(EXE)
# Allow binary names as targets directly
$(EXE_LIST): %: $(BIN_DIR)/%


# Need one of these for each binary
$(BIN_DIR)/input-test: $(addprefix $(OBJ_DIR)/, $(IDtest_SRC:.cpp=.o))
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BIN_DIR)/pong-main: $(addprefix $(OBJ_DIR)/, $(Pong_SRC:.cpp=.o))
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

-include $(DEP)

# Remove ALL the .o, binary, and .d files present
clean:
	$(RM) $(OBJ_PRES) $(EXE_PRES) $(DEP)