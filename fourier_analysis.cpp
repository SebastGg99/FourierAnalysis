#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <fftw3.h>

#define SAMPLE_RATE 44100  // Frecuencia de muestreo típica

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

void fourierTransform(const std::vector<double>& signal) {
    int N = signal.size();
    fftw_complex *in, *out;
    fftw_plan plan;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    
    for (int i = 0; i < N; i++) {
        in[i][0] = signal[i];  // Real part
        in[i][1] = 0.0;  // Imaginary part
    }

    plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    std::ofstream fout("fft_output.txt");
    for (int i = 0; i < N / 2; i++) {
        double frequency = i * SAMPLE_RATE / N;
        double magnitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        fout << frequency << " " << magnitude << std::endl;
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}

std::vector<double> readWAV(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    int numSamples = header.data_size / (header.bits_per_sample / 8);
    std::vector<double> signal(numSamples);

    for (int i = 0; i < numSamples; i++) {
        short sample;
        file.read(reinterpret_cast<char*>(&sample), sizeof(short));
        signal[i] = sample / 32768.0;
    }

    file.close();
    return signal;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " archivo.wav\n";
        return 1;
    }

    std::vector<double> signal = readWAV(argv[1]);
    fourierTransform(signal);

    std::cout << "Transformada de Fourier completada. Resultados guardados en fft_output.txt\n";
    return 0;
}