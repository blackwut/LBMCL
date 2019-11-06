# Compilation options
CXX			= g++
CXXFLAGS	= -std=c++11 -Wall -Wextra -Wpedantic -pedantic -O0 -g
LDLIBS		=
INCLUDES	= -I. -I./libs
TARGET		= lbmcl


# User defined options
PLATFORM	= 0
DEVICE		= 0
VISCOSITY	= 0.0089
VELOCITY	= 0.05
RESULTS		= ./results
TARGET_RES	= ./target_results

# User defined options in shell environment using "export" command
DIM			?= 8
PRECISION	?= SINGLE # SINGLE or DOUBLE
ITERATIONS	?= 10
EVERY		?= 1
LWS			?= 8
STRIDE		?= 8


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
	$(CXX)  -o $@ $^ $(LDLIBS) $(CXXFLAGS) $(INCLUDES) -DFP_$(PRECISION)


testall: $(TARGET)
	@ $(RM) $(RESULTS)/map.dump
	@ $(RM) $(RESULTS)/f_*.dump
	@ $(RM) $(RESULTS)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d$(DIM) -n$(VISCOSITY) -u$(VELOCITY) -i$(ITERATIONS) -e$(EVERY) -w$(LWS) -s$(STRIDE) -o -v $(RESULTS)

testall: $(TARGET)
	@ $(RM) $(RESULTS)/map.dump
	@ $(RM) $(RESULTS)/f_*.dump
	@ $(RM) $(RESULTS)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d$(DIM) -n$(VISCOSITY) -u$(VELOCITY) -i$(ITERATIONS) -e$(EVERY) -w$(LWS) -s$(STRIDE) -o -v $(RESULTS) -p $(RESULTS) -f -m


test8: $(TARGET)
	@ $(RM) $(RESULTS)/map.dump
	@ $(RM) $(RESULTS)/f_*.dump
	@ $(RM) $(RESULTS)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d 8 -n 0.0089 -u 0.05 -i 10 -e 1 -w 32 -s 8 -o -v $(RESULTS)
	@ python3 verify.py -i 10 -e 1 -t $(TARGET_RES)/8 -p $(RESULTS)


test32: $(TARGET)
	@ $(RM) $(RESULTS)/map.dump
	@ $(RM) $(RESULTS)/f_*.dump
	@ $(RM) $(RESULTS)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d 32 -n 0.0089 -u 0.05 -i 500 -e 20 -w 32 -s 32 -o -v $(RESULTS)
	@ python3 verify.py -i500 -e20 -t $(TARGET_RES)/32 -p $(RESULTS)


clean:
	$(RM) $(TARGET) *.o *~ $(RESULTS)/*.dump $(RESULTS)/*.vti
