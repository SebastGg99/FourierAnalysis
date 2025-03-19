/* ======================================================
 * Script 2: Implementación Clásica de la Transformada Rápida de Fourier (FFT)
 * ======================================================
 */

#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace std;
using Complex = complex<double>;
using CArray = vector<Complex>;

const double PI = acos(-1);

void fft(CArray &x) {
    const size_t N = x.size();
    if (N <= 1) return;

    // Divide
    CArray even(N / 2);
    CArray odd(N / 2);
    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = x[i * 2];
        odd[i] = x[i * 2 + 1];
    }

    // Conquer
    fft(even);
    fft(odd);

    // Combine
    for (size_t k = 0; k < N / 2; ++k) {
        Complex t = polar(1.0, -2 * PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
    }
}

int main() {
    const size_t N = 8;
    CArray data(N);

    // Sample data
    for (size_t i = 0; i < N; ++i) {
        data[i] = Complex(i, 0);
    }

    // Perform FFT
    fft(data);

    // Output the results
    for (size_t i = 0; i < N; ++i) {
        cout << data[i] << endl;
    }

    return 0;
}
