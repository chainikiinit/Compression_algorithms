#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cmath>
#include <math.h>
#include <string>
#include <queue>
#include <unordered_map>
#include <sstream>
#include <bitset>
#include <stdio.h> 
#include <time.h>

using namespace std;

struct RIFFHeader {
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
};

struct ChunkInfo {
    char chunk_id[4];
    uint32_t chunk_size;
};

struct FmtChunk {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

struct DataChunk {
    vector<int16_t> data; 
    int nb_of_samples;

    DataChunk(int s) : nb_of_samples{ s } {
        data.resize(s);
    }
};

struct code_word {
    int Shift;    
    int Max_Len; 
    char Symbol;  
};

vector<code_word> LZ77(string& line) {
    int window_size = 1024;
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
    bitset<8> binary(c);   // 8-êîëè÷åñòâî áèò
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

void convert_data(vector<int16_t>& data, string& transformed_data) {
    int N = data.size();
    char separator = ' ';
    for (int i = 0; i < N; i = i+2) {
        transformed_data = transformed_data + to_string(data[i]) + separator;
    }
}





int main() {
    constexpr char riff_id[4] = { 'R', 'I', 'F', 'F' };
    constexpr char format[4] = { 'W', 'A', 'V', 'E' };
    constexpr char fmt_id[4] = { 'f', 'm', 't', ' ' };
    constexpr char data_id[4] = { 'd', 'a', 't', 'a' };
    string filename = "sample3s.wav";
    ifstream ifs{ filename, ios_base::binary };
    if (!ifs) {
        cerr << "Cannot open file" << endl;
        return -1;
    }
    RIFFHeader h;
    ifs.read((char*)(&h), sizeof(h));
    if (!ifs || memcmp(h.chunk_id, riff_id, 4) || memcmp(h.format, format, 4)) {
        cerr << "Bad formatting" << endl;
        return -1;
    }
    ChunkInfo ch;
    bool fmt_read = false;
    bool data_read = false;
    FmtChunk fmt; 
    while (ifs.read((char*)(&ch), sizeof(ch))) {
        if (memcmp(ch.chunk_id, fmt_id, 4) == 0) {
            ifs.read((char*)(&fmt), ch.chunk_size);
            fmt_read = true;
        }
        else if (memcmp(ch.chunk_id, data_id, 4) == 0) {
            DataChunk dat_chunk(ch.chunk_size / sizeof(int16_t));
            ifs.read((char*)dat_chunk.data.data(), ch.chunk_size); // Use vector's data() method
            data_read = true;
           
            cout << "First 10 samples: ";
            for (int i = 0; i < min(10, dat_chunk.nb_of_samples); ++i) {
                cout << dat_chunk.data[i] << " ";
            }
            cout << endl;

            string line = "";
            cout << filename << endl;
            convert_data(dat_chunk.data, line);
            cout << line.substr(0, 50) << endl;
            
            clock_t start = clock();
            vector<code_word> result = LZ77(line);
            clock_t end = clock();
            cout << "Time:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
            cout << "Result size " << result.size() << endl;
            string binaryResult = vector_code_word_to_binary(result);
            cout << "Binary size " << binaryResult.size() << endl;
        }
        // otherwise skip the chunk
        else {
            ifs.seekg(ch.chunk_size, ios_base::cur);
        }
    }
    ifs.close();
    if (!data_read || !fmt_read) {
        cout << "Problem when reading data" << endl;
        return -1;
    }
    return 0;
}
