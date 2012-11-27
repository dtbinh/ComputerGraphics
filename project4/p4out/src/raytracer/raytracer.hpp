/**
 * @file raytacer.hpp
 * @brief Raytracer class
 *
 * Implement these functions for project 2.
 *
 * @author H. Q. Bovik (hqbovik)
 * @bug Unimplemented
 */

#ifndef _462_RAYTRACER_HPP_
#define _462_RAYTRACER_HPP_

#include "math/color.hpp"
#include "math/vector.hpp"

namespace _462 {

	struct Ray
	{
		Vector3 eyePosition;
		Vector3 rayDirection;
	};

	struct HitRecord
	{
		//hit time
		real_t time;
		//the normal of hit point
		Vector3 normal;
		//color information
		Color3 diffColor;
		Color3 ambColor;
		Color3 specularColor;
		Color3 textureColor;
		//refractive index
		real_t refractiveIndex;
	};

class Scene;

class Raytracer
{
public:

    Raytracer();

    ~Raytracer();

    bool initialize( Scene* scene, size_t width, size_t height );

    bool raytrace( unsigned char* buffer, real_t* max_time );

private:
	real_t r,l, t, b;
	Vector3 coorU, coorV, coorW;
	Vector3 getRayDirection(const Scene* scene, size_t px, size_t py, size_t width, size_t height);
	
	// the scene to trace
    Scene* scene;

    // the dimensions of the image to trace
    size_t width, height;

    // the next row to raytrace
    size_t current_row;
};

} /* _462 */

#endif /* _462_RAYTRACER_HPP_ */

