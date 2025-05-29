#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp> 
#include <tuple>
#include <cmath>
#include <sstream>
#include <bitset>
#include <stdio.h> 
#include <time.h>

using namespace std;

void convertRGBtoYCbCr2(const string& filename, vector<vector<double>>& Y, vector<vector<double>>& Cr, vector<vector<double>>& Cb) {
    cv::Mat image = cv::imread(filename);
    if (image.empty()) {
        cout << "Where is my picture?\n";
        return;
    }
    else {
        int height = image.rows;
        int width = image.cols;

        Y.resize(width, vector<double>(height)); // width * height
        Cb.resize(width, vector<double>(height));
        Cr.resize(width, vector<double>(height));
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);
                double B = pixel[0];
                double G = pixel[1];
                double R = pixel[2];

                Y[j][i] = 0.299 * R + 0.587 * G + 0.114 * B;
                Cb[j][i] = -0.1687 * R - 0.3313 * G + 0.5 * B + 128;
                Cr[j][i] = 0.5 * R - 0.4187 * G - 0.0813 * B + 128;
            }
        }
    }
}

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

void convert_component2(vector<vector<double>>& component, string& transformed_component) {
    int N = component.size();
    char separator = ' ';
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < component[i].size(); ++j) {
            transformed_component = transformed_component + separator + to_string(component[i][j]);
        }
    }
}







int main() {
    string filename = "image_1024_145kb.jpg"; // PNG файл
    vector<vector<double>> Y;
    vector<vector<double>> Cr;
    vector<vector<double>> Cb;
    cout << "0" << endl;
    clock_t start = clock();
    convertRGBtoYCbCr2(filename, Y, Cr, Cb);
    cout << "1" << endl;

    string converted_component_Y;
    string converted_component_Cr;
    string converted_component_Cb;

    convert_component2(Y, converted_component_Y);
    convert_component2(Cr, converted_component_Cr);
    convert_component2(Cb, converted_component_Cb);
    cout << "2" << endl;
    string components = converted_component_Y + converted_component_Cr + converted_component_Cb;
    vector<code_word> result = LZ77(components);
    clock_t end = clock();
    cout << filename << endl;
    cout << "Time:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
    cout << "Result size " << result.size() << endl;
    /*
    for (int i = 0; i < result.size(); ++i) {
        cout << result[i].Shift << " " << result[i].Max_Len << " " << result[i].Symbol << endl;
    }*/
    cout << "3" << endl;
    string binaryResult = vector_code_word_to_binary(result);
    //cout << binaryResult << endl;
    cout << binaryResult.size() << endl;
    cout << "Binary size " << binaryResult.size() << endl;

    return 0;
}