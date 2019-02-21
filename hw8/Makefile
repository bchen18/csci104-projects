all: scrabble

scrabble: scrabble.cpp Bag.cpp Board.cpp ConsolePrinter.cpp Dictionary.cpp Move.cpp Player.cpp
	g++ -Wall -g -std=c++11 scrabble.cpp Bag.cpp Board.cpp ConsolePrinter.cpp Dictionary.cpp Move.cpp Player.cpp Trie.cpp -o scrabble

testTrie: testTrie.cpp Trie.cpp
	g++ -Wall -g -std=c++11 testTrie.cpp Trie.cpp -o testTrie