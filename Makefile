#*****************************************************************              
# File:   Makefile                                                              
# Author: Daniel Benedí García - 20000921-T111
# Date:   
# Coms:       
#*****************************************************************              

#---------------------------------------------------------
# macros
RM = /bin/rm -rf
CC = /usr/bin/g++-11
DOCS = doxygen
#---------------------------------------------------------
# vars
EXEC = main



SRC_DIR = src
BUILD_DIR = build
TARGET = bin

SOURCES = ${EXEC}.o SWC.o K-d_tree.o heuristic.o
OBJECTS = $(patsubst %.o,$(BUILD_DIR)/%.o,$(SOURCES))

DOCS_CONFIG = doxygen-docs
GEN_DOCS = latex

#---------------------------------------------------------
# flags
CPPFLAGS= -std=c++17 -fopenmp -pthread -O3

all: $(EXEC)
#---------------------------------------------------------
$(EXEC): $(OBJECTS)
	@mkdir -p $(TARGET)
	${CC} $^ -o $(TARGET)/$@ ${CPPFLAGS}
#---------------------------------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $(CPPFLAGS) $< -o $@
#---------------------------------------------------------
docs: $(EXEC)
	${DOCS} ${DOCS_CONFIG}
#---------------------------------------------------------
clean:
	$(RM) $(TARGET) $(BUILD_DIR) ${GEN_DOCS}
