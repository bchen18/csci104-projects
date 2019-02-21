#include "Move.h"
#include "Board.h"
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace std;


//given the move string (the entire move command), creates the correct move subclass
Move::Move()
{

}

Move::~Move()
{
	
}

Move * Move::parseMove(string moveString, Player &p)
{
	string moveType = moveString.substr(0, moveString.find(" "));
	transform(moveType.begin(), moveType.end(), moveType.begin(), ::toupper); 
	//pass move
	if(moveType == "PASS")
	{
		PassMove* pass = new PassMove(&p);
		return pass;
	}
	//exchange move
	else if(moveType == "EXCHANGE")
	{
		transform(moveString.begin(), moveString.end(), moveString.begin(), ::tolower);
		ExchangeMove* exchange = new ExchangeMove(moveString.substr(moveString.find(" ") + 1), &p);
		return exchange;
	}
	//place move
	else 
	{
		stringstream ss; 
		transform(moveString.begin(), moveString.end(), moveString.begin(), ::tolower);
		string parameters = moveString.substr(moveString.find(" ") + 1);
		string directionString;
		size_t x, y;//coordinates start at 1, 1!
		string tiles;
		ss << parameters;
		ss >> directionString >> x >> y >> tiles;
		bool horizontal;
		//determine if horizontal or not
		if(directionString == "-") 
		{
			horizontal = true;
		}
		else
		{
			horizontal = false;
		}
		PlaceMove* place = new PlaceMove(x - 1, y - 1, horizontal, tiles, &p);
		return place;


	}
}

//PassMove constructor
PassMove::PassMove(Player * player): Move()
{
	_player = player;
}

PassMove::~PassMove()
{

}

//execute pass move: nothing happens
bool PassMove::execute(Board & board, Bag & bag, Dictionary & dictionary)
{
	return true;
}

//ExchangeMove constructor
ExchangeMove::ExchangeMove(string tileString, Player * p): Move()
{
	_player = p;
	_tileString = tileString;
}

ExchangeMove::~ExchangeMove()
{

}

//execute exchange move: take requested tiles from player hand, replace with new tiles from bag
bool ExchangeMove::execute(Board & board, Bag & bag, Dictionary & dictionary)
{
	//check if player has requested tiles
	if(_player -> hasTiles(_tileString, false))
	{
		//take requested tiles from player to place back in bag, then give them same amount from bag
		vector<Tile*> exchangeTiles = _player -> takeTiles(_tileString, false);
		bag.addTiles(exchangeTiles);
		vector<Tile*> newTiles = bag.drawTiles (_tileString.size());
		_player -> addTiles(newTiles);
		return true;
	}
	else
	{
		return false;
	}

}

//PlaceMove constructor
//The x and y coordinates will have already been shifted (since board starts at (1, 1))
PlaceMove::PlaceMove(size_t y, size_t x, bool horizontal, string tileString, Player * p): Move()
{
	_player = p;
	xPos = x;
	yPos = y;
	isHorizontal = horizontal;
	_tileString = tileString;
	tileVector = p -> getHandTiles();
}

PlaceMove::~PlaceMove()
{

}

//execute place move: take requested tiles from player hand, place on board
bool PlaceMove::execute(Board & board, Bag & bag, Dictionary & dictionary)
{
	//check if player has requested tiles
	if(_player -> hasTiles(_tileString, true))
	{
		//take requested tiles from player, then give them same amount from bag
		vector<Tile*> placeTiles = _player -> takeTiles(_tileString, true);
		vector<Tile*> newTiles = bag.drawTiles (placeTiles.size());
		_player -> addTiles(newTiles);
		//runs the function within Board class that executes this place move. if successful, returns true
		return board.executePlaceMove (placeTiles, xPos, yPos, isHorizontal);

	}
	else
	{
		return false;
	}
}

size_t PlaceMove::getXPos() const
{
	return xPos;
}

size_t PlaceMove::getYPos() const
{
	return yPos;
}

bool PlaceMove::checkHorizontal() const
{
	return isHorizontal;
}

string PlaceMove::getTileString() const
{
	return _tileString;
}

vector<Tile*> PlaceMove::getTileVector() const
{
	return tileVector;
}