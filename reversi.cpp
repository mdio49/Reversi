#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>

#include "reversi.h"

#define SCREEN_WIDTH TILE_SIZE * BOARD_SIZE
#define SCREEN_HEIGHT TILE_SIZE * BOARD_SIZE

#define AI_DELAY 250

// Function prototypes.
bool init();
bool load();
void loop();
void draw(State state);
void close();

void updateInput(SDL_Event e, State *state, int *aiPiece, int *aiDifficulty);
void drawBoard(SDL_Surface *surface, State state);

void gameReset(State *state);
void gameDoCurrentTurn(State *state, int x, int y, bool force);
void gameNextTurn(State *state);

bool playerCanMove(char board[BOARD_SIZE][BOARD_SIZE], int piece);
bool doAITurn(State *state, int aiDifficulty, int aiPiece, int aiTicks);

SDL_Rect getRect(int x, int y, int width, int height);
SDL_Rect getTileRect(int x, int y);
SDL_Surface *loadTexture(char *filename);

void aiTester(State *state, int *pass, int *whiteWins, int *blackWins, int *draws,
			  int whiteDiff, int blackDiff);

// The main rendering window.
SDL_Window *mainWindow = NULL;

// The surface contained by the main window.
SDL_Surface *mainSurface = NULL;

SDL_Surface *tile = NULL;
SDL_Surface *pieceWhite = NULL;
SDL_Surface *pieceBlack = NULL;
SDL_Surface *pieceWhiteHover = NULL;
SDL_Surface *pieceBlackHover = NULL;

// The main entry point of the program.
int main(int argc, char *argv[]) {
	if (!init()) {
		return EXIT_FAILURE;
	}

	if (!load()) {
		return EXIT_FAILURE;
	}

	loop();
	close();
	
	return EXIT_SUCCESS;
}

// MAIN FUNCTIONS

