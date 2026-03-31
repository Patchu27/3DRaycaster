g++ --std=c++20 -c main.cpp -I/usr/include/SFML-3.0.2/include && g++ main.o -o App -L/usr/include/SFML-3.0.2/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio && export LD_LIBRARY_PATH=/usr/include/SFML-3.0.2/lib && ./App

Use your mouse cursor to move the player around the map. Currently, view rotation is not implemented, but will be in later versions. This is a demo intented to demonstrate the efficacy of this rendering method.

    --Patchu27
