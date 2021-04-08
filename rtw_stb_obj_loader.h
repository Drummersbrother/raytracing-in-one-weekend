#ifndef RTWEEKEND_STB_OBJ_LOADER_H
#define RTWEEKEND_STB_OBJ_LOADER_H

// Disable pedantic warnings for this external library.
// GCC
#ifdef __GNUC__

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wwrite-strings"

#endif


#define OBJL_IMPLEMENTATION
#include "external/obj_loader.h"

// Restore warning levels.
// GCC
#pragma GCC diagnostic pop

#include <stdio.h>

#include "rtweekend.h"
#include "bvh.h"
#include "material.h"
#include "triangle.h"

shared_ptr<hittable> load_model_from_file(std::string filename, shared_ptr<material> model_material, bool shade_smooth){
    // from https://github.com/mojobojo/OBJLoader/blob/master/example.cc
    std::cerr << "Loading .obj file '" << filename << "'." << std::endl;
    FILE *f = std::fopen(filename.c_str(), "rb");
    objl_obj_file ObjFile;

    if (f) {
        fseek(f, 0, SEEK_END);
        long int FileSize = ftell(f);
        ftell(f);
        rewind(f);

        char* FileData = (char *) malloc(FileSize+1);
        fread(FileData, 1, FileSize, f);
        fclose(f);

        FileData[FileSize] = 0;

        objl_LoadObjMalloc(FileData, &ObjFile);
/*
        printf("Loaded OBJ File:\n");
        printf("    o:        %s\n", ObjFile.o);
        printf("    v_count:  %d\n", ObjFile.v_count);
        for (int i = 0; i < ObjFile.v_count; ++i) {
            printf("        x %f y %f z %f\n", ObjFile.v[i].x, ObjFile.v[i].y, ObjFile.v[i].z);
        }
        printf("\n");

        printf("    vt_count: %d\n", ObjFile.vt_count);
        for (int i = 0; i < ObjFile.vt_count; ++i) {
            printf("        x %f y %f\n", ObjFile.vt[i].x, ObjFile.vt[i].y);
        }
        printf("\n");

        printf("    vn_count: %d\n", ObjFile.vn_count);
        for (int i = 0; i < ObjFile.vn_count; ++i) {
            printf("        x %f y %f z %f\n", ObjFile.vn[i].x, ObjFile.vn[i].y, ObjFile.vn[i].z);
        }
        printf("\n");

        printf("    s:        %s\n", ObjFile.s);

        printf("    f_count:  %d\n", ObjFile.f_count);
        for (int i = 0; i < ObjFile.f_count; ++i) {
            printf("    f%d\n", i);
            printf("        0:\n");
            printf("            vertex:  %d\n", ObjFile.f[i].f0.vertex);
            printf("            texture: %d\n", ObjFile.f[i].f0.texture);
            printf("            normal:  %d\n", ObjFile.f[i].f0.normal);
            printf("        1:\n");
            printf("            vertex:  %d\n", ObjFile.f[i].f1.vertex);
            printf("            texture: %d\n", ObjFile.f[i].f1.texture);
            printf("            normal:  %d\n", ObjFile.f[i].f1.normal);
            printf("        2:\n");
            printf("            vertex:  %d\n", ObjFile.f[i].f2.vertex);
            printf("            texture: %d\n", ObjFile.f[i].f2.texture);
            printf("            normal:  %d\n", ObjFile.f[i].f2.normal);
        }
        printf("\n");
        */

        hittable_list triangles;
        for (int i = 0; i < ObjFile.f_count; ++i) {
            auto face = ObjFile.f[i];
            vec3 v0, v1, v2;
            auto objl_v0 = ObjFile.v[face.f0.vertex-1]; v0 = vec3(objl_v0.x, objl_v0.y, objl_v0.z);
            auto objl_v1 = ObjFile.v[face.f1.vertex-1]; v1 = vec3(objl_v1.x, objl_v1.y, objl_v1.z);
            auto objl_v2 = ObjFile.v[face.f2.vertex-1]; v2 = vec3(objl_v2.x, objl_v2.y, objl_v2.z);
            vec3 vn0, vn1, vn2;
            // what should the order be??
            auto objl_vn0 = ObjFile.vn[face.f0.normal-1]; vn0 = vec3(objl_vn0.x, objl_vn0.y, objl_vn0.z);
            auto objl_vn1 = ObjFile.vn[face.f1.normal-1]; vn1 = vec3(objl_vn1.x, objl_vn1.y, objl_vn1.z);
            auto objl_vn2 = ObjFile.vn[face.f2.normal-1]; vn2 = vec3(objl_vn2.x, objl_vn2.y, objl_vn2.z);

            triangles.add(make_shared<triangle>(v0, v1, v2, vn0, vn1, vn2, shade_smooth, model_material));
        }
        
        objl_FreeObj(&ObjFile);

        return make_shared<bvh_node>(triangles, 0, 1);

    } else {
        std::cerr << "Could not find .obj file at '" + filename + "'." << std::endl;
        assert(0);
    }
}

#endif
