SRIPATH = /home/student/04/b04902077/DSP2017/srilm-1.5.10
MACHINE_TYPE = i686-m64
LM = bigram.lm
CXX = g++
CXXFLAGS = -O3 -I$(SRIPATH)/include
vpath lib%.a $(SRIPATH)/lib/$(MACHINE_TYPE)

TARGET = mydisambig
SRC = mydisambig.cpp
OBJ = $(SRC:.cpp=.o)
NUMBERS = 1 2 3 4 5 6 7 8 9 10
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ) -loolm -ldstruct -lmisc
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	$(RM) $(OBJ) $(TARGET)

run: 
	@$(MAKE)  
	mkdir -p result2
	$(foreach var,$(NUMBERS),./$(TARGET) $(LM) Zhuyin-Big5.map testdata/$(var).txt > result2/$(var).txt;)

map:
	python mapcov.py 
