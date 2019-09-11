UNAME_S := $(shell uname -s)

CXX			= g++
CXXFLAGS	= -std=c++11 -Wall -pedantic -g
LDLIBS		=
INCLUDES	= -I./libs
TARGET		= lbmcl


ifeq ($(UNAME_S), Darwin)
	CXXFLAGS	+= -Wc++11-extensions -DCL_SILENCE_DEPRECATION
	LDLIBS		+= -framework OpenCL
else
	LDLIBS		+= -lOpenCL
endif

$(TARGET): $(TARGET).cpp $(OBJS)
	$(CXX)  -o $@ $^ $(LDLIBS) $(CXXFLAGS) $(INCLUDES)

# %.o: %.cpp
# 	$(CXX) -c $< $(LDLIBS) $(CXXFLAGS) $(INCLUDES)

test: $(TARGET)
	$(RM) /Volumes/RamDisk/velocity.*
	$(RM) /Volumes/RamDisk/rho.*
	./lbmcl 0 2 100 2

clean:
	$(RM) $(TARGET) *.o *~ 
