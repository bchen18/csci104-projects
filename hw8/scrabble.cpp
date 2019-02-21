#include <string>
#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>
#include "Tile.h"
#include "Square.h"
#include "Move.h"
#include "Dictionary.h"
#include "Player.h"
#include "ConsolePrinter.h"
#include "Trie.h"
#include <stack>
using namespace std;
class Move;
class PassMove;
class ExchangeMove;
class PlaceMove;
void findPermutations(Board& board, string currPermutation, vector<char> unUsed, vector<pair<PlaceMove*, size_t>> &wordsWork, Player *currPlayer, size_t x, size_t y, TrieSet* trie, bool horizontal);
int main (int argc, char* argv[])
{
	int handsize;
	string bagFile;
	string dictionaryFile;
	string boardFile;
	uint32_t seed;
	//read in configuration file
	ifstream configFile(argv[1]);
	bool hasInit = false;
	string initFile;
	while(!configFile.fail())
	{
		string line;
		getline(configFile, line);
		stringstream ss;
		ss << line;
		string keyword;
		ss >> keyword;
		if(keyword == "HANDSIZE:")
			ss >> handsize;
		else if(keyword == "TILES:")
			ss >> bagFile;
		else if(keyword == "DICTIONARY:")
			ss >> dictionaryFile;
		else if(keyword == "BOARD:")
			ss >> boardFile;
		else if(keyword == "SEED:")
			ss >> seed;
		//check if it has init file
		else if(keyword == "INIT:")
		{
			ss >> initFile;
			hasInit = true;
		}
		else
			ss.clear();
		ss.clear();
	}
	//construct dictionary, board, bag
	Dictionary dictionary(dictionaryFile);
	Board board(boardFile);
	Bag bag(bagFile, seed);
	//if init file present, place those tiles onto board
	if(hasInit)
	{
		ifstream init(initFile);
		//loop through whole init file, placing a tile for each encountered
		for(size_t i = 0; i < board.getRows(); i++)
		{
			string initLine;
			getline(init, initLine);
			int j = 0;
			while(!initLine.empty())
			{
				//if square has tile, create the tile with corresponding letter and score and place it
				if(initLine[0] != '.')
				{
					char tileLetter = initLine[0];
					string tileScoreString = initLine.substr(1, 2);
					stringstream ss;
					ss << tileScoreString;
					unsigned int tileScore;
					ss >> tileScore;
					Tile* currTile = new Tile(tileLetter, tileScore);
					board.placeTileOnBoard(currTile, i, j);
				}
				//move to next square
				initLine = initLine.substr(3);
				j++;
			}
		}
	}	
	//initialize players and their hands
	int playerNum = 0;
	while(playerNum < 1 || playerNum > 8)
	{
		cout << "Enter num of players: ";
		cin >> playerNum;
	}
	vector<Player*> playerList;
	for(int i = 0; i < playerNum; i++)
	{
		string pName;
		cout << "Enter player " << i + 1 << " name: ";
		cin >> pName;
		cin.ignore();
		Player* nextPlayer = new Player(pName, handsize);
		vector<Tile*> newTiles = bag.drawTiles(handsize);
		nextPlayer -> addTiles(newTiles);
		playerList.push_back(nextPlayer);

	}
	int currPlayer = 0;//keep track of current player
	//loop through every player until bag is empty
	bool firstSquareOccupied = hasInit;
	do
	{
		//print board, scores, and player hand
		ConsolePrinter::printBoard(board);
		ConsolePrinter::printHand(*playerList[currPlayer]);
		for(int i = 0; i < playerNum; i++)
			cout << "Player " << i + 1 << " score: " << playerList[i] -> getScore() << endl;
		//attempt player's move. keep looping until move is successful
		bool moveWorked = false;		
		int newScore = 0;//add this to player score at end of successful turn. >0 only if a place move
		do
		{
			//check if current player is a CPU
			if(playerList[currPlayer] -> isCPUL() || playerList[currPlayer] -> isCPUS())
			{
				//suppress all couts to spare the user
				cout.setstate(ios_base::failbit);
				//construct trie to assist with picking moves
				TrieSet* trie = new TrieSet();
				ifstream dictionaryfile(dictionaryFile);
				//initialize wordList in lowercase alphabetical order
				string word;
				while(dictionaryfile >> word)
				{
					//convert to uppercase for uniformity
					for(int i = 0; i<(int)word.size(); i++)
					{
						word[i] = toupper(word[i]);
						if(isspace(word[i]))
						{
							break;
						}
					}
					trie->insert(word); //in lowercase alphabetical order at this point
					//consume rest of line
					getline(dictionaryfile, word);
				}
				/*begin backtracking to find best move. Keep track of the most recent move with the 
				highest length/score, updating if a greater one is found.*/				
				vector<Tile*> cpuTiles = playerList[currPlayer] -> getHandTiles();
				vector<char> unUsed;
				for(size_t k = 0; k < cpuTiles.size(); k++)
				{
					unUsed.push_back(cpuTiles[k]->getUse());
				}
				//vector of pairs of legal moves and the num of tiles placed
				vector<pair<PlaceMove*, size_t>> legalMoves;
				//not first move
				if(firstSquareOccupied)
				{
					for(size_t i = 0; i < board.getRows(); i++)
					{
						for(size_t j = 0; j < board.getColumns(); j++)
						{
							if(!board.getSquare(i + 1, j + 1) -> isOccupied())
							{
								cout.clear();//enable cout again
								//cout << "beep boop checking at row " << i << " col " << j << endl;
								cout.setstate(ios_base::failbit);
								/*Find all moves that create a word and store it in legalMoves. Do horizontal and vertical
								*/							
								findPermutations(board, "", unUsed, legalMoves, playerList[currPlayer], i, j, trie, true);				
								findPermutations(board, "", unUsed, legalMoves, playerList[currPlayer], i, j, trie, false);

							}
						}
					}
				}
				else
				{
					findPermutations(board, "", unUsed, legalMoves, playerList[currPlayer], board.getStartX(), board.getStartY(), trie, true);
					findPermutations(board, "", unUsed, legalMoves, playerList[currPlayer], board.getStartX(), board.getStartY(), trie, false);
				}
				cout.clear();//enable cout again
				//no moves found, just pass
				if(legalMoves.empty())
				{
					PassMove* cpuPass = new PassMove(playerList[currPlayer]);
					moveWorked = cpuPass -> execute(board, bag, dictionary);
					delete cpuPass;
				}
				//if move(s) found, find optimal one based on CPU type and execute that one
				//--------------------------------YOU LEFT OFF HERE DINGUS---------------------------------
				else
				{
					//PlaceMove* cpuPlace;
					size_t bestMove;
					//CPUL, simply find max length placemove
					if(playerList[currPlayer] -> isCPUL())
					{
						size_t maxLen = -1;
						for(size_t g = 0; g < legalMoves.size(); g++)
						{
							if(legalMoves[g].second > maxLen)
							{
								maxLen = legalMoves[g].second;
								bestMove = g;
								//cpuPlace = legalMoves[g].first;

							}
						}
					}
					//CPUS, must go through all placemove scores and find the move with best score 
					else if(playerList[currPlayer] -> isCPUS())
					{
						size_t maxScore = -1;
						for(size_t g = 0; g < legalMoves.size(); g++)
						{
							//check current placemove. it should already be legal so just check the score :3
							vector<pair<string, unsigned int>>checkWords = board.getPlaceMoveResults(*legalMoves[g].first);							
							for(size_t i = 0; i < checkWords.size(); i++)
							{
								pair<string, unsigned int> checkPair = checkWords[i];
								if(checkPair.second > maxScore)
								{
									maxScore = checkPair.second;
									bestMove = g;
									//cpuPlace = legalMoves[g].first;
								}
							}
						}
					}
					//execute the move :^)
					PlaceMove* cpuPlace = legalMoves[bestMove].first;
					vector<pair<string, unsigned int>>bestWords = board.getPlaceMoveResults(*cpuPlace);

					//find score
					for(size_t i = 0; i < bestWords.size(); i++)
					{
						pair<string, unsigned int> checkPair = bestWords[i];
						cout << "word: " << checkPair.first;
						newScore += checkPair.second;
					}
					moveWorked = cpuPlace -> execute(board, bag, dictionary);				
					//clear legalMoves					
					for(size_t g = 0; g < legalMoves.size(); g++)
					{
						delete legalMoves[g].first;
					}
					
				}


				//cout.clear();//enable cout again
			}
			//player is human
			else
			{
				//construct move
				string playerMove;
				cout << "Player " << currPlayer + 1 << "'s move. Enter your move: ";
				getline(cin, playerMove);
				Move* pMove = Move::parseMove(playerMove, *playerList[currPlayer]);
				//check if place move. if so, attempt placement and check legality of words
				if(pMove -> isWord())
				{
					//construct new place move so getPlaceMoveResults() actually works...
					stringstream ss; 
					string parameters = playerMove.substr(playerMove.find(" ") + 1);
					string directionString;
					size_t x, y;//coordinates start at 1, 1!
					string tiles;
					ss << parameters;
					ss >> directionString >> x >> y >> tiles;
					bool horizontal;
					//determine if horizontal or not
					if(directionString == "-")
						horizontal = true;
					else
						horizontal = false;
					transform(tiles.begin(), tiles.end(), tiles.begin(), ::tolower);
					PlaceMove* newPlace = new PlaceMove(x - 1, y - 1, horizontal, tiles, playerList[currPlayer]);

					//if first move, place move must be on first tile. Otherwise jump to end of loop
					if(!firstSquareOccupied)
					{
						if(x - 1 == board.getStartX() && y -1 == board.getStartY())
							firstSquareOccupied = true;
						else
						{
							cout << "Must start on starting square." << endl;
							goto label;
						}
					}
					//check if move is possible with player hand
					bool playerHasTiles = playerList[currPlayer] -> hasTiles(tiles, true);
					if(!playerHasTiles)
					{
						cout << "No such tile(s) in hand." << endl;
						goto label;
					}
					vector<pair<string, unsigned int>>checkWords = board.getPlaceMoveResults(*newPlace);
					//check every word that would be created in move for legality
					for(size_t i = 0; i < checkWords.size(); i++)
					{
						pair<string, unsigned int> checkPair = checkWords[i];
						//if word isn't legal, void any points and jump to end of loop to begin another iteration
						if(!dictionary.isLegalWord(checkPair.first))
						{
							newScore = 0;
							cout << "Word(s) formed not in dictionary or placement error." << endl;
							goto label;
						}
						//if word is legal, include those points 
						else
							newScore += checkPair.second;
					}
					delete newPlace;//--------------ADDED------------------
				}
				moveWorked = pMove -> execute(board, bag, dictionary);
				//if move didn't work, indicate so. if any illegal word is found in placeMove, jump here
				label:if(!moveWorked)
				{
					cout << "Try another move." << endl;
				}
				delete pMove;//--------------ADDED------------------
			}
		}
		while(!moveWorked);
		playerList[currPlayer] -> changeScore((playerList[currPlayer] -> getScore()) + newScore);
		//reset to first player after last one
		currPlayer++;
		if(currPlayer > playerNum - 1)
			currPlayer -= playerNum;
	}
	while(bag.tilesRemaining() > 0 && (playerList[currPlayer] -> getHandTiles()).size() >= 0);
	//after game, determine highest score and print winner(s)
	int maxScore = -1;
	for(size_t i = 0; i < playerList.size(); i++)
	{
		if(playerList[i] -> getScore() > maxScore)
			maxScore = playerList[i] -> getScore();
	}
	cout << "Game over! Winner(s): " << endl;
	for(size_t i = 0; i < playerList.size(); i++)
	{
		if(playerList[i] -> getScore() == maxScore)
			cout << "Player " << i << endl;
		delete playerList[i];
	}
}

