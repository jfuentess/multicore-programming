include ../../../sdsl-lite-2.0.3/Make.helper

#CXX_FLAGS= -g -std=c++11 -Wextra -O9 -DNDEBUG -lrt
CXX_FLAGS= -g -std=c++11 -O9 -DNDEBUG -lrt

INCLUDES=-isystem$(INC_DIR)
LIB=$(LIB_DIR)/libsdsl.a $(LIB_DIR)/libdivsufsort.a $(LIB_DIR)/libdivsufsort64.a
LZ_FILES=../../LZscan/algorithm/lcp.cpp ../../LZscan/algorithm/parse.cpp ../../LZscan/algorithm/divsufsort.c ../../LZscan/algorithm/common.cpp

BINS: rlz

%.o: %.cpp
	@echo " [C++] Compiling $<"
	@$(MY_CXX) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@

all: $(BINS)

clean:
	@echo " [CLN] Removing object files"
	@rm -f rlz

rlz: rlz.cpp
	@echo " [BLD] Building rlz"
	@$(MY_CXX) $(CXX_FLAGS) $(INCLUDES) -o rlz rlz.cpp $(LZ_FILES)  $(LIB)

