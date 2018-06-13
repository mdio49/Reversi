#pragma once

#define BOARD_SIZE 8
#define TILE_SIZE 64

#define PIECE_EMPTY 0
#define PIECE_WHITE 1
#define PIECE_BLACK 2

#define MOVE_PASS -1

#define AI_EASY 1
#define AI_MEDIUM 2
#define AI_HARD 3
#define AI_EXPERT 4

struct _State {
	// The piece on each tile on the board.
	char board[BOARD_SIZE][BOARD_SIZE];

	// The piece of the current player.
	int turn;
};

// Stores information about the state of a Reversi game.
typedef struct _State State;

#define STATE_EMPTY { { { 0 } }, 0 }

// Resets the board to the initial state of the game.
void boardReset(char board[BOARD_SIZE][BOARD_SIZE]);

// Copies the elements of one board to another board.
void boardCopy(char output[BOARD_SIZE][BOARD_SIZE], char input[BOARD_SIZE][BOARD_SIZE]);

// Checks if the given move is valid and returns the score that would be earned
// for the given move. The last parameter indicates if any tiles should be changed.
int boardCheckMove(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int piece, bool change);

// Places a piece on the board.
int boardPlace(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int piece);

// Turns over pieces in the given direction and returns a value indicating the
// number of pieces turned over.
int doPieceTurnovers(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int dx, int dy,
					 int piece, bool change);

// Calls for the AI to make a move. Inputs the current board along with the difficulty
// and piece color of the AI. The x and y integer pointers in the function parameters
// should be changed to output the desired move by the AI.
void aiMakeMove(char board[BOARD_SIZE][BOARD_SIZE], int difficulty, int piece, int *x, int *y);
