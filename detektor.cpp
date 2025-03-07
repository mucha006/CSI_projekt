#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

// Obecn� funkce pro aplikaci konvoluce s dan�m j�drem
Mat applyConvolution(const Mat& input, const vector<vector<int>>& kernel) {
    int kernelSize = kernel.size(); // Velikost j�dra 
    int offset = kernelSize / 2; // Posun pro proch�zen� okraj� obr�zku
    Mat output = Mat::zeros(input.size(), CV_32F); // V�stupn� matice s nulami

    // Proch�zen� obr�zku (vynech�n� okraj� kv�li j�dru)
    // Okraje jsou vynech�ny, proto�e j�dro nem��e b�t aplikov�no na pixely na okraj�ch obr�zku
    for (int y = offset; y < input.rows - offset; y++) {
        for (int x = offset; x < input.cols - offset; x++) {
            float sum = 0.0f; // Inicializace sou�tu pro konvoluci

            // Proch�zen� j�dra
            // J�dro se aplikuje na okoln� pixely kolem aktu�ln�ho pixelu (x, y)
            for (int ky = -offset; ky <= offset; ky++) {
                for (int kx = -offset; kx <= offset; kx++) {
                    int pixelValue = input.at<uchar>(y + ky, x + kx); // Hodnota pixelu
                    sum += pixelValue * kernel[ky + offset][kx + offset]; // Aplikace j�dra
                }
            }
            output.at<float>(y, x) = sum; // Ulo�en� v�sledku konvoluce
        }
    }
    return output; // Vr�cen� v�sledn� matice
}

// Funkce pro aplikaci Sobelova oper�toru (vertik�ln� nebo horizont�ln�)
void applySobel(const Mat& img, Mat& output, bool horizontal = false) {
    // Sobelovo j�dro pro detekci vertik�ln�ch hran
    // Toto j�dro zv�raz�uje vertik�ln� zm�ny v intenzit� pixel�
    vector<vector<int>> sobelX = {
        {-1,  0,  1},
        {-2,  0,  2},
        {-1,  0,  1}
    };

    // Sobelovo j�dro pro detekci horizont�ln�ch hran
    // Toto j�dro zv�raz�uje horizont�ln� zm�ny v intenzit� pixel�
    vector<vector<int>> sobelY = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    // Aplikace konvoluce pro vertik�ln� a horizont�ln� gradienty
    Mat gradX = applyConvolution(img, sobelX);
    Mat gradY = applyConvolution(img, sobelY);

    // V�po�et gradientu (vertik�ln� nebo horizont�ln�)
    output = Mat::zeros(img.size(), CV_32F);
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            float Gx = gradX.at<float>(y, x); // Vertik�ln� gradient
            float Gy = gradY.at<float>(y, x); // Horizont�ln� gradient
            output.at<float>(y, x) = horizontal ? abs(Gx) : abs(Gy); // Volba horizont�ln�/vertik�ln� detekce
        }
    }
}

// Funkce pro aplikaci Prewittova oper�toru
void applyPrewitt(const Mat& img, Mat& output) {
    // Prewittovo j�dro pro detekci vertik�ln�ch hran
    // Toto j�dro je podobn� Sobelovu, ale m� men�� v�hy
    vector<vector<int>> prewittX = {
        {-1,  0,  1},
        {-1,  0,  1},
        {-1,  0,  1}
    };

    // Prewittovo j�dro pro detekci horizont�ln�ch hran
    vector<vector<int>> prewittY = {
        {-1, -1, -1},
        { 0,  0,  0},
        { 1,  1,  1}
    };

    // Aplikace konvoluce pro vertik�ln� a horizont�ln� gradienty
    Mat gradX = applyConvolution(img, prewittX);
    Mat gradY = applyConvolution(img, prewittY);

    // V�po�et velikosti gradientu
    // Velikost gradientu je kombinac� vertik�ln�ho a horizont�ln�ho gradientu
    output = Mat::zeros(img.size(), CV_32F);
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            float Gx = gradX.at<float>(y, x); // Vertik�ln� gradient
            float Gy = gradY.at<float>(y, x); // Horizont�ln� gradient
            output.at<float>(y, x) = sqrt(Gx * Gx + Gy * Gy); // V�po�et celkov�ho gradientu
        }
    }
}

// Funkce pro aplikaci Laplaceova oper�toru s nastavitelnou velikost� j�dra
void applyLaplacian(const Mat& img, Mat& output, int kernelSize = 3) {
    // Vytvo�en� Laplaceova j�dra podle zadan� velikosti
    // Laplace�v oper�tor detekuje hrany pomoc� druh� derivace
    Mat kernel = Mat::zeros(kernelSize, kernelSize, CV_32F);
    int center = kernelSize / 2; // St�ed j�dra

    // Napln�n� j�dra hodnotami
    // St�ed j�dra m� vysokou hodnotu, okoln� hodnoty jsou z�porn�
    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {
            if (i == center && j == center) {
                kernel.at<float>(i, j) = kernelSize * kernelSize - 1; // St�ed j�dra
            }
            else {
                kernel.at<float>(i, j) = -1; // Okoln� hodnoty
            }
        }
    }

    // Aplikace konvoluce
    output = Mat::zeros(img.size(), CV_32F);
    int offset = kernelSize / 2;

    // Proch�zen� obr�zku (vynech�n� okraj� kv�li j�dru)
    for (int y = offset; y < img.rows - offset; y++) {
        for (int x = offset; x < img.cols - offset; x++) {
            float sum = 0.0f; // Inicializace sou�tu pro konvoluci

            // Proch�zen� j�dra
            for (int ky = -offset; ky <= offset; ky++) {
                for (int kx = -offset; kx <= offset; kx++) {
                    int pixelValue = img.at<uchar>(y + ky, x + kx); // Hodnota pixelu
                    sum += pixelValue * kernel.at<float>(ky + offset, kx + offset); // Aplikace j�dra
                }
            }
            output.at<float>(y, x) = sum; // Ulo�en� v�sledku konvoluce
        }
    }
}

