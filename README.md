# Introduction

BomberLua is a botgame, inspired from Robocode. You don't control the character like in any other bomberman clone; instead, you write a script which will use the information provided by the game on the playing board to move the character and attack the others (which are controlled by other scripts).

Bomberman was chosen because of the particularity of the gaming board (which is not empty as Robocode's was): bombermen need to find their path on the board and to make a way to its enemies. Dodging bombs is not always easy, because you need to find a safe place to go everytime you or an enemy drops a bomb.

The game is written in C++, using SDL, SDL_image and Lua 5.1, by remram44, Insomniak (display and graphics) and acieroid (Python interface, still WIP).

# Rules

For the moment, the playing board is fixed (11x11): grid of ROCK blocks completed by BRICK blocks, except in the 4 corners, where the bombermen begin.

Only the BRICK blocks are destroyable. Bombs have a fixed range of 1 block in each direction.

The scripts are executed in parallel in separated threads. They are loaded before the beginning of the game and the placing of bombermen; however they already have access to the gaming board. A script may use this preliminary phase of 5 seconds to calculate some informations, for instance concerning pathfinding (even if the current board size does not justify it). The ready() function must be called before the 5 seconds are spent, or the bot will not be allowed to play. The ready() function returns at the beginning of the game, when all functions are available.

The functions that allow a script to act are blocking, which means they only return when the action is finished and when the bomberman can act again. The duration of the action is:

* 0.5 seconds for a movement (move() function);
* 0.1 seconds to drop a bomb (bomb() function);

# Scripts

For the moment, only the Lua and Python scripting language are supported (with a limitation for Python; cf bug 1). The support of other languages is not planned but may be added easily (please contact me if you can add support for a new language).

During the game, bots have access to the following function:

* get_self : this function returns informations on the bomberman which is controlled by the bot. It returns a table containing two fields: posx and posy, the position of the character on the board.
* get_bombers : returns a table of all bombermen. Each bomberman is represented by a number (which identifies the bomberman for the whole game); the structure associated is the same as get_self returns.
* get_field : returns the board, in form of a table. The fields w and h indicates the size of the board, and to each string "x,y" is associated the type of the cell, which may be:
** "rock": an undestroyable wall
** "brick": a wall that may be destroyed by a bomb
** un nombre : no wall, but a bomb that will explode in that amount of time (in seconds)
** "explosion" : an explosion
** "empty" : nothing
* move : allows the character to move. You may pass "left", "right", "up" or "down" to move in the desired direction.
* bomb : drops a bomb.

# Planned

Planned features for future versions:

* Several boards, of different sizes (loading from a file and random generation);
* Bonuses, appearing randomly when a BRICK block is destroyed:
** Range: each bonus improves the range of the bombs by 1 block;
** Speed: improves the walking speed of the character.
* Saving and playing back replays.

# Downloading and installing

As many of my projects, BomberLua is hosted on Github; you will find the latest version of the sources there, along with packages for so-called 'stable' versions of the program.

# Usage

A graphical "launcher" written with Qt allows you to launch the game without using the commandline. Otherwise, you can use the following parameters:

* -i to enable graphical display
* 1 to 4 scripts to load

Examples :

    ./bomberlua -i bots/test.lua bots/scripted.lua bots/test.lua # Linux
    bomberlua.exe -i bots\test.lua bots\scripted.lua bots\test.lua # Windows
