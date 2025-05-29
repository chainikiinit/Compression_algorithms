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

using namespace std;

const int FRAME_SIZE = 1152; // Размер фрейма MP3
const double PI = 3.14159265358979323846;

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

// Код Хаффмана
// A Tree node
struct Node {
    char ch;
    int freq;
    Node* left, * right;
};

// Function to allocate a new tree node
Node* getNode(char ch, int freq, Node* left, Node* right)
{
    Node* node = new Node();
    node->ch = ch;
    node->freq = freq;
    node->left = left;
    node->right = right;
    return node;
}

// Comparison object to be used to order the heap
struct comp {
    bool operator()(Node* l, Node* r) {
        return l->freq > r->freq;
    }
};

void encode(Node* root, string str, unordered_map<char, string>& huffmanCode) {
    if (root == nullptr)
        return;
    if (!root->left && !root->right) {
        huffmanCode[root->ch] = str;
    }
    encode(root->left, str + "0", huffmanCode);
    encode(root->right, str + "1", huffmanCode);
}

// Builds Huffman Tree and decode given input text
string buildHuffmanTree(string text) {
    unordered_map<char, int> freq;
    for (char ch : text) {
        freq[ch]++;
    }

    priority_queue<Node*, vector<Node*>, comp> pq;
    for (auto pair : freq) {
        pq.push(getNode(pair.first, pair.second, nullptr, nullptr));
    }
    while (pq.size() != 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top();	pq.pop();
        int sum = left->freq + right->freq;
        pq.push(getNode('\0', sum, left, right));
    }
    Node* root = pq.top();

    unordered_map<char, string> huffmanCode;
    encode(root, "", huffmanCode);

    //cout << "Huffman Codes are :\n" << '\n';
    //for (auto pair: huffmanCode) {
    //	cout << pair.first << " " << pair.second << '\n';
    //}

    //cout << "\nOriginal string was :\n" << text << '\n';

    string str = "";
    for (char ch : text) {
        str += huffmanCode[ch];
    }
    //cout << str << endl;
    return str;
}




// Дискретное Косинусное Преобразование фрейма
vector<double> dct(const vector<double>& frame) {
    vector<double> dctResult(frame.size());
    for (int k = 0; k < frame.size(); ++k) {
        double sum = 0.0;
        for (int n = 0; n < frame.size(); ++n) {
            sum += frame[n] * cos(PI / frame.size() * (n + 0.5) * k);
        }
        if (k > 0){
            double norm_factor = sqrt(2) / sqrt(frame.size());
            dctResult[k] = sum * norm_factor;
        }
        else {
            double norm_factor = 1 / sqrt(frame.size());
            dctResult[k] = sum * norm_factor;
        } 
    }
    return dctResult;
}

// Психоакустическая модель, часть 1 (Удаление частот, выше 16000 Гц)
vector<double> frequency_masking(const vector<double>& dctResult, int sample_rate) {
    int index = round((16000 * dctResult.size()) / sample_rate);
    vector<double> frequencies = {};
    for (int i = 0; i < dctResult.size(); ++i) {
        if (i > index) {
            frequencies.push_back(0);
        }
        else {
            frequencies.push_back(dctResult[i]);
        }
    }
    return frequencies;
}

// Квантование
vector<int> quantize(const vector<double>& frame, double maxVal, double minVal,  int numLevels) {
    vector<int> quantizedValues(frame.size());
    double stepSize = (maxVal - minVal) / numLevels;

    for (int i = 0; i < frame.size(); ++i) {
        double normalizedValue = (frame[i] - minVal) / (maxVal - minVal);
        double scaledValue = normalizedValue * (numLevels - 1);
        int quantizedValue = static_cast<int>(round(scaledValue));
        quantizedValue = max(0, min(quantizedValue, numLevels - 1));
        quantizedValues[i] = quantizedValue;
    }
    return quantizedValues;
}

// Психоакустическая модель, часть 2 (Маскировка сигналов)

