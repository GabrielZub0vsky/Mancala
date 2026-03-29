# Mancala

A networked implementation of the classic Mancala board game written in C, featuring an intelligent AI opponent.

## Overview

This project implements the Mancala game with a client-server architecture. Players can compete against each other or a CPU opponent that uses strategic decision-making algorithms to evaluate and execute optimal moves.

## Project Structure

```
Mancala/
├── mancalaproj/           # Core game logic and AI
│   ├── mancala3.h         # Game data structures and function declarations
│   ├── mancala3.c         # Game mechanics and AI implementation
│   └── [Eclipse project files]
│
└── mancalaserver/         # Network server and client
    ├── mancalaserver.c    # Server implementation
    ├── mancalaclient.c    # Client implementation
    ├── mancalaclient.h    # Client header definitions
    └── [Eclipse project files]
```

## Features

### Game Mechanics
- Full implementation of Mancala rules with 6 basins per player
- Marble distribution and capture logic
- Bonus turn system (when last marble lands in player's scoring basin)
- Game-end detection and final score calculation

### AI Player
The CPU opponent uses multi-criteria decision-making:
1. **Bonus Turn Detection** - Identifies moves that result in an extra turn
2. **Capture Evaluation** - Evaluates offensive and defensive capture opportunities
3. **Fallback Strategy** - Selects random or first available non-empty basin when primary strategies fail

### Network Architecture
- Client-server protocol with custom command parsing
- Supports LOGIN, NEWGAME, PLAY, and game state updates
- Asynchronous message handling between server and client

## Data Structures

### Game Entities
```c
struct Player {
    int basins[6];    // 6 playing basins per player
    int score;        // Score basin (mancala)
};

struct Board {
    struct Player opp_cpu;   // Opponent (computer)
    struct Player my_cpu;    // Player (computer in network mode)
};

struct Game {
    struct Board board;
    char turn;               // Track whose turn it is
};
```

## Compilation

The project uses Eclipse CDT project configuration files (`.cproject` and `.project`). To compile:

```bash
cd mancalaproj
gcc -o mancala mancala3.c

cd ../mancalaserver
gcc -o server mancalaserver.c
gcc -o client mancalaclient.c
```

## Usage

### Starting the Server
```bash
./server [port]
```

### Connecting a Client
```bash
./client [server_ip] [port]
```

## Game Rules

- Each player has 6 basins containing 5 marbles each, plus a scoring basin (mancala)
- Players take turns selecting a basin and distributing marbles counterclockwise
- If the last marble lands in a player's scoring basin, they get another turn
- If the last marble lands in an empty basin on the player's side, they capture the opposite basin's marbles
- The game ends when one player's basins are empty
- All remaining marbles are moved to the corresponding player's scoring basin
- The player with the highest score wins

## Author

Gabriel Zubovsky (March 2025)

## License

Not specified

## Contributing

Contributions are welcome! Feel free to submit issues or pull requests.
