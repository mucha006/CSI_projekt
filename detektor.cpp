#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

// Obecná funkce pro aplikaci konvoluce s daným jádrem
Mat applyConvolution(const Mat& input, const vector<vector<int>>& kernel) {
    int kernelSize = kernel.size(); // Velikost jádra 
    int offset = kernelSize / 2; // Posun pro procházení okrajù obrázku
    Mat output = Mat::zeros(input.size(), CV_32F); // Výstupní matice s nulami

    // Procházení obrázku (vynechání okrajù kvùli jádru)
    // Okraje jsou vynechány, protože jádro nemùže být aplikováno na pixely na okrajích obrázku
    for (int y = offset; y < input.rows - offset; y++) {
        for (int x = offset; x < input.cols - offset; x++) {
            float sum = 0.0f; // Inicializace souètu pro konvoluci

            // Procházení jádra
            // Jádro se aplikuje na okolní pixely kolem aktuálního pixelu (x, y)
            for (int ky = -offset; ky <= offset; ky++) {
                for (int kx = -offset; kx <= offset; kx++) {
                    int pixelValue = input.at<uchar>(y + ky, x + kx); // Hodnota pixelu
                    sum += pixelValue * kernel[ky + offset][kx + offset]; // Aplikace jádra
                }
            }
            output.at<float>(y, x) = sum; // Uložení výsledku konvoluce
        }
    }
    return output; // Vrácení výsledné matice
}

// Funkce pro aplikaci Sobelova operátoru (vertikální nebo horizontální)
void applySobel(const Mat& img, Mat& output, bool horizontal = false) {
    // Sobelovo jádro pro detekci vertikálních hran
    // Toto jádro zvýrazòuje vertikální zmìny v intenzitì pixelù
    vector<vector<int>> sobelX = {
        {-1,  0,  1},
        {-2,  0,  2},
        {-1,  0,  1}
    };

    // Sobelovo jádro pro detekci horizontálních hran
    // Toto jádro zvýrazòuje horizontální zmìny v intenzitì pixelù
    vector<vector<int>> sobelY = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    // Aplikace konvoluce pro vertikální a horizontální gradienty
    Mat gradX = applyConvolution(img, sobelX);
    Mat gradY = applyConvolution(img, sobelY);

    // Výpoèet gradientu (vertikální nebo horizontální)
    output = Mat::zeros(img.size(), CV_32F);
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            float Gx = gradX.at<float>(y, x); // Vertikální gradient
            float Gy = gradY.at<float>(y, x); // Horizontální gradient
            output.at<float>(y, x) = horizontal ? abs(Gx) : abs(Gy); // Volba horizontální/vertikální detekce
        }
    }
}

// Funkce pro aplikaci Prewittova operátoru
void applyPrewitt(const Mat& img, Mat& output) {
    // Prewittovo jádro pro detekci vertikálních hran
    // Toto jádro je podobné Sobelovu, ale má menší váhy
    vector<vector<int>> prewittX = {
        {-1,  0,  1},
        {-1,  0,  1},
        {-1,  0,  1}
    };

    // Prewittovo jádro pro detekci horizontálních hran
    vector<vector<int>> prewittY = {
        {-1, -1, -1},
        { 0,  0,  0},
        { 1,  1,  1}
    };

    // Aplikace konvoluce pro vertikální a horizontální gradienty
    Mat gradX = applyConvolution(img, prewittX);
    Mat gradY = applyConvolution(img, prewittY);

    // Výpoèet velikosti gradientu
    // Velikost gradientu je kombinací vertikálního a horizontálního gradientu
    output = Mat::zeros(img.size(), CV_32F);
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            float Gx = gradX.at<float>(y, x); // Vertikální gradient
            float Gy = gradY.at<float>(y, x); // Horizontální gradient
            output.at<float>(y, x) = sqrt(Gx * Gx + Gy * Gy); // Výpoèet celkového gradientu
        }
    }
}