// Функция для идентификации тональных компонентов
vector<int> findTonalComponents(const vector<double>& dctResult, double threshold) {
    vector<int> tonalIndices;
    for (int i = 1; i < dctResult.size() - 1; ++i) {
        if (dctResult[i] > dctResult[i - 1] && dctResult[i] > dctResult[i + 1] && dctResult[i] > threshold) {
            tonalIndices.push_back(i);
        }
    }
    return tonalIndices;
}

const int NUM_BARK_BANDS = 20;      // Примерное количество полос Барка
const double TONAL_SIGMA = 5.0;     // Sigma для тональных маскеров
const double NOISE_SIGMA = 15.0;    // Sigma для шумовых маскеров
const double POWER_SUMMATION = 2.5; // Показатель для нелинейного суммирования

//Оценка энергии шума в диапазонах
vector<double> estimateNoiseEnergy(const vector<double>& dctResult, int numBarkBands) {
    int bandSize = dctResult.size() / numBarkBands;
    vector<double> noiseEnergy(numBarkBands, 0.0);

    for (int i = 0; i < numBarkBands; ++i) {
        double sum = 0.0;
        for (int j = i * bandSize; j < (i + 1) * bandSize && j < dctResult.size(); ++j) {
            sum += dctResult[j] * dctResult[j]; // Энергия = amplitude^2
        }
        noiseEnergy[i] = sum / bandSize;
    }
    return noiseEnergy;
}

// Вычисление функции распространения
double spreadingFunction(double deltaF, double amplitude, double sigma) {
    return amplitude * exp(-(deltaF * deltaF) / (2 * sigma * sigma));
}

// Вычисление глобального порога маскировки
vector<double> calculateGlobalMaskingThreshold(const vector<double>& dctResult,
    const vector<int>& tonalIndices) {
    vector<double> tonalMaskingThreshold(dctResult.size(), 0.0);
    vector<double> noiseMaskingThreshold(dctResult.size(), 0.0);

    // Вклад от тональных маскеров
    for (int tonalIndex : tonalIndices) {
        double amplitude = dctResult[tonalIndex];
        for (int i = 0; i < dctResult.size(); ++i) {
            double deltaF = abs(i - tonalIndex);
            tonalMaskingThreshold[i] += spreadingFunction(deltaF, amplitude, TONAL_SIGMA);
        }
    }

    // Оценка энергии шума
    vector<double> noiseEnergy = estimateNoiseEnergy(dctResult, NUM_BARK_BANDS);

    // Вклад от шумовых маскеров (применяем энергию шума ко всему диапазону)
    int bandSize = dctResult.size() / NUM_BARK_BANDS;
    for (int i = 0; i < NUM_BARK_BANDS; ++i) {
        for (int j = i * bandSize; j < (i + 1) * bandSize && j < dctResult.size(); ++j) {
            noiseMaskingThreshold[j] += noiseEnergy[i]; 
        }
    }

    // Нелинейное суммирование
    vector<double> totalMaskingThreshold(dctResult.size());
    for (size_t i = 0; i < dctResult.size(); ++i) {
        totalMaskingThreshold[i] = pow(pow(tonalMaskingThreshold[i], POWER_SUMMATION) + pow(noiseMaskingThreshold[i], POWER_SUMMATION), 1.0 / POWER_SUMMATION);
    }
    return totalMaskingThreshold;
}


void Huffman(string& result, vector<int>& frame) {
    string line = "";
    for (int i = 0; i < frame.size(); ++i) {
        line = line + to_string(frame[i]);
    }
    result = buildHuffmanTree(line);
    
}