// Initializes the game and returns a value indicating if it was successful.
bool init() {
	// Initialize SDL.
	if (!SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}
	
	// Create the window.
	mainWindow = SDL_CreateWindow("Reversi", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
								  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (mainWindow == NULL) {
		fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// Gets the window surface.
	mainSurface = SDL_GetWindowSurface(mainWindow);

	return true;
}

// Loads content into the game and returns a value indicating if it was successful.
bool load() {
	// Load the board textures.
	tile = loadTexture("textures/tile.bmp");
	pieceWhite = loadTexture("textures/pieceWhite.bmp");
	pieceBlack = loadTexture("textures/pieceBlack.bmp");
	pieceWhiteHover = loadTexture("textures/pieceWhiteHover.bmp");
	pieceBlackHover = loadTexture("textures/pieceBlackHover.bmp");

	return tile != NULL && pieceWhite != NULL && pieceBlack != NULL &&
		pieceWhiteHover != NULL && pieceBlackHover != NULL;
}

// This function is called to initiate the start of the main game loop.
void loop() {
	// The current state of the game.
	State state = STATE_EMPTY;
	gameReset(&state);

	// Some variables for the AI.
	int aiDifficulty = AI_EASY; // The AI's difficulty.
	int aiPiece = PIECE_EMPTY; // The AI's piece color.
	int aiTicks = 0; // Used to delay the AI's move.

	// Seed the random number generator with the current time.
	srand(time(NULL));

	// An event handler.
	SDL_Event e;

	// Used to indicate if the game should exit its main loop.
	bool quit = false; 

	// Enter the main game loop.
	while (!quit) {
		// Handles events on the queue.
		while (SDL_PollEvent(&e) != 0) {
			// Checks if the user requests to exit.
			if (e.type == SDL_QUIT) {
				quit = true;
			} else {
				updateInput(e, &state, &aiPiece, &aiDifficulty);
			}
		}

		// Do a turn for the AI.
		if (!doAITurn(&state, aiDifficulty, aiPiece, aiTicks)) {
			aiTicks = SDL_GetTicks();
		}

		// Draw the game.
		draw(state);
	}
}

// Main drawing function.
void draw(State state) {
	// Fill the surface white.
	SDL_FillRect(mainSurface, NULL, SDL_MapRGB(mainSurface->format, 0xFF, 0xFF, 0xFF));

	// Draws the game board.
	drawBoard(mainSurface, state);

	// Update the surface.
	SDL_UpdateWindowSurface(mainWindow);
}

// This function is called to close the game and free resources.
void close() {
	// Destroy the window.
	SDL_DestroyWindow(mainWindow);
	mainWindow = NULL;

	// Frees surfaces.
	SDL_FreeSurface(tile);
	SDL_FreeSurface(pieceWhite);
	SDL_FreeSurface(pieceBlack);
	SDL_FreeSurface(pieceWhiteHover);
	SDL_FreeSurface(pieceBlackHover);

	// Quit SDL.
	SDL_Quit();
}

// MAIN HELPER FUNCTIONS

// Updates keyboard and mouse input.
void updateInput(SDL_Event e, State *state, int *aiPiece, int *aiDifficulty) {
	if (e.type == SDL_MOUSEBUTTONDOWN && state->turn != *aiPiece) {
		switch (e.button.button) {
			case SDL_BUTTON_LEFT:
				int x, y;
				SDL_GetMouseState(&x, &y);
				gameDoCurrentTurn(state, x / TILE_SIZE, y / TILE_SIZE, false);
				break;
			case SDL_BUTTON_RIGHT:
				gameDoCurrentTurn(state, MOVE_PASS, MOVE_PASS, false);
				break;
		}
	} else if (e.type == SDL_KEYDOWN) {
		switch (e.key.keysym.sym) {
			case SDLK_F1:
				gameReset(state);
				*aiPiece = PIECE_EMPTY;
				break;
			case SDLK_F2:
				gameReset(state);
				*aiDifficulty = AI_EASY;
				*aiPiece = PIECE_BLACK;
				break;
			case SDLK_F3:
				gameReset(state);
				*aiDifficulty = AI_MEDIUM;
				*aiPiece = PIECE_BLACK;
				break;
			case SDLK_F4:
				gameReset(state);
				*aiDifficulty = AI_HARD;
				*aiPiece = PIECE_BLACK;
				break;
			case SDLK_F5:
				gameReset(state);
				*aiDifficulty = AI_EXPERT;
				*aiPiece = PIECE_BLACK;
				break;
		}
	}
}

// Draws the game board.
void drawBoard(SDL_Surface *surface, State state) {
	for (int x = 0; x < BOARD_SIZE; x++) {
		for (int y = 0; y < BOARD_SIZE; y++) {
			if (state.board[x][y] == PIECE_WHITE) {
				SDL_BlitSurface(pieceWhite, NULL, surface, &getTileRect(x, y));
			} else if (state.board[x][y] == PIECE_BLACK) {
				SDL_BlitSurface(pieceBlack, NULL, surface, &getTileRect(x, y));
			} else {
				int mx, my;
				SDL_GetMouseState(&mx, &my);
				if (x == mx / 64 && y == my / 64) {
					SDL_Surface *hoverSurface = state.turn == PIECE_WHITE ? pieceWhiteHover : pieceBlackHover;
					SDL_BlitSurface(hoverSurface, NULL, surface, &getTileRect(x, y));
					SDL_BlitSurface(hoverSurface, NULL, surface, &getTileRect(x, y));
				} else {
					SDL_BlitSurface(tile, NULL, surface, &getTileRect(x, y));
					SDL_BlitSurface(tile, NULL, surface, &getTileRect(x, y));
				}
			}
		}
	}
}

// GAME FUNCTIONS

// Resets the game.
void gameReset(State *state) {
	boardReset(state->board);
	state->turn = PIECE_WHITE;
}

// Makes a move for the current player. A parameter can be inputted to indicate if
// the player should be forced to make a move (i.e. if the inputted move is invalid,
// then a random valid move will be made for the player).
void gameDoCurrentTurn(State *state, int x, int y, bool force) {
	if (x == MOVE_PASS || y == MOVE_PASS) {
		// Allow the player to pass if they have no valid moves.
		if (!playerCanMove(state->board, state->turn)) {
			gameNextTurn(state);
		}
	} else {
		int points = boardPlace(state->board, x, y, state->turn);
		if (points > 0) {
			gameNextTurn(state);
		} else if (force) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				for (int y = 0; y < BOARD_SIZE; y++) {
					// Makes the first valid move that can be made.
					points = boardPlace(state->board, x, y, state->turn);
					if (points > 0) {
						// Breaks out of the loop.
						x = BOARD_SIZE;
						y = BOARD_SIZE;
					}
				}
			}

			// Either the player has made a move, or the player does not have any legal
			// moves and must therefore pass their turn.
			gameNextTurn(state);
		}
	}
}

