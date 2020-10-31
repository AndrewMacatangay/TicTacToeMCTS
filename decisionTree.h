#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <ctime>
#include <stdlib.h>
using namespace std;

//Every node contains statistics so that the AI can pick the next best move
struct statistics
{
	double probability;
	int win = 0;
	int ties = 0;
	int loss = 0;
	int weight = 0;
	int total = 0;
};

//Each node corresponds to a possible game state
struct NODE
{
	NODE* parent;
	vector<NODE*> choices;
	int ID;
	statistics myData;
};

//The AI is a decision tree that decides which move to make based on probabilities
class decisionTree
{
private:

	//Data members for the tree
	NODE * Root;
	NODE* Current;
	set <int> myParents;
	vector<NODE*> myChildren;

	//Make 9! nodes which is an upper bound for the number of combinations possible
	void _makeNodes(NODE* cur, int numBranches, set <int> myParents)
	{
		//Allocate space for each node's vector. Necessary for ranged-based for-each loop
		cur->choices.resize(numBranches);
		for (int x = 0; x < cur->choices.size(); x++)
			cur->choices[x] = new NODE();

		//This counter is used to increment and assign each node an ID number
		int counter = 0;

		//For each branch of a node, create more branches
		for (NODE* branch : cur->choices)
		{

			//Each node in the child array has a parent which is cur
			branch->parent = cur;

			//First iteration
			if (cur == Root)
			{
				branch->ID = counter;
				counter++;
			}
			//Any other iteration
			else
			{
				//Add to set here
				myParents.insert(cur->ID);

				//If found, then go to next count, otherwise assign the ID and increment the count
				while ((myParents.find(counter) != myParents.end()) && counter < 9)
					counter++;

				branch->ID = counter;
				counter++;
			}

			//Base case
			if (numBranches == 1)
				return;

			//Recursive call to create more branches
			_makeNodes(branch, numBranches - 1, myParents);
		}
	}


public:
	//Default constructor
	decisionTree()
	{
		//Initialize private data members
		Root = new NODE();
		Root->parent = nullptr;
		Current = Root;

		//Build the tree and time how long it takes
		cout << "Building tree...\n";
		clock_t start = clock();
		_makeNodes(Root, 9, myParents);
		cout << "Time: " << (clock() - start) / (double)CLOCKS_PER_SEC << " seconds\n\n";
	}

	//Pick the highest win-rate from the next state(s) of the board and play that coordinate
	void getAICoordinates(char(*array)[3][3], int& row, int& col, const int& numberOfRounds, int threshold)
	{
		//Initialize variables
		int counter = 0;
		int bestIndex = 0;
		int buffer = INT_MIN;
		//srand(time(NULL));

		//Push each child into a new vector that will be sorted
		myChildren.clear();
		for (NODE* child : Current->choices)
			myChildren.push_back(child);

		//Sort the children from highest to lowest chance of winning
		if (numberOfRounds >= threshold)
		{
			sort(myChildren.begin(), myChildren.end(), [](NODE* first, NODE* second) {
				return (first->myData.probability > second->myData.probability);
			});
		}

		//Pick the greatest chance of success for the AI
		for (int x = 0; x < myChildren.size(); x++)
		{
			row = myChildren[x]->ID / 3;
			col = myChildren[x]->ID % 3;
			//Pick the next highest weight
			if (((*array)[row][col] == '*') && (myChildren[x]->myData.probability != 0))
			{
				(*array)[row][col] = 'O';
				return;
			}
			//If a weight of 0 is reached, pick at random until an opening is found
			else if (myChildren[x]->myData.probability == 0)
			{
				row = rand() % 3;
				col = rand() % 3;

				while ((*array)[row][col] != '*')
				{
					row = rand() % 3;
					col = rand() % 3;
				}

				(*array)[row][col] = 'O';
				return;
			}
		}
	}

	//Update the current game state if a game has finished
	void makeMove(int choice, int status)
	{
		//When a choice is made, move the Current pointer to the child which is also the next game state
		for (NODE* child : Current->choices)
			if (child->ID == choice)
			{
				Current = child;
				break;
			}

		//If you reach the end of the game, backpropagate to the root and update each parent node
		//A player's win is the AI's loss and vice-versa
		if (status != 2)
		{
			while (Current->parent != nullptr)
			{
				//If the AI lost
				if (status == 1)
				{
					Current->myData.loss++;
					Current->myData.weight--;
				}
				//If the AI tied
				else if (status == 0)
					Current->myData.ties++;
				//If the AI won
				else
				{
					Current->myData.win++;
					Current->myData.weight++;
				}

				Current->myData.total++;
				//int total = Current->myData.loss + Current->myData.ties + Current->myData.win;

				Current->myData.probability = (((double)Current->myData.win * 1.1) + (double)Current->myData.ties) / (double)Current->myData.total;

				//Backpropagate
				Current = Current->parent;
			}
		}
	}

	//prints data to a file for interpretation by python script 
	void printGraphData(ostream& output) const
	{
		for (NODE* child : Current->choices) {
			output << child->ID << "," << child->myData.win << "," << child->myData.ties << "," << child->myData.loss << "," << child->myData.weight << "," << child->myData.probability << endl;
		}
	}
	//Prints the optimal first move for the AI
	void printRootChildrenData()
	{
		//Check every child of the root and print the data
		cout << "\nI W T L Wt P\n";

		for (NODE* child : Current->choices)
		{
			//ID, Win, Tie, Loss, Wt, Percentage
			cout << child->ID << " " << child->myData.win << " " << child->myData.ties << " " << child->myData.loss << " " << child->myData.weight << "  " << child->myData.probability << endl;
		}

		cout << endl;
	}

	//Sort the children nodes by least frequently visited to most visited, then returns the current game state
	NODE* getCurrent()
	{
		// < thresh sort by total to get all game states
		sort(Current->choices.begin(), Current->choices.end(), [](NODE* first, NODE* second) {
			return (first->myData.total < second->myData.total);
		});

		return Current;
	}
};