int main() {
    constexpr char riff_id[4] = { 'R', 'I', 'F', 'F' };
    constexpr char format[4] = { 'W', 'A', 'V', 'E' };
    constexpr char fmt_id[4] = { 'f', 'm', 't', ' ' };
    constexpr char data_id[4] = { 'd', 'a', 't', 'a' };

    ifstream ifs{ "sample6s.wav", ios_base::binary };
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
    cout << "Chank id: " << h.chunk_id << endl;
    cout << "Chank size: " << h.chunk_size << endl;
    cout << "Format: " << h.format << endl;

    // read chunk infos iteratively
    ChunkInfo ch;
    cout << "Chank id: " << ch.chunk_id << endl;
    cout << "Chank size: " << ch.chunk_size << endl;
    bool fmt_read = false;
    bool data_read = false;
    FmtChunk fmt;  // Declare fmt here to keep it in scope

    while (ifs.read((char*)(&ch), sizeof(ch))) {

        // if fmt chunk?
        if (memcmp(ch.chunk_id, fmt_id, 4) == 0) {
            ifs.read((char*)(&fmt), ch.chunk_size);
            fmt_read = true;
            cout << "Audio format: " << fmt.audio_format << endl;
            cout << "Number of channels: " << fmt.num_channels << endl;
            cout << "Sample rate: " << fmt.sample_rate << endl;
            cout << "Bytrate: " << fmt.byte_rate << endl;
            cout << "Block align: " << fmt.block_align << endl;
            cout << "Bits per sample: " << fmt.bits_per_sample << endl;
            
            

        }
        // is data chunk?
        else if (memcmp(ch.chunk_id, data_id, 4) == 0) {
            DataChunk dat_chunk(ch.chunk_size / sizeof(int16_t));
            ifs.read((char*)dat_chunk.data.data(), ch.chunk_size); // Use vector's data() method
            data_read = true;

            cout << "Number of samples: " << dat_chunk.nb_of_samples << endl;

            // Example: print the first 10 samples
            cout << distance(dat_chunk.data.begin(), max_element(dat_chunk.data.begin(), dat_chunk.data.end())) << endl;
            cout << "First 10 samples: ";

            for (int i = 0; i < min(10, dat_chunk.nb_of_samples); ++i) {
                cout << dat_chunk.data[i] << " ";
            }
            cout << endl;
            
            string result = "";
            // Сжатие данных
            for (int i = 0; i < dat_chunk.nb_of_samples; i += 2 * FRAME_SIZE) {
                //vector<double> frame(dat_chunk.data.begin() + i, dat_chunk.data.begin() + min(i + FRAME_SIZE, dat_chunk.nb_of_samples));

                vector<double> frame = {};
                for (int j = i; j < min(i + 2 * FRAME_SIZE, dat_chunk.nb_of_samples); j = j + 2) {
                    frame.push_back(dat_chunk.data[j]);
                }
                
                if (frame.size() < FRAME_SIZE) {
                    while (frame.size() < FRAME_SIZE) {
                        frame.push_back(0);
                    }
                }

                vector<double> dctResult = dct(frame);
                dctResult[0] = 0;
                
                // Маскировка
                vector<double> frequencies = frequency_masking(dctResult, 44100);

                /*
                // 1. Ищем тональные компоненты
                double tonalThreshold = 100.0;
                vector<int> tonalIndices = findTonalComponents(dctResult, tonalThreshold);

                // 2. Вычисляем глобальный порог маскировки
                vector<double> maskingThreshold = calculateGlobalMaskingThreshold(dctResult, tonalIndices);



                for (int j = 0; j < 600; ++j) {
                    cout << maskingThreshold[j] << ", ";
                }
                cout << endl;
                */

                // Квантование
                int nuber_levels = 256; //65536
                vector<int> quantizedValues = quantize(frequencies, *max_element(frequencies.begin(), frequencies.end()), *min_element(frequencies.begin(), frequencies.end()), nuber_levels);

                string frame_result = "";
                Huffman(frame_result, quantizedValues);
                result = result + frame_result + " ";
                cout << "Frame finished" << i / FRAME_SIZE << endl;
            }
            cout << result.size() << endl;
            for (int i = 0; i < 100; ++i) {
                cout << result[i] << " ";
            }
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