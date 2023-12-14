pokemon: pokemon.cpp
		g++ -std=c++11 pokemon.cpp -o pokemon -lm -lncurses
		
clean:
		rm -f pokemon