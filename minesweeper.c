#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Make a struct for the squares on the board. 
typedef struct s {
	unsigned char isFlagged;
	unsigned char isBomb;
	unsigned char isOpened;
	unsigned char bombsSurrounding;
} square;

// Make a struct for the decision the user makes.
typedef struct d {
	unsigned char x;
	unsigned char y;
	unsigned char choice;
} decision;

// Globally define the board and its dimensions.
unsigned char width = 0; // max 255
unsigned char height = 0; // max 255
square **board;

// This function initializes the game.
void init(){

	// Seed rand with time.
	srand(time(0));

	// Say what we're doing.
	printf("%s\n", "Setting up the game...");

	// Allocate the memory for board.
	board = (square**) malloc(sizeof(square*) * height);
	for (int i=0; i<height; i++) board[i] = (square*) malloc(sizeof(square) * width);

	// Iterate through squares setting bombsSurrounding to 0.
	for (int i=0; i<height; i++) for (int j=0; j<width; j++) board[i][j].bombsSurrounding = 0;

	// Iterate through squares setting bools and neighbors' bomb-counts.
	for (int i=0; i<height; i++) for (int j=0; j<width; j++){

		// Set isOpened and isFlagged bools.
		board[i][j].isOpened = 0;
		board[i][j].isFlagged = 0;

		// Square is not a bomb if (rand % 4) is truthy, happens 1/4 of the time.
		board[i][j].isBomb = rand() % 4 ? 0:1;

		// If we're a bomb, iterate through neighboring squares, check range, increment bomb-count.
		if (board[i][j].isBomb) for (int m=i-1; m<i+2; m++) for (int n=j-1; n<j+2; n++) if (m>=0 && m<height && n>=0 && n<width) board[m][n].bombsSurrounding++;
	}
}

// This function tears down the game.
void teardown(){

	// Say what we're doing and free the board.
	printf("%s\n\n", "Ending the game...");
	free(board);
}

// This function presents the user with options for gameplay, and takes coordinates if applicable.
// Data is stored in the decision struct, the pointer to which is passed.
void getChar(decision *data){

	// Define acceptable inputs and flag for input-checking loop to exit.
	char acceptable[] = {'f','F','r','R','a','A','q','Q'};
	char flag = 0;

	// Loop to get user's input, and check that choice is valid.
	do {

		// Flush stdin.
		fflush(stdin);

		// Display options and prompt user.
		printf("\n%s", "[F]lag a mine, [R]emove a flag, [A]ssert square bomb-free, [Q]uit: ");

		// Get user's choice and add newline.
		scanf(" %c", (char*) &(data->choice));
		printf("%s\n", "");

		// Check if input is valid, if so, lowercase choice and set flag true.
		for (int i=0; i<8; i++) if (data->choice == acceptable[i]){
			if (i%2 == 1) data->choice = acceptable[i-1];
			flag = 1;
			break;
		}
	} while (!flag);
	
	// If we're not quitting, get x and y coordinates.
	if (data->choice != 'q'){

		// Get coordinates, looping until valid. Check that values are within range.
		do {

			// Get horizontal coordinate
			printf("%s%d%s", "Enter valid x coordinate 0-", width-1, ": ");
			scanf("%*[^0-9]%hhu%*[0-9^]", &(data->x));

			// Get vertical coordinate
			printf("%s%d%s", "Enter valid y coordinate 0-", height-1, ": ");
			scanf("%*[^0-9]%hhu%*[0-9^]", &(data->y));

			// Flush stdin.
			scanf("%*[^\n]");

		} while (data->x>=width || data->x<0 || data->y>=height || data->y<0);
	}
}

// Helper function that returns 2 when user has won, 0 otherwise.
// Winning is defined as all bombs are flagged, no non-bombs are flagged.
int isWon(){
	// Iterate through squares. If bomb condition doesn't match flagged condition for all squares, user hasn't won. Otherwise, they have.
	for (int i=0; i<height; i++) for (int j=0; j<width; j++) if (board[i][j].isBomb != board[i][j].isFlagged) return 0;
	return 2;
}

