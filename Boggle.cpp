// Nick Chow
// CS106B HW4 - Boggle
// This is a Boggle class and implements the Boggle object and various methods

#include "Boggle.h"
#include <fstream>
#include <iostream>
#include <string>
#include "console.h"
#include "lexicon.h"
#include "random.h"
#include "simpio.h"
#include "strlib.h"
#include "random.h"
#include "grid.h"
#include "set.h"
#include "shuffle.h"
#include "bogglegui.h"
#include "boggle.h"

using namespace std;

// letters on all 6 sides of every cube
static string CUBES[16] = {
    "AAEEGN", "ABBJOO", "ACHOPS", "AFFKPS",
    "AOOTTW", "CIMOTU", "DEILRX", "DELRVY",
    "DISTTY", "EEGHNW", "EEINSU", "EHRTVW",
    "EIOSST", "ELRTTY", "HIMNQU", "HLNNRZ"
};

// letters on every cube in 5x5 "Big Boggle" version (extension)
static string BIG_BOGGLE_CUBES[25] = {
   "AAAFRS", "AAEEEE", "AAFIRS", "ADENNN", "AEEEEM",
   "AEEGMU", "AEGMNN", "AFIRSY", "BJKQXZ", "CCNSTW",
   "CEIILT", "CEILPT", "CEIPST", "DDLNOR", "DDHNOT",
   "DHHLOR", "DHLNOR", "EIIITT", "EMOTTT", "ENSSSU",
   "FIPRSY", "GORRVW", "HIPRRY", "NOOTUW", "OOOTTU"
};

// Constructor to initialize the Boggle class
Boggle::Boggle(Lexicon& dictionary, string boardText) {
    //check to see if the GUI has been initialized
    if (BoggleGUI::isInitialized()) {
        BoggleGUI::reset();
    } else {
        BoggleGUI::initialize(4,4);
        BoggleGUI::setStatusMessage("It's your turn!");
    }
    board.resize(4,4);
    dict = dictionary;
    boardSize = 4;
    boardText = toUpperCase(boardText);
    //make a random board if input is empty
    if (boardText.length() == 0){
        int count = 0;
        for (int i = 0; i < 4; ++i){
            for (int j = 0; j < 4; ++j){
                char letter = CUBES[count][randomInteger(0,5)];
                board[i][j] = letter;
                ++count;
            }
        }
        shuffle(board);
        for (int i = 0; i < 4; ++i){
            for (int j = 0; j < 4; ++j){
                char letter = board[i][j];
                BoggleGUI::labelCube(i,j,letter);
            }
        }
    } else {    //otherwise, create the board from given text
        int count = 0;
        for (int i = 0; i < 4; ++i){
            for (int j = 0; j < 4; ++j){
                board[i][j] = boardText[count];
                ++count;
            }
        }
        BoggleGUI::labelAllCubes(boardText);
    }
}

// gets the letter of the grid at a specific position
char Boggle::getLetter(int row, int col) const {
    if (row >= boardSize || col >= boardSize){
        throw row;
    }
    return board[row][col];
}

// checks if a word is valid (longer than 4 characters,
// is found in dictionary, and if it has been found before
bool Boggle::checkWord(string word) const {
    if (word.length() == 0) {
        return false;
    }
    else if (word.length() < 4
             || !dict.contains(toLowerCase(word))
             || wordsFoundHuman.contains(toUpperCase(word))){
        BoggleGUI::setStatusMessage("You must enter an unfound 4+ letter word from the dictionary.");
        return false;
    }
    else if (wordsFoundComputer.contains(toUpperCase(word))){
        return false;
    }
    else {
        return true;
    }
}

// performs recursive backtracking search of the word
// on the board according to the input
bool Boggle::humanWordSearch(string word) {
    if (checkWord(word)){
        word = toUpperCase(word);
        char letter = word[0];
        bool result;
        BoggleGUI::clearHighlighting();
        for (int i = 0; i < 4; ++i){        //loop through the board
            for (int j = 0; j < 4; ++j){
                char position = board[i][j];
                BoggleGUI::setHighlighted(i, j, true);  //highlighting
                BoggleGUI::setAnimationDelay(100);
                if (board[i][j] == letter){
                    board[i][j] = '0';
                    result = humanWordSearchHelper(word.substr(1), board, i, j);    // calls the recursive helper method
                    if (result) {       // if the word is found, add score and add the word to the list of words found
                        wordsFoundHuman.add(word);
                        humanScore += (word.length()-3);
                        BoggleGUI::setStatusMessage("You have found a new word! \"" + word + "\"");
                        BoggleGUI::recordWord(word + "\n", BoggleGUI::HUMAN);
                        BoggleGUI::setScore(humanScore, BoggleGUI::HUMAN);
                        board[i][j] = letter;
                        return result;
                    }
                } else {
                BoggleGUI::setHighlighted(i, j, false);
                }
                board[i][j] = position;
            }
        }
        return false;
    } else {
        return false;   //word not found, return false
    }
}