// Funkce pro aplikaci Scharr oper�toru
void applyScharr(const Mat& img, Mat& output) {
    // Scharrovo j�dro pro detekci vertik�ln�ch hran
    // Scharr�v oper�tor je podobn� Sobelovu, ale poskytuje lep�� p�esnost
    vector<vector<int>> scharrX = {
        {-3,  0,  3},
        {-10, 0, 10},
        {-3,  0,  3}
    };

    // Scharrovo j�dro pro detekci horizont�ln�ch hran
    vector<vector<int>> scharrY = {
        {-3, -10, -3},
        { 0,   0,  0},
        { 3,  10,  3}
    };

    // Aplikace konvoluce pro vertik�ln� a horizont�ln� gradienty
    Mat gradX = applyConvolution(img, scharrX);
    Mat gradY = applyConvolution(img, scharrY);

    // V�po�et velikosti gradientu
    output = Mat::zeros(img.size(), CV_32F);
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            float Gx = gradX.at<float>(y, x); // Vertik�ln� gradient
            float Gy = gradY.at<float>(y, x); // Horizont�ln� gradient
            output.at<float>(y, x) = sqrt(Gx * Gx + Gy * Gy); // V�po�et celkov�ho gradientu
        }
    }
}

// Funkce pro aplikaci Kirsch oper�toru
void applyKirsch(const Mat& img, Mat& output) {
    // 8 sm�rov�ch jader pro Kirsch�v oper�tor
    // Kirsch�v oper�tor detekuje hrany v 8 r�zn�ch sm�rech
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

    // Aplikace ka�d�ho filtru a porovn�n�
    for (int y = 1; y < img.rows - 1; ++y) {
        for (int x = 1; x < img.cols - 1; ++x) {
            float maxVal = -FLT_MAX; // Inicializace maxim�ln� hodnoty
            for (const auto& kernel : kernels) {
                float response = 0; // Inicializace odpov�di j�dra
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        response += kernel.at<float>(ky + 1, kx + 1) * img.at<uchar>(y + ky, x + kx); // Aplikace j�dra
                    }
                }
                maxVal = max(maxVal, abs(response)); // Ulo�en� maxim�ln� odpov�di
            }
            output.at<float>(y, x) = maxVal; // Ulo�en� v�sledku
        }
    }
}

// Funkce pro prahov�n�
Mat applyThreshold(const Mat& input, float threshold) {
    Mat output = Mat::zeros(input.size(), CV_8U); // V�stupn� bin�rn� obr�zek
    for (int y = 0; y < input.rows; y++) {
        for (int x = 0; x < input.cols; x++) {
            float value = input.at<float>(y, x); // Hodnota pixelu
            output.at<uchar>(y, x) = (value > threshold) ? 255 : 0; // Prahov�n�
        }
    }
    return output; // Vr�cen� bin�rn�ho obr�zku
}

int main() {
    // Na�ten� obr�zku v odst�nech �edi
    //Mat img = imread("inputs/image.jpg", IMREAD_GRAYSCALE); //obrazek z: https://en.wikipedia.org/wiki/Sobel_operator#/media/File:Bikesgray.jpg
    Mat img = imread("inputs/image.png", IMREAD_GRAYSCALE); //obrazek z: https://en.wikipedia.org/wiki/Sobel_operator#/media/File:Valve_original_(1).PNG
    //Mat img = imread("inputs/image2.png", IMREAD_GRAYSCALE);

    if (img.empty()) {
        cout << "Chyba: Nelze nacist obrazek" << endl;
        return -1;
    }

    // Aplikace r�zn�ch filtr�
    Mat sobelVertical, sobelHorizontal, prewittEdges, laplacianEdges, scharrEdges, kirschEdges;

    // Sobel - vertik�ln� a horizont�ln� hrany
    applySobel(img, sobelVertical, false); 
    applySobel(img, sobelHorizontal, true); 

    // Prewitt
    applyPrewitt(img, prewittEdges);

    // Laplace - r�zn� nastaven� velikosti j�dra
    int kernelSize = 3; // Velikost j�dra 
    applyLaplacian(img, laplacianEdges, kernelSize);

    // Scharr
    applyScharr(img, scharrEdges);

    // Kirsch
    applyKirsch(img, kirschEdges);

    // Prahov�n�
    Mat sobelVerticalThresholded = applyThreshold(sobelVertical, 100); // Ka�d� pixel v obraze se porovn� s danou prahovou hodnotou jestli je nad nebo pod n�
    Mat sobelHorizontalThresholded = applyThreshold(sobelHorizontal, 100);
    Mat prewittThresholded = applyThreshold(prewittEdges, 100);
    Mat laplacianThresholded = applyThreshold(laplacianEdges, 100);
    Mat scharrThresholded = applyThreshold(scharrEdges, 100);
    Mat kirschThresholded = applyThreshold(kirschEdges, 100);

    // Zobrazen� v�sledk�
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