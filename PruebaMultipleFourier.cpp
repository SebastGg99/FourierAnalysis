#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <filesystem>
#include <fftw3.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TApplication.h>
#include <TAxis.h>

using namespace std;
namespace fs = std::filesystem;

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
        signal[i] = sample / 32768.0;  // Normalizar
    }

    file.close();
    return signal;
}

void analyzeAndSaveToPDF(const string& folderPath, const string& outputPDF) {
    vector<string> wavFiles;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".wav") {
            wavFiles.push_back(entry.path().string());
        }
    }

    if (wavFiles.empty()) {
        cerr << "No se encontraron archivos .wav en la carpeta." << endl;
        return;
    }

    TApplication app("app", nullptr, nullptr);
    TCanvas* canvas = new TCanvas("Canvas", "Análisis de Audio", 1000, 800);
    canvas->Print((outputPDF + "[").c_str()); // Abrir PDF

    for (const auto& file : wavFiles) {
        int sampleRate;
        vector<double> signal = readWAV(file, sampleRate);
        int N = signal.size();

        vector<double> timeAxis(N);
        for (int i = 0; i < N; i++) {
            timeAxis[i] = i / double(sampleRate);
        }

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

        vector<double> freqAxis(N / 2);
        vector<double> magnitude(N / 2);
        for (int i = 0; i < N / 2; i++) {
            freqAxis[i] = i * sampleRate / double(N);
            magnitude[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        }

        canvas->Clear();
        canvas->Divide(1, 2);

        canvas->cd(1);
        TGraph* graphTime = new TGraph(N, &timeAxis[0], &signal[0]);
        graphTime->SetTitle(("Forma de Onda: " + file).c_str());
        graphTime->SetLineColor(2);
        graphTime->Draw("AL");

        canvas->cd(2);
        TGraph* graphFFT = new TGraph(N / 2, &freqAxis[0], &magnitude[0]);
        graphFFT->SetTitle(("Espectro de Frecuencia: " + file).c_str());
        graphFFT->SetLineColor(4);
        graphFFT->GetXaxis()->SetLimits(0, 1200);
        graphFFT->Draw("AL");

        canvas->Print(outputPDF.c_str());

        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);
    }

    canvas->Print((outputPDF + "]").c_str()); // Cerrar PDF
    delete canvas;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " carpeta_de_archivos salida.pdf" << endl;
        return 1;
    }

    analyzeAndSaveToPDF(argv[1], argv[2]);
    cout << "Análisis completado. Resultados guardados en " << argv[2] << endl;
    return 0;
}
