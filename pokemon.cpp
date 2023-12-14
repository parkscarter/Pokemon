#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <ncurses.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <filesystem>

//Global variable, initially set to 1, set to zero to quit game
int quitFlag = 1;
int flyFlag = 1;

/*
Struct representing the relationship between a pokemon and it's moves
Sorted by pokemonId, this has one instance for each move that each pokemon
could have.
*/
typedef struct Pokemon_moves{
    int pokemonId;
    int version_group;
    int moveId;
    int pokemon_move_method;
    int level;
    int order;
} Pokemon_moves;

//Global vector of 
std::vector<Pokemon_moves*> pokemon_moves;

/*
Struct giving all details associated with a move, including move_id, 
which links to pokemon via Pokemon_moves
*/
typedef struct Moves{
    int id;
    std::string name;
    int generation;
    int type;
    int power;
    int pp;
    int accuracy;
    int priority;
    int target;
    int damageClassId;
    int effectId;
    int effectChance;
    int contestType;
    int contestEffect;
    int superContestEffect;

    //default constructor
    Moves() : id(0) {}

    //Copy Constructor
    Moves(const Moves& other){
        this->id = other.id;
        this->name = other.name;
        this->generation = other.generation;
        this->type = other.type;
        this->power = other.power;
        this->pp = other.pp;
        this->accuracy = other.accuracy;
        this->priority = other.priority;
        this->target = other.target;
        this->damageClassId = other.damageClassId;
        this->effectId = other.effectId;
        this->effectChance = other.effectChance;
        this->contestType = other.contestType;
        this->contestEffect = other.contestEffect;
        this-> superContestEffect = other.superContestEffect;
    }
} Moves;

//Global vector of Moves
std::vector<Moves*> moves;

//Gives a base stat for each pokemon and all of their stats
typedef struct Pokemon_stats{
    int pokemon_id;
    int stat_id;
    int base_stat;
    int effort;
} Pokemon_stats;

//Global vectors are declared for all db structs
std::vector<Pokemon_stats*> pokemon_stats;

//links Pokemon_stats to stat information?
typedef struct Stats{
    int id;
    int damage_class;
    std::string name;
    int is_battle_only;
    int game_idx;
} Stats;

std::vector<Stats*> stats;

/*
struct representing a Pokemon, I've added some member variables from what is
imported via the CSV
*/
typedef struct Pokemon{
    int id;
    std::string name;
    int species;
    int height;
    int weight;
    int base_xp;
    int order;
    int is_default;
    int hp;
    int attack;
    int defense;
    int speed;
    int s_attack;
    int s_defense;

    int baseSpeed;
    int maxHp;
    std::string isShiny;
    std::string gender;
    int level;
    std::vector<Moves*> moveSet;
    int xp;
    
    //default  constructor
    Pokemon() : id(0), species(0), height(0), weight(0), base_xp(0), order(0), is_default(0), level(1) {}

    //copy Constructor
    Pokemon(const Pokemon& other){
        this->id = other.id;
        this->name = other.name;
        this->species = other.species;
        this->height = other.height;
        this->weight = other.weight;
        this->base_xp = other.base_xp;
        this->order = other.order;
        this->is_default = other.is_default;
        this->hp = other.hp;
        this->attack = other.attack;
        this->defense = other.defense;
        this->speed = other.speed;
        this->s_attack = other.s_attack;
        this->s_defense = other.s_defense;
        
        this->baseSpeed = other.baseSpeed;
        this->maxHp = other.maxHp;
        this->gender = other.gender;
        this->isShiny = other.isShiny;
        this->level = other.level;
        this->xp = other.xp;

        for (Moves* move : other.moveSet) {
            Moves* newMove = new Moves(*move);
            this->moveSet.push_back(newMove);
        }
    }

    //method to choose possible moves, and randomly choose two to add to moveSet, which is a vector containing no more than 2 moves
    void setMoves(){
        size_t i;
        std::vector<Moves*> allPosMoves;
        //For all moves in pokemon_moves, if pokemonId matches and mokemonMoveMethod == 1 and level is within range
        for (Pokemon_moves* pokemonMove: pokemon_moves){
            if (pokemonMove->pokemonId == this->species && pokemonMove->pokemon_move_method == 1 && pokemonMove->level <= this->level){
                auto moveIt = std::find_if(moves.begin(), moves.end(),
                [pokemonMove](Moves* move) { return move->id == pokemonMove->moveId; });

                // Check if the move is found
                if (moveIt != moves.end()) {
                    allPosMoves.push_back(*moveIt);
                }
            }
        }
        int minLevel;
        minLevel = 100;
        //Sets pokemon's moves to moves which are in the pokemon's appropriate moveset, determined by it's level
        //Also upgrades a pokemon to the minimum level necessary to learn a move if there are no moves in it's level 1 moveset
        if (allPosMoves.empty()){
            for (Pokemon_moves* pokemonMove: pokemon_moves){
                if (pokemonMove->pokemonId == this->species && pokemonMove->pokemon_move_method == 1 && pokemonMove->level < minLevel){
                    minLevel = pokemonMove->level;
                    auto moveIt = std::find_if(moves.begin(), moves.end(),
                    [pokemonMove](Moves* move) { return move->id == pokemonMove->moveId; });

                    // Check if the move is found
                    if (moveIt != moves.end()) {
                        allPosMoves.push_back(*moveIt);
                    }
                }
            }
            this->level = minLevel;
        }
       
        else{
            int randMove = rand() % allPosMoves.size();
            this->moveSet.push_back(allPosMoves[randMove]);
            int newRandMove = rand() % allPosMoves.size();
            int unique = 1;
            for (i = 0; i < allPosMoves.size(); i ++){
                if (allPosMoves[i]->name != allPosMoves[randMove]->name){
                    unique = 0;
                }
            }
            if (allPosMoves.size() > 1 && unique == 0){
                while (allPosMoves[newRandMove]->name == allPosMoves[randMove]->name){
                    newRandMove = rand() % allPosMoves.size();
                }
                this->moveSet.push_back(allPosMoves[newRandMove]);
            }
        }
    }

    //Method to set stats for a pokemon, also sets things like isShiny and gender as strings
    void setStats(){
        if (rand() % 8192 == 1){            // 1/8192 chance of shiny
            this->isShiny = "Shiny!";
        }
        else{
            this->isShiny = "not shiny";
        }

        if (rand() % 2 == 1){
            this->gender = "male";
        }
        else{
            this->gender = "female";
        }
        //Iterates through all pokemon stats, if the stat's pokemon_id matches this pokemon, set the appropriate stats like hp, attack, speed, etc.
        for(Pokemon_stats* PokemonStat : pokemon_stats){
            if(PokemonStat->pokemon_id == this->id){
                if (PokemonStat->stat_id == 1){
                    this->hp = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + this->level + 10);
                }
                else if (PokemonStat->stat_id == 2){
                    this->attack = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + 5);
                }
                else if (PokemonStat->stat_id == 3){
                    this->defense = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + 5);
                }
                else if (PokemonStat->stat_id == 4){
                    this->speed = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + 5);
                }
                else if (PokemonStat->stat_id == 5){
                    this->s_attack = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + 5);
                }
                else if (PokemonStat->stat_id == 6){
                    this->s_defense = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + 5);
                }
            }
        }
        this->maxHp = this->hp;
        this->xp = 1;
    }

    //This method is called when a pokemon levels up, this method doesn't change the pokemon's xp
    void resetStats(){
        for(Pokemon_stats* PokemonStat : pokemon_stats){
            if(PokemonStat->pokemon_id == this->id){
                if (PokemonStat->stat_id == 1){
                    this->hp = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + this->level + 10);
                }
                else if (PokemonStat->stat_id == 2){
                    this->attack = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + 5);
                }
                else if (PokemonStat->stat_id == 3){
                    this->defense = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + 5);
                }
                else if (PokemonStat->stat_id == 4){
                    this->speed = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + 5);
                }
                else if (PokemonStat->stat_id == 5){
                    this->s_attack = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + 5);
                }
                else if (PokemonStat->stat_id == 6){
                    this->s_defense = (floor((((PokemonStat->base_stat + rand() % 16) * 2) * this->level) / 100) + 5);
                }
            }
        }
        this->maxHp = this->hp;
    }
} Pokemon;

/*
This struct represents some information about a pokemon's species. Our program simply uses species to determine a damage multiplier
*/
typedef struct Pokemon_species{
    int id;
    std::string name;
    int generation;
    int evolves_from_species;
    int evolution_chain;
    int color;
    int shape;
    int habitat;
    int gender_rate;
    int capture_rate;
    int base_happiness;
    int is_baby;
    int hatch_counter;
    int has_gender_differences;
    int growth_rate;
    int forms_switchable;
    int is_legendary;
    int is_mythical;
    int order;
    int conquest_order;
} Pokemon_species;

/*
The following (3) structs are read from the db, but not useful in my application of the game
*/
typedef struct Experience {
    int growth_rate;
    int level;
    int experience;
} Experience;

typedef struct Type_names{
    int type_id;
    int local_language;
    std::string name;
} Type_names;


typedef struct Pokemon_types{
    int pokemon_id;
    int type_id;
    int slot;
} Pokemon_types;

/*
Characters represent both pc and npc characters on the map, these objects hold a party of 6 pokemon, and some items
*/
class Character{
public:
    std::vector<Pokemon*> pokemonArray;
    int num_pokemon;
    Character() : pokemonArray(6, nullptr), num_pokemon(0) {}

    ~Character() {
        // Deallocate memory in the destructor
        for (int i = 0; i < 6; i++) {
            delete pokemonArray[i];
        }
    }

    //Adds a pokemon to the party at index if the player has less than 6 pokemon
    void addPokemon(const Pokemon& newPokemon, int index) {
        // Check if the index is valid
        if (index >= 0 && index < 6) {
            Pokemon* newPokemonInstance = new Pokemon(newPokemon);
            if (newPokemonInstance != nullptr) {
                pokemonArray[index] = newPokemonInstance;
                num_pokemon++;
            } 
            else {
                std::cerr << "Error: Memory allocation failed for new Pokemon." << std::endl;
            }
        }
    }

    int time_pen;
    int mapX;
    int mapY;
    int worldX;
    int worldY;
    char type;
    int nextX;      
    int nextY;
    char direction;
    char terrain;
    int defeated;       //1 if still alive, 0 if defeated
    int numPot;
    int numRev;
    int numBal;
    int numCoins;
};

class PC : public Character{
   
};

class NPC : public Character{

};

//A map object holds information about the current game state, including the terrain map the user can see
class Map{
public:
    Character** characters;
    int numCharacters;
    Map(int numTrainers){
        initialized = 1;
        numCharacters = numTrainers + 1;
        characters = static_cast<Character**>(malloc((numCharacters) * sizeof(Character*)));
        if (characters == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
        }
        for (int i = 0; i < numCharacters; i++) {
            characters[i] = new Character();
            if (characters[i] == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
            }
        }
    }
    ~Map() {
        // Deallocate memory in the destructor
        for (int i = 0; i < numCharacters; i++) {
            free(characters[i]);
            characters[i] = nullptr;
        }
        free(characters);
        characters = nullptr;
    }
    char grid [80][21];
    char printArr [80][21];
    int hdist [80][21];
    int rdist [80][21];
    int northGate;
    int southGate;
    int westGate;
    int eastGate;
    int worldx;
    int worldy;
    int initialized;
};


std::vector<Pokemon*> pokemon;
std::vector<Pokemon_species*> pokemon_species;
std::vector<Experience*> experience;
std::vector<Type_names*> type_names;
std::vector<Pokemon_types*> pokemon_types;
std::vector<Pokemon*> storedPokemon; 

void updatehDist(Map* m, Character * c);

void updaterDist(Map* m, Character * c);

int findNextMove(Map*** mapArray, Character *** players, Character * c, Map ** m, int numPlayers, int* worldX, int* worldY);

int setTimePen(Character * c, Map * m);

void printMap(Map* m, Character ** players, int numPlayers, int worldX, int worldY);

void createMap(int n, int s, int e, int w, Map* m, int x, int y);

void placeEnemies(Map* m, int numEnemies, Character *** players);

//Returns the position of the above map's south gate, or 0 if null
int checkNorth(Map*** mapArray, int x, int y){
    if (y != 0){
        if (mapArray[x][y - 1] == NULL){
            return 0;
        }
        else{
            return mapArray[x][y - 1]->southGate;
        }
    }
    return 0;
}

int checkSouth(Map*** mapArray, int x, int y){
    if (y != 400){
        if (mapArray[x][y + 1] == NULL){
            return 0;
        }
        else{
            return mapArray[x][y + 1]->northGate;
        }
    }
    return 0;
}

int checkWest(Map*** mapArray, int x, int y){
    if (x != 0){
        if (mapArray[x - 1][y] == NULL){
                return 0;
            }
            else{
                return mapArray[x - 1][y]->eastGate;
            }
    }
    return 0;
}

int checkEast(Map*** mapArray, int x, int y){
    if (x != 400){
        if (mapArray[x + 1][y] == NULL){
            return 0;
        }
        else{
            return mapArray[x + 1][y]->westGate;
        }
    }
    return 0;
}

    //Return 0 if True
int isValid(int x, int y){
    if (x >= 0 && x <= 79 && y >= 0 && y <= 20){
        return 0;
    }
    return 1;
}

int isOnMap(int x, int y){
    if (x > 0 && x < 79 && y > 0 && y < 20){
        return 0;
    }
    else if (x == 0){
        return 1;
    }
    else if (x == 79){
        return 3;
    }
    else if (y == 0){
        return 2;
    }
    else if (y == 20){
        return 4;
    }
    else {
        return 5;
    }
}


    //Return 0 if True
int placeValid(Map * m, int x, int y, char type){
    if (m->grid[x][y] == '#' || m->grid[x][y] == '.' || m->grid[x][y] == 'M' || m->grid[x][y] == 'C' || m->grid[x][y] == ':'){
        return 0;
    }
    else if(m->grid[x][y] == '^'){
        if (type == 'h'){
            return 0;
        }
        else{
            return 1;
        }
    }
    else{
        return 1;
    }
}

//Returns 0 if space is free, 1 otherwise
int spaceFree(Character *** characters, int numPlayers, int x, int y){
    int i;
    for (i = 0; i < numPlayers; i ++){
        if ((*characters)[i]->mapX == x && (*characters)[i]->mapY == y){
            return 1;
        }
    }
    return 0;
}

//Runs a for loop to check number of available pokemon
int checkPcPokemon(Character* c){
    int i, numPoke;
    numPoke = 0;
    for (i = 0; i < 6; i++){
        if (c->pokemonArray[i] != nullptr && c->pokemonArray[i]->hp > 0){
            numPoke ++;
        }
    }
    return numPoke;
}

int checkPokemonMoves(Pokemon* p){
    int i, numMoves;
    numMoves = 0;
    for (i = 0; i < 2; i ++){
        if (p->moveSet[i] != nullptr){
            numMoves ++;
        }
    }
    return numMoves;
}

int hasDefeatedPlayer(Map * m, Character ** characters, int numPlayers, int x, int y){
    int i;
    for (i = 0; i < numPlayers; i ++){
        if (characters[i]->mapX == x && characters[i]->mapY == y){
            if (characters[i]->defeated == 0 || checkPcPokemon(characters[0]) < 1){
                return 0;
            }
        }
    }
    return 1;
}

int checkLevelUp(Character* c){
    int i;
    for (i = 0; i < 6; i++){
        if (c->pokemonArray[i] != nullptr){
            while (c->pokemonArray[i]->xp >= ((c->pokemonArray[i]->level + 1) * (c->pokemonArray[i]->level + 1))){
                c->pokemonArray[i]->level += 1;
                c->pokemonArray[i]->resetStats();
            }
        }
    }
    return 0;
}

int calculateMDist(int x, int y){
    x -= 200;
    y -= 200;
    int absX = std::abs(x);
    int absY = std::abs(y);
    return (absX + absY);
}

