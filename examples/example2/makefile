PRJ_ROOT := ../../
PROGRAMS := example2
SOURCES := $(wildcard *.cpp)
OBJECT_FILES := $(patsubst %.cpp, %.o, $(SOURCES))
OUTPUTS += $(OBJECT_FILES)

include ../common.mk

example2: $(OBJECT_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -Xlinker -Map=$@.map $^
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $^