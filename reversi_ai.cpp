#include <stdlib.h>

#include "reversi.h"

#define MAX_DEPTH 5

struct _Move {
	int x;
	int y;
	int score;
	struct _Move *next;
};

// Stores information about a move.
typedef struct _Move Move;

// Function prototypes.
void aiMakeMove_Easy(char board[BOARD_SIZE][BOARD_SIZE], Move *validMoves, int piece,
					 int *x, int *y);
void aiMakeMove_Medium(char board[BOARD_SIZE][BOARD_SIZE], Move *validMoves, int piece,
					   int *x, int *y);
void aiMakeMove_Hard(char board[BOARD_SIZE][BOARD_SIZE], Move *validMoves, int piece,
					 int *x, int *y);
void aiMakeMove_Expert(char board[BOARD_SIZE][BOARD_SIZE], Move *validMoves, int piece,
					   int *x, int *y);

Move *getValidMoves(char board[BOARD_SIZE][BOARD_SIZE], int piece);
Move getBestMove(char board[BOARD_SIZE][BOARD_SIZE], int piece, int depth);
int getHighestScoringMove(char board[BOARD_SIZE][BOARD_SIZE], int piece);

Move *insertMove(Move *head, int x, int y, int score);
void freeMoveList(Move *head);

int getOpponentPiece(int piece);

// Calls for the AI to make a move. Inputs the current board along with the difficulty
// and piece color of the AI. The x and y integer pointers in the function parameters
// should be changed to output the desired move by the AI.
void aiMakeMove(char board[BOARD_SIZE][BOARD_SIZE], int difficulty, int piece, int *x, int *y) {
	// Get a linked list containing each valid move.
	Move *validMoves = getValidMoves(board, piece);

	switch (difficulty) {
		case AI_EASY:
			aiMakeMove_Easy(board, validMoves, piece, x, y);
			break;
		case AI_MEDIUM:
			aiMakeMove_Medium(board, validMoves, piece, x, y);
			break;
		case AI_HARD:
			aiMakeMove_Hard(board, validMoves, piece, x, y);
			break;
		case AI_EXPERT:
			aiMakeMove_Expert(board, validMoves, piece, x, y);
			break;
	}

	// Free the list of valid moves from memory.
	freeMoveList(validMoves);
}

// Calls for the easy difficulty AI to make a move. This AI will simply
// choose a random valid move.
void aiMakeMove_Easy(char board[BOARD_SIZE][BOARD_SIZE], Move *validMoves, int piece,
					 int *x, int *y) {
	// Counts the number of valid moves.
	int count = 0;
	Move *move = validMoves;
	while (move != NULL) {
		count++;
		move = move->next;
	}

	if (count > 0) {
		// Selects a move randomly.
		int index = rand() % count;
		move = validMoves;
		for (int i = 0; i < index; i++) {
			move = move->next;
		}

		*x = move->x;
		*y = move->y;
	} else {
		*x = MOVE_PASS;
		*y = MOVE_PASS;
	}
}

// Calls for the medium difficulty AI to make a move. This AI will simply choose
// a valid move that yields the most amount of points.
void aiMakeMove_Medium(char board[BOARD_SIZE][BOARD_SIZE], Move *validMoves, int piece,
					   int *x, int *y) {
	// Find the move which grants the highest score.
	int count = 0;
	Move *best = NULL;
	Move *move = validMoves;
	while (move != NULL) {
		if (best == NULL || move->score > best->score) {
			best = move;
			count = 1;
		} else if (move->score == best->score) {
			count++;
		}

		move = move->next;
	}

	if (best != NULL) {
		// If there is more than one move which gives the highest score, then 
		// choose one of those moves randomly to have variation in play.
		int index = rand() % count;
		move = validMoves;
		while (index >= 0) {
			if (move->score == best->score) {
				index--;
			}

			if (index >= 0) {
				move = move->next;
			}
		}

		*x = move->x;
		*y = move->y;
	} else {
		*x = MOVE_PASS;
		*y = MOVE_PASS;
	}
}

// Calls for the hard difficulty AI to make a move. This AI will look one move
// ahead and choose a valid move that has the greatest difference between the
// highest score the AI can earn and the highest score the opponent can earn.
void aiMakeMove_Hard(char board[BOARD_SIZE][BOARD_SIZE], Move *validMoves, int piece,
					 int *x, int *y) {
	int count = 0;
	Move *best = NULL;
	Move *move = validMoves;
	while (move != NULL) {
		// Create a fake board simulating the move.
		char tempBoard[BOARD_SIZE][BOARD_SIZE];
		boardCopy(tempBoard, board);
		boardPlace(tempBoard, move->x, move->y, piece);

		// Get the difference between the score earned by this move and the highest move that
		// the opponent can make in the next turn. If this difference is higher than the
		// largest difference currently found, then update the new best move.
		int opponent = getOpponentPiece(piece);
		move->score -= getHighestScoringMove(tempBoard, opponent);
		if (best == NULL || move->score > best->score) {
			best = move;
			best->score == move->score;
			count = 1;
		} else if (move->score == best->score) {
			count++;
		}

		move = move->next;
	}

	if (best != NULL) {
		int index = rand() % count;
		move = validMoves;
		while (index >= 0) {
			if (move->score == best->score) {
				index--;
			}

			if (index >= 0) {
				move = move->next;
			}
		}

		*x = move->x;
		*y = move->y;
	} else {
		*x = MOVE_PASS;
		*y = MOVE_PASS;
	}
}

// Calls for the expert difficulty AI to make a move. This AI will analyse a game tree
// up to a predefined maximum depth and use minimax to determine the best move.
void aiMakeMove_Expert(char board[BOARD_SIZE][BOARD_SIZE], Move *validMoves, int piece,
					 int *x, int *y) {
	Move move = getBestMove(board, piece, 0);
	*x = move.x;
	*y = move.y;
}

// Gets a linked list of moves valid for the player with the given piece.
Move *getValidMoves(char board[BOARD_SIZE][BOARD_SIZE], int piece) {
	Move *validMoves = NULL;
	for (int x = 0; x < BOARD_SIZE; x++) {
		for (int y = 0; y < BOARD_SIZE; y++) {
			int score = boardCheckMove(board, x, y, piece, false);
			if (score > 0) {
				validMoves = insertMove(validMoves, x, y, score);
			}
		}
	}

	return validMoves;
}

// Gets the best move that a player can make using a minimax algorithm.
Move getBestMove(char board[BOARD_SIZE][BOARD_SIZE], int piece, int depth) {
	if (depth > MAX_DEPTH) {
		Move move = { MOVE_PASS, MOVE_PASS, 0, NULL };
		return move;
	}

	Move *validMoves = getValidMoves(board, piece);

	// Find the move which grants the highest score.
	Move best = { MOVE_PASS, MOVE_PASS, 0, NULL };
	Move *move = validMoves;
	while (move != NULL) {
		// Create a fake board simulating the move.
		char tempBoard[BOARD_SIZE][BOARD_SIZE];
		boardCopy(tempBoard, board);
		boardPlace(tempBoard, move->x, move->y, piece);

		int opponent = getOpponentPiece(piece);
		Move m = getBestMove(tempBoard, opponent, depth + 1);
		int score = move->score - m.score;
		if (best.x == MOVE_PASS || score > best.score) {
			best.x = move->x;
			best.y = move->y;
			best.score = score;
		}

		move = move->next;
	}

	freeMoveList(validMoves);
	return best;
}

// Gets the value of the highest scoring move.
int getHighestScoringMove(char board[BOARD_SIZE][BOARD_SIZE], int piece) {
	Move *validMoves = getValidMoves(board, piece);

	// Find the move which grants the highest score.
	Move *best = NULL;
	Move *move = validMoves;
	while (move != NULL) {
		if (best == NULL || move->score > best->score) {
			best = move;
		}

		move = move->next;
	}

	int score = best != NULL ? best->score : 0;
	freeMoveList(validMoves);
	return score;
}

// Inserts a new move node into the linked list and returns the new head.
Move *insertMove(Move *head, int x, int y, int score) {
	Move *move = (Move*)malloc(sizeof(Move));
	move->x = x;
	move->y = y;
	move->score = score;
	move->next = head;
	return move;
}

// Frees a linked list of moves from memory.
void freeMoveList(Move *head) {
	Move *move = head;
	while (move != NULL) {
		Move *next = move->next;
		free(move);
		move = next;
	}
}

// Gets the piece of the opposite color.
int getOpponentPiece(int piece) {
	return piece == PIECE_WHITE ? PIECE_BLACK : PIECE_WHITE;
}
