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
ITERATIONS	?= 500
EVERY		?= 20


ifeq ($(shell uname -s), Darwin)
	PLATFORM	= 0
	DEVICE		= 2
	CXXFLAGS	+= -Wc++11-extensions -DCL_SILENCE_DEPRECATION
	LDLIBS		+= -framework OpenCL
	FOLDER		= /Volumes/RamDisk
else
	LDLIBS		+= -lOpenCL
endif


$(TARGET): $(TARGET).cpp common.h kernels.cl # CLUtil.hpp ArgsUtil.hpp StoreUtil.hpp
	$(CXX)  -o $@ $@.cpp $(LDLIBS) $(CXXFLAGS) $(INCLUDES) -DFP_$(PRECISION)


test: $(TARGET)
	@ $(RM) $(FOLDER)/map.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d$(DIM) -n$(VISCOSITY) -u$(VELOCITY) -i$(ITERATIONS) -e$(EVERY) -v $(FOLDER)


testall: $(TARGET)
	@ $(RM) $(FOLDER)/map.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d$(DIM) -n$(VISCOSITY) -u$(VELOCITY) -i$(ITERATIONS) -e$(EVERY) -v $(FOLDER) -p $(FOLDER) -f -m

# testandverify: test
# 	@ python3 verify.py -i$(ITERATIONS) -e$(EVERY) -t${SAILFISH_RES}/8 -n${FOLDER}

# test32andverify: test32
# 	@ python3 verify.py-i$(ITERATIONS) -e$(EVERY) -t${SAILFISH_RES}/32 -n${FOLDER}

# testremote:
# 	@./remoteverify.sh test

# testremote32:
# 	@./remoteverify.sh test32

clean:
	$(RM) $(TARGET) *.o *~
