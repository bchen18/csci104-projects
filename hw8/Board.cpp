#include "Board.h"
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace std;

//board constructor
//board is a 2D array of square pointers...FML
Board::Board(string board_file_namey)
{
	ifstream ifile(board_file_namey);
	ifile >> numColumns >> numRows;
	
	ifile >> startX >> startY;
	boardArray = new Square**[numRows];
	//dynamically allocate 2D array of Squares, constructing each individual Square
	for(size_t i = 0; i < numRows; i++)
	{
    	boardArray[i] = new Square*[numColumns];
    	for(size_t j = 0; j < numColumns; j++)
    	{
    		char squareType;
    		ifile >> squareType;
    		bool isStart = false;
    		//check if current Square is start square
    		if(i + 1 == startY && j + 1 == startX)
    			isStart = true;
    		//square type is normal
    		if(squareType == '.')
    		{
    			Square * newSquare = new Square(1, 1, isStart);
    			boardArray[i][j] = newSquare;
    		}
    		//square type is letter multiplier
    		else if(squareType == '2' || squareType == '3')
    		{
    			Square * newSquare = new Square((int)(squareType - 48), 1, isStart);
    			boardArray[i][j] = newSquare;
    		}
    		//square type is word multiplier
    		else
    		{
    			//double
    			if(squareType == 'd')
    			{
    				Square * newSquare = new Square(1, 2, isStart);
    				boardArray[i][j] = newSquare;
    			}
    			//triple
    			else
    			{
    				Square * newSquare = new Square(1, 2, isStart);
    				boardArray[i][j] = newSquare;
    			}
    			
    		}
    		
    	}
    }
}

//loop through board and delete all square*
Board::~Board()
{
	for(size_t i = 0; i < numRows; i++)
	{
		for(size_t j = 0; j < numColumns; j++)
		{
			delete boardArray[i][j];
		}
		delete boardArray[i];
	}
	delete boardArray;
}

