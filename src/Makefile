BIN_SOUND_TRACK_SWAP := sound-track-swap

PROJECT_DIR := ..
SOURCE_DIR := ../src

SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS := $(subst %.cpp,%.o,$(SOURCES))

$(BIN_SOUND_TRACK_SWAP) : $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@
