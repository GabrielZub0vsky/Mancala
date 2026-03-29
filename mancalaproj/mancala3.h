/*
 * mancala3.h
 *
 *  Created on: Mar 26, 2025
 *      Author: Gabriel Zubovsky
 */

#ifndef MANCALA3_H_
#define MANCALA3_H_

const int BASINS = 6;

struct Command {
	char cmd_type;
	char message[256];
};

struct Player {
    int basins[6];
    int score;
};

struct Board {
	struct Player opp_cpu;
	struct Player my_cpu;
};

struct Game {
    struct Board board;
    char turn;
};

struct MoveOutcome {
	int another_turn;
	int captured;
};

void doConnect(const char* ip_address, const int port);

void doWrite(char* msg, int fd);

void doRead(int fd, char* buf);

void parse_cmd(char* raw_cmd, struct Command* cmd);

void initPlayer(struct Player* player);

void initBoard(struct Board* board);

void initGame(struct Game* game);

void printBoard(struct Board* board);

int validateInput(struct Game* game, char turn, char* inp);

struct MoveOutcome makeMove(struct Player* player1, int basin, struct Player* player2, struct Board* board);

int checkEnd(struct Board* board);

int totalOfBasins(struct Player* player);

void printFinalScore(struct Game* game);

void marblesToScore(struct Player* player);

void endGame(struct Game* game);

int my_cpu_win_another_turn(struct Board* board);

int eval_my_cpu_capture(struct Board* board);

int eval_opp_cpu_capture(struct Board* board);

int pick_nonempty(struct Player* player);

#endif /* MANCALA3_H_ */
