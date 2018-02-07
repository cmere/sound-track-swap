PROJECT_DIR := ..
SOURCE_DIR := $(PROJECT_DIR)/src
BIN_DIR := $(PROJECT_DIR)/bin

SOUND_TRACK_SWAP := $(BIN_DIR)/sound-track-swap

SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS := $(subst %.cpp,%.o,$(SOURCES))

$(SOUND_TRACK_SWAP) : $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@
