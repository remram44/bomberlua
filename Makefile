# Compilateur
CPP = g++ -g

# Répertoires d'inclusion supplémentaires
INCLUDES = -I"/usr/include/lua5.1"

# Libraries à linker
LIBS = -lSDLmain -lSDL -lSDL_image -llua5.1

# Flags supplémentaires à utiliser lors de la compilation
CPPFLAGS = $(INCLUDES) -Wall -O2 -DWITH_LUA -DWITH_PYTHON

# Fichiers-objet à générer puis linker
OBJS = main.o BomberLua.o Output.o Socket.o GameEngine.o NetworkReceiver.o Display.o LuaBomber.o PyBomber.o

# Cible (= binaire)
TARGET = bomberlua

.PHONY: all clean

all: $(TARGET)


# Linkage
$(TARGET): $(OBJS)
	$(CPP) -o $@ $(OBJS) $(LIBS)

# Fabriquer un .o à partir d'un .cpp
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# Nettoyer
clean:
	rm -f $(OBJS)

BomberLua.o: BomberLua.h Output.h Socket.h GameEngine.h NetworkReceiver.h
Display.o: Display.h BomberLua.h
GameEngine.o: GameEngine.h BomberLua.h lua.hpp
main.o: BomberLua.h
NetworkReceiver.o: NetworkReceiver.h BomberLua.h Socket.h
Output.o: Output.h BomberLua.h Socket.h
Socket.o: Socket.h
