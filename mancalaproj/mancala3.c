/*
 * mancalaclient.c
 *
 *  Created on: Mar 26, 2025
 *      Author: Gabriel Zubovsky
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "mancala3.h"

int serverFd = -1;

void initPlayer(struct Player* player) {
    for(int i = 0; i < BASINS; i++){
        player->basins[i] = 5;
    }
    player->score = 0;
}

void initBoard(struct Board* board) {
    initPlayer(&board->opp_cpu);
    initPlayer(&board->my_cpu);
}

void initGame(struct Game* game) {
    initBoard(&(game->board));
    game->turn = 'P';
}

/** printBoard()
 * Prints out the current mancala board in a neatly formatted manner.
 */
void printBoard(struct Board* board){
    printf("\n  ");
    for(int i = 0; i < BASINS; i++){
       printf("%5d", BASINS - i);
    }

    printf("\nME:");
    for(int i = 0; i < BASINS; i++){
        printf("%5d", board->my_cpu.basins[BASINS - i - 1]);
    }

    printf("\n%d%37d", board->my_cpu.score, board->opp_cpu.score);

    printf("\n  ");
    for(int i = 0; i < BASINS; i++){
        printf("%5d", board->opp_cpu.basins[i]);
    }
    printf("    :OPP");

    printf("\n  ");
    for(int i = 0; i < BASINS; i++){
        printf("%5d", i+1);
    }
    printf("\n\n");
}

/** validateInput()
 * Checks validity of player input.
 */
