UNAME_S := $(shell uname -s)

CXX			= g++
CXXFLAGS	= -std=c++11 -Wall -pedantic -O3
LDLIBS		=
INCLUDES	= -I./libs
TARGET		= lbmcl
TARGET_CPU	= lbmcpp


ifeq ($(UNAME_S), Darwin)
	CXXFLAGS	+= -Wc++11-extensions -DCL_SILENCE_DEPRECATION
	LDLIBS		+= -framework OpenCL
else
	LDLIBS		+= -lOpenCL
endif

$(TARGET): $(TARGET).cpp $(OBJS)
	$(CXX)  -o $@ $^ $(LDLIBS) $(CXXFLAGS) $(INCLUDES)

TARGET_CPU: $(TARGET_CPU).cpp
	$(CXX)  -o $@ $^ $(LDLIBS) $(CXXFLAGS) $(INCLUDES)
# %.o: %.cpp
# 	$(CXX) -c $< $(LDLIBS) $(CXXFLAGS) $(INCLUDES)

test: $(TARGET)
	$(RM) /Volumes/RamDisk/lbmcl.*
	./lbmcl 0 2 10 1 "/Volumes/RamDisk/"
	#python3 verify.py 1

testcpu: $(TARGET_CPU)
	$(RM) /Volumes/RamDisk/lbmcl.*
	./lbmcpp 10 1

clean:
	$(RM) $(TARGET) *.o *~ 
