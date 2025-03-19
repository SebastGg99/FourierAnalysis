#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <filesystem>
#include <fftw3.h>
#include <opencv2/opencv.hpp>  // Librería OpenCV para imágenes
#include <TCanvas.h>           // ROOT para gráficos
#include <TGraph2D.h>          // ROOT para gráficos 2D
#include <TAxis.h>             // ROOT para manipulación de ejes

using namespace std;
namespace fs = std::filesystem;

// Función para leer una imagen y devolverla como matriz de double
cv::Mat readImage(const string& filename) {
    cv::Mat img = cv::imread(filename, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        cerr << "Error: No se pudo abrir la imagen: " << filename << endl;
        exit(1);
    }
    img.convertTo(img, CV_64F);  // Convertir a formato de doble precisión
    return img;
}

// Función para calcular la FFT2D de la imagen
cv::Mat computeFFT2D(const cv::Mat& img) {
    int rows = img.rows, cols = img.cols;

    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * rows * cols);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * rows * cols);
    fftw_plan plan = fftw_plan_dft_2d(rows, cols, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Copiar datos de la imagen a la entrada de FFTW
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            int index = i * cols + j;
            in[index][0] = img.at<double>(i, j);  // Real
            in[index][1] = 0.0;  // Imaginario
        }

    fftw_execute(plan);

    // Calcular magnitudes y convertir a Matriz OpenCV
    cv::Mat magnitudeImg(rows, cols, CV_64F);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            int index = i * cols + j;
            magnitudeImg.at<double>(i, j) = log(1 + sqrt(out[index][0] * out[index][0] + out[index][1] * out[index][1]));
        }

    // Liberar memoria
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return magnitudeImg;
}

// Función para analizar imágenes y guardar resultados en un PDF
void analyzeImages(const string& folderPath, const string& outputPDF) {
    vector<string> imageFiles;
    
    // Buscar archivos de imagen en la carpeta
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".jpg" || entry.path().extension() == ".png") {
            imageFiles.push_back(entry.path().string());
        }
    }

    if (imageFiles.empty()) {
        cerr << "No se encontraron imágenes en la carpeta." << endl;
        return;
    }

    TCanvas* canvas = new TCanvas("Canvas", "Análisis de Imágenes", 1000, 800);
    canvas->Divide(1, 2);  // Dos paneles para gráficos

    // Iniciar PDF con múltiples páginas
    canvas->SaveAs((outputPDF + "[").c_str());  

    for (const auto& file : imageFiles) {
        // Leer imagen y calcular FFT
        cv::Mat img = readImage(file);
        cv::Mat fftResult = computeFFT2D(img);

        // Convertir a formato gráfico para ROOT
        int rows = img.rows, cols = img.cols;
        vector<double> x, y, z;
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++) {
                x.push_back(j);
                y.push_back(i);
                z.push_back(fftResult.at<double>(i, j));
            }

        // Gráfico de la imagen original
        canvas->cd(1);
        TGraph2D* graphImage = new TGraph2D(x.size(), &x[0], &y[0], &z[0]);
        graphImage->SetTitle(("Imagen Original: " + file).c_str());
        graphImage->Draw("COLZ");

        // Gráfico del espectro de Fourier
        canvas->cd(2);
        TGraph2D* graphFFT = new TGraph2D(x.size(), &x[0], &y[0], &z[0]);
        graphFFT->SetTitle(("FFT Magnitud: " + file).c_str());
        graphFFT->Draw("COLZ");

        // Guardar la página en el PDF
        canvas->SaveAs(outputPDF.c_str());
    }

    // Cerrar el PDF
    canvas->SaveAs((outputPDF + "]").c_str());

    cout << "Análisis completado. Resultados guardados en " << outputPDF << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " carpeta_de_imagenes salida.pdf" << endl;
        return 1;
    }

    analyzeImages(argv[1], argv[2]);
    return 0;
}
