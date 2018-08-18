PROJECT_DIR := ..
SOURCE_DIR := $(PROJECT_DIR)/src
OBJECT_DIR := $(PROJECT_DIR)/build
BIN_DIR := $(PROJECT_DIR)/bin
LIB_DIR := $(PROJECT_DIR)/lib

SOUND_TRACK_SWAP := $(BIN_DIR)/sound-track-swap

#vpath %.o $(PROJECT_DIR)/build
#vpath %.cpp $(PROJECT_DIR)/src

SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
OBJECTS := $(patsubst $(SOURCE_DIR)%,$(OBJECT_DIR)%,$(OBJECTS))

INCLUDE_PATH :=

FFMPEG_LIBS :=  -lavformat    \
                -lavfilter    \
                -lavcodec     \
                -lswresample  \
                -lswscale     \
                -lavutil      \

BOOST_LIBS := -lboost_system -lboost_filesystem
LIBS := -lpthread 

CXXFLAGS := -std=c++11 -Wall -g -fPIC 
DEPFLAGS = -MT $@ -MMD -MP

$(SOUND_TRACK_SWAP): $(OBJECTS) 
	$(CXX) -o $@ $(OBJECTS) $(FFMPEG_LIBS) $(BOOST_LIBS) $(LIBS) 

$(OBJECT_DIR)/%.o: %.cpp $(OBJECT_DIR)/%.d
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INCLUDE_PATH) -c $< -o $@

%.d: ;

include $(patsubst %.o,%.d,$(OBJECTS))
