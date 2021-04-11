#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtweekend.h"
#include "rtw_stb_image.h"
#include "perlin.h"

#include <iostream>

class texture {
    public:
        virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color: public texture {
    public:
        solid_color() {}
        solid_color(color c) : color_value(c) {}

        solid_color(double red, double green, double blue): solid_color(color(red, green, blue)) {}

        virtual color value(double u, double v, const point3& p) const override {
            return color_value;
        }

    public:
        color color_value;
};

class checker_texture: public texture {
    public:
        checker_texture() {}

        checker_texture(shared_ptr<texture> _even, shared_ptr<texture> _odd): even(_even), odd(_odd) {}

        checker_texture(color c1, color c2): even(make_shared<solid_color>(c1)), odd(make_shared<solid_color>(c2)) {}

        virtual color value(double u, double v, const point3& p) const override {
            auto sines = sin(10*p.x())*sin(10*p.y())*sin(10*p.z());
            if (sines < 0){
                return odd->value(u, v, p);
            } else {
                return even->value(u, v, p);
            }
        }

    public:
        shared_ptr<texture> even, odd;
};

class noise_texture: public texture {
    public:
        noise_texture() {}
        noise_texture(double sc): scale(sc) {}

        virtual color value(double u, double v, const point3& p) const override {
            return color(1, 1, 1)*noise.turb(scale*p);
        } 
    public:
        perlin noise;
        double scale;
};

class marble_texture: public texture {
    public:
        marble_texture() {}
        marble_texture(double sc): scale(sc) {}

        virtual color value(double u, double v, const point3& p) const override {
            return color(1, 1, 1)*0.5*(1+sin(scale*p.z()+10*noise.turb(p)));
        } 
    public:
        perlin noise;
        double scale;
};

class image_texture: public texture {
    public:
        const static int bytes_per_pixel = 3;

        image_texture(): data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

        image_texture(const char* filename){
            auto components_per_pixel = 3;
            std::cerr << "Loading image texture from: '" << filename << "' as ";
            if (stbi_is_hdr(filename)){
                std::cerr << "HDR file.\n";
            } else {
                std::cerr << "gamma-corrected file.\n";
            }
            data = stbi_loadf(filename, &width, &height, &components_per_pixel, components_per_pixel);

            if (!data){
                std::cerr << "ERROR: Couldn't load texture image file: '" << filename << "'.\n";
                exit(1);
            }

            bytes_per_scanline = bytes_per_pixel*width;
        }
        
        ~image_texture(){
            delete data;
        }

        virtual color value(double u, double v, const vec3& p) const override {
            // if no texture data, return cyan as debugging color
            if (data == nullptr){
                return color(0, 1, 1);
            }

            // Clamp texture coordinates to valid range
            u = clamp(u, 0.0, 1.0);
            v = 1-clamp(v, 0.0, 1.0); // Flipping to image coords

            auto i = static_cast<int>(u*width);
            auto j = static_cast<int>(v*height);

            // Clamp the integer mapping, coords must be less than 1.0
            if (i >= width)  i = width-1;
            if (j >= height) j = height-1;

            const auto color_scale = 1.0;// /255.0;

            auto pixel = data+j*bytes_per_scanline+i*bytes_per_pixel;

            return color(color_scale*pixel[0], color_scale*pixel[1], color_scale*pixel[2]);
        }

    public:
        float *data;
        int width, height;
        int bytes_per_scanline;
};

class roughness_from_sharpness_texture: public texture {
    public:
        roughness_from_sharpness_texture() {}

        roughness_from_sharpness_texture(shared_ptr<texture> sharpness_map, double min_v, double max_v): sharpness_text(sharpness_map), 
            l_min_val(log(min_v)), l_max_val(log(max_v)) {}

        virtual color value(double u, double v, const point3& p) const override {
            return color(1, 0, 0) * clamp(
                    log(sharpness_text->value(u, v, p).length()+0.00001), l_min_val, l_max_val)
                / (l_max_val-l_min_val);
        }

    public:
        shared_ptr<texture> sharpness_text;
    private: 
        double l_min_val, l_max_val;
};


static void get_spherical_uv(const point3 &p, double& u, double&v){
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

    auto theta = acos(-p.y());
    auto phi = atan2(-p.z(), p.x()) + pi;

    u = phi / (2*pi);
    v = theta / pi;
}

static vec3 from_spherical_uv(double u, double v){
    double phi = 2*pi*u, theta = pi*v;
    // THIS IS SUPER WEIRD?? Used only (AND KEEP IT THAT WAY) for environment importance sampling
    phi -= pi;
    
    return vec3(cos(phi)*sin(theta), -cos(theta), -sin(phi)*sin(theta));
}
#endif
