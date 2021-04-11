#ifndef PDF_H
#define PDF_H

#include "rtweekend.h"
#include "onb.h"
#include "hittable.h"
#include "texture.h"

class pdf {
    public: 
        virtual ~pdf(){}

        virtual double value(const vec3& direction) const = 0;
        virtual vec3 generate() const = 0;
};

inline vec3 random_cosine_direction() {
    auto r1 = random_double();
    auto r2 = random_double();
    auto z = sqrt(1-r2);
    auto phi = 2*pi*r1;
    auto x = cos(phi)*sqrt(r2);
    auto y = sin(phi)*sqrt(r2);

    return vec3(x, y, z);
}

class cosine_pdf: public pdf {
    public: 
        cosine_pdf(const vec3& w){uvw.build_from_w(w);}

        virtual double value(const vec3& direction) const override {
            auto cosine = dot(unit_vector(direction), uvw.w());
            return (cosine < 0)? 0 : cosine/pi;
        }

        virtual vec3 generate() const override {
            return uvw.local(random_cosine_direction());
        }

    public:
        onb uvw;
};

class hittable_pdf: public pdf {
    public:
        hittable_pdf(shared_ptr<hittable> p, const point3& origin): ptr(p), o(origin) {}

        virtual double value(const vec3& direction) const override {
            return ptr->pdf_value(o, direction);
        }

        virtual vec3 generate() const override {
            return ptr->random(o);
        }
    public:
        shared_ptr<hittable> ptr;
        point3 o;
};

class mixture_pdf: public pdf {
    public:
        mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1): proportion(0.5) {p[0] = p0; p[1] = p1;}
        mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1, double prop): proportion(prop) {p[0] = p0; p[1] = p1;}
        
        virtual double value(const vec3& direction) const override {
            return proportion*(p[0]->value(direction)) + (1.0-proportion)*(p[1]->value(direction));
        }

        virtual vec3 generate() const override {
            if (random_double() < proportion){
                return p[0]->generate();
            } else {
                return p[1]->generate();
            }
        }
    public:
        double proportion;
        shared_ptr<pdf> p[2];
};

inline vec3 random_to_sphere(double radius, double distance_squared){
    auto r1 = random_double();
    auto r2 = random_double();

    auto z = 1 + r2*(sqrt(1-radius*radius/distance_squared)-1);

    auto phi = 2*pi*r1;
    auto x = cos(phi)*sqrt(1-z*z);
    auto y = sin(phi)*sqrt(1-z*z);
    
    return vec3(x, y, z);
}

class image_pdf: public pdf {
    // From http://igorsklyar.com/system/documents/papers/4/fiscourse.comp.pdf
    public:
        image_pdf(shared_ptr<image_texture>& img): 
            image(img), m_width(img->width), m_height(img->height), m_channels(3), m_pData(img->data)
        {
            unsigned int k=0;
            float angleFrac = M_PI / float(m_height);
            float theta = angleFrac * 0.5f;
            float sinTheta = 0.f;
            float *pSinTheta = (float*) alloca(sizeof(float)*m_height);
            for (unsigned int i=0; i < m_height; i++, theta += angleFrac) {
                pSinTheta[i] = sin(theta);
            }
            // convert the data into a marginal CDF along the columns
            pBuffer = (float*) malloc(m_width*(m_height+1)*sizeof(float));
            pUDist = &pBuffer[m_width*m_height];
            for (unsigned int i=0,m=0; i < m_width; i++, m+=m_height) {
                float *pVDist = &pBuffer[m];
                k = i*m_channels;
                pVDist[0] = 0.2126f * m_pData[k+0] + 0.7152f * m_pData[k+1] +
                0.0722f * m_pData[k+2];
                pVDist[0] *= pSinTheta[0];
                for (unsigned int j=1,k=(m_width+i)*m_channels; j < m_height; j++, k+=m_width*m_channels) {
                    float lum = 0.2126f * m_pData[k+0] + 0.7152f * m_pData[k+1] +
                    0.0722f * m_pData[k+2];
                    lum *= pSinTheta[j];
                    pVDist[j] = pVDist[j-1] + lum;

                }
                if (i == 0) {
                    pUDist[i] = pVDist[m_height-1];
                } else {
                    pUDist[i] = pUDist[i-1] + pVDist[m_height-1];
                }
            }
        }

        virtual double value(const vec3& direction) const override {
            double _u, _v; get_spherical_uv(unit_vector(direction), _u, _v);
            _u = 1.-_u;
            int u = _u*double(m_height-1), v = _v*double(m_width-1);
            if (u < 0) u = 0;
            if (u>=m_height) u = m_height-1;

            if (v < 0) v = 0;
            if (v>=m_width) v = m_width-1;

            float angleFrac = M_PI / float(m_height);
            float invPdfNorm = (2.f * float(M_PI*M_PI) ) / float(m_width*m_height);
            float* pVDist = &pBuffer[m_height*u];
            // compute the actual PDF
            float pdfU = (u == 0)?(pUDist[0]):(pUDist[u]-pUDist[u-1]);
            pdfU /= pUDist[m_width-1];
            float pdfV = (v == 0)?(pVDist[0]):(pVDist[v]-pVDist[v-1]);
            pdfV /= pVDist[m_height-1];
            float theta = angleFrac * 0.5 + angleFrac * u;
            float Pdf = ( pdfU * pdfV ) * sin(theta) /invPdfNorm;
            
            return Pdf;
        }

        virtual vec3 generate() const override {
            double r1=random_double(), r2=random_double();

            float maxUVal = pUDist[m_width-1];
            float* pUPos = std::lower_bound(pUDist, pUDist+m_width,
            r1 * maxUVal);
            int u = pUPos - pUDist;
            float* pVDist = &pBuffer[m_height*u];
            float* pVPos = std::lower_bound(pVDist, pVDist+m_height,
            r2 * pVDist[m_height-1]);
            int v = pVPos - pVDist;

            double _u=double(u)/m_height, _v=double(v)/m_width;
            _u = 1.-_u;

            return from_spherical_uv(_u, _v);
        }
    public:
        shared_ptr<image_texture> image;
        int m_width, m_height, m_channels;
        float* m_pData, *pUDist, *pBuffer; 
};

#endif
