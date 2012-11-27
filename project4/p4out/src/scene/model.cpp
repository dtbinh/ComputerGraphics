/**
 * @file model.cpp
 * @brief Model class
 *
 * @author Eric Butler (edbutler)
 * @author Zeyang Li (zeyangl)
 */

#include "scene/model.hpp"
#include "scene/material.hpp"
#include <GL/gl.h>
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>


namespace _462 {

Model::Model() : mesh( 0 ), material( 0 ) {

}
Model::~Model() { }

void Model::render() const
{
    if ( !mesh )
        return;
    if ( material )
        material->set_gl_state();
    mesh->render();
    if ( material )
        material->reset_gl_state();
}

bool Model::intersectTest(Ray ray, real_t t0, real_t t1, HitRecord* hitRecord) 
{
	
	
	Ray instancingRay;
	instancingRay.eyePosition = inverseMtx.transform_point(ray.eyePosition);
	instancingRay.rayDirection = inverseMtx.transform_vector(ray.rayDirection);
	if(boundingBoxIntersectTest(instancingRay,t0, t1)){
		Vector3 vtx1, vtx2, vtx3;
		Vector2 texCoord;
		bool intersected = false;
		real_t tMax = t1;
		for(int i = 0; i < mesh->num_triangles(); i++){
			MeshTriangle triangle = (mesh->get_triangles())[i];
			if(mesh->triangleIntersectTest(instancingRay, triangle, t0, tMax, hitRecord, texCoord)){
				tMax = hitRecord->time;
				intersected = true;
			}
		}
		if(!intersected){
			return false;
		}else{
			hitRecord->normal = normalize(normalMtx * hitRecord->normal);
			hitRecord->diffColor = material->diffuse; 
			hitRecord->refractiveIndex = material->refractive_index;
			hitRecord->ambColor = material->ambient; 
			hitRecord->specularColor = material->specular;
			int width, height;
			material->get_texture_size(&width, &height);
			if(texCoord.x > 1 || texCoord.x < 0)
				texCoord.x = texCoord.x - (int)texCoord.x;
			if(texCoord.y > 1 || texCoord.y < 0)
				texCoord.y = texCoord.y - (int)texCoord.y;
			hitRecord->textureColor = material->get_texture_pixel((int)(texCoord.x * width), (int)(texCoord.y * height));
			return true;
		}
	}
	return false;
}

void Model::bounding_box(){
	boundingBox.max = mesh->getMax((mesh->get_triangles())[0]);
	boundingBox.min = mesh->getMin((mesh->get_triangles())[0]);
	for(int i = 1; i < mesh->num_triangles(); i++){
		boundingBox.max = vmax(boundingBox.max, mesh->getMax((mesh->get_triangles())[i]));
		boundingBox.min = vmin(boundingBox.max, mesh->getMin((mesh->get_triangles())[i]));
	}


}


} /* _462 */

