UNAME_S := $(shell uname -s)

CXX			= g++
CXXFLAGS	= -std=c++11 -Wall -pedantic -O3
LDLIBS		=
INCLUDES	= -I./libs
TARGET		= lbmcl
TARGET_CPU	= lbmcpp
FOLDER		= ./results


ifeq ($(UNAME_S), Darwin)
	CXXFLAGS	+= -Wc++11-extensions -DCL_SILENCE_DEPRECATION
	LDLIBS		+= -framework OpenCL
	FOLDER		= /Volumes/RamDisk
else
	LDLIBS		+= -lOpenCL
endif


$(TARGET): $(TARGET).cpp $(OBJS)
	$(CXX)  -o $@ $^ $(LDLIBS) $(CXXFLAGS) $(INCLUDES)

$(TARGET_CPU): $(TARGET_CPU).cpp
	$(CXX)  -o $@ $^ $(LDLIBS) $(CXXFLAGS) $(INCLUDES)

test: $(TARGET)
	@ $(RM) $(FOLDER)/map*.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P0 -D2 -d8 -v0.0089 -u0.05 -i5 -e1 -k /Volumes/RamDisk -p /Volumes/RamDisk -f

test32: $(TARGET)
	@ $(RM) $(FOLDER)/map*.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P0 -D2 -d32 -v0.0089 -u5.00000000000000027756e-02f -i500 -e20 -k /Volumes/RamDisk

testcpu: $(TARGET_CPU)
	$(RM) $(FOLDER)/lbmcl.*
	./lbmcpp 2 1

clean:
	$(RM) $(TARGET) *.o *~ 
