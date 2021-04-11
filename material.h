#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "hittable.h"
#include "vec3.h"
#include "texture.h"
#include "onb.h"
#include "pdf.h"

struct scatter_record {
    ray specular_ray;
    bool is_specular;
    color attenuation;
    shared_ptr<pdf> pdf_ptr;
};

class material{
    public: 
        virtual color emitted(
                const ray& r_in, const hit_record& rec, double u, double v, const point3& p
        ) const {
            return color(0, 0, 0);
        }
        virtual bool scatter(
                const ray& r_in, const hit_record& rec, scatter_record& srec
        ) const {
            return false;
        };

        virtual double scattering_pdf(
            const ray& r_in, const hit_record& rec, const ray& scattered) const {return 0;}

};

class lambertian: public material{
    public:
        lambertian(const color& a): albedo(make_shared<solid_color>(a)){}
        lambertian(shared_ptr<texture> a ): albedo(a) {}

        virtual bool scatter(
            const ray& ray_in, const hit_record& rec, scatter_record& srec
        ) const override {
            srec.is_specular = false;
            srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
            srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
            return true;
        }
        double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override {
            auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
            return cosine < 0 ? 0 : cosine/pi;
        }

    public:
        shared_ptr<texture> albedo;

};

class diffuse_light: public material {
    public:
        diffuse_light(shared_ptr<texture> a): emit(a) {}
        diffuse_light(color c): emit(make_shared<solid_color>(c)) {}

        virtual bool scatter(const ray& r_in, const hit_record&, scatter_record& srec) const override {return false;}

        virtual color emitted(const ray& ray_in, const hit_record& rec, double u, double v, const point3& p) const override {
            if (rec.front_face)
                return emit->value(u, v, p);
            else
                return color(0, 0, 0);
        }

    public:
        shared_ptr<texture> emit;
};

class metal: public material {
    public:
        metal(const color& a, double f): albedo(a), fuzz(f<1? f:1){}

        virtual bool scatter(
            const ray& ray_in, const hit_record& rec, scatter_record& srec
        ) const override {
            vec3 reflected = reflect(unit_vector(ray_in.direction()), rec.normal);
            srec.attenuation = albedo;

            srec.is_specular = true;
            srec.specular_ray = ray(rec.p, reflected+fuzz*random_in_unit_sphere(), ray_in.time());

            srec.pdf_ptr = 0;
            return true;
        }
    public:
        color albedo;
        double fuzz;
};

class glossy: public material {
    public:
        // Fuzz texture interpreted as the magnitude of the fuzz texture.
        glossy(shared_ptr<texture>& a, shared_ptr<texture>& f): albedo(a), fuzz(f){}

        virtual bool scatter(
            const ray& ray_in, const hit_record& rec, scatter_record& srec
        ) const override {
            vec3 reflected = reflect(unit_vector(ray_in.direction()), rec.normal);
            srec.attenuation = albedo->value(rec.u, rec.v, rec.p);

            srec.is_specular = true;
            double fuzz_factor = (fuzz->value(rec.u, rec.v, rec.p)).length();
            srec.specular_ray = ray(rec.p, reflected+fuzz_factor*random_in_unit_sphere(), ray_in.time());

            srec.pdf_ptr = 0;
            return true;
        }
    public:
        shared_ptr<texture> albedo, fuzz;
};

class dielectric: public material {
    public:
        dielectric(double index_of_refraction): ir(index_of_refraction){}

        virtual bool scatter(
            const ray& ray_in, const hit_record& rec, scatter_record& srec
        ) const override {
            srec.is_specular = true;
            srec.pdf_ptr = nullptr;
            srec.attenuation = color(1, 1, 1);

            double refraction_ratio = rec.front_face? (1.0/ir):ir;

            vec3 unit_direction = unit_vector(ray_in.direction());
            vec3 direction;
            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0-cos_theta*cos_theta);

            bool cannot_refract = refraction_ratio*sin_theta>1.0;

            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double()){
                direction = reflect(unit_direction, rec.normal);
            } else {
                direction = refract(unit_direction, rec.normal, refraction_ratio);
            }

            srec.specular_ray = ray(rec.p, direction, ray_in.time());
            return true;
        }
    public:
        double ir; // IOR

    private:
        static double reflectance(double cosine, double ref_idx){
            // Schlick's approxmiation for reflectance
            auto r0 = (1-ref_idx)/(1+ref_idx);
            r0 *= r0;
            return r0+(1-r0)*pow((1-cosine), 5);
        }
};

class isotropic: public material {
    public:
        isotropic(color c): albedo(make_shared<solid_color>(c)) {}
        isotropic(shared_ptr<texture> a): albedo(a) {}

        virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
            srec.is_specular = true;
            srec.specular_ray = ray(rec.p, random_in_unit_sphere(), r_in.time());
            srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
            return true;
        }
    public:
            shared_ptr<texture> albedo;
};

