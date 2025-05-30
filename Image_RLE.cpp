#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp> 
#include <tuple>
#include <cmath>
#include <stdio.h> 
#include <time.h>

using namespace std;

void convertRGBtoYCbCr1(const string& filename, vector<vector<double>>& Y, vector<vector<double>>& Cr, vector<vector<double>>& Cb) {
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

void convert_component(vector<vector<double>>& component, vector<double>& transformed_component) {
    int N = component.size();
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < component[i].size(); ++j) {
            transformed_component.push_back(component[i][j]);
        }
    }
}

void RLE1(string& result, vector<double>& component) {
    string line = "";
    for (int i = 0; i < component.size(); ++i) {
        line = line + to_string(component[i]);
    }

    char separator = ',';
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


string string_to_ascii_binary1(const string& inputString) {
    stringstream binaryStream;
    for (char c : inputString) {
        int asciiCode = (int)c;
        for (int i = 7; i >= 0; --i) {
            binaryStream << ((asciiCode >> i) & 1);
        }
    }
    return binaryStream.str();
}


int main() {
    string filename = "image_256_14kb.jpg"; // PNG ôàéë

    vector<vector<double>> Y;
    vector<vector<double>> Cr;
    vector<vector<double>> Cb;

    // Ïîëó÷àåì êîìïîíåíòû YCbCr
    clock_t start = clock();
    convertRGBtoYCbCr1(filename, Y, Cr, Cb);
    cout << "0" << endl;
    int N = Y.size();
    vector<double> converted_component_Y;
    vector<double> converted_component_Cr;
    vector<double> converted_component_Cb;
    
    convert_component(Y, converted_component_Y);
    convert_component(Cr, converted_component_Cr);
    convert_component(Cb, converted_component_Cb);
    cout << "1" << endl;
    string result_Y = "";
    string result_Cr = "";
    string result_Cb = "";

    RLE1(result_Y, converted_component_Y);
    cout << "2" << endl;
    RLE1(result_Cr, converted_component_Cr);
    cout << "3" << endl;
    RLE1(result_Cb, converted_component_Cb);
    clock_t end = clock();
    cout << filename << endl;
    cout << "Time:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
    cout << "Result size " << result_Y.size() << " " << result_Cr.size() << " " << result_Cb.size() << endl;
    string ascii_result_Y = string_to_ascii_binary1(result_Y);
    string ascii_result_Cr = string_to_ascii_binary1(result_Cr);
    string ascii_result_Cb = string_to_ascii_binary1(result_Cb);
    cout << "Binary size " << ascii_result_Y.size() << " " << ascii_result_Cr.size() << " " << ascii_result_Cb.size() << " " << endl;
    return 0;
}
