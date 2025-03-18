#include <iostream>      // Librería para entrada y salida estándar
#include <fstream>       // Librería para manejar archivos
#include <cmath>         // Librería para funciones matemáticas
#include <vector>        // Librería para manejar vectores
#include <filesystem>    // Librería para manipulación de archivos y directorios
#include <fftw3.h>       // Librería para transformar de Fourier rápida (FFT)
#include <TCanvas.h>     // ROOT: Librería para gráficos
#include <TGraph.h>      // ROOT: Gráficos en ROOT
#include <TAxis.h>       // ROOT: Manipulación de ejes

using namespace std;
namespace fs = std::filesystem;

#define SAMPLE_RATE 44100  // Frecuencia de muestreo típica para audio

// Estructura para leer el encabezado de un archivo WAV
struct WAVHeader {
    char riff[4];            // Identificador "RIFF"
    int overall_size;        // Tamaño total del archivo menos 8 bytes
    char wave[4];            // Identificador "WAVE"
    char fmt_chunk_marker[4]; // Identificador "fmt "
    int length_of_fmt;       // Tamaño del subchunk "fmt"
    short format_type;       // Tipo de formato (1 = PCM)
    short channels;          // Número de canales (1 = mono, 2 = estéreo)
    int sample_rate;         // Frecuencia de muestreo (Hz)
    int byterate;            // Bytes por segundo
    short block_align;       // Tamaño de un frame de audio en bytes
    short bits_per_sample;   // Bits por muestra (16, 24, 32 bits, etc.)
    char data_chunk_header[4]; // Identificador "data"
    int data_size;           // Tamaño del bloque de datos de audio
};

// Función para leer un archivo WAV y extraer la señal de audio
vector<double> readWAV(const string& filename, int& sampleRate) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo WAV: " << filename << endl;
        exit(1);
    }

    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));
    sampleRate = header.sample_rate;

    int numSamples = header.data_size / (header.bits_per_sample / 8);
    vector<double> signal(numSamples);

    for (int i = 0; i < numSamples; i++) {
        short sample;
        file.read(reinterpret_cast<char*>(&sample), sizeof(short));
        signal[i] = sample / 32768.0;  // Normalizar el valor entre -1 y 1
    }

    file.close();
    return signal;
}

// Función para analizar archivos WAV y guardar gráficos en un PDF
void analyzeAndSaveToPDF(const string& folderPath, const string& outputPDF) {
    vector<string> wavFiles;
    
    // Buscar archivos .wav en el directorio especificado
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".wav") {
            wavFiles.push_back(entry.path().string());
        }
    }

    if (wavFiles.empty()) {
        cerr << "No se encontraron archivos .wav en la carpeta." << endl;
        return;
    }

    TCanvas* canvas = new TCanvas("Canvas", "Análisis de Audio", 1000, 800);
    canvas->Divide(1, 2);  // Dos paneles para gráficos

    // Iniciar PDF con múltiples páginas
    canvas->SaveAs((outputPDF + "[").c_str());  

    for (const auto& file : wavFiles) {
        int sampleRate;
        vector<double> signal = readWAV(file, sampleRate);
        int N = signal.size();

        // Crear eje de tiempo
        vector<double> timeAxis(N);
        for (int i = 0; i < N; i++) {
            timeAxis[i] = i / double(sampleRate);
        }

        // FFT
        fftw_complex *in, *out;
        fftw_plan plan;
        in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
        out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
        
        for (int i = 0; i < N; i++) {
            in[i][0] = signal[i];
            in[i][1] = 0.0;
        }

        plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan);

        // Calcular magnitudes espectrales
        vector<double> freqAxis(N / 2);
        vector<double> magnitude(N / 2);
        for (int i = 0; i < N / 2; i++) {
            freqAxis[i] = i * sampleRate / double(N);
            magnitude[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        }

        // Gráfico de la forma de onda
        canvas->cd(1);
        TGraph* graphTime = new TGraph(N, &timeAxis[0], &signal[0]);
        graphTime->SetTitle(("Forma de Onda: " + file).c_str());
        graphTime->SetLineColor(2);
        graphTime->Draw("AL");

        // Gráfico del espectro de frecuencia
        canvas->cd(2);
        TGraph* graphFFT = new TGraph(N / 2, &freqAxis[0], &magnitude[0]);
        graphFFT->SetTitle(("Espectro de Frecuencia: " + file).c_str());
        graphFFT->SetLineColor(4);
        graphFFT->Draw("AL");
        graphFFT->GetXaxis()->SetLimits(0, 1200);

        // Guardar la página en el PDF
        canvas->SaveAs(outputPDF.c_str());

        // Liberar memoria
        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);
    }

    // Cerrar el PDF
    canvas->SaveAs((outputPDF + "]").c_str());

    cout << "Análisis completado. Resultados guardados en " << outputPDF << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " carpeta_de_archivos salida.png" << endl;
        return 1;
    }

    analyzeAndSaveToPDF(argv[1], argv[2]);
    return 0;
}
