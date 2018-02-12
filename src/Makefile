PROJECT_DIR := ..
SOURCE_DIR := $(PROJECT_DIR)/src
BIN_DIR := $(PROJECT_DIR)/bin

SOUND_TRACK_SWAP := $(BIN_DIR)/sound-track-swap

SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
OBJECTS := $(patsubst $(SOURCE_DIR)%,$(BIN_DIR)%,$(OBJECTS))

FFMPEG_LIBS :=  -lavformat                        \
                -lavfilter                        \
                -lavcodec                         \
                -lswresample                      \
                -lswscale                         \
                -lavutil                          \

BOOST_LIBS := -lboost_system -lboost_filesystem

CXXFLAGS := -std=c++11 -D__STDC_CONSTANT_MACROS -Wall -g $(CXXFLAGS)

$(SOUND_TRACK_SWAP): $(OBJECTS)
	$(CXX) $(OBJECTS) $(FFMPEG_LIBS) $(BOOST_LIBS) -o $@

$(BIN_DIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $^ -o $@