int changeMap(Map*** mapArray, Map ** m, int direction, int numPlayers, int* worldX, int* worldY){
    int newX, newY, oldPCx, oldPCy, newPCx, newPCy, i, pot, rev, bal, numPokemon, num_coin;
    std::vector<Pokemon*> pokemon;
    pokemon = (*m)->characters[0]->pokemonArray;
    oldPCx = (*m)->characters[0]->mapX;
    oldPCy = (*m)->characters[0]->mapY;
    pot = (*m)->characters[0]->numPot;
    rev = (*m)->characters[0]->numRev;
    bal = (*m)->characters[0]->numBal;
    numPokemon = (*m)->characters[0]->num_pokemon;
    num_coin = (*m)->characters[0]->numCoins;


    if (direction == 1 && (*m)->worldx > 0){       //WEST
        newX = (*m)->worldx - 1;
        newY = (*m)->worldy;
        newPCx = 78;
        newPCy = oldPCy;
    }
    else if (direction == 2 && (*m)->worldy > 0){   //NORTH
        newX = (*m)->worldx;
        newY = (*m)->worldy - 1;
        newPCx = oldPCx;
        newPCy = 19;
    }
    else if (direction == 3 && (*m)->worldx < 400){   //EAST
        newX = (*m)->worldx + 1;
        newY = (*m)->worldy;
        newPCx = 1;
        newPCy = oldPCy;
    }
    else if (direction == 4 && (*m)->worldy < 400){   //SOUTH
        newX = (*m)->worldx;
        newY = (*m)->worldy + 1;
        newPCx = oldPCx;
        newPCy = 1;
    }

    *worldX = newX;
    *worldY = newY;

    *m = mapArray[newX][newY];

    (*m)->characters[0]->mapX = newPCx;
    (*m)->characters[0]->mapY = newPCy;
    (*m)->characters[0]->time_pen = 5;
    (*m)->worldx = newX;
    (*m)->worldy = newY;
    (*m)->characters[0]->num_pokemon = numPokemon;
    (*m)->characters[0]->pokemonArray = pokemon;
    (*m)->characters[0]->numCoins = num_coin;

    refresh();

    updatehDist(*m, (*m)->characters[0]);
    updaterDist(*m, (*m)->characters[0]);

    if ((*m)->initialized == 1){
        createMap(checkNorth(mapArray, newX, newY), checkSouth(mapArray, newX, newY), checkEast(mapArray, newX, newY), checkWest(mapArray, newX, newY), *m, newX, newY);
        (*m)->characters[0]->type = '@';
        (*m)->characters[0]->defeated = 1;
        (*m)->characters[0]->numBal = bal;
        (*m)->characters[0]->numRev = rev;
        (*m)->characters[0]->numPot = pot;
        (*m)->worldx = newX;
        (*m)->worldy = newY;
        placeEnemies(*m, numPlayers - 1, &((*m)->characters));
    }

    refresh();

    printMap(*m, (*m)->characters, numPlayers, *worldX, *worldY);

    for (i = 0; i < numPlayers; i++){
        findNextMove(mapArray, &(mapArray[newX][newY]->characters), mapArray[newX][newY]->characters[i], &mapArray[newX][newY], numPlayers, worldX, worldY);
    }

    return 0;

}

void checkAllMoves(Map*** mapArray, Character *** players, int numPlayers, Map * m, int * worldX, int * worldY){
    int i, j;
    //Nested for loop checks all players against eachother, finds new next move if next move points to 
    //where a player is
    for (i = 0; i < numPlayers; i ++){
        for (j = 0; j < numPlayers; j ++){
            if (j != i){
                if ((*players)[i]->nextX == (*players)[j]->mapX && (*players)[i]->nextY == (*players)[j]->mapY){
                    findNextMove(mapArray, players, (*players)[i], &m, numPlayers, worldX, worldY);
                }
            }
        }
    }
}

int selectSwap(Character * c){
    int i, chosenPokemon;
    for (i = 0; i < 25; i ++){
        move(i,0);
        clrtoeol();
    }
    mvprintw(0,0, "Choose one of your pokemon to swap into the Pokedex (press '1', '2', etc.)");
    for (i = 0; i < c->num_pokemon; i ++){
        mvprintw(i + 1,0, "%d: %s; level %d", i + 1, c->pokemonArray[i]->name.c_str(), c->pokemonArray[i]->level);
    }

    refresh();
    
    chosenPokemon = -1;

    while((chosenPokemon < 0 || chosenPokemon > c->num_pokemon - 1)){
        chosenPokemon = getch() - '0' - 1;  
    }
    return chosenPokemon;

}

int swapPokemon(Character * c){
    size_t i, scrollPos;
    int ui, num;
    scrollPos = 0;
    ui = 'a';
    while (1){

        clear();

        mvprintw(0, 0, "Here is your Pokedex: (use arrow keys, press enter to select)");

        for (i = 0; i < storedPokemon.size() && i < 23; i ++){

            if (i == scrollPos) {
                attron(A_REVERSE); // Turn on reverse video
            }

            mvprintw(i+1, 0, "%s; level:%d", storedPokemon[i]->name.c_str(), storedPokemon[i]->level);

            if (i == scrollPos) {
                attroff(A_REVERSE); // Turn off reverse video after printing the line
            }
        }
        refresh();

        ui = getch();
        if (ui == KEY_UP && scrollPos > 0){
            scrollPos -= 1;
        }
        else if(ui == KEY_DOWN && scrollPos < storedPokemon.size() - 1){
            scrollPos += 1;
        }
        else if(ui == '\n'){
            num = selectSwap(c);
            Pokemon* temp = new Pokemon(*storedPokemon[scrollPos]);
            storedPokemon[scrollPos] = new Pokemon(*c->pokemonArray[num]);
            c->pokemonArray[num] = new Pokemon(* temp);
            mvprintw(7,0, "Successfully swapped, %s is now in your active deck", c->pokemonArray[num]->name.c_str());
            mvprintw(8,0, "%s was moved to your Pokedex", storedPokemon[scrollPos]->name.c_str());

            return 0;
        }
        else if (ui == 27){
            return 0;
        }

        //for (j = 0; j < storedPokemon.size(); j++){
        //    mvprintw(j+7, 0, "%d: %s; level: %d", j, storedPokemon[j]->name.c_str(), storedPokemon[j]->level);
        //}
    }
}

