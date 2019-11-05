# Compilation options
CXX			= g++
CXXFLAGS	= -std=c++11 -Wall -Wextra -Wpedantic -pedantic -O3
LDLIBS		=
INCLUDES	= -I. -I./libs
TARGET		= lbmcl


# User defined options
PLATFORM	= 0
DEVICE		= 0
VISCOSITY	= 0.0089
VELOCITY	= 0.05
FOLDER		= ./results
SAILFISH_RES= ./sailfish_results

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
	FOLDER		= /Volumes/RamDisk
else
	LDLIBS		+= -lOpenCL
endif


$(TARGET): $(TARGET).cpp common.h kernels.cl
	$(CXX)  -o $@ $@.cpp $(LDLIBS) $(CXXFLAGS) $(INCLUDES) -DFP_$(PRECISION)


test: $(TARGET)
	@ $(RM) $(FOLDER)/map.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d$(DIM) -n$(VISCOSITY) -u$(VELOCITY) -i$(ITERATIONS) -e$(EVERY) -w$(LWS) -s$(STRIDE) -o -v $(FOLDER) -p $(FOLDER) -f -m

test8: $(TARGET)
	@ $(RM) $(FOLDER)/map.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d 8 -n 0.0089 -u 0.05 -i 10 -e 1 -w 32 -s 8 -o -v $(FOLDER)
	@ python3 verify.py -i 10 -e 1 -t $(SAILFISH_RES)/8 -p $(FOLDER)

test32: $(TARGET)
	@ $(RM) $(FOLDER)/map.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d 32 -n 0.0089 -u 0.05 -i 500 -e 20 -w 32 -s 32 -o -v $(FOLDER)
	@ python3 verify.py -i500 -e20 -t $(SAILFISH_RES)/32 -p $(FOLDER)


clean:
	$(RM) $(TARGET) *.o *~ $(FOLDER)/*.dump $(FOLDER)/*.vti
