
#include "rtweekend.h"

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>

inline double pdf(const vec3& p){
    return 1/(4*pi);
}

int main() {
    int N = 10000000;
    auto sum = 0.0;
    for(int i=0; i<N; i++){
        vec3 d = random_unit_vector();
        auto cosine_squared = d.z()*d.z();// see pow(dot(d, vec3(0, 0, 1)), 2)
        auto pdf_val = pdf(d);
        if(pdf_val==0) continue;
        sum += cosine_squared/pdf_val;
    }

    std::cout << std::fixed << std::setprecision(12);
    std::cout << "I = " << (sum/N)-((4./3.)*pi) << "\n";
    return 0;
}
