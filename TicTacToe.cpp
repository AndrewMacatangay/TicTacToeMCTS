#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <iomanip>
//#include "stdafx.h"
#include "decisionTree.h"
using namespace std;

//Prints the current game state
void printBoard(const char myBoard[3][3])
{
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
			cout << myBoard[x][y] << " ";
		cout << endl;
	}
	cout << endl;
}

//-1 for loss, 0 for tie, 1 for win
int checkWin(const char myBoard[3][3], int numberOfMoves, bool& gameIsOver, int numberOfRounds, int threshold)
{
	int status = 2;
	//Check rows
	for (int x = 0; x < 3; x++)
		if ((myBoard[x][0] == myBoard[x][1]) && (myBoard[x][1] == myBoard[x][2]))
			if (myBoard[x][0] == 'X')
				status = 1;
			else if (myBoard[x][0] == 'O')
				status = -1;

	//Check columns
	for (int x = 0; x < 3; x++)
		if ((myBoard[0][x] == myBoard[1][x]) && (myBoard[1][x] == myBoard[2][x]))
			if (myBoard[0][x] == 'X')
				status = 1;
			else if (myBoard[0][x] == 'O')
				status = -1;

	//Check diagonals top to bottom
	if ((myBoard[0][0] == myBoard[1][1]) && (myBoard[1][1] == myBoard[2][2]))
		if (myBoard[1][1] == 'X')
			status = 1;
		else if (myBoard[0][0] == 'O')
			status = -1;

	//Check diagonals bottom to top
	if ((myBoard[2][0] == myBoard[1][1]) && (myBoard[1][1] == myBoard[0][2]))
		if (myBoard[1][1] == 'X')
			status = 1;
		else if (myBoard[1][1] == 'O')
			status = -1;

	//When the end of the game is reached. Print the results after training is finished
	if (status == 1)
	{
		if (numberOfRounds >= threshold)
			cout << "You win!" << endl;
		gameIsOver = !gameIsOver;
		return 1;
	}
	else if (status == -1)
	{
		if (numberOfRounds >= threshold)
			cout << "You lose!" << endl;
		gameIsOver = !gameIsOver;
		return -1;
	}
	else if (numberOfMoves == 9)
	{
		if (numberOfRounds >= threshold)
			cout << "Tie game!" << endl;
		gameIsOver = !gameIsOver;
		return 0;
	}

	return status;
}

//Convert the board position to a node ID
int convertToID(int& row, int& col)
{
	return (row * 3) + col;
}

//The board is being passed by address, and dereferenced here
void resetGame(char(*array)[3][3], bool& gameIsOver, bool& isPlayer, int& numberOfMoves, int& numberOfRounds)
{
	//Reset game board
	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 3; y++)
			(*array)[x][y] = '*';

	//Reset game variables
	gameIsOver = false;
	isPlayer = true;
	numberOfMoves = 0;

	//Increments round number
	numberOfRounds++;
}

int main()
{
	//open output file for data analysis
	string filename = "Data.txt";
	ofstream outFS(filename);
	//outFS.open(filename);
	if (!outFS.is_open())
	{
		cout << "Output file not open" << endl;
	}
	outFS << "File is open" << endl;

	//Create AI, gameboard, and random seed
	decisionTree AI;
	char myBoard[3][3];
	srand(time(NULL));

	//Set gameboard to all vacant coordinates
	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 3; y++)
			myBoard[x][y] = '*';

	//Initialize game variables
	bool gameIsOver = false;
	bool isPlayer = true;
	int numberOfMoves = 0;
	int numberOfRounds = 0;
	int threshold = 1000;

	//Begin clock to time training time
	cout << "Training AI" << endl;
	clock_t start = clock();

	while (!gameIsOver)
	{
		//Initialize row and column variables
		int row;
		int col;

		//Once training is done, print the time of training and introduction message
		if ((numberOfRounds == threshold) && (numberOfMoves == 0))
		{
			cout << "Time: " << (clock() - start) / (double)CLOCKS_PER_SEC << " seconds for " << threshold << " simulated games!\n";
			cout << "Welcome to TicTacToe AI!\n\n";
			outFS << "File is open" << endl;
			break;
		}

		//Player makes the first move
		if (isPlayer)
		{
			//AI is not training but is playing against the user
			if (numberOfRounds >= threshold)
			{
				//Prompt the user for input
				if (numberOfMoves == 0)
					cout << "Round number " << numberOfRounds - threshold + 1 << endl;
				cout << "Pick a coordinate(row, col): ";
				cin >> row >> col;

				//Safety checks before getting valid input
				if ((row < 0) || (row > 2))
					cout << "Invalid row!" << endl;
				else if ((col < 0) || (col > 2))
					cout << "Invalid column!" << endl;
				else
				{
					//Assume all vaild inputs 0 - 2
					if (myBoard[row][col] == '*' && isPlayer)
					{
						//Update board and current pointer in tree
						myBoard[row][col] = 'X';

						//Switch players and increment moves
						isPlayer = !isPlayer;
						numberOfMoves++;
					}

				}
			}

			//If the AI is training, this branch is taken
			else
			{
				//Get the current game state while sorting by least frequent game state visits
				NODE* temp = AI.getCurrent();

				//Get the least frequently traveled path and play that coordinate
				row = temp->choices[0]->ID / 3;
				col = temp->choices[0]->ID % 3;
				myBoard[row][col] = 'X';

				//Switch players and increment moves
				isPlayer = !isPlayer;
				numberOfMoves++;
			}
		}

		//AI makes second move
		else
		{

			//Debugging purposes
			if (numberOfRounds >= threshold)
			{
				cout << "AI's Move: " << endl;
			}

			//Possibly optimize this
			if (numberOfRounds < threshold)
			{
				//Get the current game state while sorting by least frequent game state visits
				NODE* temp = AI.getCurrent();

				//Get the least frequently traveled path and play that coordinate
				row = temp->choices[0]->ID / 3;
				col = temp->choices[0]->ID % 3;
				myBoard[row][col] = 'O';
			}
			//Let AI choose based on win percentage
			else
				AI.getAICoordinates(&myBoard, row, col, numberOfRounds, threshold);


			//Switch players and increment moves
			isPlayer = !isPlayer;
			numberOfMoves++;
		}

		//Print the board if AI is not training
		if (numberOfRounds >= threshold)
			printBoard(myBoard);

		//AI updates the Current game state in tree. Row and column numbers were taken from above statements
		AI.makeMove(convertToID(row, col), checkWin(myBoard, numberOfMoves, gameIsOver, numberOfRounds, threshold));

		if (gameIsOver)
		{
			//For debugging purposes
			/*if (numberOfRounds >= threshold)
			AI.printRootChildrenData();
			else
			cout << "Round number: " << numberOfRounds << endl;*/

			//Reset and start a new game
			resetGame(&myBoard, gameIsOver, isPlayer, numberOfMoves, numberOfRounds);
		}
	}

	cout << "Game over!" << endl;
	outFS.close();
	return 0;
}