// Funkce pro aplikaci Laplaceova operátoru s nastavitelnou velikostí jádra
void applyLaplacian(const Mat& img, Mat& output, int kernelSize = 3) {
    // Vytvoøení Laplaceova jádra podle zadané velikosti
    // Laplaceùv operátor detekuje hrany pomocí druhé derivace
    Mat kernel = Mat::zeros(kernelSize, kernelSize, CV_32F);
    int center = kernelSize / 2; // Støed jádra

    // Naplnìní jádra hodnotami
    // Støed jádra má vysokou hodnotu, okolní hodnoty jsou záporné
    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {
            if (i == center && j == center) {
                kernel.at<float>(i, j) = kernelSize * kernelSize - 1; // Støed jádra
            }
            else {
                kernel.at<float>(i, j) = -1; // Okolní hodnoty
            }
        }
    }

    // Aplikace konvoluce
    output = Mat::zeros(img.size(), CV_32F);
    int offset = kernelSize / 2;

    // Procházení obrázku (vynechání okrajù kvùli jádru)
    for (int y = offset; y < img.rows - offset; y++) {
        for (int x = offset; x < img.cols - offset; x++) {
            float sum = 0.0f; // Inicializace souètu pro konvoluci

            // Procházení jádra
            for (int ky = -offset; ky <= offset; ky++) {
                for (int kx = -offset; kx <= offset; kx++) {
                    int pixelValue = img.at<uchar>(y + ky, x + kx); // Hodnota pixelu
                    sum += pixelValue * kernel.at<float>(ky + offset, kx + offset); // Aplikace jádra
                }
            }
            output.at<float>(y, x) = sum; // Uložení výsledku konvoluce
        }
    }
}

// Funkce pro aplikaci Scharr operátoru
void applyScharr(const Mat& img, Mat& output) {
    // Scharrovo jádro pro detekci vertikálních hran
    // Scharrùv operátor je podobný Sobelovu, ale poskytuje lepší pøesnost
    vector<vector<int>> scharrX = {
        {-3,  0,  3},
        {-10, 0, 10},
        {-3,  0,  3}
    };

    // Scharrovo jádro pro detekci horizontálních hran
    vector<vector<int>> scharrY = {
        {-3, -10, -3},
        { 0,   0,  0},
        { 3,  10,  3}
    };

    // Aplikace konvoluce pro vertikální a horizontální gradienty
    Mat gradX = applyConvolution(img, scharrX);
    Mat gradY = applyConvolution(img, scharrY);

    // Výpoèet velikosti gradientu
    output = Mat::zeros(img.size(), CV_32F);
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            float Gx = gradX.at<float>(y, x); // Vertikální gradient
            float Gy = gradY.at<float>(y, x); // Horizontální gradient
            output.at<float>(y, x) = sqrt(Gx * Gx + Gy * Gy); // Výpoèet celkového gradientu
        }
    }
}

// Funkce pro aplikaci Kirsch operátoru
void applyKirsch(const Mat& img, Mat& output) {
    // 8 smìrových jader pro Kirschùv operátor
    // Kirschùv operátor detekuje hrany v 8 rùzných smìrech
    vector<Mat> kernels = {
        (Mat_<float>(3, 3) << 5,  5,  5, -3,  0, -3, -3, -3, -3),
        (Mat_<float>(3, 3) << 5,  5, -3,  5,  0, -3, -3, -3, -3),
        (Mat_<float>(3, 3) << 5, -3, -3,  5,  0, -3,  5, -3, -3),
        (Mat_<float>(3, 3) << -3, -3, -3,  5,  0, -3,  5,  5, -3),
        (Mat_<float>(3, 3) << -3, -3, -3, -3,  0, -3,  5,  5,  5),
        (Mat_<float>(3, 3) << -3, -3,  5, -3,  0,  5, -3, -3,  5),
        (Mat_<float>(3, 3) << -3,  5, -3, -3,  0,  5, -3, -3, -3),
        (Mat_<float>(3, 3) << -3, -3, -3, -3,  0,  5, -3, -3,  5)
    };

    output = Mat::zeros(img.size(), CV_32F);

    // Aplikace každého filtru a porovnání
    for (int y = 1; y < img.rows - 1; ++y) {
        for (int x = 1; x < img.cols - 1; ++x) {
            float maxVal = -FLT_MAX; // Inicializace maximální hodnoty
            for (const auto& kernel : kernels) {
                float response = 0; // Inicializace odpovìdi jádra
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        response += kernel.at<float>(ky + 1, kx + 1) * img.at<uchar>(y + ky, x + kx); // Aplikace jádra
                    }
                }
                maxVal = max(maxVal, abs(response)); // Uložení maximální odpovìdi
            }
            output.at<float>(y, x) = maxVal; // Uložení výsledku
        }
    }
}

