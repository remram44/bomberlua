# Compiler
CPP = g++ -g

# Additional include directories
INCLUDES = -I"/usr/include/lua5.1"

# Libraries to be linked
LIBS = -lSDLmain -lSDL -lSDL_image -llua5.1

# Additional flags to use during compilation
CPPFLAGS = $(INCLUDES) -Wall -O2 -DWITH_LUA -DWITH_PYTHON

# Object files to generate then link
OBJS = main.o BomberLua.o Output.o Socket.o GameEngine.o NetworkReceiver.o Display.o LuaBomber.o PyBomber.o

# Target (= binary)
TARGET = bomberlua

.PHONY: all clean

all: $(TARGET)


# Link
$(TARGET): $(OBJS)
	$(CPP) -o $@ $(OBJS) $(LIBS)

# Make a .o from a .cpp
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# Clean up
clean:
	rm -f $(OBJS)

BomberLua.o: BomberLua.h Output.h Socket.h GameEngine.h NetworkReceiver.h
Display.o: Display.h BomberLua.h
GameEngine.o: GameEngine.h BomberLua.h lua.hpp
main.o: BomberLua.h
NetworkReceiver.o: NetworkReceiver.h BomberLua.h Socket.h
Output.o: Output.h BomberLua.h Socket.h
Socket.o: Socket.h
