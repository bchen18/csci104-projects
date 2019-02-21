#include "Dictionary.h"
#include <fstream>
#include <algorithm>


using namespace std;

Dictionary::Dictionary(string dictionary_file_name)
{
	ifstream ifile(dictionary_file_name);
	while(!ifile.fail())
	{
		string nextWord;
		getline(ifile, nextWord);
		transform(nextWord.begin(), nextWord.end(), nextWord.begin(), ::tolower); 
		words.insert(nextWord);//adds next word, converted to lowercase
	}
}

Dictionary::~Dictionary()
{
	
}

bool Dictionary::isLegalWord (string const & word) const
{
	set<string>::iterator iter;
	iter = words.find(word);
	if(iter == words.end())
	{
		return false;
	}
	else
	{
		return true;
	}
   
}

