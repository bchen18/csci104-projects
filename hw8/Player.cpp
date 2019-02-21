#include "Player.h"
#include <set>
#include <iostream>

using namespace std;

Player::Player(string const & name, size_t maxTiles)
{
	playerName = name;
	if(playerName == "CPUL" || playerName == "cpul")
	{
		CPUL = true;
	}
	else
	{
		CPUL = false;
	}
	if(playerName == "CPUS" || playerName == "cpus")
	{
		CPUS = true;
	}
	else
	{
		CPUS = false;
	}
	maxTileNum = maxTiles;
	score = 0;
}

Player::~Player()
{
	for(size_t i = 0; i < maxTileNum; i++)
	{
		delete playerTiles[i];
	}
}

//returns player's hand of tiles
vector<Tile*> Player::getHandTiles() const
{
	return playerTiles;
}

/*check if player has necessary tiles. "move" is only the tiles requested, not the entire move command
if resolveBlanks is true: if there is a "?" letter then the next letter is what the player wants it to be (basically ignore it)
*/
bool Player::hasTiles(string const & move, bool resolveBlanks) const
{
	//create copy of player's hand to properly check the requested tiles
	vector<Tile*> playerCopy (playerTiles);

	//loop through requested tiles to compare with hand
	size_t count = 0;
	while(count < move.size())
	{
		char letter = move[count];

		bool hasLetter = false;
		//loop through hand, checking if current tile exists
		for(size_t i = 0; i < playerCopy.size(); i++)
		{

			if(letter == (playerCopy[i] -> getLetter()))
			{
				hasLetter = true;
				//deletes detected letter to account for dupes
				playerCopy.erase(playerCopy.begin() + i);
				break;
			}
		}
			//if letter not found, immediately end and return false
		if(!hasLetter)
		{

			return false;
		}
		//if resolveBanks is true and letter was a "?", skip the next letter
		if(letter == '?' && resolveBlanks)
		{
			count++;
		}
		count++;
	}
	return true;
	
}

//removes and returns vector of tiles from player's hand, based on the requested move. 
vector<Tile*> Player::takeTiles (string const & moveString, bool resolveBlanks)
{
	vector<Tile*> moveTiles;
	size_t count = 0;
	while(count < moveString.size())
	{
		char letter = moveString[count];
		//loop through hand, checking if current tile exists
		for(size_t i = 0; i < playerTiles.size(); i++)
		{
			if(letter == (playerTiles[i] -> getUse()))
			{
				//if need to resolve a blank tile, assign it as the next letter
				if(letter == '?' && resolveBlanks)
				{
					playerTiles[i] -> useAs(moveString[count + 1]);
				}
				//adds tile to vector to be returned 
				moveTiles.push_back(playerTiles[i]);
				//deletes letter from player's hand
				playerTiles.erase(playerTiles.begin() + i);
				break;
			}
		}
		//if resolveBlanks is true and letter was a "?", skip the next letter
		if(letter == '?' && resolveBlanks)
		{
			count++;
		}
		count++;
	}
	return moveTiles;
}

//adds tiles
void Player::addTiles (vector<Tile*> const & tilesToAdd)
{
	for(size_t i = 0; i < tilesToAdd.size(); i++)
	{
		playerTiles.push_back(tilesToAdd[i]);
	}
}

//returns player's score
int Player::getScore() const
{
	return score;
}

void Player::changeScore(int newScore)
{
	score = newScore;
}

size_t Player::getMaxTiles() const
{
	return maxTileNum;
}

bool Player::isCPUL() const
{
	return CPUL;
}

bool Player::isCPUS() const
{
	return CPUS;
}