#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>
#include <fstream>
#include <stdio.h> 
#include <time.h>


using namespace std;

void RLE(string& result, string& line) {

    char separator = '.';
    int i = 0;
    while (i < line.size()) {
        int count = 0;
        char symbol = line[i];
        while (line[i] == symbol) {
            count++;
            i++;
        }
        result = result + to_string(count) + separator + symbol;
    }
}


string string_to_ascii_bunary(const string& inputString) {
    stringstream binaryStream;
    for (char c : inputString) {
        int asciiCode = (int)c;
        for (int i = 7; i >= 0; --i) {
            binaryStream << ((asciiCode >> i) & 1);
        }
    }
    return binaryStream.str();
}

string read_from_file(const string& filename) {
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error" << endl;
        return "";
    }
    stringstream buffer;
    buffer << inputFile.rdbuf();
    inputFile.close();
    return buffer.str();
}




int main(){
    string filename = "text_1100kb.txt";
    string text = read_from_file(filename);
    string result = "";
    clock_t start = clock();
    RLE(result, text);
    clock_t end = clock();
    cout << filename << endl;
    cout << "Time:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
    cout << "Result size " << result.size() << endl;
    string ascii_result = string_to_ascii_bunary(result);
    cout << "Binary size " << ascii_result.size() << endl;

    return 0;
}