int validateInput(struct Game* game, char turn, char* inp){
    int inp_length = strlen(inp);

    if(inp_length == 1){
        int index = atoi(inp);

        if(index >= 1 && index <= 6){
            if(game->turn == 'P'){
                return (game->board.opp_cpu.basins[index-1] > 0) ? index : 0;
            }
            if(game->turn == 'C'){
                return (game->board.my_cpu.basins[index-1] > 0) ? index : 0;
            }
        }
    }
    return 0;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/** makeMove()
 * Makes the move as requested by the player.
 */
struct MoveOutcome makeMove(struct Player* me, int basin /*1..6*/, struct Player* opp, struct Board* board)
{
	struct MoveOutcome move_outcome;
	move_outcome.another_turn = 0;
	move_outcome.captured = 0;

    int marbles = me->basins[basin - 1];
    me->basins[basin - 1] = 0;
    while(marbles > 0){
        if(marbles > 0){
            /*
            * Distribute marbles into remaining own basins.
            */
            int my_remaining_basins = MIN(BASINS - basin, marbles);
            //printf("marbles: %d, my remaining basins: %d\n", marbles, my_remaining_basins);
            for(int i = 0; i < my_remaining_basins; i++){
                me->basins[basin + i]++;
                marbles--;

                if(marbles == 0 && me->basins[basin + i] == 1){
                    int opp_marbles_across = opp->basins[5 - (basin + i)];
                    opp->basins[5 - (basin + i)] = 0;
                    me->basins[basin + i] = 0;
                    me->score += (1 + opp_marbles_across);
                    move_outcome.captured = 1 + opp_marbles_across;
                }
                //printBoard(board);
            }
        }
        if(marbles > 0){
            /*
            * Drop one marble into own score.
            */
            //printf("marbles: %d, my score: %d\n", marbles, me->score);
            me->score++;
            marbles--;
            //printf("marbles: %d, my score: %d\n", marbles, me->score);
            if(marbles == 0){
                // if marble dropped into score is the last marble
            	move_outcome.another_turn = 1;
                return move_outcome;
            }
            //printBoard(board);
        }
        if (marbles > 0) {
            /*
             * Distribute remaining marbles not to exceed opponent's basins.
             */
            int opp_basins = MIN(marbles, BASINS);
            //printf("marbles: %d, opp basins: %d\n", marbles, opp_basins);
            for (int i = 0; i < opp_basins; i++) {
                opp->basins[i]++;
                marbles--;
            }
            //printBoard(board);
        }

        /*
         * Reset basin to start from the beginning of my basins.
         */
        basin = 0;
    }

    return move_outcome;
}

/** checkEnd()
 * Checks the board after every turn to see if the game needs to end.
 */
int checkEnd(struct Board* board){
    if(totalOfBasins(&(board->opp_cpu)) == 0 || totalOfBasins(&(board->my_cpu)) == 0){
        return 1;
    }
    return 0;
}

/** totalOfBasins()
 * Calculates the total amount of marbles in a player's basins, excluding the scoring basin.
 */
int totalOfBasins(struct Player* player){
    int total = 0;
	for(int i = 0; i < BASINS; i++){
		total += player->basins[i];
	}
    return total;
}

/** printFinalScore()
 * Prints the formatted score of the game once it ends.
 */
void printFinalScore(struct Game* game){
    if(game->turn == 'E'){
        printf("Final score:\nPlayer: %d\nCPU: %d", game->board.opp_cpu.score, game->board.my_cpu.score);
    }
}

/** marblesToScore
 * Sums a player's basins to the scoring.
 */
void marblesToScore(struct Player* player){
	player->score += totalOfBasins(player);
	for(int i = 0; i < BASINS; i++){
		player->basins[i] = 0;
	}
}

/** endGame()
 * Ends the mancala game once one player has no marbles left in basins.
 */
void endGame(struct Game* game){
    if(totalOfBasins(&(game->board.opp_cpu)) == 0){
        marblesToScore(&(game->board.my_cpu));
    }
    else {
        marblesToScore(&(game->board.opp_cpu));
    }
}

/**
 * Analyzes the board for moves that will win another turn
 * and returns a basin number for the CPU to play.
 */
int my_cpu_win_another_turn(struct Board* board) {
	int bonus_turn[BASINS];
	for(int n = 0; n < BASINS; n++){
		struct Board board_clone;
		for(int i = 0; i < BASINS; i++){
			board_clone.opp_cpu.basins[i] = board->opp_cpu.basins[i];
			board_clone.my_cpu.basins[i] = board->my_cpu.basins[i];
		}
		if(board_clone.my_cpu.basins[n] > 0){
			bonus_turn[n] = makeMove(&board_clone.my_cpu, n+1, &board_clone.opp_cpu, &board_clone).another_turn;
		}
		else {
			bonus_turn[n] = 0;
		}
	}
	int closest_scoring = 0;
	for(int b = BASINS - 1; b >= 0; b--){
		if(bonus_turn[b] == 1){
			closest_scoring = b + 1;
		}
	}
	return closest_scoring;
}


/**
 * Analyzes the board for the move that results in the largest possible capture
 * of my CPU's marbles by the opp cpu and returns a basin number for my CPU to play.
 */
int eval_opp_cpu_capture(struct Board* board){
	int captured[BASINS];
	for(int n = 0; n < BASINS; n++){
		struct Board board_clone;
		for(int i = 0; i < BASINS; i++){
			board_clone.opp_cpu.basins[i] = board->opp_cpu.basins[i];
			board_clone.my_cpu.basins[i] = board->my_cpu.basins[i];
		}
		if(board_clone.opp_cpu.basins[n] > 0){
			captured[n] = makeMove(&board_clone.opp_cpu, n+1, &board_clone.my_cpu, &board_clone).captured;
		}
		else {
			captured[n] = 0;
		}
	}

	int most_captured, most_captured_basin = 0;
	for(int b = 0; b < BASINS; b++){
		if(captured[b] > most_captured){
			most_captured = captured[b];
			most_captured_basin = b;
		}
	}

	if(board->opp_cpu.basins[most_captured_basin] == 0){
		most_captured_basin = pick_nonempty(&(board->opp_cpu)) - 1;
	}

	return BASINS - most_captured_basin;
}

/**
 * Analyzes the board for the move that results in the largest capture
 * of opp CPU's marbles and returns a basin number for my CPU to play.
 */
int eval_my_cpu_capture(struct Board* board) {
	int captured[BASINS];
	for(int n = 0; n < BASINS; n++){
		struct Board board_clone;
		for(int i = 0; i < BASINS; i++){
			board_clone.opp_cpu.basins[i] = board->opp_cpu.basins[i];
			board_clone.my_cpu.basins[i] = board->my_cpu.basins[i];
		}
		if(board_clone.my_cpu.basins[n] > 0){
			captured[n] = makeMove(&board_clone.my_cpu, n+1, &board_clone.opp_cpu, &board_clone).captured;
		}
		else {
			captured[n] = 0;
		}
	}
	int most_captured = 0;
	int most_captured_basin = 0;
	for(int b = 0; b < BASINS; b++){
		if(captured[b] > most_captured){
			most_captured = captured[b];
			most_captured_basin = b + 1;
		}
	}

	if(board->my_cpu.basins[most_captured_basin] == 0){
		most_captured_basin = pick_nonempty(&(board->my_cpu));
	}

	return most_captured_basin;
}


/**
 * Chooses, at random, a non-empty basin for the CPU to play.
 */
int pick_nonempty(struct Player* player){
	srand(time(NULL));
	for(int a = 0; a < 25; a++){
		int rand_basin = (rand() % 6);
		if(player->basins[rand_basin] > 0){
			//printf("picking random basin %d\n", rand_basin);
			return rand_basin + 1;
		}
	}
	// if there are many empty basins, pick first non-empty
	for(int i = 0; i < BASINS; i++){
		if(player->basins[i] > 0){
			return i + 1;
		}
	}

	return 3;
}

/**
 *
 */
void doConnect(const char* ip_address, const int port) {
	// Create socket
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd < 0) {  perror("socket() fail");  exit(1);  }

	// Connect to remote server
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port); // TCP
	addr.sin_addr.s_addr = inet_addr(ip_address);
	const socklen_t addrlen = sizeof(addr);

	int retval = connect(serverFd, (struct sockaddr*) &addr, addrlen);

	if (retval < 0) {
		perror("connect() fail.");
		exit(1);
	}
}

/**
 *
 */
