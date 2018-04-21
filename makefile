#==========================================
# httpd makefile
#==========================================
SHELL = /bin/bash

SRC_DIR = ./code
INC_DIR = ./code

SRC_FILE := $(shell find $(SRC_DIR) -name '*.c')
SRC_FILE += $(shell find $(SRC_DIR) -name '*.cpp')

INC_FILE := $(shell find $(INC_DIR) -name '*.h')
INC_FILE += $(shell find $(INC_DIR) -name '*.hpp')

TEMP_INC_DIR := $(dir $(INC_FILE))
INC_DIR := $(foreach tmp, $(TEMP_INC_DIR), -I$(tmp))

OBJS = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SRC_FILE)))

# Target with the date time
TARGET := httpd-$(shell date --rfc-3339='date')

#compile tools
CC 	:= gcc
CXX := g++

#libs as
LIBS := -lpthread
LIBS +=

#compile flags 
CFLAGS := -w -Wall -g -O0 
CFLAGS += $(INC_DIR)
CXXFLAGS := -std=c++0x $(CFLAGS)
CFLAGS += -std=c99 

#=========================================================
all: $(TARGET)

SEE:
	@echo -e "SRC_FILE: $(SRC_FILE)"
	@echo -e "OBJS: $(OBJS)"
$(TARGET):$(OBJS) 
	@$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo  -e "\033[31m\033[1m make $@ done. \033[0m";	#red

%.o:%.c
	@$(CC) $(CFLAGS) -c $^ -o $@
	@echo -e "\033[32m\033[1m make $@ done. \033[0m";	#green

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $^ -o $@
	@echo -e "\033[32m\033[1m make $@ done. \033[0m";#green

clean:
	@rm -f $(TARGET])
	@echo  -e "\033[31m\033[1m remove $(TARGET) done. \033[0m";	#red

cleanall:
	@rm -f $(TARGET)
	@rm -f $(OBJS)
	@echo  -e "\033[31m\033[1m Removed $(TARGET) done. \033[0m";	#red
	@echo -e "\033[32m\033[1m Removed: $(OBJS) done. \033[0m";		#yellow

install:
	@$(shell rm -f /usr/bin/httpd-2018*)
	@echo -e "Removed the old version"
	@$(shell cp $(TARGET) /usr/bin/$(TARGET))
	@$(shell cp ./httpd_test.conf /etc/httpd_test.conf)
	@echo -e "\033[32m\033[1m copy $(TARGET) to /usr/bin/$(TARGET) \033[0m";	#green
	@echo -e "\033[32m\033[1m copy httpd_test.conf to /etc/httpd_test.conf \033[0m";		#green

