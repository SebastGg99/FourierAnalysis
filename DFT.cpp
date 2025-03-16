/* ======================================================
 * Script 1: Implementación Clásica de la Transformada Discreta de Fourier (DFT)
 * ======================================================
 */

#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
 
using namespace std;
const double PI = acos(-1);
 
using Complex = complex<double>;
using CArray = vector<Complex>;


CArray DFT(const vector<double>& signal) {
    int N = signal.size();
    CArray dft(N);
     
    for (int k = 0; k < N; k++) {
        Complex sum(0.0, 0.0);
        for (int n = 0; n < N; n++) {
            double angle = -2.0 * PI * k * n / N;
            sum += signal[n] * Complex(cos(angle), sin(angle));
        }
        dft[k] = sum;
    }
    return dft;
}
 
int main() {
    vector<double> signal = {1.0, 0.0, -1.0, 0.0}; // Ejemplo de señal
    CArray dft = DFT(signal);
     
    cout << "Frecuencia\tMagnitud" << endl;
    for (int i = 0; i < dft.size(); i++) {
        cout << i << "\t" << abs(dft[i]) << endl;
    }
    return 0;
} 