/*This function attempts to return all potential words in uppercase and their scores formed by the place move. if the move results in out of bounds tiles, 
tiles placed on a preoccupied square, or all tiles not touching any tiles already placed, returns a "garbage" vector containing a string that will
always fail dictionary test to indicate a general failure. If no errors like that are found, returns the usual vector, with the last value
being the "main" word. These returned words still need to be checked in main for legality in dictionary.
*/
vector<pair<string, unsigned int>> Board::getPlaceMoveResults(const PlaceMove &m) //const
{
	string placeTiles = m.getTileString();
	vector<Tile*> pTileVector = m.getTileVector();
	vector<pair<string, unsigned int>> resultWords;
	//if player is really dumb and tries to place tiles at negative index, returns "garbage" vector
	if(m.getXPos() < 0 || m.getYPos() < 0)
	{
		cout << "Out of bounds." << endl;
		pair<string, unsigned int> newPair("!", -1);
		resultWords.push_back(newPair);
		return resultWords;
	}

	//HORIZONTAL place move
	if(m.checkHorizontal())
	{
		//check if out of bounds
		if(m.getXPos() + placeTiles.size() > numColumns || m.getYPos() > numRows)
		{
			cout << "Out of bounds." << endl;
			pair<string, unsigned int> newPair("!", -1);
			resultWords.push_back(newPair);
			return resultWords;
		}
		//check if first tile already occupied
		if((*boardArray[m.getXPos()][m.getYPos()]).isOccupied())
		{
			cout << "First tile occupied." << endl;
			pair<string, unsigned int> newPair("!", -1);
			resultWords.push_back(newPair);
			return resultWords;
		}
		bool touchesTile = false;
		for(size_t xCor = m.getXPos(); xCor < m.getXPos() + placeTiles.size(); xCor++)
		{					
			//check if tile touches an already-placed tile
			if(xCor == m.getXPos() && (int)xCor - 1 >= 0)//first tile needs left side checked
				if((*boardArray[xCor - 1][m.getYPos()]).isOccupied())
					touchesTile = true;
			if(xCor == m.getXPos() + placeTiles.size() - 1 && xCor + 1 < numColumns)//last tile needs right side checked	
				if((*boardArray[xCor + 1][m.getYPos()]).isOccupied())
					touchesTile = true;
			if((int)m.getYPos() - 1 >= 0)//check upper side
				if((*boardArray[xCor][m.getYPos() - 1]).isOccupied())
					touchesTile = true;
			if(m.getYPos() + 1 < numRows)//check lower side
				if((*boardArray[xCor][m.getYPos() + 1]).isOccupied())	
					touchesTile = true;
		}
		//if touching tile wasn't found, return "garbage" vector
		if(!touchesTile)// && (m.getXPos() != startX && m.getYPos() != startY))
		{
			cout << "Doesn't touch a tile." << endl;
			pair<string, unsigned int> newPair("!", -1);
			resultWords.push_back(newPair);
			return resultWords;
		}

		//run search algorithm to find words and their scores
		string mainWord;//"main" word to be stored at back of vector to be returned
		unsigned int mainWordScore = 0;
		unsigned int mainWordWMult = 1;
		//loop through each tile to be placed (xCor)
		for(size_t xCor = m.getXPos(); xCor < m.getXPos() + placeTiles.size(); xCor++)
		{
			//need to check horizontally for words if first tile 
			if(xCor == m.getXPos() && xCor > 0)
			{
				size_t count = 0;//keep track of location in getTileString()
				size_t xIter = xCor;
				//traverse leftwards until beginning of "word" reached
				while((*boardArray[xIter - 1][m.getYPos()]).isOccupied())
					xIter--;
				//pop back rightwards until end of "word" reached, storing resultant "word" as mainWord
				while((boardArray[m.getYPos()][xIter] -> isOccupied() || count < placeTiles.size()) && xIter < numColumns)
				{
					//cout << "beginning" << endl;
					//if letter to push is part of getTileString(), push that letter
					if(xIter >= m.getXPos() && xIter <= m.getXPos() + placeTiles.size() && !(*boardArray[xIter][m.getYPos()]).isOccupied() && count < placeTiles.size())
					{
						if(placeTiles[count] == '?')//skip to next letter if it's a blank tile
							count++;
						else
						{
							//brute force search pTileVector until current letter is found, and then add that score
							size_t loc = -1;
							//cout << "count=" << count << endl;
							//cout << "trying to find " << placeTiles[count] << endl;
							for(size_t i = 0; i < pTileVector.size(); i++)
							{
								if((pTileVector[i] -> getLetter()) == placeTiles[count])
									loc = i;
							}

							//cout << "loc= " << loc << endl;
							mainWordScore += (pTileVector[loc] -> getPoints()) * ((*boardArray[xIter][m.getYPos()]).getLMult());
						}
						mainWord.push_back(placeTiles[count]);
						count++;

					}
					else//push the letter of the current tile already on the board
					{
						//cout << "on board: " << (*boardArray[xIter][m.getYPos()]).getLetter() << endl;
						mainWord.push_back((*boardArray[xIter][m.getYPos()]).getLetter());
						mainWordScore += (*boardArray[xIter][m.getYPos()]).getScore() * (*boardArray[xIter][m.getYPos()]).getLMult();						
					}
					mainWordWMult *= (*boardArray[xIter][m.getYPos()]).getWMult();
					xIter++;
					if (xIter >= numColumns)
						break;
					
				}
				//cout << "done" << endl;
			}

			//check vertically for words			
			/*first check if there are any tiles above/below current tile and if current tile isn't already occupied (we only want to check
			  squares where we want to place down a tile)
			*/
			//cout << "before line 196, xCor=" << xCor << ", YPos=" << m.getYPos() << endl;
			if(((*boardArray[xCor][m.getYPos() - 1]).isOccupied() || (*boardArray[xCor][m.getYPos() + 1]).isOccupied()) && ((int)m.getYPos() - 1 >= 0 
				|| m.getYPos() + 1 < numRows) && !(*boardArray[xCor][m.getYPos()]).isOccupied())
			{
				size_t yIter = m.getYPos();
				size_t count = 0;//num of blank tiles, aka num to shift over by
				//traverse upwards until beginning of "word" reached
				while((*boardArray[xCor][yIter - 1]).isOccupied())
					yIter--;
				//pop back downwards until end of "word" reached, storing resultant "word" as string
				string word;
				unsigned int wordScore = 0;
				unsigned int wordWMult = 1;	
				while(((*boardArray[xCor][yIter]).isOccupied() || yIter == m.getYPos()) && yIter < numRows && xCor - m.getXPos() + count < placeTiles.size())
				{
					if(yIter == m.getYPos())//if letter to push is part of tiles being placed, push that letter
					{
						if(placeTiles[xCor - m.getXPos() + count] == '?')//shift over if it's a blank tile
							count++;							
						else
						{
							size_t loc = -1;
							//brute force search pTileVector until current letter is found, and then add that score
							for(size_t i = 0; i < pTileVector.size(); i++)
							{
								if((pTileVector[i] -> getLetter()) == placeTiles[xCor - m.getXPos() + count])
									loc = i;
							}

							wordScore += (pTileVector[loc] -> getPoints()) * (*boardArray[xCor][yIter]).getLMult();
						}
						word.push_back(placeTiles[xCor - m.getXPos() + count]);
					}
					else//push letter of current tile already on board
					{
						word.push_back((*boardArray[xCor][yIter]).getLetter());
						wordScore += (*boardArray[xCor][yIter]).getScore() * (*boardArray[xCor][yIter]).getLMult();
					}
					wordWMult *= (*boardArray[xCor][yIter]).getWMult();
					yIter++;
					if (yIter >= numRows)
						break;
				}
				//add result to vector to be returned. word may not be legal!
				transform(word.begin(), word.end(), word.begin(), ::tolower);
				pair<string, unsigned int> newPair(word, wordScore * wordWMult);
				if(newPair.first.size() > 1)
					resultWords.push_back(newPair);	
			}
		}
		//add "main" word to end of vector to be returned
		transform(mainWord.begin(), mainWord.end(), mainWord.begin(), ::tolower);
		pair<string, unsigned int> mainPair(mainWord, mainWordScore * mainWordWMult);
		if(mainPair.first.size() > 1)
			resultWords.push_back(mainPair);
		return resultWords;
	}

	//VERTICAL move. similar to horizontal, but flip when you check horizontally/vertically, x/y, etc
	else
	{
		//check if out of bounds
		if(m.getXPos() > numColumns || m.getYPos() + placeTiles.size() > numRows)
		{
			cout << "Out of bounds." << endl;
			pair<string, unsigned int> newPair("!", -1);
			resultWords.push_back(newPair);
			return resultWords;
		}
		//check if first tile already occupied
		if((*boardArray[m.getXPos()][m.getYPos()]).isOccupied())
		{
			cout << "First tile occupied." << endl;
			pair<string, unsigned int> newPair("!", -1);
			resultWords.push_back(newPair);
			return resultWords;
		}
		bool touchesTile = false;
		for(size_t yCor = m.getYPos(); yCor < m.getYPos() + placeTiles.size(); yCor++)
		{		
			//cout << "checking at XPos=" << m.getXPos() << " yCor=" << yCor << endl;			
			//check if tile touches an already-placed tile
			if(yCor == m.getYPos() && (int)yCor - 1 >= 0)//first tile needs top side checked
				if((*boardArray[m.getXPos()][yCor - 1]).isOccupied())
					touchesTile = true;
			if(yCor == m.getYPos() + placeTiles.size() - 1 && yCor + 1 < numRows)//last tile needs bottom side checked
				if((*boardArray[m.getXPos()][yCor + 1]).isOccupied())
					touchesTile = true;
			if((int)m.getXPos() - 1 >= 0)//check left side
				if((*boardArray[m.getXPos() - 1][yCor]).isOccupied())
					touchesTile = true;
			if(m.getXPos() + 1 < numColumns)//check right side
				if((*boardArray[m.getXPos() + 1][yCor]).isOccupied())
					touchesTile = true;
		}
		//if touching tile wasn't found, return "garbage" vector
		if(!touchesTile)// && (m.getXPos() != startX && m.getYPos() != startY))
		{
			cout << "Doesn't touch a tile." << endl;
			pair<string, unsigned int> newPair("!", -1);
			resultWords.push_back(newPair);
			return resultWords;
		}

		//run search algorithm to find words and their scores
		string mainWord;//"main" word to be stored at back of vector to be returned
		unsigned int mainWordScore = 0;
		unsigned int mainWordWMult = 1;
		//loop through each tile to be placed (yCor)
		for(size_t yCor = m.getYPos(); yCor < m.getYPos() + placeTiles.size(); yCor++)
		{
			//need to check vertically for words if first tile 
			if(yCor == m.getYPos() && yCor > 0)
			{
				size_t count = 0;//keep track of location in getTileString()
				size_t yIter = yCor;
				//traverse upwards until beginning of "word" reached
				while((*boardArray[m.getXPos()][yIter - 1]).isOccupied())
					yIter--;
				//pop back downwards until end of "word" reached, storing resultant "word" as mainWord
	
				while((boardArray[m.getXPos()][yIter] -> isOccupied() || count < placeTiles.size()) && yIter < numRows)
				{
					//if letter to push is part of getTileString(), push that letter
					if(yIter >= m.getYPos() && yIter <= m.getYPos() + placeTiles.size() && !(*boardArray[m.getXPos()][yIter]).isOccupied() && count < placeTiles.size())
					{
						if(placeTiles[count] == '?')//skip to next letter if it's a blank tile
							count++;
						else
						{
							//brute force search pTileVector until current letter is found, and then add that score
							size_t loc = -1;
							for(size_t i = 0; i < pTileVector.size(); i++)
							{
								if((pTileVector[i] -> getLetter()) == placeTiles[count])
									loc = i;
							}
							
							mainWordScore += (pTileVector[loc] -> getPoints()) * (*boardArray[m.getXPos()][yIter]).getLMult();
						}
						mainWord.push_back(placeTiles[count]);
						count++;
					}
					else//push the letter of the current tile already on the board
					{
						mainWord.push_back((*boardArray[m.getXPos()][yIter]).getLetter());
						mainWordScore += (*boardArray[m.getXPos()][yIter]).getScore() * (*boardArray[m.getXPos()][yIter]).getLMult();
					}
					mainWordWMult *= (*boardArray[m.getXPos()][yIter]).getWMult();
					yIter++;
					if (yIter >= numRows)
						break;
				}				
			}

			//check horizontally for words
			
			/*first check if there are any tiles left/right of current tile AND if current tile isn't already occupied (we only want to check
			  squares where we want to place down a tile)
			*/
			if(((*boardArray[m.getXPos() - 1][yCor]).isOccupied() || (*boardArray[m.getXPos() + 1][yCor]).isOccupied()) && ((int)m.getXPos() - 1 >= 0 
				|| m.getXPos() + 1 < numColumns) && !(*boardArray[m.getXPos()][yCor]).isOccupied())
			{
				size_t xIter = m.getXPos();
				size_t count = 0;
				//traverse leftwards until beginning of "word" reached
				while((*boardArray[xIter - 1][yCor]).isOccupied())
					xIter--;
				//pop back rightwards until end of "word" reached, storing resultant "word" as string
				string word;
				unsigned int wordScore = 0;
				unsigned int wordWMult = 1;	
				while(((*boardArray[xIter][yCor]).isOccupied() || xIter == m.getXPos()) && xIter < numColumns && yCor - m.getYPos() + count < placeTiles.size())
				{
					if(xIter == m.getXPos())//if letter to push is part of tiles being placed, push that letter
					{
						if(placeTiles[yCor - m.getYPos() + count] == '?')//shift over if blank tile
							count++;
						else
						{
							//brute force search pTileVector until current letter is found, and then add that score
							size_t loc = -1;
							for(size_t i = 0; i < pTileVector.size(); i++)
							{
								if((pTileVector[i] -> getLetter()) == placeTiles[yCor - m.getYPos() + count])
									loc = i;
							}

							wordScore += (pTileVector[loc] -> getPoints()) * (*boardArray[xIter][yCor]).getLMult();
						}
						word.push_back(placeTiles[yCor - m.getYPos() + count]);
					}
					else//push letter of current tile already on board
					{
						word.push_back((*boardArray[xIter][yCor]).getLetter());
						wordScore += (*boardArray[xIter][yCor]).getScore() * (*boardArray[xIter][yCor]).getLMult();
					}					
					wordWMult *= (*boardArray[xIter][yCor]).getWMult();
					xIter++;
					if (xIter >= numColumns)
						break;
				}
				//add result to vector to be returned. word may not be legal!
				transform(word.begin(), word.end(), word.begin(), ::tolower); 
				pair<string, unsigned int> newPair(word, wordScore * wordWMult);
				if(newPair.first.size() > 1)
					resultWords.push_back(newPair);				
			}		
		}
		//add "main" word to end of vector to be returned
		transform(mainWord.begin(), mainWord.end(), mainWord.begin(), ::tolower);
		pair<string, unsigned int> mainPair(mainWord, mainWordScore * mainWordWMult);
		if(mainPair.first.size() > 1)
			resultWords.push_back(mainPair);
		return resultWords;
	}
}

