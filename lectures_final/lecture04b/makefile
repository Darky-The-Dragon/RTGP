# Template Makefile for RTGP lab lecture exercises - MacOS environment - TO CHECK AND ADAPT FOR M1 AND M2 SYSTEMS
# author: Davide Gadia
# Real-Time Graphics Programming - a.a. 2024/2025
# Master degree in Computer Science
# Universita' degli Studi di Milano

# name of the file
FILENAME = lecture04b

# Xcode compiler
CXX = clang++

# Include path
IDIR = ../../include

# Libraries path
LDIR = ../../libs/mac

# MacOS frameworks
MACFW = -framework OpenGL -framework IOKit -framework Cocoa -framework CoreVideo

# compiler flags:
CXXFLAGS  = -g -O0 -x c++ -mmacosx-version-min=15.0 -Wall -Wno-invalid-offsetof -std=c++11 -I$(IDIR)

# linker flags:
LDFLAGS = -L$(LDIR) -lglfw3 -lassimp -lz -lminizip -lkubazip -lpoly2tri -ldraco -lpugixml -lpolyclipping $(MACFW)

SOURCES = ../../include/glad/glad.c $(FILENAME).cpp


TARGET = $(FILENAME).out

.PHONY : all
all:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET)

.PHONY : clean
clean :
	-rm $(TARGET)
	-rm -R $(TARGET).dSYM
