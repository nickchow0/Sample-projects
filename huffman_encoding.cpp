// Nick Chow
// CS106B HW#6 - Huffman Encoding
// This is a program that contains an algorithm called Huffman Encoding
// to compress and decode files. It contains methods to build a frequency table
// encoding tree, encoding map, encode, decode data, and a method to free memory.

#include "encoding.h"
#include "huffmanutil.h"
#include "HuffmanNode.h"
#include "pqueue.h"
#include "map.h"
#include "iostream"
#include "strlib.h"
#include "bitstream.h"

using namespace std;

// This is a helper method for encoding data
void buildEncodingTreeHelper(HuffmanNode* encodingTree, Map<int, string>& encodingMap, string code);
// This is a helper method for decoding data
int decodeDataHelper(HuffmanNode* encodingTree, int bit, ibitstream& input);

// This method builds a frequency table from the input
Map<int, int> buildFrequencyTable(istream& input) {
    Map <int, int> freqTable;
    freqTable.put(PSEUDO_EOF, 1);
    if (input.peek() == -1) { // if input is empty, return a map with PSEUDO_EOF only
        return freqTable;
    }
    while (true){
        int character = input.get();
        if (character != -1) {
            freqTable[character]++; // add each character to the map and increase count each time
        } else {
            break; // end of file
        }
    }
    return freqTable;
}

// This method builds a encoding tree table from a frequency table by using a priority queue.
// The method will use put the characters with the lowest frequencies at the bottom of the tree.
HuffmanNode* buildEncodingTree(const Map<int, int>& freqTable) {
    PriorityQueue<HuffmanNode*> PQueue;
    for (int i:freqTable){ // enqueue each character from the frequency table with a HuffmanNode
        HuffmanNode* node = new HuffmanNode(i, freqTable[i]);
        PQueue.enqueue(node, freqTable[i]);
    }
    while (PQueue.size() > 1) { // while there are more than 1 elements in the pqueue
        HuffmanNode* node = new HuffmanNode();
        // dequeue the first 2 nodes, make the 2 node children
        // of a node with count of the nodes combined
        node->zero = PQueue.dequeue();
        node->one = PQueue.dequeue();
        node->count = node->zero->count + node->one->count;
        PQueue.enqueue(node, node->count); // enqueue the parent node
    }
    HuffmanNode* root = PQueue.dequeue(); // dequeue the last node and make it the root of the encoding tree
    return root;
}

// This method builds an encoding map using an encoding tree
Map<int, string> buildEncodingMap(HuffmanNode* encodingTree) {
    Map<int, string> encodingMap;
    if (encodingTree == NULL) {
        // do nothing if tree is empty
    } else {
        buildEncodingTreeHelper(encodingTree, encodingMap, ""); // call the helper method
    }
    freeTree(encodingTree); // free memory
    return encodingMap;
}

// This is a recursive helper method for building the encoding tree, it walks through the tree and
// gets the code of the character according to the path
void buildEncodingTreeHelper(HuffmanNode* encodingTree, Map<int, string>& encodingMap, string code) {
    // base case: if the node is a leaf, return the code
    if (encodingTree->isLeaf()) {
        encodingMap.put(encodingTree->character, code);
        return;
    } else { // add 0 or 1 to the character code if the node has a child
        if (encodingTree->zero != NULL) {
            buildEncodingTreeHelper(encodingTree->zero, encodingMap, code + "0");
        }
        if (encodingTree->one != NULL) {
            buildEncodingTreeHelper(encodingTree->one, encodingMap, code + "1");
        }
    }
}

// This method encodes data from an input using an encoding map and outputs it to a obitstream
void encodeData(istream& input, const Map<int, string>& encodingMap, obitstream& output) {
    while (true){
        int character = input.get(); // get character from input
        string code = encodingMap[character]; // find the corresponding code from the encoding map
        if (character != -1) { // if it is not end of file
            for (int i = 0; i < code.length(); ++i) { // output each character of the character code
                if (code[i] == '1') {
                    output.writeBit(1);
                } else {
                    output.writeBit(0);
                }
            }
        } else {
            // end of file, append code for PSEUDO_EOF to the end
            code = encodingMap[256];
            for (int i = 0; i < code.length(); ++i) { // output each character of the character code
                if (code[i] == '1') {
                    output.writeBit(1);
                } else {
                    output.writeBit(0);
                }
            }
            break;
        }
    }
}

// This method decodes data from an input using an encoding tree and outputs it to a obitstream
void decodeData(ibitstream& input, HuffmanNode* encodingTree, ostream& output) {
    while (true) {
        int bit = input.readBit(); // get character from input
        int character = decodeDataHelper(encodingTree, bit, input); // call the helper method
        if (character == PSEUDO_EOF) { // end of file
            freeTree(encodingTree); // free memory
            break;
        }
        output.put(character); // output the character
    }
}

// This is a recursive helper method for decoding, it walks through the tree and
// gets the character by using the code to find the path
int decodeDataHelper(HuffmanNode* encodingTree, int bit, ibitstream& input) {
    // base case:  if the node is a leaf, return the character
    if (encodingTree->isLeaf()) {
        return encodingTree->character;
    }
    else if (bit == 0) { // if bit is 0
        if (encodingTree->zero->isLeaf()) { // if the zero node is a leaf
            return encodingTree->zero->character; // return the character of the zero node
        } else { // else, recursively call this method with the zero node and the next bit
            return decodeDataHelper(encodingTree->zero, input.readBit(), input);
        }
    }
    else if (bit == 1) { // if bit is 1
        if (encodingTree->one->isLeaf()) { // if the zero node is a leaf
            return encodingTree->one->character; // return the character of the zero node
        } else { // else, recursively call this method with the zero node and the next bit
            return decodeDataHelper(encodingTree->one, input.readBit(), input);
        }
    }
    return -1;
}

// This method takes an input the outputs the encoded data with a header of the frequency table
void compress(istream& input, obitstream& output) {
    Map<int, int> freqTable = buildFrequencyTable(input); // build a frequency table
    output << freqTable;   // write header
    HuffmanNode* encodingTree = buildEncodingTree(freqTable); // build an encoding tree
    Map<int, string> encodingMap = buildEncodingMap(encodingTree); // build an encoding map
    input.clear();             // removes any current eof/failure flags
    input.seekg(0, ios::beg);  // tells the stream to seek back to the beginning
    encodeData(input, encodingMap, output); // encode the data
    freeTree(encodingTree); // free memory
}

// This method takes an input the outputs the decoded data
void decompress(ibitstream& input, ostream& output) {
    Map<int, int> freqTable; // create an empty frequency table
    input >> freqTable;    // read header and populate frequency table
    HuffmanNode* encodingTree = buildEncodingTree(freqTable); // build encoding tree using the header
    decodeData(input, encodingTree, output); // decode the data and output it
    freeTree(encodingTree); // free memory
}

// This recursive method frees the memory by deleteing each node of a tree
void freeTree(HuffmanNode* node) {
    if (node == NULL) { // do nothing if tree is empty
        return;
    }
    if (node->isLeaf()) { // delete the node if it is a leaf
        delete node;
        return;
    }
    if (node->zero != NULL) { // go to zero node if it is not null
        return freeTree(node->zero); // recursively call this method
    }
    if (node->one != NULL) { // go to one node if it is not null
        return freeTree(node->one); // recursively call this method
    }
}