// Funkce pro prahování
Mat applyThreshold(const Mat& input, float threshold) {
    Mat output = Mat::zeros(input.size(), CV_8U); // Výstupní binární obrázek
    for (int y = 0; y < input.rows; y++) {
        for (int x = 0; x < input.cols; x++) {
            float value = input.at<float>(y, x); // Hodnota pixelu
            output.at<uchar>(y, x) = (value > threshold) ? 255 : 0; // Prahování
        }
    }
    return output; // Vrácení binárního obrázku
}

int main() {
    // Naètení obrázku v odstínech šedi
    //Mat img = imread("inputs/image.jpg", IMREAD_GRAYSCALE); //obrazek z: https://en.wikipedia.org/wiki/Sobel_operator#/media/File:Bikesgray.jpg
    Mat img = imread("inputs/image.png", IMREAD_GRAYSCALE); //obrazek z: https://en.wikipedia.org/wiki/Sobel_operator#/media/File:Valve_original_(1).PNG
    //Mat img = imread("inputs/image2.png", IMREAD_GRAYSCALE);

    if (img.empty()) {
        cout << "Chyba: Nelze nacist obrazek" << endl;
        return -1;
    }

    // Aplikace rùzných filtrù
    Mat sobelVertical, sobelHorizontal, prewittEdges, laplacianEdges, scharrEdges, kirschEdges;

    // Sobel - vertikální a horizontální hrany
    applySobel(img, sobelVertical, false); 
    applySobel(img, sobelHorizontal, true); 

    // Prewitt
    applyPrewitt(img, prewittEdges);

    // Laplace - rùzné nastavení velikosti jádra
    int kernelSize = 3; // Velikost jádra 
    applyLaplacian(img, laplacianEdges, kernelSize);

    // Scharr
    applyScharr(img, scharrEdges);

    // Kirsch
    applyKirsch(img, kirschEdges);

    // Prahování
    Mat sobelVerticalThresholded = applyThreshold(sobelVertical, 100); // Každý pixel v obraze se porovná s danou prahovou hodnotou jestli je nad nebo pod ní
    Mat sobelHorizontalThresholded = applyThreshold(sobelHorizontal, 100);
    Mat prewittThresholded = applyThreshold(prewittEdges, 100);
    Mat laplacianThresholded = applyThreshold(laplacianEdges, 100);
    Mat scharrThresholded = applyThreshold(scharrEdges, 100);
    Mat kirschThresholded = applyThreshold(kirschEdges, 100);

    // Zobrazení výsledkù
    imshow("Puvodni obrazek", img);
    imshow("Sobel - Vertikalni hrany", sobelVerticalThresholded);
    imshow("Sobel - Horizontalni hrany", sobelHorizontalThresholded);
    imshow("Prewitt", prewittThresholded);
    imshow("Laplacian - Jadro " + std::to_string(kernelSize) + "x" + std::to_string(kernelSize), laplacianThresholded);
    imshow("Scharr", scharrThresholded);
    imshow("Kirsch", kirschThresholded);

    //imwrite("sobel_horizontal.png", sobelHorizontal);

    waitKey(0);
    return 0;
}