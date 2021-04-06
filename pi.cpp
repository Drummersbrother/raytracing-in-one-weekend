#include "rtweekend.h"

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>

int main() {
    long long int inside_circle = 0;
    long long int inside_circle_stratified = 0;
    int sqrt_N = 1000;
    for (int i = 0; i < sqrt_N; i++) {
        for (int j = 0; j < sqrt_N; j++) {
            auto x = random_double(-1, 1);
            auto y = random_double(-1, 1);
            if (x*x+y*y < 1) inside_circle++;

            x = 2*((i+random_double())/sqrt_N)-1;
            y = 2*((j+random_double())/sqrt_N)-1;
            if (x*x+y*y < 1) inside_circle_stratified++;
        }
    }
    std::cout << std::fixed << std::setprecision(12);
    auto N = static_cast<long long int>(sqrt_N)*sqrt_N;
    std::cout << "Absolute error of regular estimate of pi = " << fabs((4*double(inside_circle)/N)-pi) << " .\n";
    std::cout << "Absolute error of stratified estimate of pi = " << fabs((4*double(inside_circle_stratified)/N)-pi) << " .\n";
    return 0;
}
