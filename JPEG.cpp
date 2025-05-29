#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp> 
#include <tuple>
#include <cmath>
#include <stdio.h> 
#include <time.h>

using namespace std;

const double PI = acos(-1.0);

void convertRGBtoYCbCr(const string& filename, vector<vector<double>>& Y, vector<vector<double>>& Cr, vector<vector<double>>& Cb) {
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


void averaging_values(vector<vector<double>>& Y, vector<vector<double>>& Cr, vector<vector<double>>& Cb) {
    int width = Cr.size();
    int height = Cr[0].size();

    for (int i = 0; i < height; i = i + 2) {
        for (int j = 0; j < width; j = j + 2) {
            double cb = (Cb[j][i] + Cb[j + 1][i] + Cb[j][i + 1] + Cb[j + 1][i + 1]) / 4;
            double cr = (Cr[j][i] + Cr[j + 1][i] + Cr[j][i + 1] + Cr[j + 1][i + 1]) / 4;
            Y[j][i] = Y[j][i] - 128;
            Cb[j][i] = Cb[j+1][i] = Cb[j][i+1] = Cb[j+1][i+1] = cb - 128;
            Cr[j][i] = Cr[j + 1][i] = Cr[j][i + 1] = Cr[j + 1][i + 1] = cr - 128;
        }
    }
}


vector<int> zigzag_entry(vector<vector<double>>& block) {
    vector<int> result;
    // Оставляем только 10 наиболее значимых элементов 
    result.push_back(block[0][0]); result.push_back(block[0][1]); result.push_back(block[1][0]);
    result.push_back(block[0][2]); result.push_back(block[1][1]); result.push_back(block[2][0]);
    result.push_back(block[0][3]); result.push_back(block[1][2]); result.push_back(block[2][1]);
    result.push_back(block[3][0]);

    for (int i = 0; i < 54; ++i) {
        result.push_back(0);
    }
    return result;
}

/*
vector<int> zigzag_entry(vector<vector<double>>& block) {
    int rows = block.size();
    int cols = block[0].size();
    vector<int> result;

    for (int i = 0; i < rows + cols - 1; ++i) {
        int start_col = (i < cols) ? i : cols - 1;
        int start_row = (i < cols) ? 0 : i - cols + 1;

        while (start_col >= 0 && start_row < rows) {
            result.push_back(block[start_row][start_col]);
            start_col--;
            start_row++;
        }
    }
    return result;
}*/


vector<int> quantization(int flag, vector<vector<double>>& block) {
    int width = block.size();
    int height = block[0].size();

    vector<vector<int>> quantization_matrix(width, vector<int>(height));
    vector<vector<float>> result(width, vector<float>(height));

    if (flag == 1) {
        vector<vector<int>> quantization_matrix = {
        {16, 11, 10, 16, 24, 40, 51, 61},
        {12, 12, 14, 19, 26, 58, 60, 55},
        {14, 13, 16, 24, 40, 57, 69, 56},
        {14, 17, 22, 29, 51, 87, 80, 62},
        {18, 22, 37, 56, 68, 109, 103, 77},
        {24, 35, 55, 64, 81, 104, 113, 92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103, 99}
        };
    }
    else {
        vector<vector<int>> quantization_matrix = {
        {17, 18, 24, 47, 99, 99, 99, 99},
        {18, 21, 26, 66, 99, 99, 99, 99},
        {24, 26, 56, 99, 99, 99, 99, 99},
        {47, 66, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99}
        };
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            result[j][i] = (round(block[j][i] / (double)quantization_matrix[j][i]));
        }
    }

    vector<int> line = zigzag_entry(block);

    return line;
}


double C(int& N, int& x, int& y) {
    if (x == 0) {
        return sqrt(1.0 / N);
    }
    else {
        return sqrt(2.0 / N) * cos((2 * y + 1) * x * PI / (2 * N));
    }
}

