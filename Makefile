UNAME_S := $(shell uname -s)

PLATFORM	= 0
DEVICE		= 0
DIM_TEST	= 8
VISCOSITY	= 0.0089
VELOCITY	= 0.05
FOLDER		= ./results


CXX			= g++
CXXFLAGS	= -std=c++11 -Wall -pedantic -O3
LDLIBS		=
INCLUDES	= -I./libs
TARGET		= lbmcl


ifeq ($(UNAME_S), Darwin)
	PLATFORM	= 0
	DEVICE		= 2
	CXXFLAGS	+= -Wc++11-extensions -DCL_SILENCE_DEPRECATION
	LDLIBS		+= -framework OpenCL
	FOLDER		= /Volumes/RamDisk
else
	LDLIBS		+= -lOpenCL
endif


$(TARGET): $(TARGET).cpp $(OBJS)
	$(CXX)  -o $@ $^ $(LDLIBS) $(CXXFLAGS) $(INCLUDES)


test: $(TARGET)
	@ $(RM) $(FOLDER)/map*.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d$(DIM_TEST) -v$(VISCOSITY) -u$(VELOCITY) -i5 -e1 -k $(FOLDER) -p $(FOLDER) -f

test32: $(TARGET)
	@ $(RM) $(FOLDER)/map*.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d32 -v$(VISCOSITY) -u$(VELOCITY) -i500 -e20 -k $(FOLDER)

clean:
	$(RM) $(TARGET) *.o *~ 
