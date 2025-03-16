/* ======================================================
 * Script 2: Implementación Clásica de la Transformada Rápida de Fourier (FFT)
 * ======================================================
 */

#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
 
using namespace std;
const double PI = acos(-1);
 
using Complex =  complex<double>;
using CArray = vector<Complex>;
 
void FFT(CArray& x) {
    int N = x.size();
    if (N <= 1) return;
     
    CArray even(N/2), odd(N/2);
    for (int i = 0; i < N/2; i++) {
        even[i] = x[i*2];
        odd[i] = x[i*2 + 1];
    }
     
    FFT(even);
    FFT(odd);
     
    for (int k = 0; k < N/2; k++) {
        Complex t = polar(1.0, -2.0 * PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N/2] = even[k] - t;
    }
}
 
int main() {
    CArray signal = {1.0, 0.0, -1.0, 0.0}; // Ejemplo de señal
    FFT(signal);
     
    cout << "Frecuencia\tMagnitud" << endl;
    for (int i = 0; i < signal.size(); i++) {
        cout << i << "\t" << abs(signal[i]) << endl;
    }
    return 0;
}