vector<vector<double>> matrixCP(vector<vector<double>>& block) { // Упрощенное ДКП для блока 8*8
    int width = block.size();
    int height = block[0].size();

    vector<vector<double>> coefficient(width, vector<double>(height));
    vector<vector<double>> transposed_coefficient(width, vector<double>(height));
    vector<vector<double>> matrix_coefficient(width, vector<double>(height));
    vector<vector<double>> matrix_coefficient_final(width, vector<double>(height));

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            coefficient[j][i] = C(width, j, i);
            transposed_coefficient[i][j] = C(width, j, i);
        }
    }

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < width; ++j) {
            matrix_coefficient[i][j] = 0;
            for (int k = 0; k < width; k++)
                matrix_coefficient[i][j] += coefficient[i][k] * block[k][j];
        }
    }

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < width; ++j) {
            matrix_coefficient_final[i][j] = 0;
            for (int k = 0; k < width; k++)
                matrix_coefficient_final[i][j] += matrix_coefficient[i][k] * transposed_coefficient[k][j];
        }
    }
    return matrix_coefficient_final;
}


void cosine_transform(vector<vector<double>>& Y, vector<vector<double>>& Cr, vector<vector<double>>& Cb, 
    vector<int>& converted_component_Y, vector<int>& converted_component_Cr, vector<int>& converted_component_Cb) {
    int width = Cr.size();
    int height = Cr[0].size();
    int N = 8;

    for (int i = 0; i < height; i = i + N) {
        for (int j = 0; j < width; j = j + N) {

            vector<vector<double>> coefficient_Y(N, vector<double>(N));
            vector<vector<double>> coefficient_Cr(N, vector<double>(N));
            vector<vector<double>> coefficient_Cb(N, vector<double>(N));

            for (int k = i; k < i + N; ++k) {
                for (int l = j; l < j + N; ++l) {
                    int pixel_x = l + l - j;  //j + l
                    int pixel_y = k + k - i; //i+k 

                    if (pixel_x < width && pixel_y < height) { // Check boundary condition
                        coefficient_Y[l - j][k - i] = Y[pixel_y][pixel_x];
                        coefficient_Cr[l - j][k - i] = Cr[pixel_y][pixel_x];
                        coefficient_Cb[l - j][k - i] = Cb[pixel_y][pixel_x];
                    }
                    /*else {
                        coefficient_Y[l-j][k-i] = 0;  
                        coefficient_Cr[l-j][k-i] = 128; 
                        coefficient_Cb[l-j][k-i] = 128;
                    }*/

                }
            }
  
            vector<vector<double>> DCT_Y = matrixCP(coefficient_Y);
            vector<vector<double>> DCT_Cr = matrixCP(coefficient_Cr);
            vector<vector<double>> DCT_Cb = matrixCP(coefficient_Cb);

            vector<int> block_Y = quantization(1, DCT_Y);
            vector<int> block_Cr = quantization(2, DCT_Cr);
            vector<int> block_Cb = quantization(2, DCT_Cb);

            for (int k = 0; k < block_Y.size(); ++k) {
                converted_component_Y.push_back(block_Y[k]);
                converted_component_Cr.push_back(block_Cr[k]);
                converted_component_Cb.push_back(block_Cb[k]);
            }
            block_Y.clear();
            block_Cr.clear();
            block_Cb.clear();
        }
    }
}


void RLE(string& result, vector<int>& component) {
    string line = "";
    for (int i = 0; i < component.size(); ++i) {
        line = line + to_string(component[i]);
    }

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


string string_to_ascii_binary(const string& inputString) {
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
    string filename = "image_256_14kb.jpg"; // PNG файл
    vector<vector<double>> Y;
    vector<vector<double>> Cr;
    vector<vector<double>> Cb;
    clock_t start = clock();
    // Получаем компоненты YCbCr
    convertRGBtoYCbCr(filename, Y, Cr, Cb);
    cout << "0" << endl;
    averaging_values(Y, Cr, Cb);
    cout << "1" << endl;
    vector<int> converted_component_Y;
    vector<int> converted_component_Cr;
    vector<int> converted_component_Cb;

    cosine_transform(Y, Cr, Cb, converted_component_Y, converted_component_Cr, converted_component_Cb);
    cout << "2" << endl;
    string result = "";
    clock_t end = clock();
    cout << filename << endl;
    cout << "Time:" << (double)(end - start) / CLOCKS_PER_SEC << endl;

    RLE(result, converted_component_Y);
    RLE(result, converted_component_Cr);
    RLE(result, converted_component_Cb);
    cout << "Result size " << result.size() << endl;

    cout << "3" << endl;
    string ascii_result = string_to_ascii_binary(result);
    cout << "Binary size " << ascii_result.size() << endl;
    return 0;
}