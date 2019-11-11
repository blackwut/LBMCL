# Compilation options
CXX			= g++
CXXFLAGS	= -std=c++11 -Wall -Wextra -Wpedantic -pedantic -O3
LDLIBS		=
INCLUDES	= -I. -I./libs
TARGET		= lbmcl


# User defined options for tests
PLATFORM	= 0
DEVICE		= 0
DIM			= 8
VISCOSITY	= 0.0089
VELOCITY	= 0.05
ITERATIONS	= 10
EVERY		= 1
LWS			= 8
STRIDE		= 8
PRECISION	= single # single or double
OPTIMIZE	= true
DUMP_PATH	= ./results
DUMP_MAP	= false
DUMP_F		= false

RESULTS		= ./results
TARGET_RES	= ./target_results


ifeq ($(PRECISION),SINGLE)
	MORE_FLAGS += -F
endif
ifeq ($(OPTIMIZE),true)
	MORE_FLAGS += -o
endif
ifneq ($(DUMP_PATH),)
	MORE_FLAGS += -p $(DUMP_PATH)
endif
ifeq ($(DUMP_MAP),true)
	MORE_FLAGS += -m
endif
ifeq ($(DUMP_F),true)
	MORE_FLAGS += -f
endif

ifeq ($(shell uname -s), Darwin)
	PLATFORM	= 0
	DEVICE		= 2
	CXXFLAGS	+= -Wc++11-extensions -DCL_SILENCE_DEPRECATION
	LDLIBS		+= -framework OpenCL
	RESULTS		= /Volumes/RamDisk
else
	LDLIBS		+= -lOpenCL
endif


$(TARGET): main.cpp
	$(CXX)  -o $@ $^ $(LDLIBS) $(CXXFLAGS) $(INCLUDES)


test: $(TARGET)
	@ $(RM) $(RESULTS)/map.dump
	@ $(RM) $(RESULTS)/f_*.dump
	@ $(RM) $(RESULTS)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d$(DIM) -n$(VISCOSITY) -u$(VELOCITY) -i$(ITERATIONS) -e$(EVERY) -w$(LWS) -s$(STRIDE) -v $(RESULTS) $(MORE_FLAGS)

testall: $(TARGET)
	@ $(RM) $(RESULTS)/map.dump
	@ $(RM) $(RESULTS)/f_*.dump
	@ $(RM) $(RESULTS)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d$(DIM) -n$(VISCOSITY) -u$(VELOCITY) -i$(ITERATIONS) -e$(EVERY) -w$(LWS) -s$(STRIDE) -o -v $(RESULTS) -p $(RESULTS) -f -m


test8: $(TARGET)
	@ $(RM) $(RESULTS)/map.dump
	@ $(RM) $(RESULTS)/f_*.dump
	@ $(RM) $(RESULTS)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d 8 -n 0.0089 -u 0.05 -i 10 -e 1 -w 32 -s 8 -v $(RESULTS) $(MORE_FLAGS)
	@ python3 verify.py -i 10 -e 1 -t $(TARGET_RES)/8 -p $(RESULTS)


test32: $(TARGET)
	@ $(RM) $(RESULTS)/map.dump
	@ $(RM) $(RESULTS)/f_*.dump
	@ $(RM) $(RESULTS)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d 32 -n 0.0089 -u 0.05 -i 500 -e 20 -w 32 -s 32 -v $(RESULTS) $(MORE_FLAGS)
	@ python3 verify.py -i500 -e20 -t $(TARGET_RES)/32 -p $(RESULTS)


clean:
	$(RM) $(TARGET) *.o *~ $(RESULTS)/*.dump $(RESULTS)/*.vti
