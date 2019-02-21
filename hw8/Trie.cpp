#include "Trie.h"

using namespace std;

TrieSet::TrieSet()
{
	TrieNode *tRoot = new TrieNode;
	root = tRoot;
	root->parent = nullptr;
	root->inSet = false;
	root->numChildren = 0;
	for(int i = 0; i < 26; i++)
	{
		root->children[i] = nullptr;
	}
}

TrieSet::~TrieSet()
{
	clear(root);	
}

void TrieSet::clear(TrieNode* currNode)
{
	if(currNode != nullptr)
	{		
		for(int i = 0; i < 26; i++)
		{
			if(currNode -> children[i] != nullptr)
			{
				clear(currNode -> children[i]);
			}
		}
		if(currNode == root)
		{
			root = nullptr;
			delete currNode;
		}
		else
		{
			delete currNode;
		}
		
	}
}

//inserts string, assumed to be lower case
void TrieSet::insert (string input)
{
	transform(input.begin(), input.end(), input.begin(), ::tolower);
	TrieNode *currNode = root;
	//iterate through each letter in whole string
	size_t count = 0;
	while(count < input.size())
	{
		char currLetter = input[count];
		//if currNode doesn't have this letter as a child, add it
		if(currNode->children[currLetter - 97] == nullptr)
		{
			TrieNode *nextNode = new TrieNode;
			nextNode -> parent = currNode;
			nextNode -> inSet = false;
			nextNode -> numChildren = 0;
			for(int i = 0; i < 26; i++)
			{
				nextNode->children[i] = nullptr;
			}
			currNode -> children[currLetter - 97] = nextNode;
			currNode -> numChildren++;
		}
		//advance down to next node and next letter
		currNode = currNode -> children[currLetter - 97];
		count++;
	}
	//set end node as a string
	currNode -> inSet = true;
	
}

void TrieSet::remove (string input)
{
	transform(input.begin(), input.end(), input.begin(), ::tolower);
	TrieNode *currNode = root;
	//iterate through each letter in whole string
	size_t count = 0;
	while(count < input.size())
	{
		char currLetter = input[count];
		//cout << "currLetter: " << currLetter << endl;
		//if currNode doesn't have this letter as a child, end function
		if(currNode->children[currLetter - 97] == nullptr)
		{
			return;
		}
		//advance down to next node and next letter
		currNode = currNode -> children[currLetter - 97];
		count++;
	}
	//check if the node denotes end of the string. if so, go back up the trie
	if(currNode -> inSet)
	{
		//remove indication this is a word
		currNode -> inSet = false;

		while(currNode != root)
		{
			//if currNode has no children and is not a string, delete it :3
			TrieNode *parentNode = currNode -> parent;
			if(currNode -> numChildren <= 0 && currNode -> inSet == false)
			{
				//cout << "deleting..." << endl;
				//in the parent, find currNode in the children list and set it to null
				//cout << "parent node initially has " << parentNode -> numChildren << " children" << endl;
				for(int i = 0; i < 26; i++)
				{
					if(parentNode -> children[i] == currNode)
					{

						parentNode -> children[i] = nullptr;
					}
				}
				parentNode -> numChildren--;
				//cout << "parent node now has " << parentNode -> numChildren << " children" << endl;
				delete currNode;

			}
			currNode = parentNode;
		}
	}
	else
	{
		return;
	}
	
}

TrieNode* TrieSet::prefix(string px)
{
	transform(px.begin(), px.end(), px.begin(), ::tolower);
	//cout << "searching for " << px << endl;
	TrieNode *currNode = root;
	//iterate through each letter in whole string
	size_t count = 0;
	while(count < px.size())
	{
		char currLetter = px[count];
		//not an alphabetical character
		if(currLetter < 97 || currLetter > 122)
		{
			return nullptr;
		}
		//cout << "searching at " << currLetter << endl;
		//if currNode doesn't have this letter as a child, end function
		if(currNode->children[currLetter - 97] == nullptr)
		{
			//cout << "couldn't find" << endl;
			return nullptr;
		}
		//advance down to next node and next letter
		currNode = currNode -> children[currLetter - 97];
		count++;
	}
	return currNode;
}
