#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <fftw3.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TApplication.h>
#include <TPad.h>
#include <TFile.h>
#include <TImage.h>
#include <TStyle.h>
#include <TPaveLabel.h>

using namespace std;

#define SAMPLE_RATE 44100  // Frecuencia de muestreo

struct WAVHeader {
    char riff[4];
    int overall_size;
    char wave[4];
    char fmt_chunk_marker[4];
    int length_of_fmt;
    short format_type;
    short channels;
    int sample_rate;
    int byterate;
    short block_align;
    short bits_per_sample;
    char data_chunk_header[4];
    int data_size;
};

// Función para leer un archivo WAV
vector<double> readWAV(const char* filename, int& sampleRate) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo WAV." << endl;
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
        signal[i] = sample / 32768.0;  // Normalizar
    }

    file.close();
    return signal;
}

// Función para generar la visualización y guardarla en PDF
void plotGraphs(const vector<double>& signal, int sampleRate, const string& outputFilename) {
    int N = signal.size();
    vector<double> timeAxis(N);
    for (int i = 0; i < N; i++) {
        timeAxis[i] = i / double(sampleRate);
    }

    // Calcular la FFT
    fftw_complex *in, *out;
    fftw_plan plan;
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

    for (int i = 0; i < N; i++) {
        in[i][0] = signal[i];  // Parte real
        in[i][1] = 0.0;  // Parte imaginaria
    }

    plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    // Crear vectores para la gráfica de la FFT
    vector<double> freqAxis(N / 2);
    vector<double> magnitude(N / 2);

    for (int i = 0; i < N / 2; i++) {
        freqAxis[i] = i * sampleRate / N;
        magnitude[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
    }

    // Configuración de ROOT
    TApplication app("app", nullptr, nullptr);
    TCanvas* canvas = new TCanvas("Canvas", "Análisis de Audio", 1000, 800);
    canvas->Divide(1, 2);  // Dividir en 2 pads (1 columna, 2 filas)

    // Graficar la forma de onda
    canvas->cd(1);
    TGraph* graphTime = new TGraph(N, &timeAxis[0], &signal[0]);
    graphTime->SetTitle("Forma de Onda;Tiempo (s);Amplitud");
    graphTime->SetLineColor(2);
    graphTime->Draw("AL");

    // Graficar la FFT
    canvas->cd(2);
    TGraph* graphFFT = new TGraph(N / 2, &freqAxis[0], &magnitude[0]);
    graphFFT->SetTitle("Espectro de Frecuencia;Frecuencia (Hz);Magnitud");
    graphFFT->SetLineColor(4);
    graphFFT->Draw("AL");

    // Guardar en PDF
    canvas->SaveAs(outputFilename.c_str());

    // Limpiar memoria
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " archivo.wav salida.pdf" << endl;
        return 1;
    }

    int sampleRate;
    vector<double> signal = readWAV(argv[1], sampleRate);

    // Generar y guardar el gráfico
    plotGraphs(signal, sampleRate, argv[2]);

    cout << "Gráfico guardado en " << argv[2] << endl;
    return 0;
}