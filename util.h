//
// util.h
// File that runs the compression program. It uses trees and binary
// queues to compress and decompress files using the standard Huffman algorithm for
// encoding and decoding
//
// Tyler Strach
// U. of Illinois, Chicago
// Fall 2022
//

#pragma once

#include <fstream> // for file reading
#include <queue> // for priority_queue

typedef hashmap hashmapF;
typedef unordered_map <int, string> hashmapE;

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};

struct compare
{
    bool operator()(const HuffmanNode *lhs,
        const HuffmanNode *rhs)
    {
        return lhs->count > rhs->count;
    }
};

/**
 * recursive function that frees every node in the Huffman Tree
 * @param root
 */
void _freeTree(HuffmanNode* root){
    if(root == nullptr)
        return;
    _freeTree(root->zero);
    _freeTree(root->one);

    root->zero = nullptr;
    root->one = nullptr;
    delete root;
}
//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
    _freeTree(node);
}

//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmapF &map) {
    ifstream inPut(filename);
    // if open, reads the file, if not, treats file name as string and reads filename
    if(isFile){
        char cur;
        // while still characters in file
        while(inPut.get(cur)){
            // if the char already exists in map, add 1 to frequency
            if(map.containsKey(cur)){
                int curVal = map.get(cur);
                map.put(cur, curVal+1);
            }
            else // add with frequency of 1
                map.put(cur, 1);
        }
    }
    else{ // for only reading the file name
        for(char cur : filename){
            // if the char already exists in map, add 1 to frequency
            if(map.containsKey(cur)){
                int curVal = map.get(cur);
                map.put(cur, curVal+1);
            }
            else // add with frequency of 1
                map.put(cur, 1);
        }
    }
    map.put(PSEUDO_EOF, 1); // add the end of file character
}

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmapF &map) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> pq;
    vector<int> allChars = map.keys();

    for(int key : allChars){
        //create the new node for each char
        auto* curNode = new HuffmanNode;
        curNode->character = key;
        curNode->count = map.get(key);
        curNode->zero = nullptr;
        curNode->one = nullptr;
        pq.push(curNode);
    }

    while(pq.size() > 1){
        HuffmanNode* firstNode = pq.top(); // get the first node
        pq.pop();

        if(pq.top() != nullptr){ // if it is a real node
            HuffmanNode* secondNode = pq.top(); // get the second node
            pq.pop();

            // create the node parent that combines them
            HuffmanNode* sumNode = new HuffmanNode;
            sumNode->character = NOT_A_CHAR;
            sumNode->count = (firstNode->count + secondNode->count);
            sumNode->zero = firstNode;
            sumNode->one = secondNode;
            pq.push(sumNode);
        }
    }
    return pq.top();
}

//
// *Recursive helper function for building the encoding map.
//
void _buildEncodingMap(HuffmanNode* node, hashmapE &encodingMap, string str) {
    if(node->character != NOT_A_CHAR){
        encodingMap.emplace(node->character, str);
        return;
    }
    _buildEncodingMap(node->zero, encodingMap, str+="0");
    str = str.substr(0, str.size()-1); // subtract off previous value '0'
    _buildEncodingMap(node->one, encodingMap, str+="1");
    str = str.substr(0, str.size()-1); // subtract off previous value '1'
}

//
// *This function builds the encoding map from an encoding tree.
//
hashmapE buildEncodingMap(HuffmanNode* tree) {
    hashmapE encodingMap;
    _buildEncodingMap(tree, encodingMap, "");
    return encodingMap;
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, hashmapE &encodingMap, ofbitstream& output,
              int &size, bool makeFile) {
    string buildString;
    char cur;

    if(makeFile){
        // for each character in the input stream
        while(input.get(cur)){
            string encode = encodingMap.at(cur); // get the encoding and write each bit to the output stream
            for(char &bit : encode){
                if(bit == '0')
                    output.writeBit(0);
                if(bit == '1')
                    output.writeBit(1);
                size++;
            }
            buildString += encode; // add encoding to the output string for testing
        }
        // add each bit from the EOF encoding
        string eof = encodingMap.at(PSEUDO_EOF);
        for(char &bit : eof){
            if(bit == '0')
                output.writeBit(0);
            if(bit == '1')
                output.writeBit(1);
            size++;
        }
        buildString += eof;// add encoding to the output string for testing
    }
    return buildString;
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    string buildString;
    HuffmanNode* curNode = encodingTree;

    while(!input.eof()) { // keep taking in characters until the end of file
        if(curNode->character == PSEUDO_EOF) // once it reaches the eof encoding, ends the loop
            break;
        // when the node is a character encoding, adds to the output and reset tree
        if (curNode->character != NOT_A_CHAR) {
            buildString += curNode->character;
            output.put(curNode->character);
            curNode = encodingTree;
        }
        // while the node is not an encoding, keep progressing down the tree
        int bit = input.readBit();
        if (bit == 0)
            curNode = curNode->zero;
        else if (bit == 1)
            curNode = curNode->one;
    }

    return buildString;  // TO DO: update this return
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file.  This function
// creates a compressed file named (filename + ".huf") and also
// returns a string version of the bit pattern.
//
string compress(string filename) {
    // opens the file and tests if the file is openable
    bool isFile = false;
    ifstream inFile(filename);
    if(inFile)
        isFile = true;

    // builds the frequency map
    hashmapF map;
    buildFrequencyMap(filename, isFile, map);

    // builds the encodingTree and encodingMap
    HuffmanNode* root = buildEncodingTree(map);
    hashmapE encodingMap = buildEncodingMap(root);

    // creates the input and new output streams for the encoding
    ifstream input(filename);
    ofbitstream output(filename + ".huf");
    output << map;
    int size = 0;

    string encodedMessage = encode(input, encodingMap, output, size, true);
    _freeTree(root);
    return encodedMessage; // encode the file and return
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function creates a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function returns a string version of the
// uncompressed file.  Note: this function reverses the compression function
//
string decompress(string filename) {
    ifbitstream input(filename);

    // string parsing to correctly name the output file
    int pos = filename.find(".huf");
    if (pos >= 0)
        filename = filename.substr(0, pos);

    pos = filename.find(".");
    string ext = filename.substr(pos, filename.length() - pos);
    filename = filename.substr(0, pos);

    ofstream output(filename + "_unc" + ext);

    // get the frequency map from the first part of encoded file
    hashmapF frequencyMap;
    input >> frequencyMap;

    // build the encoding tree
    HuffmanNode* root = buildEncodingTree(frequencyMap);

    string decodedMessage = decode(input, root, output);
    _freeTree(root);
    return decodedMessage; // decode the file and return
}