void doRead(int fd, char* buf) {
	//printf("reading from %d\n", fd);

	char cmd_length[1];
	int read_len = read(fd, cmd_length, 1);
	if (read_len > 0) {
		int count = cmd_length[0];
		//printf("input has %d bytes\n", count);

		int read_cmd = read(fd, buf, count);
		if (read_cmd > 0) {
			buf[read_cmd] = '\0';

			printf("\n----------------------------\nClient received: '%s' (%d bytes)\n", buf, strlen(buf));
		}
		else {
			printf("read command failed: %d (%d:%s)\n", read_cmd, errno, strerror(errno));
		}
	}
	else if (read_len == 0) {
		// nothing to read
	}
	else {
		printf("read byte count failed: %d (%d:%s)\n", read_len, errno, strerror(errno));
	}
}

/**
 *
 */
void doWrite(char* msg, int clientFd) {
	//printf("message %s has %d bytes\n", msg, strlen(msg));

	char msg2[257];
	msg2[0] = strlen(msg) + 1;
	strncpy(msg2+1, msg, 256);
	//printf("writing %s (%d bytes)\n", msg2, strlen(msg2));

	int bytes_to_write = strlen(msg2)+1;
	const int bytes_written = write(clientFd, msg2, bytes_to_write);
	if (bytes_written == bytes_to_write) {
		printf("\tClient wrote %s (%d bytes)\n", msg2, bytes_to_write);
	}
	else {
		printf("write failed: %d\n", bytes_written);
	}
}

/**
 *	Parses raw commands sent by the remote server.
 */
void parse_cmd(char* raw_cmd, struct Command* cmd){
	if(strncmp(raw_cmd,"LOGIN", 5) == 0){
		cmd->cmd_type = 'L';
	}
	else if(strncmp(raw_cmd,"NEWGAME", 7) == 0){
		cmd->cmd_type = 'N';
	}
	else if(strncmp(raw_cmd,"PLAY", 4) == 0){
		cmd->cmd_type = 'P';
	}
	else if(strncmp(raw_cmd,"OPP", 3) == 0){
		cmd->cmd_type = 'O';
		strncpy(cmd->message, raw_cmd+4, 1);
	}
	else if(strncmp(raw_cmd,"DONE", 4) == 0){
		cmd->cmd_type = 'D';
		strncpy(cmd->message, raw_cmd+5, 256);
	}
	else {
		printf("Invalid command: %s\n", raw_cmd);
	}
}

/**
 * Evaluates all possible moves to respond with and returns the best one.
 */
int eval_response(struct Board* board){
	int move = 1;
	int (*criteria[3])(struct Board*) = {my_cpu_win_another_turn, eval_opp_cpu_capture, eval_my_cpu_capture};
	for(int i = 0; i < 3; i++){
		move = criteria[i](board);
		printf("after analyzing criterion %d, optimal basin is %d\n", i+1, move);
		if(move > 0){
			break;
		}
	}
	return move;
}

/**
 *
 */
int main(int argc, char** argv){
	if(argc != 3){
		perror("Usage:  IP_ADDR  PORT");
		exit(1);
	}

	const char* ip_addr = argv[1];
	const int port = atoi(argv[2]);
	printf("Connecting to %s:%d\n", ip_addr, port);

	doConnect(ip_addr, port);
	printf("Client connected: %d\n", serverFd);

	struct Game* game = NULL;
	int client_active = 1;

	while(client_active){
		char buffer[1024];
		buffer[0] = 0;
		doRead(serverFd, buffer);
		if (strlen(buffer) > 0) {
			struct Command cmd;
			cmd.cmd_type = 0;
			cmd.message[0] = 0;

			parse_cmd(buffer, &cmd);
			printf("\tCommand: %c, message: %s\n\n", cmd.cmd_type, cmd.message);

			switch(cmd.cmd_type){
				case 'L':
					doWrite("GabrielZub0vsky:GHZ", serverFd);
					break;

				case 'N':
					if(game != NULL){
						printFinalScore(game);
						free(game);
					}
					game = malloc(sizeof(struct Game));
					initGame(game);
					break;

				case 'P':
					int move = eval_response(&(game->board));

					if(game->board.my_cpu.basins[move-1] == 0){
						move = pick_nonempty(&(game->board.my_cpu));
					}

					makeMove(&(game->board.my_cpu), move, &(game->board.opp_cpu), &(game->board));
					printf("\nAfter my move: \n");
					printBoard(&(game->board));

					char my_move[1];
					sprintf(my_move, "%d", move);
					doWrite(my_move, serverFd);

					break;

				case 'O':
					makeMove(&(game->board.opp_cpu), atoi(cmd.message), &(game->board.my_cpu), &(game->board));
					printf("\nAfter opp move: \n");
					printBoard(&(game->board));
					break;

				case 'D':
					client_active = 0;
					printf("Ending... message to client: %s\n", cmd.message);
					break;
			}
		}
	}

	// cleanup
	close(serverFd);
	free(game);
	return 0;
}





