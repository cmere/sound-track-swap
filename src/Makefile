PROJECT_DIR := ..
SOURCE_DIR := $(PROJECT_DIR)/src
OBJECT_DIR := $(PROJECT_DIR)/build
BIN_DIR := $(PROJECT_DIR)/bin
LIB_DIR := $(PROJECT_DIR)/lib

SOUND_TRACK_SWAP := $(BIN_DIR)/sound-track-swap

vpath %.o $(PROJECT_DIR)/build
vpath %.cpp $(PROJECT_DIR)/src

SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
OBJECTS := $(patsubst $(SOURCE_DIR)%,$(OBJECT_DIR)%,$(OBJECTS))

INCPATH       = -I/usr/lib/x86_64-linux-gnu/qt5/mkspecs/linux-g++-64 -I. -I. -Iexternal/kiss_fft/tools -Iexternal/kiss_fft -isystem /usr/include/x86_64-linux-gnu/qt5 -isystem /usr/include/x86_64-linux-gnu/qt5/QtGui -isystem /usr/include/x86_64-linux-gnu/qt5/QtConcurrent -isystem /usr/include/x86_64-linux-gnu/qt5/QtCore -I/usr/local/include/mlt -I. 

CXXFLAGS := -fPIC

AUDIO_ALIGNMENT_LIB := audio-alignment

.PHONY: $(AUDIO_ALIGNMENT_LIB)

FFMPEG_LIBS :=  -lavformat                        \
                -lavfilter                        \
                -lavcodec                         \
                -lswresample                      \
                -lswscale                         \
                -lavutil                          \

BOOST_LIBS := -lboost_system -lboost_filesystem

CXXFLAGS := -std=c++11 -D__STDC_CONSTANT_MACROS -Wall -g $(CXXFLAGS)

LIBS          = $(SUBLIBS) -L/usr/X11R6/lib64 -lQt5Gui -lQt5Core -lGL -lpthread -lmlt -lmlt++

$(SOUND_TRACK_SWAP): $(OBJECTS) ${AUDIO_ALIGNMENT_LIB}
	$(CXX) -o $@ $(CXXFLAGS) $(OBJECTS) -L $(LIB_DIR) -l$(AUDIO_ALIGNMENT_LIB) $(FFMPEG_LIBS) $(BOOST_LIBS) $(LIBS) 

$(OBJECT_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCPATH) -c $(CXXFLAGS) $^ -o $@

${AUDIO_ALIGNMENT_LIB}:
	$(MAKE) --directory=$@

