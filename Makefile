UNAME_S := $(shell uname -s)

PLATFORM	= 0
DEVICE		= 0
DIM_TEST	= 8
VISCOSITY	= 0.0089
VELOCITY	= 0.05
FOLDER		= ./results
SAILFISH_RES= ./sailfish_results


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
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d$(DIM_TEST) -v$(VISCOSITY) -u$(VELOCITY) -i10 -e1 -k $(FOLDER) -p $(FOLDER) -f

test32: $(TARGET)
	@ $(RM) $(FOLDER)/map*.dump
	@ $(RM) $(FOLDER)/f_*.dump
	@ $(RM) $(FOLDER)/lbmcl.*.vti
	@ ./lbmcl -P$(PLATFORM) -D$(DEVICE) -d32 -v$(VISCOSITY) -u$(VELOCITY) -i500 -e20 -k $(FOLDER)

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