// Advances to the next turn.
void gameNextTurn(State *state) {
	if (state->turn == PIECE_WHITE) {
		state->turn = PIECE_BLACK;
	} else {
		state->turn = PIECE_WHITE;
	}
}

// Checks if a player with the given piece can make a move.
bool playerCanMove(char board[BOARD_SIZE][BOARD_SIZE], int piece) {
	for (int x = 0; x < BOARD_SIZE; x++) {
		for (int y = 0; y < BOARD_SIZE; y++) {
			int score = boardCheckMove(board, x, y, piece, false);
			if (score > 0) {
				return true;
			}
		}
	}

	return false;
}

// Makes a move for the AI if it is their turn and returns a value indicating whether
// or not it is their turn. The AI's turn will be delayed by a brief moment in order
// to prevent the AI from instantly making a move after the player.
bool doAITurn(State *state, int aiDifficulty, int aiPiece, int aiTicks) {
	if (state->turn == aiPiece) {
		// Wait a brief moment.
		if (SDL_GetTicks() - aiTicks >= AI_DELAY) {
			// Make a copy of the game board to pass into the AI to ensure that the AI
			// doesn't make any explicit changes to the game board.
			char board[BOARD_SIZE][BOARD_SIZE];
			boardCopy(board, state->board);

			// Obtain the desired x and y move from the AI.
			int x = MOVE_PASS, y = MOVE_PASS;
			aiMakeMove(board, aiDifficulty, aiPiece, &x, &y);
			gameDoCurrentTurn(state, x, y, true);
		}

		return true;
	}

	return false;
}

// BOARD FUNCTIONS

// Resets the board to the initial state of the game.
void boardReset(char board[BOARD_SIZE][BOARD_SIZE]) {
	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 8; y++) {
			if (x == 3 && y == 3 || x == 4 && y == 4) {
				board[x][y] = PIECE_WHITE;
			} else if (x == 3 && y == 4 || x == 4 && y == 3) {
				board[x][y] = PIECE_BLACK;
			} else {
				board[x][y] = PIECE_EMPTY;
			}
		}
	}
}

// Copies the elements of one board to another board.
void boardCopy(char output[BOARD_SIZE][BOARD_SIZE], char input[BOARD_SIZE][BOARD_SIZE]) {
	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 8; y++) {
			output[x][y] = input[x][y];
		}
	}
}

// Checks if the given move is valid and returns the score that would be earned
// for the given move. The last parameter indicates if any tiles should be changed.
int boardCheckMove(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int piece, bool change) {
	if (piece == PIECE_EMPTY || board[x][y] != PIECE_EMPTY) {
		return 0;
	}

	int score = 0;
	score += doPieceTurnovers(board, x, y, 1, 0, piece, change);
	score += doPieceTurnovers(board, x, y, -1, 0, piece, change);
	score += doPieceTurnovers(board, x, y, 0, 1, piece, change);
	score += doPieceTurnovers(board, x, y, 0, -1, piece, change);
	score += doPieceTurnovers(board, x, y, 1, 1, piece, change);
	score += doPieceTurnovers(board, x, y, 1, -1, piece, change);
	score += doPieceTurnovers(board, x, y, -1, 1, piece, change);
	score += doPieceTurnovers(board, x, y, -1, -1, piece, change);
	return score;
}