/*finds all working permutations, storing working words into wordsWork. First attempts to add all chars in unUsed to 
currPermutation. If the resulting placeMove using the new currPermutation creates all prefixes/words, 
recursively call again on the new currPermutation. If not, immediately stop to prevent redundant permutations.
If any legal word is found when checking the placeMove, store in wordsWork. Base case when unUsed is empty(?)
*/
void findPermutations(Board &board, string currPermutation, vector<char> unUsed, vector<pair<PlaceMove*, size_t>> &wordsWork, Player *currPlayer, size_t x, size_t y, TrieSet* trie, bool horizontal)
{
	//if no more letters to try adding, end this branch
	if(unUsed.empty())
	{
		return;
	}
	//try adding every unUsed char to currPermutation and testing the resulting placeMove
	for(size_t i = 0; i < unUsed.size(); i++)
	{
		//copy over unUsed vector to use for future recursive call
		vector<char> notUsed;
		for(size_t a = 0; a < unUsed.size(); a++)
		{
			notUsed.push_back(unUsed[a]);
		}
		
		//if the current char being checked is not a blank (? tile)
		if(unUsed[i] != '?')
		{
			string newPermutation = currPermutation;
			
			newPermutation.push_back(unUsed[i]);
			PlaceMove* newPlace = new PlaceMove(x, y, horizontal, newPermutation, currPlayer);
			vector<pair<string, unsigned int>>checkWords = board.getPlaceMoveResults(*newPlace);
			/*check if placement was legal and words formed are a prefix*/
			bool foundWord = false;
			bool failed = false;
			for(size_t i = 0; i < checkWords.size(); i++)
			{
				pair<string, unsigned int> checkPair = checkWords[i];
				//if word isn't a prefix, end this branch
				if(trie->prefix(checkPair.first) == nullptr)
				{
					foundWord = false;
					failed = true;
					continue;
					
				}
				//if word is legal (not just a prefix), add to wordsWork
				else if((trie->prefix(checkPair.first)) -> inSet)
				{
					foundWord = true;
					cout.clear();
					//cout << "found word: " << checkPair.first << endl;
					cout.setstate(ios_base::failbit);
				}
			}
			
			//if a found word was legal (not just a prefix), add the placemove and its num of tiles used to wordsWork
			if(foundWord)
			{
				cout.clear();
				//cout << "found move!" << endl;
				cout.setstate(ios_base::failbit);	
				pair<PlaceMove*, size_t> WordPair(newPlace, newPermutation.size());
				wordsWork.push_back(WordPair);
			}
			else
			{
				delete newPlace;
			}
			//recursively call on this successful move. use the notUsed vector to keep all other values intact
			if(!failed)
			{
				notUsed.erase(notUsed.begin() + i);
				findPermutations(board, newPermutation, notUsed, wordsWork, currPlayer, x, y, trie, horizontal);
			}
		}
		//if current char is a blank tile, need to try every single letter as it...
		else
		{
			for(size_t j = 0; j < 26; j++)
			{
				string newPermutation = currPermutation;
				char currBlankChar = 'a' + j;
				newPermutation.push_back('?');
				newPermutation.push_back(currBlankChar);
				PlaceMove* newPlace = new PlaceMove(x, y, horizontal, newPermutation, currPlayer);
				vector<pair<string, unsigned int>>checkWords = board.getPlaceMoveResults(*newPlace);
				/*check if placement was legal and words formed are a prefix*/
				bool foundWord = false;
				bool failed = false;
				for(size_t i = 0; i < checkWords.size(); i++)
				{
					pair<string, unsigned int> checkPair = checkWords[i];
					//if one word isn't a prefix, immediately end this branch
					if(trie->prefix(checkPair.first) == nullptr)
					{
						foundWord = false;
						failed = true;
						continue;
					}
					//if all words are legal (not just a prefix), add to wordsWork
					else if(trie->prefix(checkPair.first) -> inSet)
					{
						foundWord = true;
						
					}
				}
				//if a found word was legal (not just a prefix), add the placemove and its num of tiles used to wordsWork
				if(foundWord)
				{
					cout.clear();
					//cout << "found move!" << endl;
					cout.setstate(ios_base::failbit);	
					pair<PlaceMove*, size_t> WordPair(newPlace, newPermutation.size() - 1);
					wordsWork.push_back(WordPair);
				}
				else
				{
					delete newPlace;
				}
				//recursively call on this successful move
				if(!failed)
				{
					notUsed.erase(notUsed.begin() + i);
					findPermutations(board, newPermutation, notUsed, wordsWork, currPlayer, x, y, trie, horizontal);
				}
			}

		}
	}

}




