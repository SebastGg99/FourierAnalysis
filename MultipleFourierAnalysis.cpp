#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <fftw3.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TText.h>

using namespace std;

vector<double> readWavFile(const string& filename, int& sampleRate) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: No se pudo abrir el archivo WAV: " << filename << endl;
        exit(1);
    }
    file.seekg(24);
    file.read(reinterpret_cast<char*>(&sampleRate), sizeof(int));
    file.seekg(44);
    vector<double> signal;
    short sample;
    while (file.read(reinterpret_cast<char*>(&sample), sizeof(short))) {
        signal.push_back(sample / 32768.0);
    }
    return signal;
}

vector<double> computeFFT(const vector<double>& signal, int sampleRate, vector<double>& freqAxis) {
    int N = signal.size();
    vector<double> magnitude(N / 2);
    fftw_complex *in, *out;
    fftw_plan plan;
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    
    for (int i = 0; i < N; i++) {
        in[i][0] = signal[i];
        in[i][1] = 0.0;
    }
    fftw_execute(plan);
    
    for (int i = 0; i < N / 2; i++) {
        magnitude[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        freqAxis.push_back(i * (sampleRate / static_cast<double>(N)));
    }
    
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    
    return magnitude;
}

void plotFFT(const vector<double>& freqAxis, const vector<double>& magnitude) {
    TCanvas* canvas = new TCanvas("Canvas", "Análisis de Audio", 1000, 800);
    int N = magnitude.size();
    TGraph* graph = new TGraph(N, freqAxis.data(), magnitude.data());
    graph->SetTitle("Espectro de Frecuencia");
    graph->GetXaxis()->SetTitle("Frecuencia (Hz)");
    graph->GetYaxis()->SetTitle("Magnitud");
    graph->GetXaxis()->SetLimits(0, freqAxis.back()); // Asegurar que el gráfico comience en 0
    graph->Draw("AL");
    
    double maxMag = 0;
    double maxFreq = 0;
    for (int i = 0; i < N; i++) {
        if (magnitude[i] > maxMag) {
            maxMag = magnitude[i];
            maxFreq = freqAxis[i];
        }
    }
    cout << "Frecuencia dominante: " << maxFreq << " Hz" << endl;
    TText* text = new TText(maxFreq, maxMag, Form("%.1f Hz", maxFreq));
    text->SetTextSize(0.03);
    text->SetTextColor(kRed);
    text->Draw();
    
    canvas->SaveAs("fft_analysis.pdf");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " archivo.wav" << endl;
        return 1;
    }
    
    int sampleRate;
    vector<double> signal = readWavFile(argv[1], sampleRate);
    vector<double> freqAxis;
    vector<double> magnitude = computeFFT(signal, sampleRate, freqAxis);
    plotFFT(freqAxis, magnitude);
    
    return 0;
}
