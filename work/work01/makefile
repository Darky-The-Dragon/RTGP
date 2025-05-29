# Template Makefile for RTGP lab lecture exercises - MacOS environment
# author: Davide Gadia
# Real-Time Graphics Programming - a.a. 2024/2025
# Master degree in Computer Science
# Universita' degli Studi di Milano
#
# TO CHECK AND ADAPT FOR M Series SoC:
# - frameworks and compiler flags worked in Intel-based systems, they may need to be changed to work with the more recent processors by Apple
# - linker flags may need to be adapted to current versions and requirements of the libraries 
#   (in particular, Assimp depends on several other libraries, and new dependencies are introduced at every update)

# name of the file
FILENAME = work01

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
LDFLAGS = -L$(LDIR) -lglfw3 -lassimp -lz -lminizip -lkubazip -lpoly2tri -ldraco -lpugixml $(MACFW)

SOURCES = ../../include/glad/glad.c $(FILENAME).cpp


TARGET = $(FILENAME).out

.PHONY : all
all:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET)

.PHONY : clean
clean :
	-rm $(TARGET)
	-rm -R $(TARGET).dSYM
