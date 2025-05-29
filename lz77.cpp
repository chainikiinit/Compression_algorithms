#include <iostream>
#include <vector>
#include <sstream>
#include <bitset>
#include <fstream>
#include <string>
#include <stdio.h> 
#include <time.h>

using namespace std;

struct code_word {
    int Shift;    // Смещение
    int Max_Len;  // Максимальная длина
    char Symbol;  // Следующий символ
};

vector<code_word> LZ77(string& line) {
    int window_size = 4096;
    vector<code_word> result = {};
    int len = line.size();
    int current_position = 0;

    while (current_position < len) {
        int max_len = 0;
        int shift = 0;

        for (int i = max(0, current_position - window_size); i < current_position; ++i) {
            int match_len = 0;

            while ((current_position + match_len < len) && 
                (i + match_len < current_position) &&
                (line[i + match_len] == line[current_position + match_len])) {
                match_len++;
            }

            if (match_len > max_len) {
                max_len = match_len;
                shift = current_position - i;

            }

        }


        if (max_len > 0) {
            code_word word;
            word.Shift = shift;
            word.Max_Len = max_len;
            word.Symbol = line[current_position + max_len];
            result.push_back(word);
            current_position = current_position + max_len + 1;
        }
        else {
            code_word word;
            word.Shift = 0;
            word.Max_Len = 0;
            word.Symbol = line[current_position];
            result.push_back(word);
            current_position++;
        }
    }

    return result;
}



string int_to_binary(int num) {
    bitset<16> binary(num);
    return binary.to_string();
}

string char_to_binary(char c) {
    bitset<8> binary(c);   // 8-количество бит
    return binary.to_string();
}

string code_word_to_binary_string(const code_word& word) {
    stringstream binaryStream;
    binaryStream << int_to_binary(word.Shift);
    binaryStream << int_to_binary(word.Max_Len);
    binaryStream << char_to_binary(word.Symbol);

    return binaryStream.str();
}

string vector_code_word_to_binary(const vector<code_word>& codeWords) {
    string binaryStrings;
    for (const auto& word : codeWords) {
        binaryStrings = binaryStrings + code_word_to_binary_string(word);
    }
    return binaryStrings;

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
    string filename = "text_20kb.txt";
    string line = read_from_file(filename);
    clock_t start = clock();
    vector<code_word> result = LZ77(line);
    clock_t end = clock();
    cout << filename << endl;
    cout << "Time:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
    /*
    for (int i = 0; i < result.size(); ++i) {
        cout << result[i].Shift << " " << result[i].Max_Len << " " << result[i].Symbol << endl;
    }*/
    cout << "Result size " << result.size() << endl;
    string binaryResult = vector_code_word_to_binary(result);
    cout << "Binary size " << binaryResult.size() << endl;
}