// helper method for humanWordSearch, implements recursive backtracking method
bool Boggle::humanWordSearchHelper(string word, Grid<char> board, int i, int j) {
    // base case
    if (word.length() == 0){
        return true;
    } else {
        for (int ci = i-1; ci <= i+1; ci++){        // loop through the surrounding letters
            for (int cj = j-1; cj <= j+1; cj++){
                if (board.inBounds(ci, cj)){
                    if (board[ci][cj] == word[0]){
                        BoggleGUI::setHighlighted(ci, cj, true);  //highlighting
                        BoggleGUI::setAnimationDelay(100);
                        char letter = board[ci][cj];
                        board[ci][cj] = '0';    //choose
                        if (humanWordSearchHelper(word.substr(1), board, ci, cj)){  //recursive call
                            return true;
                        }
                        board[ci][cj] = letter; //unchoose
                    }
                }
            }
        }
        BoggleGUI::setStatusMessage("This word cannot be formed on the board.");
        BoggleGUI::clearHighlighting();
        return false;
    }
}

//get the user's score
int Boggle::getScoreHuman() const {
    return humanScore;
}

//get the words found by the user
Set<string> Boggle::getWordsFoundHuman() const {
    return wordsFoundHuman;
}

//get the words found by the computer
Set<string> Boggle::getWordsFoundComputer() const {
    return wordsFoundComputer;
}

// performs recursive backtracking search for all possible words from the board
Set<string> Boggle::computerWordSearch() {
    string chosen;
    for (int i = 0; i < 4; ++i){        //loop through the board
        for (int j = 0; j < 4; ++j){
            chosen = board[i][j];
            board[i][j] = '0';
            computerWordSearchHelper(chosen, board, i, j);  // calls the recursive helper method
            board[i][j] = chosen[0];
        }
    }
    if (computerScore > humanScore){
        BoggleGUI::setStatusMessage("Ha ha ha, I destroyed you. Better luck next time, puny human!");
    } else {
        BoggleGUI::setStatusMessage("WOW, you defeated me! Congratulations!");
    }
    return wordsFoundComputer;
}

void Boggle::computerWordSearchHelper(string chosen, Grid<char> board, int i, int j) {
    // base case
    if (!dict.containsPrefix(toLowerCase(chosen))){
        return;
    }
    // if chosen is valid and found in dict, add it to wordsFoundComputer and add points
    else if (checkWord(chosen)){
        wordsFoundComputer.add(toUpperCase(chosen));
        computerScore += (chosen.length()-3);
        BoggleGUI::recordWord(chosen + "\n", BoggleGUI::COMPUTER);
        BoggleGUI::setScore(computerScore, BoggleGUI::COMPUTER);
    } else {
        for (int ci = i-1; ci <= i+1; ci++){        // loop through the surrounding letters
            for (int cj = j-1; cj <= j+1; cj++){
                if (board.inBounds(ci,cj) && board[ci][cj] != '0'){
                    char letter = board[ci][cj];
                    chosen += board[ci][cj];        // choose
                    board[ci][cj] = '0';
                    computerWordSearchHelper(chosen, board, ci, cj);        // recursive call and explore
                    chosen = chosen.substr(0,chosen.length()-1);
                    board[ci][cj] = letter;         // unchoose
                }
            }
        }
    }
    return;
}

//get the computer's score
int Boggle::getScoreComputer() const {
    return computerScore;
}

//overloading the << operator
ostream& operator<<(ostream& out, Boggle& boggle) {
    for (int i = 0; i < 4; ++i){
        for (int j = 0; j < 4; ++j){
        out << boggle.board[i][j];
        }
        out << endl;
    }
    return out;
}