// Places a piece on the board.
int boardPlace(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int piece) {
	// Performs any turnovers.
	int score = boardCheckMove(board, x, y, piece, true);

	// If at least one piece could be turned over, then place the piece on the board.
	if (score > 0) {
		board[x][y] = piece;
	}

	return score;
}

// Turns over pieces in the given direction and returns a value indicating the
// number of pieces turned over.
int doPieceTurnovers(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int dx, int dy,
					 int piece, bool change) {
	for (int i = 1; x + (i * dx) >= 0 && x + (i * dx) < BOARD_SIZE &&
		 y + (i * dy) >= 0 && y + (i * dy) < BOARD_SIZE; i++) {
		int currentPiece = board[x + (i * dx)][y + (i * dy)];
		if (currentPiece == piece) {
			if (change == true) {
				// Changes the tiles.
				for (int j = i - 1; j > 0; j--) {
					board[x + (j * dx)][y + (j * dy)] = piece;
				}
			}

			// Returns the number of tiles changed.
			return i - 1;
		}

		// Stops if the tile is empty.
		if (currentPiece == PIECE_EMPTY) {
			break;
		}
	}

	// No pieces were turned over.
	return 0;
}

// MISC. HELPER FUNCTIONS

// Returns a rectangle with the given parameters.
SDL_Rect getRect(int x, int y, int width, int height) {
	SDL_Rect rect = { x, y, width, height };
	return rect;
}

// Gets a rectangle to draw a tile at the given x and y tile position.
SDL_Rect getTileRect(int x, int y) {
	return getRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
}

// Loads a texture and outputs an error if it couldn't be loaded.
SDL_Surface *loadTexture(char *filename) {
	SDL_Surface *surface = SDL_LoadBMP(filename);
	if (surface == NULL) {
		fprintf(stderr, "Unable to load board textures! SDL_Error: %s\n", SDL_GetError());
	}

	return surface;
}

// FUNCTIONS USED FOR TESTING AND DEBUGGING

// A function used to test two AIs of different difficulty against each other.
void aiTester(State *state, int *pass, int *whiteWins, int *blackWins, int *draws,
			  int whiteDiff, int blackDiff) {
	if (state->turn == PIECE_WHITE) {
		int x = 0, y = 0;
		aiMakeMove(state->board, whiteDiff, PIECE_WHITE, &x, &y);
		gameDoCurrentTurn(state, x, y, true);

		if (x == MOVE_PASS && y == MOVE_PASS) {
			(*pass)++;
		} else {
			*pass = 0;
		}
	} else if (state->turn == PIECE_BLACK) {
		int x = 0, y = 0;
		aiMakeMove(state->board, blackDiff, PIECE_BLACK, &x, &y);
		gameDoCurrentTurn(state, x, y, true);

		if (x == MOVE_PASS && y == MOVE_PASS) {
			(*pass)++;
		} else {
			*pass = 0;
		}
	}

	if (*pass == 2) {
		int whitePieces = 0;
		int blackPieces = 0;

		for (int x = 0; x < BOARD_SIZE; x++) {
			for (int y = 0; y < BOARD_SIZE; y++) {
				if (state->board[x][y] == PIECE_WHITE) {
					whitePieces++;
				} else if (state->board[x][y] == PIECE_BLACK) {
					blackPieces++;
				}
			}
		}

		if (whitePieces > blackPieces) {
			(*whiteWins)++;
		} else if (blackPieces > whitePieces) {
			(*blackWins)++;
		} else {
			(*draws)++;
		}

		printf("White: %d | Black: %d | Draw: %d\n", *whiteWins, *blackWins, *draws);

		gameReset(state);
		*pass = 0;
	}
}
