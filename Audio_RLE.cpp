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
#include <stdio.h> 
#include <time.h>
#include <sstream>
#include <bitset>

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
    vector<int16_t> data; // Use vector instead of raw pointer
    int nb_of_samples;

    DataChunk(int s) : nb_of_samples{ s } {
        data.resize(s);
    }
};


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


void convert_data1(vector<int16_t>& data, string& transformed_data) {
    int N = data.size();
    char separator = ' ';
    for (int i = 0; i < N; i = i+2) {
        transformed_data = transformed_data + to_string(data[i])  + separator ;
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

    // first read RIFF header
    RIFFHeader h;
    ifs.read((char*)(&h), sizeof(h));
    if (!ifs || memcmp(h.chunk_id, riff_id, 4) || memcmp(h.format, format, 4)) {
        cerr << "Bad formatting" << endl;
        return -1;
    }

    // read chunk infos iteratively
    ChunkInfo ch;

    bool fmt_read = false;
    bool data_read = false;
    FmtChunk fmt;  // Declare fmt here to keep it in scope

    while (ifs.read((char*)(&ch), sizeof(ch))) {

        // if fmt chunk?
        if (memcmp(ch.chunk_id, fmt_id, 4) == 0) {
            ifs.read((char*)(&fmt), ch.chunk_size);
            fmt_read = true;

        }
        // is data chunk?
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
            cout << "RLE" << endl;
            cout << filename << endl;
            convert_data1(dat_chunk.data, line);
            cout << line.substr(0, 50) << endl;

            string result = "";
            clock_t start = clock();
            RLE(result, line);
            clock_t end = clock();
            cout << "Time:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
            cout << "Result size " << result.size() << endl;
            string ascii_result = string_to_ascii_bunary(result);
            cout << "Binary size " << ascii_result.size() << endl;
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