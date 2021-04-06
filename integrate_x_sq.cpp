#include "rtweekend.h"

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>

inline double pdf(double x){
    return 3*x*x/8;
}

int main() {
    int N = 10000;
    auto sum = 0.0;
    for(int i=0; i<N; i++){
        auto x = pow(random_double(0, 8), 1./3.);
        if(x==0) continue;
        sum += x*x/pdf(x);
    }

    std::cout << std::fixed << std::setprecision(12);
    std::cout << "I = " << sum/N << "\n";
    return 0;
}