int enterPokeMart(Character * c){
    int i, ui;
    ui = 'a';
    for (i = 0; i < 24; i ++){
        move(i,0);
        clrtoeol();
    }
    
    mvprintw(0, 0, "Entered PokeMart! (press < to exit)");
    mvprintw(1, 0, "Here you can buy items, you have %d coins", c->numCoins);
    mvprintw(2, 0, "1: Purchase Revive (two coins)");
    mvprintw(3, 0, "2: Purchase Potion (one coin)");
    mvprintw(4, 0, "3: Purchase Pokeball (three coins)");
    mvprintw(6, 0, "Press 'p' to swap pokemon from 'Pokedex'");

    refresh();
    while (ui != '<'){
        ui = 'a';
        while (ui != '1' && ui != '2' && ui != '3' && ui != '<' && ui != 'p'){
            ui = getch();
        }
        if (ui == '1'){
            if (c->numCoins > 1){
                c->numRev ++;
                c->numCoins -= 2;
            }
            else{
                mvprintw(5,0, "You don't have enough money!");
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            
        }
        else if (ui == '2'){
            if (c->numCoins > 0){
                c->numPot ++;
                c->numCoins --;
            }
            else{
                mvprintw(5,0, "You don't have enough money!");
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
        }
        else if (ui == '3'){
            if (c->numCoins > 2){
                c->numBal ++;
                c->numCoins -= 3;
            }
            else{
                mvprintw(5,0, "You don't have enough money!");
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
        }
        else if (ui == 'p' && storedPokemon.size() > 0){
            swapPokemon(c);
            for (i = 0; i < 7; i ++){
                move(i,0);
                clrtoeol();
            }
    
            mvprintw(0, 0, "Entered PokeMart! (press < to exit)");
            mvprintw(1, 0, "Here you can buy items, you have %d coins", c->numCoins);
            mvprintw(2, 0, "1: Purchase Revive (two coins)");
            mvprintw(3, 0, "2: Purchase Potion (one coin)");
            mvprintw(4, 0, "3: Purchase Pokeball (three coins)");
            mvprintw(6, 0, "Press 'p' to swap pokemon from 'Pokedex'");

            refresh();
        }
        move(1,0);
        clrtoeol();
        mvprintw(1, 0, "Here you can buy items, you have %d coins", c->numCoins);
        refresh();
    }

    return 0;
}

int enterPokeCenter(Character* c){
    int i;

    for(i = 0; i < 6; i ++){
        if (c->pokemonArray[i] != nullptr){
            c->pokemonArray[i]->hp = c->pokemonArray[i]->maxHp;
        }
    }

    for (i = 0; i < 24; i ++){
        move(i,0);
        clrtoeol();
    }
    
    mvprintw(0, 0, "Entered PokeCenter! All of your pokemon have been restored to max hp!");
    mvprintw(1, 0, "(Press '<' to exit)    You have %d coins", c->numCoins);
    mvprintw(2, 0, "Here, you can also level up your pokemon (select number)");
    for (i = 0; i < c->num_pokemon; i++){
        mvprintw(i + 3, 0, "%d: %s; level %d (%d coins)", i + 1, c->pokemonArray[i]->name.c_str(), c->pokemonArray[i]->level, 2 * (c->pokemonArray[i]->level + 1));
    }
    refresh();
    int ui = 'a';
    while (ui != '<'){
        ui = getch();
        if (ui - '1' < c->num_pokemon && ui - '1' >= 0 && c->numCoins >= 2 * (c->pokemonArray[ui - '1']->level + 1)){        //if ui = '1' or '2' etc (num_pokemon)
            c->pokemonArray[ui - '1']->level += 1;
            c->pokemonArray[ui - '1']->xp = c->pokemonArray[ui - '1']->level * c->pokemonArray[ui - '1']->level;
            c->pokemonArray[ui - '1']->resetStats();
            c->numCoins -= (2 * (c->pokemonArray[ui - '1']->level));

            for (i = 0; i < 24; i ++){
                move(i,0);
                clrtoeol();
            }
    
            mvprintw(0, 0, "Entered PokeCenter! All of your pokemon have been restored to max hp!");
            mvprintw(1, 0, "(Press '<' to exit)    You have %d coins", c->numCoins);
            mvprintw(2, 0, "Here, you can also level up your pokemon (select number)");
            for (i = 0; i < c->num_pokemon; i++){
                mvprintw(i + 3, 0, "%d: %s; level %d (%d coins)", i + 1, c->pokemonArray[i]->name.c_str(), c->pokemonArray[i]->level, 2 * (c->pokemonArray[i]->level + 1));
            }
            refresh();
        }     
    }
    return 0;
}

int swapPcPokemon(Character* c){
    int i, chosenPokemon;
    for (i = 0; i < 10; i++){
        move(i,0);
        clrtoeol();
    }
    mvprintw(1,0, "Select a pokemon (type number to choose):");
    for (i=2; i < c->num_pokemon + 2; i++){
        if (c->pokemonArray[i-2] != nullptr){
            mvprintw(i,0, "%d: %s; hp: %d/%d", i - 1, c->pokemonArray[i-2]->name.c_str(), c->pokemonArray[i-2]->hp, c->pokemonArray[i-2]->maxHp);
        }
    }
    refresh();
    chosenPokemon = -1;
    while((chosenPokemon < 0 || chosenPokemon > c->num_pokemon - 1) || c->pokemonArray[chosenPokemon]->hp <= 0){
        chosenPokemon = getch() - '0' - 1;  
    }
    return chosenPokemon;
}

int revPokemon(Character* c){
    int i, chosenPokemon;
    for (i = 0; i < 10; i++){
        move(i,0);
        clrtoeol();
    }
    mvprintw(1,0, "Revive a pokemon (type number to choose):");
    for (i=2; i < c->num_pokemon + 2; i++){
        if (c->pokemonArray[i-2] != nullptr){
            mvprintw(i,0, "%d: %s; hp: %d/%d", i - 1, c->pokemonArray[i-2]->name.c_str(), c->pokemonArray[i-2]->hp, c->pokemonArray[i-2]->maxHp);
        }
    }
    refresh();
    chosenPokemon = -1;
    while((chosenPokemon < 0 || chosenPokemon > c->num_pokemon - 1)){
        chosenPokemon = getch() - '0' - 1;  
    }
    return chosenPokemon;
}

int healPokemon(Character* c){
    int i, chosenPokemon;
    for (i = 0; i < 10; i++){
        move(i,0);
        clrtoeol();
    }
    mvprintw(1,0, "Heal a pokemon (type number to choose):");
    for (i=2; i < c->num_pokemon + 2; i++){
        if (c->pokemonArray[i-2] != nullptr){
            mvprintw(i,0, "%d: %s; hp: %d/%d", i - 1, c->pokemonArray[i-2]->name.c_str(), c->pokemonArray[i-2]->hp, c->pokemonArray[i-2]->maxHp);
        }
    }
    refresh();
    chosenPokemon = -1;
    while((chosenPokemon < 0 || chosenPokemon > c->num_pokemon - 1) || c->pokemonArray[chosenPokemon]->hp < 0){
        chosenPokemon = getch() - '0' - 1;  
    }
    return chosenPokemon;
}

void printTrainerBattle(Character* pc, Character* npc, int op, int mp){
    int i, numMoves;
    for (i = 0; i < 30; i ++){
        move(i,0);
        clrtoeol();
    }
    numMoves = checkPokemonMoves(pc->pokemonArray[mp]);
    mvprintw(0,0, "In battle with other trainer %c's %s; hp: %d/%d", npc->type, npc->pokemonArray[op]->name.c_str(), npc->pokemonArray[op]->hp, npc->pokemonArray[op]->maxHp);
    mvprintw(1,0, "Your current pokemon: %s; hp: %d/%d", pc->pokemonArray[mp]->name.c_str(), pc->pokemonArray[mp]->hp, pc->pokemonArray[mp]->maxHp);
    mvprintw(4,0, "'b': open bag,'p': switch pokemon, 's': view the other trainer's pokemon");
    for (i = 1; i < numMoves + 1; i++){
        mvprintw(i + 1,0,"choose move %d: %s; move power: %d", i, pc->pokemonArray[mp]->moveSet[i - 1]->name.c_str(), pc->pokemonArray[mp]->moveSet[i - 1]->power);
    }
    refresh();
}

int showNpcPokemon(Character * npc){
    int i;
    char buff [256];
    for (i = 0; i < 30; i++){
        move(i,0);
        clrtoeol();
    }
    
    mvprintw(2,0,"Press any key to esc");
    for (i = 0; i < npc->num_pokemon; i ++){
        //snprintf(buff, sizeof(buff), "name: %s, level: %d", npc->pokemonArray[i]->name.c_str(), npc->pokemonArray[i]->level);
        snprintf(buff, sizeof(buff), "POKEMON %d: \n    %s (%s), level: %d, move 1: %s, move 2: %s \n    stats: hp: %d, attack: %d, defense: %d, speed: %d\n    s_attack: %d, s_defense: %d, gender: %s, xp: %d", i + 1, npc->pokemonArray[i]->name.c_str(), npc->pokemonArray[i]->isShiny.c_str(),npc->pokemonArray[i]->level, npc->pokemonArray[i]->moveSet[0]->name.c_str(), npc->pokemonArray[i]->moveSet[1]->name.c_str(), npc->pokemonArray[i]->hp, npc->pokemonArray[i]->attack, npc->pokemonArray[i]->defense, npc->pokemonArray[i]->speed, npc->pokemonArray[i]->s_attack, npc->pokemonArray[i]->s_defense, npc->pokemonArray[i]->gender.c_str(), npc->pokemonArray[i]->xp);
        mvprintw((4 * i) + 3, 0, buff);
    }
    refresh();
    getch();
    return 0;
}


int enterBattle(Character * pc, Character * npc){
    int i, chosenPokemon, otp, ui, moveIdx, turnP, opm, idx, damage, aod, random;
    double crit, stab;
    chosenPokemon = -1;
    //npc->defeated = 0;
    //npc->time_pen = INT_MAX;
    for (i = 0; i < 30; i ++){
        move(i,0);
        clrtoeol();
    }
    mvprintw(0, 0, "You have challenged %c to a battle", npc->type);
    mvprintw(1, 0, "First, choose a pokemon: (Type number to choose)");
    for (i=2; i < pc->num_pokemon + 2; i++){
        if (pc->pokemonArray[i-2] != nullptr){
            mvprintw(i,0, "%d: %s; hp: %d/%d", i-1, pc->pokemonArray[i - 2]->name.c_str(), pc->pokemonArray[i - 2]->hp, pc->pokemonArray[i - 2]->maxHp);
        }
    }

    refresh();

    while((chosenPokemon < 0 || chosenPokemon > 6) || pc->pokemonArray[chosenPokemon]->hp <= 0){
        chosenPokemon = getch() - '0' - 1;  
    }
    otp = 0;
    while(npc->pokemonArray[otp]->hp <= 0){
        otp += 1;
    }
    while (checkPcPokemon(pc) > 0 && checkPcPokemon(npc) > 0){  //while both the pc and trainer have pokemon (with hp > 0)
        printTrainerBattle(pc, npc, otp, chosenPokemon);
        ui = 0;
        while(ui != 'b' && ui != 'p' && ui != 's'&& ui != '1' && ui != '2'){
            ui = getch();
            moveIdx = -1;
            if (ui == 'b'){
                moveIdx = 2;
            }
            else if (ui == 'p'){
                moveIdx = 3;
            }
            else if (ui == 's'){
                moveIdx = 4;
            }
            else if (ui == '1'){
                moveIdx = 0;
            }
            else if (ui == '2'){
                moveIdx = 1;
            }
        }
        opm = rand() % (checkPokemonMoves(npc->pokemonArray[otp]));

        if (moveIdx == 2){
            for (i = 0; i < 20; i++){
                move(i,0);
                clrtoeol();
            }
            mvprintw(0,0, "Choose an item (1,2, or 3)");
            mvprintw(1,0, "0:Back");
            mvprintw(2,0, "1:Revive; %d remaining", pc->numRev);
            mvprintw(3,0, "2:Potion; %d remaining", pc->numPot);
            ui = 'a';
            while (ui != '1' && ui != '2' && ui != '0'){
                ui = getch();
            }
            if (ui == '1'){
                if (pc->numRev > 0){
                    idx = revPokemon(pc);
                    if(pc->pokemonArray[idx]->hp < pc->pokemonArray[idx]->maxHp / 2){
                        pc->pokemonArray[idx]->hp = pc->pokemonArray[idx]->maxHp / 2;
                        mvprintw(8,0,"You have revived %s", pc->pokemonArray[idx]->name.c_str());
                        refresh();
                        usleep(1500000);
                    }
                    else{
                        mvprintw(8,0,"%s already has over half hp", pc->pokemonArray[idx]->name.c_str());
                        refresh();
                        usleep(1500000);
                    }
                    pc->numRev --;
                }
                else{
                    mvprintw(5,0,"You have no revives!");
                    refresh();
                    usleep(1500000);
                }
                turnP = 3;
            }
            else if(ui == '2'){
                //usePotion(c, chosenPokemon);
                if (pc->numPot > 0){
                    idx = healPokemon(pc);
                    if(pc->pokemonArray[idx]->hp > 0){
                        pc->pokemonArray[idx]->hp += 20;
                        mvprintw(8,0,"You have healed %s!", pc->pokemonArray[idx]->name.c_str());
                        refresh();
                        usleep(1500000);
                        if (pc->pokemonArray[idx]->hp > pc->pokemonArray[idx]->maxHp){
                            pc->pokemonArray[idx]->hp = pc->pokemonArray[idx]->maxHp;
                        }
                    }
                    pc->numPot --;
                }
                else{
                    mvprintw(5,0,"You have no potions!");
                    refresh();
                    usleep(1500000);
                }
                turnP = 3;
            }
            else if (ui == '0'){
                turnP = 3;
            }
        }
        else if (moveIdx == 3){
            turnP = 3;
            chosenPokemon = swapPcPokemon(pc);
        }
        else if (moveIdx == 4){
            turnP = 3;
            showNpcPokemon(npc);
        }
        else if (moveIdx == 0 || moveIdx == 1){
            if (pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->priority > npc->pokemonArray[otp]->moveSet[opm]->priority){
                turnP = 0;
            }
            else if (npc->pokemonArray[otp]->moveSet[opm]->priority > pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->priority){
                turnP = 1;
            } 
            else {
                if (pc->pokemonArray[chosenPokemon]->speed > npc->pokemonArray[otp]->speed){
                    turnP = 0;
                }
                else if (npc->pokemonArray[otp]->speed > pc->pokemonArray[chosenPokemon]->speed){
                    turnP = 1;
                }
                else{
                    if ((rand() % 2) == 1){
                        turnP = 0;
                    }
                    else {
                        turnP = 1;
                    }
                }
            }
        }
         //turnP == 0 means the player's pokemon goes first (has higher priority)
        //turnP == 1 means the wild pokemon goes first
        if (turnP == 0){
            if (rand() % 100 < pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->accuracy){
                if (rand() % 256 < (pc->pokemonArray[chosenPokemon]->baseSpeed / 2)){
                    crit = 1.5;
                }
                else{
                    crit = 1;
                }
                random = rand() % 10 + 2;
                stab = 1;
                for(Pokemon_types* pokemon_type : pokemon_types){
                    if (pokemon_type->pokemon_id == pc->pokemonArray[chosenPokemon]->id && pokemon_type->type_id == pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->type){
                        stab = 1.5;
                    }
                }
                aod = pc->pokemonArray[chosenPokemon]->attack / npc->pokemonArray[otp]->defense;
                damage = pc->pokemonArray[chosenPokemon]->level * 2;
                damage = (damage / 5) + 2;
                damage *= pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->power;
                damage *= aod;
                damage = (damage / 50) + 2;
                damage *= crit;
                damage *= random;
                damage *= stab;
                npc->pokemonArray[otp]->hp -= damage;
                mvprintw(5,0,"%s used move %s; Hit for %d damage!", pc->pokemonArray[chosenPokemon]->name.c_str(), pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->name.c_str(), damage);
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            else{
                mvprintw(5,0,"%s used move %s; Miss!", pc->pokemonArray[chosenPokemon]->name.c_str(), pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->name.c_str());
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            
            if (npc->pokemonArray[otp]->hp <= 0){
                pc->pokemonArray[chosenPokemon]->xp += npc->pokemonArray[otp]->level;
                npc->pokemonArray[otp]->hp = 0;
                if (npc->pokemonArray[otp + 1] != nullptr){
                    otp += 1;
                    continue;
                }
                else{
                    npc->defeated = 0;
                    npc->time_pen = INT_MAX;
                    pc->numCoins += 5;
                    mvprintw(5,0,"You have defeated trainer %c in battle", npc->type);
                    refresh();
                    usleep(1500000);
                    return 0;
                }
            }
            if (rand() % 100 < npc->pokemonArray[otp]->moveSet[opm]->accuracy){
                if (rand() % 256 < (npc->pokemonArray[otp]->baseSpeed / 2)){
                    crit = 1.5;
                }
                else{
                    crit = 1;
                }
                random = rand() % 10 + 2;
                stab = 1;
                for(Pokemon_types* pokemon_type : pokemon_types){
                    if (pokemon_type->pokemon_id == npc->pokemonArray[otp]->id && pokemon_type->type_id == npc->pokemonArray[otp]->moveSet[opm]->type){
                        stab = 1.5;
                    }
                }
                aod = npc->pokemonArray[otp]->attack / pc->pokemonArray[chosenPokemon]->defense;
                damage = npc->pokemonArray[otp]->level * 2;
                damage = (damage / 5) + 2;
                damage *= npc->pokemonArray[otp]->moveSet[opm]->power;
                damage *= aod;
                damage = (damage / 50) + 2;
                damage *= crit;
                damage *= random;
                damage *= stab;
                pc->pokemonArray[chosenPokemon]->hp -= damage;
                mvprintw(5,0,"%s used move %s; Hit for %d damage!", npc->pokemonArray[otp]->name.c_str(), npc->pokemonArray[otp]->moveSet[opm]->name.c_str(), damage);
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            else{
                mvprintw(5,0,"%s used move %s; Miss!", npc->pokemonArray[otp]->name.c_str(), npc->pokemonArray[otp]->moveSet[opm]->name.c_str());
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }

            if (pc->pokemonArray[chosenPokemon]->hp <= 0){
                pc->pokemonArray[chosenPokemon]->hp = 0;
                if(checkPcPokemon(pc) > 0){
                    mvprintw(5,0,"%s has been knocked out", pc->pokemonArray[chosenPokemon]->name.c_str());
                    refresh();
                    usleep(1500000);
                    chosenPokemon = swapPcPokemon(pc);
                    continue;
                }
                else{
                    mvprintw(5,0,"%s has defeated you in battle", npc->pokemonArray[otp]->name.c_str());
                    refresh();
                    usleep(1500000);
                    return 0;
                }
            }
        }
        //This means the wild pokemon's move hits first
        else if (turnP == 1){
            if (rand() % 100 < npc->pokemonArray[otp]->moveSet[opm]->accuracy){
                if (rand() % 256 < (npc->pokemonArray[otp]->baseSpeed / 2)){
                    crit = 1.5;
                }
                else{
                    crit = 1;
                }
                random = rand() % 10 + 2;
                stab = 1;
                for(Pokemon_types* pokemon_type : pokemon_types){
                    if (pokemon_type->pokemon_id == npc->pokemonArray[otp]->id && pokemon_type->type_id == npc->pokemonArray[otp]->moveSet[opm]->type){
                        stab = 1.5;
                    }
                }
                aod = npc->pokemonArray[otp]->attack / pc->pokemonArray[chosenPokemon]->defense;
                damage = npc->pokemonArray[otp]->level * 2;
                damage = (damage / 5) + 2;
                damage *= npc->pokemonArray[otp]->moveSet[opm]->power;
                damage *= aod;
                damage = (damage / 50) + 2;
                damage *= crit;
                damage *= random;
                damage *= stab;
                pc->pokemonArray[chosenPokemon]->hp -= damage;
                mvprintw(5,0,"%s used move %s; Hit for %d damage", npc->pokemonArray[otp]->name.c_str(), npc->pokemonArray[otp]->moveSet[opm]->name.c_str(), damage);
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            else{
                mvprintw(5,0,"%s used move %s; Miss!", npc->pokemonArray[otp]->name.c_str(), npc->pokemonArray[otp]->moveSet[opm]->name.c_str());
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            if (pc->pokemonArray[chosenPokemon]->hp <= 0){
                pc->pokemonArray[chosenPokemon]->hp = 0;
                if(checkPcPokemon(pc) > 0){
                    mvprintw(5,0,"%s has been knocked out", pc->pokemonArray[chosenPokemon]->name.c_str());
                    refresh();
                    usleep(1500000);
                    chosenPokemon = swapPcPokemon(pc);
                    continue;
                }
                else{
                    mvprintw(5,0,"%s has defeated you in battle", npc->pokemonArray[otp]->name.c_str());
                    refresh();
                    usleep(1500000);
                    return 0;
                }
            }
            if (rand() % 100 < pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->accuracy){
                if (rand() % 256 < (pc->pokemonArray[chosenPokemon]->baseSpeed / 2)){
                    crit = 1.5;
                }
                else{
                    crit = 1;
                }
                random = rand() % 10 + 2;
                stab = 1;
                for(Pokemon_types* pokemon_type : pokemon_types){
                    if (pokemon_type->pokemon_id == pc->pokemonArray[chosenPokemon]->id && pokemon_type->type_id == pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->type){
                        stab = 1.5;
                    }
                }
                aod = pc->pokemonArray[chosenPokemon]->attack / npc->pokemonArray[otp]->defense;
                damage = pc->pokemonArray[chosenPokemon]->level * 2;
                damage = (damage / 5) + 2;
                damage *= pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->power;
                damage *= aod;
                damage = (damage / 50) + 2;
                damage *= crit;
                damage *= random;
                damage *= stab;
                npc->pokemonArray[otp]->hp -= damage;
                mvprintw(5,0,"%s used move %s; Hit for %d damage", pc->pokemonArray[chosenPokemon]->name.c_str(), pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->name.c_str(), damage);
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            else{
                mvprintw(5,0,"%s used move %s; Miss!", npc->pokemonArray[otp]->name.c_str(), pc->pokemonArray[chosenPokemon]->moveSet[moveIdx]->name.c_str());
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            if (npc->pokemonArray[otp]->hp <= 0){
                pc->pokemonArray[chosenPokemon]->xp += npc->pokemonArray[otp]->level;
                npc->pokemonArray[otp]->hp = 0;
                if (npc->pokemonArray[otp + 1] != nullptr){
                    otp += 1;
                    continue;
                }
                else{
                    npc->defeated = 0;
                    npc->time_pen = INT_MAX;
                    pc->numCoins += 5;
                    mvprintw(5,0,"You have defeated trainer %c in battle", npc->type);
                    refresh();
                    usleep(1500000);
                    return 0;
                }
            }
        }
        else if (turnP == 2){
            if (rand() % 100 < npc->pokemonArray[otp]->moveSet[opm]->accuracy){
                if (rand() % 256 < (npc->pokemonArray[otp]->baseSpeed / 2)){
                    crit = 1.5;
                }
                else{
                    crit = 1;
                }
                random = rand() % 10 + 2;
                stab = 1;
                for(Pokemon_types* pokemon_type : pokemon_types){
                    if (pokemon_type->pokemon_id == npc->pokemonArray[otp]->id && pokemon_type->type_id == npc->pokemonArray[otp]->moveSet[opm]->type){
                        stab = 1.5;
                    }
                }
                aod = npc->pokemonArray[otp]->attack / pc->pokemonArray[chosenPokemon]->defense;
                damage = npc->pokemonArray[otp]->level * 2;
                damage = (damage / 5) + 2;
                damage *= npc->pokemonArray[otp]->moveSet[opm]->power;
                damage *= aod;
                damage = (damage / 50) + 2;
                damage *= crit;
                damage *= random;
                damage *= stab;
                pc->pokemonArray[chosenPokemon]->hp -= damage;
                mvprintw(5,0,"%s used move %s; Hit for %d damage!", npc->pokemonArray[otp]->name.c_str(), npc->pokemonArray[otp]->moveSet[opm]->name.c_str(), damage);
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            else{
                mvprintw(5,0,"%s used move %s; Miss!", npc->pokemonArray[otp]->name.c_str(), npc->pokemonArray[otp]->moveSet[opm]->name.c_str());
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            if (pc->pokemonArray[chosenPokemon]->hp <= 0){
                pc->pokemonArray[chosenPokemon]->hp = 0;
                if(checkPcPokemon(pc) > 0){
                    mvprintw(5,0,"%s has been knocked out", pc->pokemonArray[chosenPokemon]->name.c_str());
                    refresh();
                    usleep(1500000);
                    chosenPokemon = swapPcPokemon(pc);
                }
                else{
                    mvprintw(5,0,"%s has defeated you in battle", npc->pokemonArray[otp]->name.c_str());
                    refresh();
                    usleep(1500000);
                    return 0;
                }
            }
        }
    }

    return 0;
}

int showTrainers(Character ** players, int numPlayers){
    int i, xDiff, yDiff;
    int scrollPos = 0;
    int ui = 'a';
    while (1){

        clear();

        mvprintw(0, 0, "List of Trainers (press 'esc' to close): ");

        for (i = scrollPos + 1; i < numPlayers && i < scrollPos + 24; i ++){
            xDiff = players[0]->mapX - players[i]->mapX;        //xDiff is positive if PC is to the right of NPC
            yDiff = players[0]->mapY - players[i]->mapY;        //yDiff is positive if PC is under NPC
            char buff[256];
            if (xDiff >= 0 && yDiff >= 0){
                snprintf(buff, sizeof(buff), "%c, %d north and %d west", players[i]->type, yDiff, xDiff);
            }
            else if (xDiff >= 0 && yDiff < 0){
                yDiff *= -1;
                snprintf(buff, sizeof(buff), "%c, %d south and %d west", players[i]->type, yDiff, xDiff);
            }
            else if (xDiff < 0 && yDiff >= 0){
                xDiff *= -1;
                snprintf(buff, sizeof(buff), "%c, %d north and %d east", players[i]->type, yDiff, xDiff);
            }
            else if (xDiff < 0 && yDiff < 0){
                xDiff *= -1;
                yDiff *= -1;
                snprintf(buff, sizeof(buff), "%c, %d south and %d east", players[i]->type, yDiff, xDiff);
            }
            if (players[i]->defeated == 0){
                strcat(buff, " (defeated)");
            }
            else{
                strcat(buff," (Not defeated)");
            }
            mvprintw(i - scrollPos, 0, buff);
        }
        refresh();

        ui = getch();
        if (ui == KEY_UP && scrollPos > 0){
            scrollPos -= 1;
        }
        else if(ui == KEY_DOWN && scrollPos + 24 < numPlayers){
            scrollPos += 1;
        }

        if (ui == 27){
            return 0;
        }
    }
}

//This function "Randomly" places the pc on a map m
void placePC(Map ** m, Character * c){
    c->type = '@';
    c->defeated = 1;
    c->numBal = 2;
    c->numPot = 2;
    c->numRev = 2;
    c->numCoins = 2;
    int i;
    int temp = 0;
    int random = rand() % 2 + 1;
    if (random > 1){        //50 % chance each branch is taken
        random = rand() % 78 + 1;
        for (i = 19; i > 0; i--){
            if ((*m)->grid[random][i] == '#'){
                temp = i;
                i = 0;
            }
        }
        c->mapX = random;
        c->mapY = temp;
    }
    else {
        random = rand() % 19 + 1;
        for (i = 78; i > 0; i--){
            if ((*m)->grid[i][random] == '#'){
                temp = i;
                i = 0;
            }
        }
        c->mapX = temp;
        c->mapY = random;
    }
    c->worldX = (*m)->worldx;
    c->worldY = (*m)->worldy;
}

/*
This function is called on repeat forever, it takes an array of characters, 
chooses the one with the lowest time penalty. indicating it's that character's turn.
Every player has (nextX, nextY) variables associated with it, which should always be 
set to within one space of the (mapX, mapY) variables. The selected player is moved to
that next location. Then subtracts that players time pen from all of the players time pen.
*/
void gameTurn(Map*** mapArray, Map* m, Character *** players, int numPlayers, int * worldX, int * worldY){
    int i;
    
    Character * c = nullptr;
    
    int mintp = INT_MAX;
    //Finds character with lowest time_pen and sets c to that character (ready to move)
    for (i = 0; i < numPlayers; i ++){
        if ((*players)[i]->time_pen < mintp){
            mintp = (*players)[i]->time_pen;
            c = (*players)[i];
        }
    }

    //Sets location to the next location
    c->mapX = c->nextX;
    c->mapY = c->nextY;

    //Subtracts mintp from all other characters, If a player's time_pen is 0, meaning there was multiple players with the same
    for (i = 0; i < numPlayers; i ++){          //time pen. Subtracting zero from zero essentially iterates through the player array
        (*players)[i]->time_pen -= mintp;       //for all of the characters with matching time_pen. This also gives PC the first turn 
                                                //at the beginning of game.          
    }
    //Finds the next move for the character which just moved, sets time_pen appropriately
    findNextMove(mapArray, players, c, &m, numPlayers, worldX, worldY);

    m->worldx = *worldX;
    m->worldy = *worldY;

    checkAllMoves(mapArray, players, numPlayers, m, &(m->worldx), &(m->worldy));

    printMap(m, m->characters, numPlayers, m->worldx, m->worldy);
}

void takePCmove(Map*** mapArray, Character *** players, Character * c, Map ** m, int numPlayers, int* worldX, int* worldY){
    int x, y, i;
    int ui = getch();
    std::vector<Pokemon*> pokemona;
    if (ui == 'f' && flyFlag == 0){
        takePCmove(mapArray, players, c, m, numPlayers, worldX, worldY);
        return;
    }
    switch (ui){
        case 'q':
            quitFlag = 0;
            break;

        case '7':
        case 'y':
            if (placeValid((*m), c->mapX - 1, c->mapY - 1, '@') == 0 && hasDefeatedPlayer((*m), (*players), numPlayers, c->mapX - 1, c->mapY - 1) == 1){
                (*m)->characters[0]->nextX = (*m)->characters[0]->mapX - 1;
                (*m)->characters[0]->nextY = (*m)->characters[0]->mapY - 1;
            }
            else{
                move(23,0);
                clrtoeol();
                mvprintw(23, 0, "You can't move there!");
                refresh();
            }
            break;

        case '8':
        case 'w':
        case 'k':
            if (placeValid((*m), c->mapX, c->mapY - 1, '@') == 0 && hasDefeatedPlayer((*m), (*players), numPlayers, c->mapX, c->mapY - 1) == 1){
                (*m)->characters[0]->nextX = (*m)->characters[0]->mapX;
                (*m)->characters[0]->nextY = (*m)->characters[0]->mapY - 1;
            }
            else{
                move(23,0);
                clrtoeol();
                mvprintw(23, 0, "You can't move there!");
                refresh();
            }
            break;

        case '9':
        case 'u':
            if (placeValid((*m), c->mapX + 1, c->mapY - 1, '@') == 0 && hasDefeatedPlayer((*m), (*players), numPlayers, c->mapX + 1, c->mapY - 1) == 1){
                (*m)->characters[0]->nextX = (*m)->characters[0]-> mapX + 1;
                (*m)->characters[0]->nextY = (*m)->characters[0]-> mapY - 1;
            }
            else{
                move(23,0);
                clrtoeol();
                mvprintw(23, 0, "You can't move there!");
                refresh();
            }
            break;

        case '6':
        case 'd':
        case 'l':
            if (placeValid((*m), c->mapX + 1, c->mapY, '@') == 0 && hasDefeatedPlayer((*m), (*players), numPlayers, c->mapX + 1, c->mapY) == 1){
                (*m)->characters[0]->nextX = (*m)->characters[0]->mapX + 1;
                (*m)->characters[0]->nextY = (*m)->characters[0]->mapY;
            }
            else{
                move(23,0);
                clrtoeol();
                mvprintw(23, 0, "You can't move there!");
                refresh();
            }
            break;

        case '3':
        case 'n':
            if (placeValid((*m), c->mapX + 1, c->mapY + 1, '@') == 0 && hasDefeatedPlayer((*m), (*players), numPlayers, c->mapX + 1, c->mapY + 1) == 1){
                (*m)->characters[0]->nextX = (*m)->characters[0]->mapX + 1;
                (*m)->characters[0]->nextY = (*m)->characters[0]->mapY + 1;
            }
            else{
                move(23,0);
                clrtoeol();
                mvprintw(23, 0, "You can't move there!");
                refresh();
            }
            break;

        case '2':
        case 's':
        case 'j':
            if (placeValid((*m), c->mapX, c->mapY + 1, '@') == 0 && hasDefeatedPlayer((*m), (*players), numPlayers, c->mapX, c->mapY + 1) == 1){
                (*m)->characters[0]->nextX = (*m)->characters[0]-> mapX;
                (*m)->characters[0]->nextY = (*m)->characters[0]-> mapY + 1;
            }
            else{
                move(23,0);
                clrtoeol();
                mvprintw(23, 0, "You can't move there!");
                refresh();
            }
            break;

        case '1':
        case 'b':
            if (placeValid((*m), c->mapX - 1, c->mapY + 1, '@') == 0 && hasDefeatedPlayer((*m), (*players), numPlayers, c->mapX - 1, c->mapY + 1) == 1){
                (*m)->characters[0]->nextX = (*m)->characters[0]->mapX - 1;
                (*m)->characters[0]->nextY = (*m)->characters[0]->mapY + 1;
            }
            else{
                move(23,0);
                clrtoeol();
                mvprintw(23, 0, "You can't move there!");
                refresh();
            }
            break;

        case '4':
        case 'a':
        case 'h':
            if (placeValid((*m), c->mapX - 1, c->mapY, '@') == 0 && hasDefeatedPlayer((*m), (*players), numPlayers, c->mapX - 1, c->mapY) == 1){
                (*m)->characters[0]->nextX = (*m)->characters[0]-> mapX - 1;
                (*m)->characters[0]->nextY = (*m)->characters[0]-> mapY;
            }
            else{
                move(23,0);
                clrtoeol();
                mvprintw(23, 0, "You can't move there!");
                refresh();
            }
            break;
        case ' ':
        case '5':
            (*m)->characters[0]->nextX = (*m)->characters[0]->mapX;
            (*m)->characters[0]->nextY = (*m)->characters[0]->mapY;
            break;
        case '>':
            if ((*m)->grid[c->mapX][c->mapY] == 'C'){
                enterPokeCenter(c);
                printMap(*m, (*players), numPlayers, (*m)->worldx, (*m)->worldy);
            }
            else if ((*m)->grid[c->mapX][c->mapY] == 'M'){
                enterPokeMart(c);
                printMap(*m, (*players), numPlayers, (*m)->worldx, (*m)->worldy);
            }
            break;
        case 't':
            showTrainers((*players), numPlayers);
            return;
        case 'B':
            int idx;
            for (i = 0; i < 30; i++){
                    move(i,0);
                    clrtoeol();
                }
                mvprintw(0,0, "Choose an item (1,2, or 3)");
                mvprintw(1,0, "0:Back");
                mvprintw(2,0, "1:Revive; %d remaining", c->numRev);
                mvprintw(3,0, "2:Potion; %d remaining", c->numPot);
                ui = 'a';
                while (ui != '1' && ui != '2' && ui != '0'){
                    ui = getch();
                }
                if (ui == '1'){
                    if (c->numRev > 0){// && c->pokemonArray[idx]->hp < (c->pokemonArray[idx]->maxHp / 2)){
                        idx = revPokemon(c);
                        if(c->pokemonArray[idx]->hp < c->pokemonArray[idx]->maxHp / 2){
                            c->pokemonArray[idx]->hp = c->pokemonArray[idx]->maxHp / 2;
                            mvprintw(8,0,"You have revived %s", c->pokemonArray[idx]->name.c_str());
                            refresh();
                            usleep(1500000);
                        }
                        else{
                            mvprintw(8,0,"%s already has over half hp", c->pokemonArray[idx]->name.c_str());
                            refresh();
                            usleep(1500000);
                        }
                        c->numRev --;
                    }
                    else{
                        mvprintw(5,0,"You have no revives!");
                        refresh();
                        usleep(1500000);
                    }
                }
                else if(ui == '2'){
                    //usePotion(c, chosenPokemon);
                    if (c->numPot > 0){
                        idx = healPokemon(c);
                        if(c->pokemonArray[idx]->hp > 0){
                            c->pokemonArray[idx]->hp += 20;
                            mvprintw(8,0,"You have healed %s!", c->pokemonArray[idx]->name.c_str());
                            refresh();
                            usleep(1500000);
                            if (c->pokemonArray[idx]->hp > c->pokemonArray[idx]->maxHp){
                                c->pokemonArray[idx]->hp = c->pokemonArray[idx]->maxHp;
                            }
                        }
                        c->numPot --;
                    }
                    else{
                        mvprintw(5,0,"You have no potions!");
                        refresh();
                        usleep(1500000);
                    }
                }
                return;
        case 'p':
            showNpcPokemon(c);
            return;

        case 'f':           //FLY
            if (flyFlag == 0){
                break;
            }
            int numPokemon;
            numPokemon = (*m)->characters[0]->num_pokemon;
            pokemona = (*m)->characters[0]->pokemonArray;
            x = 300;
            move(0,0);
            clrtoeol();
            mvprintw(0, 0, "Type desired map location like this: X(key: Space)Y(key: Enter)");
            while (x < -200 || x > 200 || y < -200 || y > 200){
                char formatString[] = "%d %d";
                mvscanw(1, 25, formatString, &x, &y);
            }
            x += 200;
            y += 200;
            
            *worldX = x;
            *worldY = y;
            mapArray[x][y]->worldx = x;
            mapArray[x][y]->worldy = y;
            
            if(mapArray[x][y]->initialized == 1){
                createMap(checkNorth(mapArray, x, y), checkSouth(mapArray, x, y), checkEast(mapArray, x, y), checkWest(mapArray, x, y), mapArray[x][y], x, y);
                placePC(&(mapArray[x][y]), (mapArray[x][y])->characters[0]);
                updatehDist(mapArray[x][y], mapArray[x][y]->characters[0]);
                updaterDist(mapArray[x][y], mapArray[x][y]->characters[0]);
                placeEnemies(mapArray[x][y], numPlayers - 1, &(mapArray[x][y]->characters));
            }
            else{
                placePC(&(mapArray[x][y]), (mapArray[x][y])->characters[0]);
                updatehDist(mapArray[x][y], mapArray[x][y]->characters[0]);
                updaterDist(mapArray[x][y], mapArray[x][y]->characters[0]);
            }
            (mapArray[x][y])->characters[0]->num_pokemon = numPokemon;
            (mapArray[x][y])->characters[0]->pokemonArray = pokemona;
            printMap(mapArray[x][y],(mapArray[x][y])->characters, numPlayers, x, y);

            for (i = 0; i < numPlayers; i++){
                findNextMove(mapArray, &(mapArray[x][y]->characters), mapArray[x][y]->characters[i], &mapArray[x][y], numPlayers, worldX, worldY);
            }
            //printMap(mapArray[x][y], (mapArray[x][y])->characters, numPlayers, x, y);

        default:
            //I dont wan't to break, but rather take a new input
            takePCmove(mapArray, players, c, m, numPlayers, worldX, worldY);
            break;
    }
}

int capturePokemon(Pokemon* p, Character* c){
    int i;
    i = 0;
    if (c->num_pokemon < 6){
        while(c->pokemonArray[i] != nullptr){
            i++;
        }
        c->pokemonArray[i] = new Pokemon(*p);
        c->pokemonArray[i]->hp = c->pokemonArray[i]->maxHp;
        c->num_pokemon ++;
        return 0;
    }
    return 0;
}

void printBattleState(Pokemon* p, Character* c, int idx){
    int i, numMoves;
    for (i = 0; i < 30; i ++){
        move(i,0);
        clrtoeol();
    }
    numMoves = checkPokemonMoves(c->pokemonArray[idx]);
    mvprintw(0,0, "In battle with a wild %s; hp: %d/%d", p->name.c_str(), p->hp, p->maxHp);
    mvprintw(1,0, "Your current pokemon: %s; hp: %d/%d", c->pokemonArray[idx]->name.c_str(), c->pokemonArray[idx]->hp, c->pokemonArray[idx]->maxHp);
    mvprintw(4,0, "press 'b' to use an bag item, 'r' to run, or 'p' to switch pokemon");
    for (i = 1; i < numMoves + 1; i++){
        mvprintw(i + 1,0,"choose move %d: %s; move power: %d", i, c->pokemonArray[idx]->moveSet[i - 1]->name.c_str(), c->pokemonArray[idx]->moveSet[i - 1]->power);
    }
}

//p is the other pokemon, c is my pc
int pokemonBattle(Pokemon* p, Character* c){
    int i, chosenPokemon, ui, moveIdx, numOpm, opm, turnP, idx, random, damage, aod;
    float crit, stab;
    chosenPokemon = -1;
    for (i = 0; i < 30; i ++){
        move(i,0);
        clrtoeol();
    }

    //numPoke = checkPcPokemon(c);
    numOpm = checkPokemonMoves(p);
    mvprintw(0,0, "You encountered a wild %s! level: %d", p->name.c_str(), p->level);
    mvprintw(1,0, "First, select a pokemon (type number to choose):");
    for (i=2; i < c->num_pokemon + 2; i++){
        if (c->pokemonArray[i-2] != nullptr){
            mvprintw(i,0, "%d: %s; hp: %d/%d", i-1, c->pokemonArray[i - 2]->name.c_str(), c->pokemonArray[i - 2]->hp, c->pokemonArray[i - 2]->maxHp);
        }
    }
    
    refresh();
    while((chosenPokemon < 0 || chosenPokemon > c->num_pokemon - 1) || c->pokemonArray[chosenPokemon]->hp <= 0){
        chosenPokemon = getch() - '0' - 1;  
    }
    //chosenPokemon now holds the index of which pokemon is chosen
    //this is my chosen pokemon: c->pokemonArray[chosenPokemon]
    
    while(p->hp > 0 && checkPcPokemon(c) > 0){
        //choose a move
        //choose ai move, compare priority
        //subtract hp
        printBattleState(p, c, chosenPokemon);
        
        ui = 0;
        while(ui != 'b' && ui != 'r' && ui != 'p' && ui != '1' && ui != '2'){
            ui = getch();
            moveIdx = -1;
            if (ui == 'b'){
                moveIdx = 2;
            }
            else if (ui == 'r'){
                moveIdx = 3;
            }
            else if (ui == 'p'){
                moveIdx = 4;
            }
            else if (ui == '1'){
                moveIdx = 0;
            }
            else if (ui == '2'){
                moveIdx = 1;
            }
        }
        
        opm = rand() % (numOpm);
        if (moveIdx == 2){
            for (i = 0; i < 20; i++){
                move(i,0);
                clrtoeol();
            }
            mvprintw(0,0, "Choose an item (1,2, or 3)");
            mvprintw(1,0, "0:Back");
            mvprintw(2,0, "1:Revive; %d remaining", c->numRev);
            mvprintw(3,0, "2:Potion; %d remaining", c->numPot);
            mvprintw(4,0, "3:PokeBall; %d remaining", c->numBal);
            ui = 'a';
            while (ui != '1' && ui != '2' && ui != '3' && ui != '0'){
                ui = getch();
            }
            if (ui == '1'){
                //useRevive(c, chosenPokemon);
                if (c->numRev > 0){// && c->pokemonArray[idx]->hp < (c->pokemonArray[idx]->maxHp / 2)){
                    idx = revPokemon(c);
                    if(c->pokemonArray[idx]->hp < c->pokemonArray[idx]->maxHp / 2){
                        c->pokemonArray[idx]->hp = c->pokemonArray[idx]->maxHp / 2;
                        mvprintw(8,0,"You have revived %s", c->pokemonArray[idx]->name.c_str());
                        refresh();
                        usleep(1500000);
                    }
                    else{
                        mvprintw(8,0,"%s already has over half hp", c->pokemonArray[idx]->name.c_str());
                        refresh();
                        usleep(1500000);
                    }
                    c->numRev --;
                }
                else{
                    mvprintw(5,0,"You have no revives!");
                    refresh();
                    usleep(1500000);
                }
                turnP = 3;
            }
            else if(ui == '2'){
                //usePotion(c, chosenPokemon);
                if (c->numPot > 0){
                    idx = healPokemon(c);
                    if(c->pokemonArray[idx]->hp > 0){
                        c->pokemonArray[idx]->hp += 20;
                        mvprintw(8,0,"You have healed %s!", c->pokemonArray[idx]->name.c_str());
                        refresh();
                        usleep(1500000);
                        if (c->pokemonArray[idx]->hp > c->pokemonArray[idx]->maxHp){
                            c->pokemonArray[idx]->hp = c->pokemonArray[idx]->maxHp;
                        }
                    }
                    c->numPot --;
                }
                else{
                    mvprintw(5,0,"You have no potions!");
                    refresh();
                    usleep(1500000);
                }
                turnP = 3;
            }
            else if(ui == '3'){
                //Use pokeball
                if (c->numBal > 0){
                    c->numBal --;
                    if (c->num_pokemon < 6){
                        if (rand() % 180 > (p->level + p->hp)){
                            capturePokemon(p, c);
                            mvprintw(5,0,"You have captured %s", p->name.c_str());
                            refresh();
                            usleep(1500000);
                            return 0;
                        }
                        else{

                            mvprintw(5,0,"%s has fled the battle", p->name.c_str());
                            refresh();
                            usleep(1500000);
                            return 0;
                        }
                    }
                    mvprintw(5,0,"You already have max pokemon!");
                    Pokemon* copied = new Pokemon(*p);
                    copied->hp = copied->maxHp;
                    storedPokemon.push_back(copied);
                    mvprintw(6,0,"%s has been stored in the PokeMart", copied->name.c_str());
                    refresh();
                    usleep(1500000);
                    return 0;
                }
                else{
                    mvprintw(5,0,"You have no pokeballs!");
                }
                turnP = 2;
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
                mvprintw(5,0,"%s has fled the battle", p->name.c_str());
            }
            else if (ui == '0'){
                turnP = 3;
            }
        }
        else if (moveIdx == 3){
            if ((rand() % 10) < 4){
                mvprintw(5,0,"You have fled the battle");
                refresh();
                usleep(1500000);
                return 0;
            }
            else {
                turnP = 2;
                mvprintw(5,0,"attempt to flee failed");
                refresh();
                usleep(1500000);
            }
        }
        else if (moveIdx == 4){
            turnP = 3;
            chosenPokemon = swapPcPokemon(c);
        }
        else if (moveIdx == 0 || moveIdx == 1){
            if (c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->priority > p->moveSet[opm]->priority){
                turnP = 0;
            }
            else if (p->moveSet[opm]->priority > c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->priority){
                turnP = 1;
            } 
            else {
                if (c->pokemonArray[chosenPokemon]->speed > p->speed){
                    turnP = 0;
                }
                else if (p->speed > c->pokemonArray[chosenPokemon]->speed){
                    turnP = 1;
                }
                else{
                    if ((rand() % 2) == 1){
                        turnP = 0;
                    }
                    else {
                        turnP = 1;
                    }
                }
            }
        }

        //turnP == 0 means the player's pokemon goes first (has higher priority)
        //turnP == 1 means the wild pokemon goes first
        if (turnP == 0){
            if (rand() % 100 < c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->accuracy){
                if (rand() % 256 < (c->pokemonArray[chosenPokemon]->baseSpeed / 2)){
                    crit = 1.5;
                }
                else{
                    crit = 1;
                }
                random = rand() % 10 + 2;
                stab = 1;
                for(Pokemon_types* pokemon_type : pokemon_types){
                    if (pokemon_type->pokemon_id == c->pokemonArray[chosenPokemon]->id && pokemon_type->type_id == c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->type){
                        stab = 1.5;
                    }
                }
                aod = c->pokemonArray[chosenPokemon]->attack / p->defense;
                damage = c->pokemonArray[chosenPokemon]->level * 2;
                damage = (damage / 5) + 2;
                damage *= c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->power;
                damage *= aod;
                damage = (damage / 50) + 2;
                damage *= crit;
                damage *= random;
                damage *= stab;
                p->hp -= damage;
                mvprintw(5,0,"%s used move %s; Hit for %d damage!", c->pokemonArray[chosenPokemon]->name.c_str(), c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->name.c_str(), damage);
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            else{
                mvprintw(5,0,"%s used move %s; Miss!", c->pokemonArray[chosenPokemon]->name.c_str(), c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->name.c_str());
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            
            if (p->hp <= 0){
                if (c->num_pokemon < 6){
                    //capturePokemon(p, c);
                    mvprintw(5,0,"%s has fled the battle", p->name.c_str());
                    refresh();
                    usleep(1500000);
                }
                else{
                    mvprintw(5,0,"You already have max pokemon!");
                    refresh();
                    usleep(1500000);
                }
                c->numCoins += 1;
                c->pokemonArray[chosenPokemon]->xp += p->level;
                return 0;
            }
            if (rand() % 100 < p->moveSet[opm]->accuracy){
                if (rand() % 256 < (p->baseSpeed / 2)){
                    crit = 1.5;
                }
                else{
                    crit = 1;
                }
                random = rand() % 10 + 2;
                stab = 1;
                for(Pokemon_types* pokemon_type : pokemon_types){
                    if (pokemon_type->pokemon_id == p->id && pokemon_type->type_id == p->moveSet[opm]->type){
                        stab = 1.5;
                    }
                }
                aod = p->attack / c->pokemonArray[chosenPokemon]->defense;
                damage = p->level * 2;
                damage = (damage / 5) + 2;
                damage *= p->moveSet[opm]->power;
                damage *= aod;
                damage = (damage / 50) + 2;
                damage *= crit;
                damage *= random;
                damage *= stab;
                c->pokemonArray[chosenPokemon]->hp -= damage;
                mvprintw(5,0,"%s used move %s; Hit for %d damage!", p->name.c_str(), p->moveSet[opm]->name.c_str(), damage);
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            else{
                mvprintw(5,0,"%s used move %s; Miss!", p->name.c_str(), p->moveSet[opm]->name.c_str());
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }

            if (c->pokemonArray[chosenPokemon]->hp <= 0){
                c->pokemonArray[chosenPokemon]->hp = 0;
                if(checkPcPokemon(c) > 0){
                    mvprintw(5,0,"%s has been knocked out", c->pokemonArray[chosenPokemon]->name.c_str());
                    refresh();
                    usleep(1500000);
                    chosenPokemon = swapPcPokemon(c);
                }
                else{
                    mvprintw(5,0,"%s has defeated you in battle", p->name.c_str());
                    refresh();
                    usleep(1500000);
                    return 0;
                }
            }
        }
        //This means the wild pokemon's move hits first
        else if (turnP == 1){
            if (rand() % 100 < p->moveSet[opm]->accuracy){
                if (rand() % 256 < (p->baseSpeed / 2)){
                    crit = 1.5;
                }
                else{
                    crit = 1;
                }
                random = rand() % 10 + 2;
                stab = 1;
                for(Pokemon_types* pokemon_type : pokemon_types){
                    if (pokemon_type->pokemon_id == p->id && pokemon_type->type_id == p->moveSet[opm]->type){
                        stab = 1.5;
                    }
                }
                aod = p->attack / c->pokemonArray[chosenPokemon]->defense;
                damage = p->level * 2;
                damage = (damage / 5) + 2;
                damage *= p->moveSet[opm]->power;
                damage *= aod;
                damage = (damage / 50) + 2;
                damage *= crit;
                damage *= random;
                damage *= stab;
                c->pokemonArray[chosenPokemon]->hp -= damage;
                mvprintw(5,0,"%s used move %s; Hit for %d damage", p->name.c_str(), p->moveSet[opm]->name.c_str(), damage);
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            else{
                mvprintw(5,0,"%s used move %s; Miss!", p->name.c_str(), p->moveSet[opm]->name.c_str());
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            if (c->pokemonArray[chosenPokemon]->hp <= 0){
                c->pokemonArray[chosenPokemon]->hp = 0;
                if(checkPcPokemon(c) > 0){
                    mvprintw(5,0,"%s has been knocked out", c->pokemonArray[chosenPokemon]->name.c_str());
                    refresh();
                    usleep(1500000);
                    chosenPokemon = swapPcPokemon(c);
                    continue;
                }
                else{
                    mvprintw(5,0,"%s has defeated you in battle", p->name.c_str());
                    refresh();
                    usleep(1500000);
                    return 0;
                }
            }
            if (rand() % 100 < c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->accuracy){
                if (rand() % 256 < (c->pokemonArray[chosenPokemon]->baseSpeed / 2)){
                    crit = 1.5;
                }
                else{
                    crit = 1;
                }
                random = rand() % 10 + 2;
                stab = 1;
                for(Pokemon_types* pokemon_type : pokemon_types){
                    if (pokemon_type->pokemon_id == c->pokemonArray[chosenPokemon]->id && pokemon_type->type_id == c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->type){
                        stab = 1.5;
                    }
                }
                aod = c->pokemonArray[chosenPokemon]->attack / p->defense;
                damage = c->pokemonArray[chosenPokemon]->level * 2;
                damage = (damage / 5) + 2;
                damage *= c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->power;
                damage *= aod;
                damage = (damage / 50) + 2;
                damage *= crit;
                damage *= random;
                damage *= stab;
                p->hp -= damage;
                mvprintw(5,0,"%s used move %s; Hit for %d damage", c->pokemonArray[chosenPokemon]->name.c_str(), c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->name.c_str(), damage);
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            else{
                mvprintw(5,0,"%s used move %s; Miss!", p->name.c_str(), c->pokemonArray[chosenPokemon]->moveSet[moveIdx]->name.c_str());
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            if (p->hp <= 0){
                if (c->num_pokemon < 6){
                    //capturePokemon(p, c);
                    mvprintw(5,0,"%s has fled the battle", p->name.c_str());
                    refresh();
                    usleep(1500000);
                }
                else{
                    mvprintw(5,0,"You already have max pokemon!");
                    refresh();
                    usleep(1500000);
                }
                c->pokemonArray[chosenPokemon]->xp += p->level;
                c->numCoins += 1;
                return 0;
            }
        }
        else if (turnP == 2){
            if (rand() % 100 < p->moveSet[opm]->accuracy){
                if (rand() % 256 < (p->baseSpeed / 2)){
                    crit = 1.5;
                }
                else{
                    crit = 1;
                }
                random = rand() % 10 + 2;
                stab = 1;
                for(Pokemon_types* pokemon_type : pokemon_types){
                    if (pokemon_type->pokemon_id == p->id && pokemon_type->type_id == p->moveSet[opm]->type){
                        stab = 1.5;
                    }
                }
                aod = p->attack / c->pokemonArray[chosenPokemon]->defense;
                damage = p->level * 1.8;
                damage = (damage / 5) + 2;
                damage *= p->moveSet[opm]->power;
                damage *= aod;
                damage = (damage / 50) + 2;
                damage *= crit;
                damage *= random;
                damage *= stab;
                c->pokemonArray[chosenPokemon]->hp -= damage;
                mvprintw(5,0,"%s used move %s; Hit for %d damage!", p->name.c_str(), p->moveSet[opm]->name.c_str(), damage);
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            else{
                mvprintw(5,0,"%s used move %s; Miss!", p->name.c_str(), p->moveSet[opm]->name.c_str());
                refresh();
                usleep(1500000);
                move(5,0);
                clrtoeol();
            }
            if (c->pokemonArray[chosenPokemon]->hp <= 0){
                c->pokemonArray[chosenPokemon]->hp = 0;
                if(checkPcPokemon(c) > 0){
                    mvprintw(5,0,"%s has been knocked out", c->pokemonArray[chosenPokemon]->name.c_str());
                    refresh();
                    usleep(1500000);
                    chosenPokemon = swapPcPokemon(c);
                }
                else{
                    mvprintw(5,0,"%s has defeated you in battle", p->name.c_str());
                    refresh();
                    usleep(1500000);
                    return 0;
                }
            }
        }
    }

    return 0;
}

/*
Choose a random pokemon, set it's level
sets its moves and stats, then starts a battle with it
*/
int encounterPokemon(Character *** players, int* worldX, int* worldY){
    int pokeIdx, mDist;
    pokeIdx = rand() % 1092;
    Pokemon* newPokemon = new Pokemon(*pokemon[pokeIdx]);
    mDist = calculateMDist(*worldX, *worldY);
    if (mDist < 4){
        newPokemon->level = 1;
    }
    else if (mDist < 201){
        newPokemon->level = rand() % ((mDist / 2) - 1) + 1;
    }
    else if (mDist < 398){
        newPokemon->level = rand() % (100 - ((mDist - 200) / 2)) + ((mDist - 200) / 2);
    }
    else if (mDist < 400){
        newPokemon->level = rand() % 2 + 99;
    }
    else{
        newPokemon->level = 100;
    }
    newPokemon->setMoves();
    newPokemon->setStats();

    pokemonBattle(newPokemon, (*players)[0]);

    checkLevelUp((*players)[0]);

    return 0;
}
/*
Input of player array, a specific character, the map struct, and the number of players in array
This function sets (nextX, nextY)
*/
int findNextMove(Map *** mapArray, Character *** players, Character * c, Map ** m, int numPlayers, int* worldX, int* worldY){
    int i, j;
    int minDist = INT_MAX;
    if (c->type == '@'){
        takePCmove(mapArray, &(*m)->characters, (*m)->characters[0], m, numPlayers, worldX, worldY);
        if (quitFlag == 0){
            return 1;
        }
        (*m)->characters[0]->mapX = (*m)->characters[0]->nextX;
        (*m)->characters[0]->mapY = (*m)->characters[0]->nextY;

        if ((*m)->grid[(*m)->characters[0]->mapX][(*m)->characters[0]->mapY] == ':'){
            if (rand() % 10 == 1 && checkPcPokemon(c) > 0){
                encounterPokemon(players, worldX, worldY);
            }

        }
        else if ((*m)->grid[(*m)->characters[0]->mapX][(*m)->characters[0]->mapY] == 'M'){
            move(23,0);
            clrtoeol();
            mvprintw(23,0,"Press '>' to enter PokeMart ('shift' + '.')");
        }
        else if ((*m)->grid[(*m)->characters[0]->mapX][(*m)->characters[0]->mapY] == 'C'){
            move(23,0);
            clrtoeol();
            mvprintw(23,0,"Press '>' to enter PokeMart ('shift' + '.')");
        }
        if (isOnMap(c->mapX, c->mapY) > 0 && isOnMap(c->mapX, c->mapY) < 5){                //player entered Some Gate
            changeMap(mapArray, m, isOnMap((*m)->characters[0]->mapX, (*m)->characters[0]->mapY), numPlayers, worldX, worldY);
            setTimePen(c, *m);
            return 0;
        }

        for (i = 1; i < numPlayers; i ++){
            if ((*players)[0]->mapX == (*players)[i]->mapX && (*players)[0]->mapY == (*players)[i]->mapY && (*players)[i]->defeated == 1 && checkPcPokemon((*players)[0]) > 0){
                enterBattle((*players)[0], (*players)[i]);
                checkLevelUp((*players)[0]);
                printMap(*m, (*players), numPlayers, (*m)->worldx, (*m)->worldy);
                break;
            }
        }

        updatehDist(*m, c);
        updaterDist(*m, c);
        setTimePen(c, *m);
        return 0;
        //usleep(500000);
        //Don't move '@' right now, move manually soon
        //update rdist and hdist
    }
    else if (c->type == 'h'){
        for (i = c->mapY - 1; i < c->mapY + 2; i ++){
            for (j = c->mapX - 1; j < c->mapX + 2; j ++){
                //Find the min dist from a neighboring cell, set nextX, nextY
                if ((*m)->hdist[j][i] < minDist && spaceFree(players, numPlayers, j, i) == 0 && isValid(j, i) == 0 && isOnMap(j, i) == 0){
                    minDist = (*m)->hdist[j][i];
                    c->nextX = j;
                    c->nextY = i;
                }
            }
        }
    }
    else if (c->type == 'r'){
        //For every neighboring cell,
        for (i = c->mapY - 1; i < c->mapY + 2; i ++){
            for (j = c->mapX - 1; j < c->mapX + 2; j ++){
                //Find the min dist from a neighboring cell, set nextX, nextY
                if ((*m)->rdist[j][i] < minDist && spaceFree(players, numPlayers, j, i) == 0  && isValid(j, i) == 0 && isOnMap(j,i) == 0){

                    minDist = (*m)->rdist[j][i];
                    //Set character's next to the proper location
                    c->nextX = j;
                    c->nextY = i;
                }
            }
        }
    }
    else if (c->type == 'e'){
        int number;
        //If it isn't initialized...
        if (c->direction != 'r' && c->direction != 'l' && c->direction != 'u' && c->direction != 'd'){
            c->direction = 'r';
        }
        //If it's direction is r
        else if (c->direction == 'r'){
            if (isValid(c->mapX + 1, c->mapY) == 0 && placeValid(*m, c->mapX + 1, c->mapY, 'w') == 0 && spaceFree(players, numPlayers, c->mapX + 1, c->mapY) == 0 && isOnMap(c->mapX + 1, c->mapY) == 0){
                c->nextX = c->mapX + 1;
                c->nextY = c->mapY;
            }
            else {
                number = rand() % 3 + 1;
                if (number == 1){
                    c->direction = 'u';
                }
                else if (number == 2){
                    c->direction = 'l';
                }
                else if (number == 3){
                    c->direction = 'd';
                }
            }
        }
        else if (c->direction == 'l'){
            if (isValid(c->mapX - 1, c->mapY) == 0 && placeValid(*m, c->mapX - 1, c->mapY, 'w') == 0 && spaceFree(players, numPlayers, c->mapX - 1, c->mapY) == 0 && isOnMap(c->mapX - 1, c->mapY) == 0){
                c->nextX = c->mapX - 1;
                c->nextY = c->mapY;
            }
            else {
                number = rand() % 3 + 1;
                if (number == 1){
                    c->direction = 'u';
                }
                else if (number == 2){
                    c->direction = 'r';
                }
                else if (number == 3){
                    c->direction = 'd';
                }
            }
        }
        else if (c->direction == 'u'){
            if (isValid(c->mapX, c->mapY - 1) == 0 && placeValid(*m, c->mapX, c->mapY - 1, 'w') == 0 && spaceFree(players, numPlayers, c->mapX, c->mapY - 1) == 0 && isOnMap(c->mapX, c->mapY - 1) == 0){
                c->nextX = c->mapX;
                c->nextY = c->mapY - 1;
            }
            else {
                number = rand() % 3 + 1;
                if (number == 1){
                    c->direction = 'l';
                }
                else if (number == 2){
                    c->direction = 'r';
                }
                else if (number == 3){
                    c->direction = 'd';
                }
            }
        }
        else if (c->direction == 'd'){
            if (isValid(c->mapX, c->mapY + 1) == 0 && placeValid(*m, c->mapX, c->mapY + 1, 'w') == 0 && spaceFree(players, numPlayers, c->mapX, c->mapY + 1) == 0 && isOnMap(c->mapX, c->mapY + 1) == 0){
                c->nextX = c->mapX;
                c->nextY = c->mapY + 1;
            }
            else {
                number = rand() % 3 + 1;
                if (number == 1){
                    c->direction = 'l';
                }
                else if (number == 2){
                    c->direction = 'r';
                }
                else if (number == 3){
                    c->direction = 'u';
                }
            }
        }
    }

    else if (c->type == 'p'){
        int number;
        //If it isn't initialized...
        if (c->direction != 'r' && c->direction != 'l' && c->direction != 'u' && c->direction != 'd'){
            number = rand() % 2 + 1;
            if (number == 1){
                c->direction = 'r';
            }
            else {
                c->direction = 'u';
            }
        }
        //If it's direction is r
        else if (c->direction == 'r'){
            if (isValid(c->mapX + 1, c->mapY) == 0 && placeValid(*m, c->mapX + 1, c->mapY, 'w') == 0 && spaceFree(players, numPlayers, c->mapX + 1, c->mapY) == 0 && isOnMap(c->mapX + 1, c->mapY) == 0){
                c->nextX = c->mapX + 1;
                c->nextY = c->mapY;
            }
            else {
                c->direction = 'l';
            }
        }
        else if (c->direction == 'l'){
            if (isValid(c->mapX - 1, c->mapY) == 0 && placeValid(*m, c->mapX - 1, c->mapY, 'w') == 0 && spaceFree(players, numPlayers, c->mapX - 1, c->mapY) == 0 && isOnMap(c->mapX - 1, c->mapY) == 0){
                c->nextX = c->mapX - 1;
                c->nextY = c->mapY;
            }
            else {
                c->direction = 'r';
            }
        }
        else if (c->direction == 'u'){
            if (isValid(c->mapX, c->mapY - 1) == 0 && placeValid(*m, c->mapX, c->mapY - 1, 'w') == 0 && spaceFree(players, numPlayers, c->mapX, c->mapY - 1) == 0 && isOnMap(c->mapX, c->mapY - 1) == 0){
                c->nextX = c->mapX;
                c->nextY = c->mapY - 1;
            }
            else {
                c->direction = 'd';
            }
        }
        else if (c->direction == 'd'){
            if (isValid(c->mapX, c->mapY + 1) == 0 && placeValid(*m, c->mapX, c->mapY + 1, 'w') == 0 && spaceFree(players, numPlayers, c->mapX, c->mapY + 1) == 0 && isOnMap(c->mapX, c->mapY + 1) == 0){
                c->nextX = c->mapX;
                c->nextY = c->mapY + 1;
            }
            else {
                c->direction = 'u';
            }
        }
    }
    else if (c->type == 'w'){
        int number;
        //If it isn't initialized...
        if (c->direction != 'r' && c->direction != 'l' && c->direction != 'u' && c->direction != 'd'){
            c->direction = 'r';
            c->terrain = (*m)->grid[c->mapX][c->mapY];
        }
        //If it's direction is r
        else if (c->direction == 'r'){
            if (isValid(c->mapX + 1, c->mapY) == 0 && placeValid(*m, c->mapX + 1, c->mapY, 'w') == 0 && spaceFree(players, numPlayers, c->mapX + 1, c->mapY) == 0 && (*m)->grid[c->mapX + 1][c->mapY] == c->terrain && isOnMap(c->mapX + 1, c->mapY) == 0){
                c->nextX = c->mapX + 1;
                c->nextY = c->mapY;
            }
            else {
                number = rand() % 3 + 1;
                if (number == 1){
                    c->direction = 'u';
                }
                else if (number == 2){
                    c->direction = 'l';
                }
                else if (number == 3){
                    c->direction = 'd';
                }
            }
        }
        else if (c->direction == 'l'){
            if (isValid(c->mapX - 1, c->mapY) == 0 && placeValid(*m, c->mapX - 1, c->mapY, 'w') == 0 && spaceFree(players, numPlayers, c->mapX - 1, c->mapY) == 0 && (*m)->grid[c->mapX - 1][c->mapY] == c->terrain && isOnMap(c->mapX - 1, c->mapY) == 0){
                c->nextX = c->mapX - 1;
                c->nextY = c->mapY;
            }
            else {
                number = rand() % 3 + 1;
                if (number == 1){
                    c->direction = 'u';
                }
                else if (number == 2){
                    c->direction = 'r';
                }
                else if (number == 3){
                    c->direction = 'd';
                }
            }
        }
        else if (c->direction == 'u'){
            if (isValid(c->mapX, c->mapY - 1) == 0 && placeValid(*m, c->mapX, c->mapY - 1, 'w') == 0 && spaceFree(players, numPlayers, c->mapX, c->mapY - 1) == 0 && (*m)->grid[c->mapX][c->mapY - 1] == c->terrain && isOnMap(c->mapX, c->mapY - 1) == 0){
                c->nextX = c->mapX;
                c->nextY = c->mapY - 1;
            }
            else {
                number = rand() % 3 + 1;
                if (number == 1){
                    c->direction = 'l';
                }
                else if (number == 2){
                    c->direction = 'r';
                }
                else if (number == 3){
                    c->direction = 'd';
                }
            }
        }
        else if (c->direction == 'd'){
            if (isValid(c->mapX, c->mapY + 1) == 0 && placeValid(*m, c->mapX, c->mapY + 1, 'w') == 0 && spaceFree(players, numPlayers, c->mapX, c->mapY + 1) == 0 && (*m)->grid[c->mapX][c->mapY + 1] == c->terrain && isOnMap(c->mapX, c->mapY + 1) == 0){
                c->nextX = c->mapX;
                c->nextY = c->mapY + 1;
            }
            else {
                number = rand() % 3 + 1;
                if (number == 1){
                    c->direction = 'l';
                }
                else if (number == 2){
                    c->direction = 'r';
                }
                else if (number == 3){
                    c->direction = 'u';
                }
            }
        }
    }

    setTimePen(c, *m);

    return 0;
    
}

int setTimePen(Character * c, Map * m){
    if (c->type == '@'){
        if (m->grid[c->mapX][c->mapY] == '.' || m->grid[c->mapX][c->mapY] == '#' || m->grid[c->mapX][c->mapY] == 'C' || m->grid[c->mapX][c->mapY] == 'M'){
            c->time_pen = 10;
        }
        else if(m->grid[c->mapX][c->mapY] == ':'){
            c->time_pen = 20;
        }
        else {
            c->time_pen = INT_MAX;
        }
    }
    if (m->grid[c->nextX][c->nextY] == '.' || m->grid[c->nextX][c->nextY] == '#'){
        c->time_pen = 10;
    }
    else if (m->grid[c->nextX][c->nextY] == 'C' || m->grid[c->nextX][c->nextY] == 'M'){
        c->time_pen = 50;
        if (c->type == '@'){
            c->time_pen = 10;
        }
    }
    else if (m->grid[c->nextX][c->nextY] == '%'){
        c->time_pen = INT_MAX;
    }
    else if (m->grid[c->nextX][c->nextY] == '^'){
        c->time_pen = INT_MAX;
        if (c->type == 'h'){
            c->time_pen = 20;
        }
    }
    else if (m->grid[c->nextX][c->nextY] == ':'){
        c->time_pen = 20;
        if (c->type == 'h'){
            c->time_pen = 15;
        }
    }
    if (c->defeated == 0){
        c->time_pen = INT_MAX;
    }
    return 0;
}

/*
placeEnemies iterates for int numEnemies times, first chooses a type
Choose a location for the player to be spawned, give the npc some pokemon,

*/
void placeEnemies(Map* m, int numEnemies, Character *** players){
    char type = 'a';
    int random, i, placed, xRand, yRand, j, randPoke, mDist, randIdx;
    //For num enemies
    for (i = 0; i < numEnemies; i ++){
        j = 0;
        randPoke = 0;
        placed = 1;

        //Choose type, first hiker, then rival, then random choice (To ensure hiker and rival are spawned)
        if (i == 0){
            type = 'h';
        }
        else if (i == 1){
            type = 'r';
        }
        else{
            random = rand() % 6 + 1;
            switch(random){
                case 1:
                    type = 'h';
                    break;
                case 2:
                    type = 'r';
                    break;
                case 3:
                    type = 'p';
                    break;
                case 4:
                    type = 'w';
                    break;
                case 5:
                    type = 's';
                    break;
                case 6:
                    type = 'e';
                    break;
            }
        }
        
        //After type is chosen, while the character isn't placed, choose a random location, see if it's available,
        //and place the character
        while(placed != 0){
            xRand = rand() % 78 + 1;
            yRand = rand() % 19 + 1;
            if(placeValid(m, xRand, yRand, type) == 0){
                (*players)[i + 1]->mapX = xRand;
                (*players)[i + 1]->mapY = yRand;
                (*players)[i + 1]->worldX = m->worldx;
                (*players)[i + 1]->worldY = m->worldy;
                (*players)[i + 1]->type = type;
                (*players)[i + 1]->defeated = 1;

                // rand() % (max + 1 - min) + min

                while(randPoke < 6 && j < 6){
                    randIdx = rand() % 1092;
                    
                    (*(*players)[i + 1]).addPokemon(*pokemon[randIdx], j);
                    mDist = calculateMDist(m->worldx, m->worldy);

                    if (mDist < 4){
                        (*players)[i + 1]->pokemonArray[j]->level = 1;
                    }
                    else if (mDist < 201){
                        (*players)[i + 1]->pokemonArray[j]->level = rand() % ((mDist / 2) - 1) + 1;
                    }
                    else if (mDist < 398){
                        (*players)[i + 1]->pokemonArray[j]->level = rand() % (100 - ((mDist - 200) / 2)) + ((mDist - 200) / 2);
                    }
                    else if (mDist < 400){
                        (*players)[i + 1]->pokemonArray[j]->level = rand() % 2 + 99;
                    }
                    else{
                        (*players)[i + 1]->pokemonArray[j]->level = 100;
                    }
                    
                    (*players)[i + 1]->pokemonArray[j]->setMoves();
                    (*players)[i + 1]->pokemonArray[j]->setStats();
                    randPoke = rand() % 10;
                    j++;
                }

                placed = 0;
            }
        }
    }
}

//This implements Dijkstra's algorithm

//Here, eType refer1es for which kind of enemy the cost graph is generated
//calling eType = 1 stands for Hiker, eType = 2 stands for Rival, etc.
void updatehDist(Map* m, Character * c){
    int visited [80][21];           //Holds 1 if visited, 0 if not
    int cost [80][21];
    int i, j, minDist, minX, minY;
    int p;

    for (i = 0; i < 21; i ++){
        for (j = 0; j < 80; j ++){
            visited[j][i] = 0;
            if (m->grid[j][i] == '#'){
                cost[j][i] = 10;
            }
            else if (m->grid[j][i] == '.'){
                cost[j][i] = 10;
            }
            else if (m->grid[j][i] == 'C'){
                cost[j][i] = 50;
            }
            else if (m->grid[j][i] == 'M'){
                cost[j][i] = 50;
            }
            else if (m->grid[j][i] == ':'){
                cost[j][i] = 15;
            }
            else if (m->grid[j][i] == '^'){
                cost[j][i] = 20;
            }
            if (m->grid[j][i] == '~' || m->grid[j][i] == '%' || j == 0 || j == 79 || i == 0 || i == 20){
                cost[j][i] = INT_MAX;
                visited[j][i] = 1;
            }
        }
    }

    //This loop initializes the dist[][] array to int_max except for where the player is
    for (i = 0; i < 21; i ++){
        for (j = 0; j < 80; j ++){
            if (c->mapX == j && c->mapY == i){
                m->hdist[j][i] = 0;
            }
            else{
                m->hdist[j][i] = INT_MAX;
            }
        }
    }
    minDist = INT_MAX;

    //main for loop

    for(p = 0; p < 80 * 21; p++){         //p < 80 * 21
        minDist = INT_MAX;
        minX = 0;
        minY = 0;
        //Find the location with the smallest distance that hasn't been visited yet
        for (i = 0; i < 21; i++) {
            for (j = 0; j < 80; j++) {
                if (visited[j][i] == 0 && m->hdist[j][i] < minDist){            
                    minDist = m->hdist[j][i];
                    minX = j;
                    minY = i;
                }
            }
        }

        visited[minX][minY] = 1;
        //visited[x][y] = 1 stands for True, 0 stands for False
        //Explore the neighbors of (minX,minY), update dist
        for (i = minY - 1; i < minY + 2; i ++){                         //For the surrounding corrdinates of (minX,minY)
            for (j = minX - 1; j < minX + 2; j ++){
                if (cost[j][i] == INT_MAX){
                }
                else if (isValid(j,i) == 0 && visited[j][i] != 1){                  //if the location can be moved (from, since we're working backwards)
                    if (m->hdist[minX][minY] + cost[minX][minY] < m->hdist[j][i]){
                        m->hdist[j][i] = m->hdist[minX][minY] + cost[minX][minY];         //add the cost to get to the current cell
                    }
                }
            }
        }
    }
}

void updaterDist(Map* m, Character * c){
    int visited [80][21];           //Holds 1 if visited, 0 if not
    int cost [80][21];
    int i, j, minDist, minX, minY;
    int p;

    for (i = 0; i < 21; i ++){
        for (j = 0; j < 80; j ++){
            visited[j][i] = 0;
            if (m->grid[j][i] == '#'){
                cost[j][i] = 10;
            }
            else if (m->grid[j][i] == '.'){
                cost[j][i] = 10;
            }
            else if (m->grid[j][i] == 'C'){
                cost[j][i] = 50;
            }
            else if (m->grid[j][i] == 'M'){
                cost[j][i] = 50;
            }
            else if (m->grid[j][i] == ':'){
                cost[j][i] = 20;
            }
            else if (m->grid[j][i] == '^'){
                cost[j][i] = INT_MAX;
            }
            if (m->grid[j][i] == '~' || m->grid[j][i] == '%' || j == 0 || j == 79 || i == 0 || i == 20){
                cost[j][i] = INT_MAX;
                visited[j][i] = 1;
            }
        }
    }

    //This loop initializes the dist[][] array to int_max except for where the player is
    for (i = 0; i < 21; i ++){
        for (j = 0; j < 80; j ++){
            if (c->mapX == j && c->mapY == i){
                m->rdist[j][i] = 0;
            }
            else{
                m->rdist[j][i] = INT_MAX;
            }
        }
    }
    minDist = INT_MAX;

    //main for loop

    for(p = 0; p < 80 * 21; p++){         //p < 80 * 21
        minDist = INT_MAX;
        minX = 0;
        minY = 0;
        //Find the location with the smallest distance that hasn't been visited yet
        for (i = 0; i < 21; i++) {
            for (j = 0; j < 80; j++) {
                if (visited[j][i] == 0 && m->rdist[j][i] < minDist){            
                    minDist = m->rdist[j][i];
                    minX = j;
                    minY = i;
                }
            }
        }

        visited[minX][minY] = 1;
        //visited[x][y] = 1 stands for True, 0 stands for False
        //Explore the neighbors of (minX,minY), update dist
        for (i = minY - 1; i < minY + 2; i ++){                         //For the surrounding corrdinates of (minX,minY)
            for (j = minX - 1; j < minX + 2; j ++){
                if (cost[j][i] == INT_MAX){
                }
                else if (isValid(j,i) == 0 && visited[j][i] != 1){                  //if the location can be moved (from, since we're working backwards)
                    if (m->rdist[minX][minY] + cost[minX][minY] < m->rdist[j][i]){
                        m->rdist[j][i] = m->rdist[minX][minY] + cost[minX][minY];         //add the cost to get to the current cell
                    }
                }
            }
        }
    }
}




void printDist(Map* m){
    int i,j;
    for (i = 0; i < 21; i++){
        for (j = 0; j < 80; j++){
            if (m->hdist[j][i] == INT_MAX){
                printf("   ");
            }
            else{
                printf("%02d ", m->hdist[j][i] % 100);
            }
        }
        printf("\n");
    }
    printf("\n");
    for (i = 0; i < 21; i++){
        for (j = 0; j < 80; j++){
            if (m->rdist[j][i] == INT_MAX){
                printf("   ");
            }
            else{
                printf("%02d ", m->rdist[j][i] % 100);
            }
        }
        printf("\n");
    }
}

//For every space on the map, see if a character sits on that space. If there is a character,
//print char, else, print the terrain
void printMap(Map* m, Character ** players, int numPlayers, int worldX, int worldY){
    int i,j,t;
    int hasChar;
    int temp;
    for (i = 1; i < 23; i++){
        move(i,0);
        clrtoeol();
    }
    
    init_color(10, 500,500,500);                   //color 1 is grey
    init_color(20, 750,500,250);                   //color 2 is brown
    init_color(30, 0, 400, 0);                  //Darker Green
    init_color(40, 600, 800, 400);              //TREE Green
    init_pair(1, COLOR_WHITE, COLOR_BLACK);     //Pair 1 prints White             
    init_pair(2, COLOR_GREEN, COLOR_BLACK);     //Pair 2 prints Green 
    init_pair(3, 10, COLOR_BLACK);               //Pair 3 prints Grey
    init_pair(4, 20, COLOR_BLACK);               //Pair 4 prints Brown
    init_pair(5, COLOR_BLUE, COLOR_BLACK);     //Pair 5 prints Blue
    init_pair(6, 30, COLOR_BLACK);              //Pair 6 prints long grass
    init_pair(7, 40, COLOR_BLACK);              //Pair 7 prints Tree green
    init_pair(8, COLOR_YELLOW, COLOR_BLACK);    //Pair 8 prints yellow
    init_pair(9, COLOR_RED, COLOR_BLACK);    //Pair 9 prints red

    attron(COLOR_PAIR(1));

    for (j = 0; j < 21; j++){
        for (i = 0; i < 80; i ++){
            hasChar = 1;
            for (t = 0; t < numPlayers; t++){
                if (players[t]->mapX == i && players[t]->mapY == j){
                    hasChar = 0;
                    temp = t;
                    break;
                }
            }
            if (hasChar == 0){
                attron(COLOR_PAIR(1));
                mvaddch(j + 1, i, players[temp]->type);
            }
            else{
                if (m->grid[i][j] == '%'){
                    attron(COLOR_PAIR(3));
                    mvaddch(j + 1, i, m->grid[i][j]);
                }
                else if (m->grid[i][j] == '#'){
                    attron(COLOR_PAIR(4));
                    mvaddch(j + 1, i, m->grid[i][j]);
                }
                else if (m->grid[i][j] == '~'){
                    attron(COLOR_PAIR(5));
                    mvaddch(j + 1, i, m->grid[i][j]);
                }
                else if (m->grid[i][j] == '.'){
                    attron(COLOR_PAIR(2));
                    mvaddch(j + 1, i, m->grid[i][j]);
                }
                else if (m->grid[i][j] == ':'){
                    attron(COLOR_PAIR(6));
                    mvaddch(j + 1, i, m->grid[i][j]);
                }
                else if (m->grid[i][j] == '^'){
                    attron(COLOR_PAIR(7));
                    mvaddch(j + 1, i, m->grid[i][j]);
                }
                else if (m->grid[i][j] == 'M'){
                    attron(COLOR_PAIR(8));
                    mvaddch(j + 1, i, m->grid[i][j]);
                }
                else if (m->grid[i][j] == 'C'){
                    attron(COLOR_PAIR(9));
                    mvaddch(j + 1, i, m->grid[i][j]);
                }
            }
        }
    }
    attron(COLOR_PAIR(1));
    attroff(A_NORMAL);
    move(22, 0);
    clrtoeol();
    mvprintw(22, 0, "Current map: (%d, %d)", worldX - 200, worldY - 200);
    refresh();
}

int isFull(const Map* m){
    int i,j;
    for (j = 0; j < 21; j ++){
        for (i = 0; i < 80; i++){
            if (m->grid[i][j] == ' '){
                return 1;
            }
       }
    }
    return 0;
}

int numChars(int x, int y, char c, const Map* m){
    int count = 0;
    int j, i;
    for (i = y - 1; i  < y + 2; i ++){
        for (j = x - 1; j < x + 2; j ++){
            if (m->grid[j][i] == c){
                count += 1;
            }
        }
    }
    return count;
}
//This function is my workhorse which grows a region around a growing radius
//It is first called for radius 1 on each of my seed locations, then r = 2 and so on until the map is filled
void changeR(int x, int y, int r, Map* m){
    char type = (*m).grid[x][y];          //type set to input point
    int a, b;
    int fixa = 0;
    int fixb = 0;

    b = y - r;                      //b = y value, up r
    a = x;                          //a = x vaule::     (a,b) represents the location r spaces above input (x,y)
    if (b < 0){
        fixb = b - 1;
        b = 1;                      //Sets b to 1 if b was off the map
    }

    if (m->grid[a][b] == ' '){                                                          //      x
        m->grid[a][b] = type;           //Sets (a,b) to the new type if it was blank            x
    }                 
                                                                
    int i = 0;
    while (i < r){        //moves a to the right r times, each time... 
        a += 1;
        if (a > 79){
            fixa += 1;
            break;
        }
        if (m->grid[a][b] == ' '){                                                      //      xx
            m->grid[a][b] = type;       //set (a,b) to to new type if it was blank              x
        }                                                                           //
        i += 1;
    }

    i = 0;
    b += fixb;                      //this line accounts for if b was out of bounds
    fixb = 0;
    while (i < (r * 2)){  //Moves b down and changes to new type  
        b += 1;
        if (b > 20){
            fixb += 1;
            break;
        }
        if (m->grid[a][b] == ' '){          //      xx    
            m->grid[a][b] = type;           //      xx
        }                               //       x
        i += 1;
    }
    i = 0;
    a += fixa;                      //this line accounts for if a was out of bounds the last time it was changed
    fixa = 0;

    while (i < (r * 2)){   //moves a to the left and changes space to new type 
        a -= 1;
        if (a < 0){
            fixa -= 1;
            break;
        }
        if (m->grid[a][b] == ' '){      //      xx
            m->grid[a][b] = type;       //      xx
        }                           //         xxx
        i += 1;
    }

    i = 0;
    b += fixb;
    fixb = 0;
    while (i < (r * 2)){                   //Moves b up to finish a 3x3 fill
        b -= 1;
        if (b < 0){
            break;
        }
        if (m->grid[a][b] == ' '){      //     xxx
            m->grid[a][b] = type;       //     xxx
        }                               //     xxx
        i += 1;
    }

    i = 0;
    while (i < (r - 1)){            // Moves a to the right to fill in the last section on top
        a += 1;                     //would fill spot g in the case r was 2
        if (a > 79){
            break;
        }
                                    //    xgxxx
        if (m->grid[a][b] == ' '){  //    xxxxx
            m->grid[a][b] = type;   //    xxxxx
        }                           //    xxxxx
        i += 1;                     //    xxxxx  
    }
}

void createMap(int n, int s, int e, int w, Map* m, int x, int y){
    m->initialized = 0;
    int i, j;
    int seed;
    m->worldx = x;
    m->worldy = y;

    int tg1x = rand() % 78 + 1;     
    int tg1y = rand() % 19 + 1;     //tall grass seed
    int tg2x = rand() % 78 + 1;
    int tg2y = rand() % 19 + 1;     //tall grass seed
    int sg1x = rand() % 78 + 1;
    int sg1y = rand() % 19 + 1; 
    int sg2x = rand() % 78 + 1;
    int sg2y = rand() % 19 + 1; 
    int wx = rand() % 78 + 1;
    int wy = rand() % 19 + 1;       //water seed

    int lhsPath = rand() % 15 + 3;      //Sets paths in random spots (not on the edge)
    int rhsPath = lhsPath;

    while (rhsPath == lhsPath || rhsPath == lhsPath + 1 || rhsPath == lhsPath - 1){         //This ensures rhs path isn't the same as lhs path
        rhsPath = rand() % 15 + 3;      //Sets paths in random spots (not on the edge)
    }
    int topPath = rand() % 74 + 3;      //Sets paths in random spots (not on the edge)
    int botPath = topPath;

    while (topPath == botPath || topPath == botPath + 1 || topPath == botPath - 1){         //This ensures rhs path isn't the same as lhs path
        botPath = rand() % 74 + 3;      //Sets paths in random spots (not on the edge)
    }

    //This section handles cases where a map's neighbor is already generated
    if (n != 0){
        topPath = n;
    }
    if (s != 0){
        botPath = s;
    }
    if (e != 0){
        rhsPath = e;
    }
    if (w != 0){
        lhsPath = w;
    }

    m->northGate = topPath;
    m->eastGate = rhsPath;
    m->southGate = botPath;
    m->westGate = lhsPath;
    
    //This fills the grid with ' ' characters
    for (j = 0; j < 21; j ++){
        for (i = 0; i < 80; i ++)
            m->grid[i][j] = ' ';
    }

    //These for loops set up the boundary with '%'
    for (j = 0; j < 80; j ++){
        m->grid[j][0] = '%';
    }
    for (j = 0; j < 80; j ++){
        m->grid[j][20] = '%';
    }
    for (j = 0; j < 21; j ++){
        m->grid[0][j] = '%';
    }
    for (j = 0; j < 21; j ++){
        m->grid[79][j] = '%';
    }

    //This section places all of the seed cells in their location
    m->grid[tg1x][tg1y] = ':';
    m->grid[tg2x][tg2y] = ':';
    m->grid[sg1x][sg1y] = '.';
    m->grid[sg2x][sg2y] = '.';
    m->grid[wx][wy] = '~';

    //This section fills in the map from the seed locations
    int r = 1;
    while(isFull(m) == 1){
        changeR(tg1x, tg1y, r, m);
        changeR(sg1x, sg1y, r, m);
        changeR(tg2x, tg2y, r, m);
        changeR(sg2x, sg2y, r, m);
        changeR(wx, wy, r, m);
        r += 1;
    }

    //This technique "smooths out" my enviornment and makes it look nicer
    for (j = 1; j < 20; j ++){
        for (i = 1; i < 79; i ++){
            seed = rand() % 50 + 1;
            if (seed < (10 * numChars(i, j, '~', m))){         
                m->grid[i][j] = '~';
            }
        }
    }

    for (j = 1; j < 20; j ++){
        for (i = 1; i < 79; i ++){
            seed = rand() % 50 + 1;
            if (seed < (10 * numChars(i, j, ':', m))){         
                m->grid[i][j] = ':';
            }
        }
    }

    for (j = 1; j < 20; j ++){
        for (i = 1; i < 79; i ++){
            seed = rand() % 50 + 1;
            if (seed < (10 * numChars(i, j, '.', m))){         
                m->grid[i][j] = '.';
            }
        }
    }
    //Randomly sets some squares to '^""
    //Adjust the numbers in the equation compared to treeSeed in order to adjust spawn habits 
    //like overall likelyhood a tree/rock will spawn along with adjusting likelyhood to form groups
    
    for (j = 1; j < 20; j ++){
        for (i = 1; i < 79; i ++){
            seed = rand() % 100 + 1;
            if (seed < (2 + 20 * numChars(i, j, '^', m)) && (m->grid[i][j] == '.' || m->grid[i][j] == ':')){
                m->grid[i][j] = '^';
            }
        }
    }
    // Same thing for rocks
    for (j = 1; j < 20; j ++){
        for (i = 1; i < 79; i ++){
            seed = rand() % 250 + 1;
            if (seed < (3 + 4 * numChars(i, j, '%', m))){
                m->grid[i][j] = '%';
            }
        }
    }
    // C/P to Populate with more trees/ rocks and to favor groupings
    for (j = 1; j < 20; j ++){
        for (i = 1; i < 79; i ++){
            seed = rand() % 100 + 1;
            if (seed < (2 + 18 * numChars(i, j, '^', m)) && (m->grid[i][j] == '.' || m->grid[i][j] == ':')){
                m->grid[i][j] = '^';
            }
        }
    }
    // Same thing for rocks
    for (j = 1; j < 20; j ++){
        for (i = 1; i < 79; i ++){
            seed = rand() % 150 + 1;
            if (seed < (1 + 12 * numChars(i, j, '%', m))){         
                m->grid[i][j] = '%';
            }
        }
    }
    int a, b, prob;
    float mod;
    //This is the horizontal path
    a = 0;
    b = lhsPath;
    m->grid[a][b] = '#';
    a += 1;
    while (a < 80){
        prob = rand() % (201) - 100;        //(-100-100)
        mod = b - rhsPath;
        if (b != rhsPath && a > 75){
            mod *= 2;
        }
        prob += (a * mod);
        if (b == rhsPath && a > 75){
            prob *= .1;
        }
        if (prob < -80 && b < 19){                  // 15% prob down
            m->grid[a][b] = '#';
            b += 1;
            m->grid[a][b] = '#';
            a += 1;
            m->grid[a][b] = '#';
            a += 1;
        }
        else if (prob > 80 && b > 2){                  // 15% prob up
            m->grid[a][b] = '#';
            b -= 1;
            m->grid[a][b] = '#';
            a += 1;
            m->grid[a][b] = '#';
            a += 1;
        }
        else{                                //Else go straight across
            m->grid[a][b] = '#';
            a += 1;
        }
    }

    //This is the vertical path
    a = topPath;
    b = 0;
    m->grid[a][b] = '#';
    b += 1;
    while (b < 21){
        prob = rand() % (201) - 100;        //(-100 - 100)
        mod = a - botPath;                      
        if (mod > -3 && mod < 3){
            mod *= 2;
        }
        if (b > 17 && a != botPath){
            mod *= 3;
        }

        prob += (mod * pow(b, 1.18));               //This ensures all paths exit the correct spot

        if (b > 16 && a == botPath){
            prob *= .1;
        }

        if (prob < -85 && a < 77){                  // 15% prob right
            m->grid[a][b] = '#';
            a += 1;
        }
        else if (prob > 85 && a > 2){                  // 15% prob left
            m->grid[a][b] = '#';
            a -= 1;
        }
        else{                                //Else go straight down
            m->grid[a][b] = '#';
            b += 1;
        }
    }
    int pc = rand() % 76 + 2;
    int pm = rand() % 76 + 2;
    a = pc;
    b = 20;
    while (b > 18){
        a = rand() % 76 + 2; 
        b = 20;
        while (m->grid[a][b] != '#' && m->grid[a + 1][b] != '#'){
            b -= 1;
        }
        b += 1;
    }
    int maxD;

    if (abs(x - 200) > abs(y - 200)){
        maxD = abs(x - 200);
    }
    else {
        maxD = abs(y - 200);
    }

    int martProb = rand() % (101);         // Random # from 0 - 100
    int centerProb = rand() % (101);         // Random # from 0 - 100

    if (maxD == 0){
        centerProb = 0;
        martProb = 0;
    } 
    
    if (centerProb < ((-45 * maxD) / 200) + 50){
        m->grid[a][b] = 'C';
        m->grid[a + 1][b] = 'C';
        m->grid[a][b + 1] = 'C';
        m->grid[a + 1][b + 1] = 'C';
    }
    b = 0;
    a = pm;

    while (b < 2){
        a = rand() % 76 + 2; 
        b = 0;
        while (m->grid[a][b] != '#' && m->grid[a + 1][b] != '#'){
            b += 1;
        }
        b -= 1;
    }

    if (martProb < ((-45 * maxD) / 200) + 50){
        m->grid[a][b] = 'M';
        m->grid[a][b - 1] = 'M';
        m->grid[a + 1][b] = 'M';
        m->grid[a + 1][b - 1] = 'M';
    }
   

    if (x == 0){
        for (i = 0; i < 21; i ++){
            m->grid[0][i] = '%';
        }
    }
    if (x == 400){
        for (i = 0; i < 21; i ++){
            m->grid[79][i] = '%';
        }
    }
    if (y == 0){
        for (i = 0; i < 80; i ++){
            m->grid[i][0] = '%';
        }
    }
    if (y == 400){
        for (i = 0; i < 80; i ++){
            m->grid[i][20] = '%';
        }
    }

    //This section sets the cost matrix
    
    for (i = 0; i < 21; i ++){
        for (j = 0; j < 80; j ++){
            m->hdist[j][i] = INT_MAX; 
            m->rdist[j][i] = INT_MAX;
        }
    }
    
}

int main(int argc, char *argv[]){
    
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    //int quitFlag = 1;

    mvprintw(0, 0, "Loading...");
    refresh();
    flyFlag = 1;
    int numTrainers = 10;
    int opt;
    while((opt = getopt(argc, argv, "fn:")) != -1){
        switch (opt){
            case 'n':
                numTrainers = atoi(optarg);
                break;
            case 'f':
                flyFlag = 0;
                break;
            default:
                fprintf(stderr, "Usage: %s [-n numtrainers]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    const int rows = 401; // Set your desired number of rows
    const int columns = 401; // Set your desired number of columns
    int i, j;
    srand(time(NULL));

        
    Map*** mapArray = (Map***)malloc(rows * sizeof(Map**));
    if (mapArray == NULL) {
        // Handle memory allocation error
        exit(1);
    }
    for (i = 0; i < rows; i++) {
        mapArray[i] = (Map**)malloc(columns * sizeof(Map*));
        if (mapArray[i] == NULL) {
            // Handle memory allocation error
            exit(1);
        }
        for (int j = 0; j < columns; j++) {
            mapArray[i][j] = new Map(numTrainers); // Create a Map instance for each element
        }
    }
    
    
    //Don't change anything in above this in Main
    const char* filename = nullptr;
    const char* dbPath = "/pokedex/pokedex/data/csv/";
    const char* homeDir = "/share/cs327";
    int len = 0;

   
    for (i = 0; i < 9; i ++){
        if (i == 0){
            filename = "pokemon.csv";
        }
        else if (i == 1){
            filename = "moves.csv";
        }
        else if (i == 2){
            filename = "pokemon_moves.csv";
        }
        else if (i == 3){
            filename = "pokemon_species.csv";
        }
        else if (i == 4){
            filename = "experience.csv";
        }
        else if (i == 5){
            filename = "type_names.csv";
        }
        else if (i == 6){
            filename = "pokemon_stats.csv";
        }
        else if (i == 7){
            filename = "stats.csv";
        }
        else if (i == 8){
            filename = "pokemon_types.csv";
        }
        len = strlen(homeDir) + strlen(dbPath) + strlen(filename);

        char* file = static_cast<char*>(malloc(static_cast<size_t>(len + 1)));
        strcpy(file, homeDir);
        strcat(file, dbPath);
        strcat(file, filename);

        std::ifstream inputFile(file);

        //If not open, try opening the file through getenv("HOME")
        if(!inputFile.is_open()){
            //const char* homeDir2 = getenv("HOME");
            const char* homeDir2 = ".";
            dbPath = "/data/";
            int len = strlen(homeDir2) + strlen(dbPath) + strlen(filename) + 1;
            char* file2 = static_cast<char*>(malloc(static_cast<size_t>(len + 1)));
            strcpy(file2, homeDir2);
            strcat(file2, dbPath);
            strcat(file2, filename);
            inputFile.open(file2);
        }

        if (inputFile.is_open()) {
            std::string line;
            std::getline(inputFile, line);
            while (std::getline(inputFile, line)) {
                // Process each line of the file
                std::istringstream ss(line);
                std::string field;

                if (filename != nullptr && strcmp(filename, "pokemon.csv") == 0){
                    // Create a new instance of the struct
                    Pokemon* p = new Pokemon;

                    // Parse and assign data from the CSV row
                    if (std::getline(ss, field, ',')) {
                        p->id = std::stoi(field);
                    }
                    if (std::getline(ss, field, ',')) {
                        p->name = field;
                    }
                    if (std::getline(ss, field, ',')) {
                        p->species = std::stoi(field);
                    }
                    if (std::getline(ss, field, ',')) {
                        p->height = std::stoi(field);
                    }
                    if (std::getline(ss, field, ',')) {
                        p->weight = std::stoi(field);
                    }
                    if (std::getline(ss, field, ',')) {
                        p->base_xp = std::stoi(field);
                    }
                    if (std::getline(ss, field, ',')) {
                        p->order = std::stoi(field);
                    }
                    if (std::getline(ss, field, ',')) {
                        p->is_default = std::stoi(field);
                    }
                    pokemon.push_back(p);
                } 
                else if (filename != nullptr && strcmp(filename, "moves.csv") == 0){
                    Moves* m = new Moves;
                    m->superContestEffect = INT_MAX;

                    if (std::getline(ss, field, ',')) {
                        m->id = std::stoi(field);
                    }
                    if (std::getline(ss, field, ',')) {
                        m->name = field;
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->generation = INT_MAX;
                        }
                        else{
                            m->generation = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {\
                        if (field == ""){
                            m->type = INT_MAX;
                        }
                        else{
                            m->type = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->power = 0;
                        }
                        else{
                            m->power = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->pp = INT_MAX;
                        }
                        else{
                            m->pp = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->accuracy = INT_MAX;
                        }
                        else{
                            m->accuracy = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->priority = INT_MAX;
                        }
                        else{
                            m->priority = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->target = INT_MAX;
                        }
                        else{
                            m->target = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->damageClassId = INT_MAX;
                        }
                        else{
                            m->damageClassId = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->effectId = INT_MAX;
                        }
                        else{
                            m->effectId = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->effectChance = INT_MAX;
                        }
                        else{
                            m->effectChance = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->contestType = INT_MAX;
                        }
                        else{
                            m->contestType = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->contestEffect = INT_MAX;
                        }
                        else{
                            m->contestEffect = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            m->superContestEffect = INT_MAX;
                        }
                        else{
                            m->superContestEffect = std::stoi(field);
                        }
                    }
                    moves.push_back(m);
                }
                else if (filename != nullptr && strcmp(filename, "pokemon_moves.csv") == 0){
                    Pokemon_moves* pm = new Pokemon_moves;

                    pm->order = INT_MAX;

                    if (std::getline(ss, field, ',')) {
                        pm->pokemonId = std::stoi(field);
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pm->version_group = INT_MAX;
                        }
                        else{
                            pm->version_group = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pm->moveId = INT_MAX;
                        }
                        else{
                            pm->moveId = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pm->pokemon_move_method = INT_MAX;
                        }
                        else{
                            pm->pokemon_move_method = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pm->level = INT_MAX;
                        }
                        else{
                            pm->level = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pm->order = INT_MAX;
                        }
                        else{
                            pm->order = std::stoi(field);
                        }
                    }
                    pokemon_moves.push_back(pm);
                }
                else if (filename != nullptr && strcmp(filename, "pokemon_species.csv") == 0){
                    Pokemon_species* ps = new Pokemon_species;

                    ps->conquest_order = INT_MAX;

                    if (std::getline(ss, field, ',')) {
                        ps->id = std::stoi(field);
                    }
                    if (std::getline(ss, field, ',')) {
                        ps->name = field;
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->generation = INT_MAX;
                        }
                        else{
                            ps->generation = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->evolves_from_species = INT_MAX;
                        }
                        else{
                            ps->evolves_from_species = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->evolution_chain = INT_MAX;
                        }
                        else{
                            ps->evolution_chain = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->color = INT_MAX;
                        }
                        else{
                            ps->color = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->shape = INT_MAX;
                        }
                        else{
                            ps->shape = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->habitat = INT_MAX;
                        }
                        else{
                            ps->habitat = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->gender_rate = INT_MAX;
                        }
                        else{
                            ps->gender_rate = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->capture_rate = INT_MAX;
                        }
                        else{
                            ps->capture_rate = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->base_happiness = INT_MAX;
                        }
                        else{
                            ps->base_happiness = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->is_baby = INT_MAX;
                        }
                        else{
                            ps->is_baby = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->hatch_counter = INT_MAX;
                        }
                        else{
                            ps->hatch_counter = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->has_gender_differences = INT_MAX;
                        }
                        else{
                            ps->has_gender_differences = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->growth_rate = INT_MAX;
                        }
                        else{
                            ps->growth_rate = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->forms_switchable = INT_MAX;
                        }
                        else{
                            ps->forms_switchable = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->is_legendary = INT_MAX;
                        }
                        else{
                            ps->is_legendary = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->is_mythical = INT_MAX;
                        }
                        else{
                            ps->is_mythical = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->order = INT_MAX;
                        }
                        else{
                            ps->order = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            ps->conquest_order = INT_MAX;
                        }
                        else{
                            ps->conquest_order = std::stoi(field);
                        }
                    }
                    pokemon_species.push_back(ps);
                    
                }
                else if (filename != nullptr && strcmp(filename, "experience.csv") == 0){
                    Experience* e = new Experience;

                    e->experience = INT_MAX;
                    
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            e->growth_rate = INT_MAX;
                        }
                        else{
                            e->growth_rate = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            e->level = INT_MAX;
                        }
                        else{
                            e->level = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            e->experience = INT_MAX;
                        }
                        else{
                            e->experience = std::stoi(field);
                        }
                    }
                    experience.push_back(e);
                }
                else if (filename != nullptr && strcmp(filename, "type_names.csv") == 0){
                    Type_names* tn = new Type_names;
                    
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            tn->type_id = INT_MAX;
                        }
                        else{
                            tn->type_id = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            tn->local_language = INT_MAX;
                        }
                        else{
                            tn->local_language = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            tn->name = "";
                        }
                        else{
                            tn->name = field;
                        }
                    }
                    type_names.push_back(tn);
                }
                else if (filename != nullptr && strcmp(filename, "pokemon_stats.csv") == 0){
                    Pokemon_stats* pst = new Pokemon_stats;

                    pst->effort = INT_MAX;

                    
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pst->pokemon_id = INT_MAX;
                        }
                        else{
                            pst->pokemon_id = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pst->stat_id = INT_MAX;
                        }
                        else{
                            pst->stat_id = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pst->base_stat = INT_MAX;
                        }
                        else{
                            pst->base_stat = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pst->effort = INT_MAX;
                        }
                        else{
                            pst->effort = std::stoi(field);
                        }
                    }
                    pokemon_stats.push_back(pst);
                }
                else if (filename != nullptr && strcmp(filename, "stats.csv") == 0){
                    Stats* s = new Stats;
                    s->game_idx = INT_MAX;
                    
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            s->id = INT_MAX;
                        }
                        else{
                            s->id = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            s->damage_class = INT_MAX;
                        }
                        else{
                            s->damage_class = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            s->name = "";
                        }
                        else{
                            s->name = field;
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            s->is_battle_only = INT_MAX;
                        }
                        else{
                            s->is_battle_only = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            s->game_idx = INT_MAX;
                        }
                        else{
                            s->game_idx = std::stoi(field);
                        }
                    }
                    stats.push_back(s);
                }
                else if (filename != nullptr && strcmp(filename, "pokemon_types.csv") == 0){
                    Pokemon_types* pt = new Pokemon_types;
                    
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pt->pokemon_id = INT_MAX;
                        }
                        else{
                            pt->pokemon_id = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pt->type_id = INT_MAX;
                        }
                        else{
                            pt->type_id = std::stoi(field);
                        }
                    }
                    if (std::getline(ss, field, ',')) {
                        if (field == ""){
                            pt->slot = INT_MAX;
                        }
                        else{
                            pt->slot = std::stoi(field);
                        }
                    }
                    pokemon_types.push_back(pt);
                }
                
            }
            
            inputFile.close();
        } 
        else {
            std::cerr << "Failed to open the file." << std::endl;
        }
        free(file); 
    }
    

    //This is how the game starts, we allocate space for the first map, place it, then print it
    //mapArray[200][200] = (Map*)malloc(sizeof(Map));   //Here, (200,200) is the center map

    createMap(0,0,0,0,mapArray[200][200], 200, 200);
    int currX = 200;
    int currY = 200;

    placePC(&(mapArray[currX][currY]), mapArray[200][200]->characters[0]);

    updatehDist(mapArray[currX][currY], mapArray[200][200]->characters[0]);
    updaterDist(mapArray[currX][currY], mapArray[200][200]->characters[0]);
    

    //place all the enemies across the map, put them in characters array
    placeEnemies(mapArray[currX][currY], numTrainers, &(mapArray[200][200]->characters));

    int pokeId1 = rand() % 1092;
    int pokeId2 = rand() % 1092;
    int pokeId3 = rand() % 1092;

    Pokemon* newPokemon1 = new Pokemon(*pokemon[pokeId1]);
    Pokemon* newPokemon2 = new Pokemon(*pokemon[pokeId2]);
    Pokemon* newPokemon3 = new Pokemon(*pokemon[pokeId3]);
    newPokemon1->setStats();
    newPokemon2->setStats();
    newPokemon3->setStats();
    Pokemon* chosenPokemon = nullptr;
    
    mvprintw(0, 0, "Choose a starter pokemon; type 1, 2, or 3: ");
    mvprintw(1, 0, "1: (%s;    starting hp: %d    starting speed: %d)", newPokemon1->name.c_str(), newPokemon1->hp, newPokemon1->speed);
    mvprintw(2, 0, "2: (%s;    starting hp: %d    starting speed: %d)", newPokemon2->name.c_str(), newPokemon2->hp, newPokemon2->speed);
    mvprintw(3, 0, "3: (%s;    starting hp: %d    starting speed: %d)", newPokemon3->name.c_str(), newPokemon3->hp, newPokemon3->speed);

    mvprintw(5, 0, "Welcome to Pokemon, Read this for some instruction:");
    mvprintw(7, 0, "Pick a starter pokemon, then you'll enter the center map in a world of pokemon.");
    mvprintw(8, 0, "You are the '@', other trainers are represented by characters like 'r' and 'e'.");
    mvprintw(9, 0, "To move, use these keys:");
    mvprintw(10, 0, "y (up & left) | u (up & right) | h (left) | j (down)");
    mvprintw(11, 0, "k (up) | l (right) | b (down and left) | n (down and right)");
    mvprintw(12, 0, "You can also use wasd to move your character around the map");
    mvprintw(13, 0, "Battle other trainers, encounter wild pokemon, visit PokeCenters and Pokemarts,");
    mvprintw(14, 0, "Explore the massive world by moving through gates to other maps,");
    mvprintw(15, 0, "the game gets more difficult as you travel futher from the center of the world");
    mvprintw(16, 0, "Enjoy the game!");

    refresh();

    int response;
    int chosen;
    chosen = 1;
    while (chosen == 1){
        response = getch();
        if (response == '1'){
            chosenPokemon = new Pokemon(*newPokemon1);
            chosen = 0;
        }
        else if (response == '2'){
            chosenPokemon = new Pokemon(*newPokemon2);
            chosen = 0;
        }
        else if (response == '3'){
            chosenPokemon = new Pokemon(*newPokemon3);
            chosen = 0;
        }
        else{
            mvprintw(4, 0, "Please type '1', '2', or '3' to choose a pokemon:");
        }
    }
    
    (*mapArray[200][200]->characters[0]).addPokemon(*chosenPokemon, 0);
    (*mapArray[200][200]->characters[0]->pokemonArray[0]).setMoves();
    (*mapArray[200][200]->characters[0]->pokemonArray[0]).setStats();

    mvprintw(23,0, "Controls: h(L), j(D), k(U), l(R), y(U&L), u(U&R), b(D&L), n(D&R)");
    printMap(mapArray[currX][currY], mapArray[200][200]->characters, numTrainers + 1, 200, 200);

    for (i = 0; i < numTrainers + 1; i ++){
        findNextMove(mapArray, &(mapArray[200][200]->characters), mapArray[200][200]->characters[i], &mapArray[currX][currY], numTrainers + 1, &currX, &currY);
    }

    if (quitFlag == 0){
        for (i = 0; i < 23; i ++){
            move(i,0);
            clrtoeol();
        }
    
        mvprintw(0, 0, "But how will you be the best if you quit... ");
        refresh();

        usleep(1000000);

        for (i = 0; i < rows; i++) {
            for (j = 0; j < columns; j++) {
                delete mapArray[i][j];
            }
            free(mapArray[i]);
        }
        free(mapArray);
    
        endwin();

        return 0;
    }

    mapArray[200][200]->characters[0]->mapX = mapArray[200][200]->characters[0]->nextX;
    mapArray[200][200]->characters[0]->mapY = mapArray[200][200]->characters[0]->nextY;

    mapArray[200][200]->characters[0]->time_pen = 15;

    for (i = 1; i < 23; i ++){
        move(i,0);
        clrtoeol();
    }
    
    for (i = 1; i < numTrainers + 1; i ++){
        if (mapArray[200][200]->characters[0]->mapX == mapArray[200][200]->characters[i]->mapX && mapArray[200][200]->characters[0]->mapY == mapArray[200][200]->characters[i]->mapY){
            enterBattle(mapArray[200][200]->characters[0], mapArray[200][200]->characters[i]);
            checkLevelUp(mapArray[200][200]->characters[0]);
            break;
        }
    }
    
    updatehDist(mapArray[currX][currY], mapArray[200][200]->characters[0]);
    updaterDist(mapArray[currX][currY], mapArray[200][200]->characters[0]);

    printMap(mapArray[currX][currY], mapArray[200][200]->characters, numTrainers + 1, 200, 200);

    //Forever, ...
    while (quitFlag == 1){   
        gameTurn(mapArray, mapArray[currX][currY], &(mapArray[currX][currY]->characters), numTrainers + 1, &currX, &currY);
    }   
    
    for (i = 0; i < 24; i ++){
        move(i,0);
        clrtoeol();
    }
    
    mvprintw(0, 0, "But how will you be the best if you quit... ");
    refresh();

    usleep(1000000);

   for (i = 0; i < rows; i++) {
        for (j = 0; j < columns; j++) {
            delete mapArray[i][j];
        }
        free(mapArray[i]);
    }
    free(mapArray);
    
    endwin();

    return 0;
}
