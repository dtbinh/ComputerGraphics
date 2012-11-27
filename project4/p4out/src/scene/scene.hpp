/**
 * @file scene.hpp
 * @brief Class definitions for scenes.
 *
 * @author Eric Butler (edbutler)
 * @author Kristin Siu (kasiu)
 */

#ifndef _462_SCENE_SCENE_HPP_
#define _462_SCENE_SCENE_HPP_

#include "math/vector.hpp"
#include "math/quaternion.hpp"
#include "math/matrix.hpp"
#include "math/camera.hpp"
#include "scene/material.hpp"
#include "scene/mesh.hpp"
#include "raytracer/raytracer.hpp"
#include <string>
#include <vector>

namespace _462 {


class Geometry
{
public:
    Geometry();
    virtual ~Geometry();

    /*
       World transformation are applied in the following order:
       1. Scale
       2. Orientation
       3. Position
    */

    // The world position of the object.
    Vector3 position;

    // The world orientation of the object.
    // Use Quaternion::to_matrix to get the rotation matrix.
    Quaternion orientation;

    // The world scale of the object.
    Vector3 scale;

	Matrix4 transformMtx, inverseMtx,transposeMtx;
	Matrix3 normalMtx;
	struct box
	{
		Vector3 min;
		Vector3 max;
	};

	box boundingBox;

    /**
     * Renders this geometry using OpenGL in the local coordinate space.
     */
    virtual void render() const = 0;
	virtual void bounding_box() = 0;
	
	virtual bool intersectTest(Ray ray, real_t t0, real_t t1, HitRecord *hitRecord) = 0;
	void matrixInitialize(){
		make_transformation_matrix(&transformMtx, position, orientation, scale);
		make_inverse_transformation_matrix(&inverseMtx, position, orientation, scale);
		transpose(&transposeMtx, inverseMtx);
		make_normal_matrix(&normalMtx, transformMtx);
	};

	float max(float f1, float f2){
		if(f1 >= f2)
			return f1;
		else
			return f2;
	}

	float min(float f1, float f2){
		if(f1 <= f2)
			return f1;
		else
			return f2;
	}

	bool boundingBoxIntersectTest(Ray ray, real_t t0, real_t t1){
		real_t xmin, xmax, ymin, ymax, zmin, zmax;
		real_t xa, ya, za;
		xa = 1/ray.rayDirection.x;
		if(xa >= 0){
			xmin = xa * (boundingBox.min.x - ray.eyePosition.x);
			xmax = xa * (boundingBox.max.x - ray.eyePosition.x);
		}else{
			xmin = xa * (boundingBox.max.x - ray.eyePosition.x);
			xmax = xa * (boundingBox.min.x - ray.eyePosition.x);
		}

		ya = 1/ray.rayDirection.y;
		if(ya >= 0){
			ymin = ya * (boundingBox.min.y - ray.eyePosition.y);
			ymax = ya * (boundingBox.max.y - ray.eyePosition.y);
		}else{
			ymin = ya * (boundingBox.max.y - ray.eyePosition.y);
			ymax = ya * (boundingBox.min.y - ray.eyePosition.y);
		}

		za = 1/ray.rayDirection.z;
		if(za >= 0){
			zmin = za * (boundingBox.min.z - ray.eyePosition.z);
			zmax = za * (boundingBox.max.z - ray.eyePosition.z);
		}else{
			zmin = za * (boundingBox.max.z - ray.eyePosition.z);
			zmax = za * (boundingBox.min.z - ray.eyePosition.z);
		}

		if ( (xmin > ymax) || (ymin > xmax) ) 
			return false;
		if (ymin > xmin)
			xmin = ymin;
		if (ymax < xmax)
			xmax = ymax;
		if ( (xmin > zmax) || (zmin > xmax) ) 
			return false;
		if (zmin > xmin)
			xmin = zmin;
		if (zmax < xmax)
			xmax = zmax;
		return ( (xmin < t1) && (xmax > t0) );
	
	}
};


struct PointLight
{
    struct Attenuation
    {
        real_t constant;
        real_t linear;
        real_t quadratic;
    };

    PointLight();

    // The position of the light, relative to world origin.
    Vector3 position;
    // The color of the light (both diffuse and specular)
    Color3 color;
    // attenuation
    Attenuation attenuation;
};

/**
 * The container class for information used to render a scene composed of
 * Geometries.
 */
class Scene
{
public:

    /// the camera
    Camera camera;
    /// the background color
    Color3 background_color;
    /// the amibient light of the scene
    Color3 ambient_light;
    /// the refraction index of air
    real_t refractive_index;

    /// Creates a new empty scene.
    Scene();

    /// Destroys this scene. Invokes delete on everything in geometries.
    ~Scene();

    // accessor functions
    Geometry* const* get_geometries() const;
    size_t num_geometries() const;
    const PointLight* get_lights() const;
    size_t num_lights() const;
    Material* const* get_materials() const;
    size_t num_materials() const;
    Mesh* const* get_meshes() const;
    size_t num_meshes() const;

    /// Clears the scene, and invokes delete on everything in geometries.
    void reset();

    // functions to add things to the scene
    // all pointers are deleted by the scene upon scene deconstruction.
    void add_geometry( Geometry* g );
    void add_material( Material* m );
    void add_mesh( Mesh* m );
    void add_light( const PointLight& l );

private:

    typedef std::vector< PointLight > PointLightList;
    typedef std::vector< Material* > MaterialList;
    typedef std::vector< Mesh* > MeshList;
    typedef std::vector< Geometry* > GeometryList;

    // list of all lights in the scene
    PointLightList point_lights;
    // all materials used by geometries
    MaterialList materials;
    // all meshes used by models
    MeshList meshes;
    // list of all geometries. deleted in dctor, so should be allocated on heap.
    GeometryList geometries;

private:

    // no meaningful assignment or copy
    Scene(const Scene&);
    Scene& operator=(const Scene&);

};

} /* _462 */

#endif /* _462_SCENE_SCENE_HPP_ */