class mixed: public material{
    public:
        mixed(const shared_ptr<material>& a, const shared_ptr<material>& b, double r): mat_a(a), mat_b(b), ratio(r) {}

        virtual bool scatter(
            const ray& ray_in, const hit_record& rec, scatter_record& srec
        ) const override {
            return choose_mat()->scatter(ray_in, rec, srec);
        }

        virtual color emitted(
                const ray& r_in, const hit_record& rec, double u, double v, const point3& p
        ) const override {
            return choose_mat()->emitted(r_in, rec, u, v, p);
        }

        virtual double scattering_pdf(
            const ray& r_in, const hit_record& rec, const ray& scattered) const override {
                return (ratio * mat_a->scattering_pdf(r_in, rec, scattered)) + 
                    ((1. - ratio) * mat_b->scattering_pdf(r_in, rec, scattered));
            }

    public:
        shared_ptr<material> mat_a, mat_b;
        double ratio;

    private:
        inline shared_ptr<material> choose_mat() const {
            if (random_double() < ratio){
                return mat_a;
            } else {
                return mat_b;
            }
        }
};

// See https://people.sc.fsu.edu/~jburkardt/data/mtl/mtl.html
// The MTL format is based on the Phong shading model, so this uses a bit of reinterpretation
// See https://www.scratchapixel.com/lessons/3d-basic-rendering/phong-shader-BRDF , and https://www.psychopy.org/api/visual/phongmaterial.html , and http://vr.cs.uiuc.edu/node198.html
// There are a few properties, which we allow to vary based on textures: 
// diffuse color: albedo for lambertian 
// specular color: albedo for metal
// emissive color: emissive :)
//
// sharpness map: remapped to fuzz := 1-log_10(sharpness)/4, sharpness clamped to [1, 10000]
//
// How to decide what happens? |color_for_type| / (sum ^type |color|), i.e. if color components add to 1, everything is fine, if not it is normalized
//
class mtl_material: public material {
    public:
        mtl_material(
                shared_ptr<texture> diffuse_a, 
                shared_ptr<texture> specular_a, 
                shared_ptr<texture> emissive_a,
                shared_ptr<texture> transparency_map, 
                shared_ptr<texture> sharpness_map, 
                int illum): 
            emissive_text(emissive_a), 
            diffuse_text(diffuse_a), 
            specular_text(specular_a), 
            transparency_text(transparency_map), 
            roughness_text(make_shared<roughness_from_sharpness_texture>(sharpness_map, 1, 10000))
        {
            diffuse_mat = make_shared<lambertian>(diffuse_text);
            specular_mat = make_shared<glossy>(specular_text, roughness_text);
            emissive_mat = make_shared<diffuse_light>(emissive_text);
        }

        virtual bool scatter(
            const ray& ray_in, const hit_record& rec, scatter_record& srec
        ) const override {
            double transp_prob = transparency_prob(rec.u, rec.v, rec.p);
            if (transp_prob > random_double()){
                
                srec.attenuation = transparency_text->value(rec.u, rec.v, rec.p);
                srec.is_specular = true;
                // Continue in the same direction, starting from hitpoint
                srec.specular_ray = ray(rec.p, ray_in.direction(), ray_in.time());

                return false;
            }
            return choose_mat(rec.u, rec.v, rec.p)->scatter(ray_in, rec, srec);
        }

        virtual color emitted(
                const ray& r_in, const hit_record& rec, double u, double v, const point3& p
        ) const override {
            return emissive_mat->emitted(r_in, rec, u, v, p);
        }

        virtual double scattering_pdf(
            const ray& r_in, const hit_record& rec, const ray& scattered) const override {
                // We don't need to care about the transparent case, this only integrates over scattered rays (note specular are scatterd, but not diffuse)
                double diff_prob = diffuse_prob(rec.u, rec.v, rec.p);
                return diff_prob*(diffuse_mat->scattering_pdf(r_in, rec, scattered)) 
                    + (1-diff_prob)*specular_mat->scattering_pdf(r_in, rec, scattered);
            }
    public:
        shared_ptr<texture> emissive_text, diffuse_text, specular_text, transparency_text, roughness_text;
    private:
        shared_ptr<material> emissive_mat, diffuse_mat, specular_mat;
        inline double transparency_prob(double u, double v, const point3& p) const {
            double diff = diffuse_text->value(u, v, p).length();
            double spec = specular_text->value(u, v, p).length();
            double transp = transparency_text->value(u, v, p).length();
            return transp / (transp+diff+spec+0.00001);
        }

        inline double diffuse_prob(double u, double v, const point3& p) const {
            double diff = diffuse_text->value(u, v, p).length();
            double spec = specular_text->value(u, v, p).length();
            return diff / (diff+spec+0.00001);
        }

        inline shared_ptr<material> choose_mat(double u, double v, const point3& p) const {
            if (diffuse_prob(u, v, p) > random_double()){
                return diffuse_mat;
            } else {
                return specular_mat;
            }
        }
};

#endif
