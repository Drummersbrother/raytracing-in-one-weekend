#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "rtweekend.h"
#include "hittable.h"

#define EPS 0.000001

class triangle: public hittable {
    public:
        triangle() {}
        triangle(const vec3 v0, const vec3 v1, const vec3 v2, shared_ptr<material> m) {
            triangle(v0, v1, v2, vec3(), vec3(), vec3(), false, m);
        }
        triangle(const vec3 v0, const vec3 v1, const vec3 v2, const vec3 vn0, const vec3 vn1, const vec3 vn2, bool smooth_shading, shared_ptr<material> m): mat_ptr(m) {
            verts[0] = v0; 
            verts[1] = v1; 
            verts[2] = v2; 
            vert_normals[0] = unit_vector(vn0);
            vert_normals[1] = unit_vector(vn1);
            vert_normals[2] = unit_vector(vn2);
            smooth_normals = smooth_shading;
            double a=(v0-v1).length(), b=(v1-v2).length(), c=(v2-v0).length();
            double s=(a+b+c)/2.; area = sqrt(fabs(s*(s-a)*(s-b)*(s-c)));
            middle_normal = unit_vector(cross(v0-v1, v0-v2));
        }
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;
        virtual double pdf_value(const point3& o, const vec3& v) const override;
        virtual vec3 random(const vec3& o) const override;
    public:
        vec3 verts[3];
        shared_ptr<material> mat_ptr;
        vec3 vert_normals[3];
        bool smooth_normals;
    private:
        double area;
        vec3 middle_normal;
};

bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    // MT algorithm, https://web.archive.org/web/20200927071045/https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
    auto v0_v1 = verts[1] - verts[0];
    auto v0_v2 = verts[2] - verts[0];
    auto dir = r.direction();
    auto parallel_vec = cross(dir, v0_v2);
    auto det = dot(v0_v1, parallel_vec);
    // If det < 0, this is a back-facing intersection, change hit_record front_face
    // ray and triangle are parallel if det is close to 0
    if (fabs(det) < EPS) return false;
    auto inv_det = 1. / det;

    auto tvec = r.origin() - verts[0];
    auto u = dot(tvec, parallel_vec)*inv_det;
    if (u < 0 || u > 1) return false;

    auto qvec = cross(tvec, v0_v1);
    auto v = dot(dir, qvec)*inv_det;
    if (v < 0 || u + v > 1) return false;

    double t = dot(v0_v2, qvec)*inv_det;

    if (t < t_min || t > t_max) return false;
    
    rec.t = t;
    rec.u = u;
    rec.v = v;
    rec.p = r.at(t);
    rec.mat_ptr = mat_ptr;

    rec.front_face = true;
    vec3 normal = middle_normal;
    if (smooth_normals){
        double a=u, b=v, c=(1-u-v);
        // What does u and v map to?
        normal = a*vert_normals[1]+b*vert_normals[2]+c*vert_normals[0];
    }

    rec.set_face_normal(r, (det>=-EPS)? normal:-normal);
    return true;

}

bool triangle::bounding_box(double time0, double time1, aabb& output_box) const {
    vec3 max_extent = max(max(verts[0], verts[1]), verts[2]);
    vec3 min_extent = min(min(verts[0], verts[1]), verts[2]);
    double eps = 0.001; auto epsv = vec3(eps, eps, eps);
    output_box = aabb(min_extent-epsv, max_extent+epsv);
    return true;
}

double triangle::pdf_value(const point3& o, const vec3& v) const {
    hit_record rec;
    if (!this->hit(ray(o, v), EPS, infinity, rec))
        return 0;

    // from https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=4121581
    vec3 R1 = verts[0]-o, R2=verts[1]-o, R3=verts[2]-o;
    double r1=R1.length(), r2=R2.length(), r3=R3.length();
    double N = dot(R1, cross(R2, R3));
    double D = r1*r2*r3 + dot(R1, R2)*r3 + dot(R1, R3)*r2 + dot(R2, R3)*r3;

    double omega = atan2(N, D);

    return 1./omega;
}

vec3 triangle::random(const point3& o) const {
    // From https://math.stackexchange.com/questions/18686/uniform-random-point-in-triangle-in-3d
    double r1 = random_double(), r2 = random_double();
    double ca = (1.-sqrt(r1)), 
           cb = sqrt(r1)*(1.-r2), 
           cc = r2*sqrt(r1);
    vec3 random_in_triangle = verts[0]*ca + verts[1]*cb + verts[2]*cc;
    return random_in_triangle-o;
}

#endif
