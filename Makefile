#*****************************************************************              
# File:   Makefile                                                              
# Author: Daniel Benedí García - 20000921-T111
# Date:   
# Coms:       
#*****************************************************************              

#---------------------------------------------------------
# macros
RM = /bin/rm -rf
CC = g++
DOCS = doxygen
#---------------------------------------------------------
# vars
EXEC = main


SRC_DIR = src
BUILD_DIR = build
TARGET = bin

OBJECTS = $(patsubst %.o,$(BUILD_DIR)/%.o)

DOCS_CONFIG = doxygen-docs
GEN_DOCS = latex

#---------------------------------------------------------
# flags
CPPFLAGS= -g -O2 -std=gnu++11 -fopenmp

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
