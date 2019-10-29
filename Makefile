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
PRECISION	= DOUBLE # SINGLE or DOUBLE

UNAME_S := $(shell uname -s)


ifeq ($(UNAME_S), Darwin)
	PLATFORM	= 0
	DEVICE		= 2
	CXXFLAGS	+= -Wc++11-extensions -DCL_SILENCE_DEPRECATION
	LDLIBS		+= -framework OpenCL
	FOLDER		= /Volumes/RamDisk
else
	LDLIBS		+= -lOpenCL
endif

%.hpp: ;

$(TARGET): $(TARGET).cpp common.h kernels.cl #CLUtil.hpp ArgsUtil.hpp StoreUtil.hpp
	$(CXX)  -o $@ $@.cpp $(LDLIBS) $(CXXFLAGS) $(INCLUDES) -DFP_$(PRECISION)


test8: $(TARGET)
	@ $(RM) $(FOLDER)/map.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d8 -n$(VISCOSITY) -u$(VELOCITY) -i10 -e1 -v $(FOLDER) -p $(FOLDER) -f -m

test32: $(TARGET)
	@ $(RM) $(FOLDER)/map.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d32 -n$(VISCOSITY) -u$(VELOCITY) -i500 -e20 -v $(FOLDER)

testandverify: test
	@ python3 verify.py -i10 -e1 -t${SAILFISH_RES}/8 -n${FOLDER}

test32andverify: test32
	@ python3 verify.py -i500 -e20 -t${SAILFISH_RES}/32 -n${FOLDER}

testremote:
	@./remoteverify.sh test

testremote32:
	@./remoteverify.sh test32

clean:
	$(RM) $(TARGET) *.o *~