/*This function is only called by the PlaceMove::execute function. It just places the tiles and returns true. Should always work, since
getPlaceMoveResults should check if the placemove works anyway
*/
bool Board::executePlaceMove (vector<Tile*> placeTiles, size_t x, size_t y, bool horizontal)
{
	size_t shift = 0;//shift based on num of already-occupied squares
	size_t count = 0;//keep track of current tile to place
	//HORIZONTAL place move
	if(horizontal)
	{
		shift = x;//shift to keep track of which index on board
		while(count < placeTiles.size())
		{
			if(!boardArray[shift][y] -> isOccupied())
			{
				boardArray[shift][y] -> placeTile(placeTiles[count]);
				count++;
			}
			shift++;
		}
	}
	//VERTICAL place move
	else
	{
		shift = y;//shift to keep track of which index on board
		while(count < placeTiles.size())
		{
			if(!boardArray[x][shift] -> isOccupied())
			{
				boardArray[x][shift] -> placeTile(placeTiles[count]);
				count++;
			}
			shift++;
		}
	}
	return true;	
}

Square * Board::getSquare (size_t x, size_t y) const
{
	return boardArray[x - 1][y - 1];
}

size_t Board::getRows() const
{
	return numRows;
}

size_t Board::getColumns() const
{
	return numColumns;
}

size_t Board::getStartX() const
{
	return startX;
}

size_t Board::getStartY() const
{
	return startY;
}

void Board::placeTileOnBoard(Tile* currTile, size_t y, size_t x)
{
	if(!boardArray[x][y] -> isOccupied())
	{
		boardArray[x][y] -> placeTile(currTile);
	}
}