// This function reveals a square and, if applicable, its appropriate neighbors. Pass x and y of square to reveal.
// ALGORITHM: The square is opened. If the square has no bombs surrounding, reveal is called on each neighbor that is not opened.
// To specifiy neighbors, a double for loop is used and boundaries are tested, like in the init function.
void reveal(unsigned char x, unsigned char y){

	// Open the square.
	board[y][x].isOpened = 1;
	// If square has no bombs surounding, iterate through neighbors with double for-loop, test boundaries, then test whether neighbor square is opened. If not, open it.
	if (!board[y][x].bombsSurrounding) for (int m=y-1; m<y+2; m++) for (int n=x-1; n<x+2; n++) if (m>=0 && m<height && n>=0 && n<width) if (!board[m][n].isOpened) reveal(n,m);
}

// This function uses the decision struct to update the board.
// The function returns a 1 if gameover, 2 if won, otherwise 0.
char update(decision *data){
	
	// Switch through user choices.
	switch (data->choice){
		case 'f':
			board[data->y][data->x].isFlagged = 1;
			break;
		case 'r':
			board[data->y][data->x].isFlagged = 0;
			break;
		case 'a':
			reveal(data->x, data->y);
			if (board[data->y][data->x].isBomb) return 1;
	}
	// Use isWon function to determine win state, and return.
	return isWon();
}

// This function prints the state of the board.
void display(){

	// Define full width characters. NOTE: these are not regular digits, they are special in Unicode.
	const char fullwidth[9][4] = {"０","１","２","３","４","５","６","７","８"};

	// Iterate through rows (i) and columns (j).
	for (int i=0; i<height; i++){
		for (int j=0; j<width; j++){

			// Check if square is opened.
			if (board[i][j].isOpened){

				// Check if square is bomb.
				if (board[i][j].isBomb) printf("%s", "＠");
				else printf("%s", fullwidth[board[i][j].bombsSurrounding]);
			}
			// Check if square is flagged.
			else if (board[i][j].isFlagged) printf("%s", "＃");
			else printf("%s", "＇");
		}
		// Print a newline.
		printf("%s\n", "");
	}
}

// Main function, called on execution.
int main(int argc, char const *argv[]){

	// Check argument count and validity of arguments, if good set width and height.
	if (argc == 3 && atoi(argv[1]) < 256 &&  atoi(argv[2]) < 256 && atoi(argv[1]) > 0 &&  atoi(argv[2]) > 0){
		width = (char) atoi(argv[1]);
		height = (char) atoi(argv[2]);
	}

	// Otherwise, inform user and exit.
	else {
		printf("%s%s%s\n", "Please pass valid width and height (max 255 each) ex:\n$ ", argv[0], " 10 10");
		return 0;
	}

	// Setup the game and make var for gameover.
	init();
	char gameover = 0;

	// Make data var and loop until user quits, then free decision.
	decision *data = (decision*) malloc(sizeof(decision));
	do {

		// Display the board, get choice, and update.
		display();
		getChar(data);
		gameover = update(data);

		// On loss, open all bombs and print BOOM.
		if (gameover == 1) {
			for (int i=0; i<height; i++) for (int j=0; j<width; j++) if (board[i][j].isBomb) board[i][j].isOpened = 1;
			display();
			printf("%s\n", "BOOM! Game over.");
		}

		// On win, print CONGRATS.
		else if (gameover == 2) printf("%s\n", "CONGRATS! You win.");
		// Otherwise print OK.
		else printf("%s\n", "OK!");

	} while (data->choice != 'q' && !gameover);

	// Free decision.
	free(data);

	// Finally, teardown and return.
	teardown();
	return 0;
}