#include "physics/collisions.hpp"

namespace _462 {
Vector3 relativeVelocity;
Vector3 direction;
real_t epsilon = 0.01;


void VelCorrection(SphereBody& sbody)
{
	double speed = length(sbody.velocity);
	if(speed < epsilon){
		sbody.velocity = Vector3::Zero;
	}
}
bool collides( SphereBody& body1, SphereBody& body2, real_t collision_damping )
{
    // TODO detect collision. If there is one, update velocity
	Vector3 relativeVelocity = body1.velocity - body2.velocity;
	direction = (body2.position - body1.position)/ length(body2.position - body1.position);
	if(dot(direction, relativeVelocity) < 0) //not moving towards each other
	 return false;
	else{
		if(distance(body1.position, body2.position) < (body1.radius + body2.radius)){ // collision
			Vector3 newV1, newV2; //new velocity of each object
			newV2 = body2.velocity + 2 * direction * body1.mass / (body1.mass + body2.mass) * dot(relativeVelocity, direction);
			newV1 = (body1.mass * body1.velocity + body2.mass * body2.velocity - body2.mass * newV2) / body1.mass;
			body1.velocity = newV1  * ( 1 - collision_damping);
			body2.velocity = newV2  * ( 1 - collision_damping);
			if(length(body1.velocity) < epsilon)
				body1.velocity = Vector3(0,0,0);
			if(length(body2.velocity) < epsilon)
				body2.velocity = Vector3(0,0,0);
			return true;
		}
		else 
			return false;
	}
    
}

bool collides( SphereBody& body1, TriangleBody& body2, real_t collision_damping )
{
    // TODO detect collision. If there is one, update velocity
	Vector3 triangleNormal = normalize(cross(body2.vertices[1] - body2.vertices[0], body2.vertices[2] - body2.vertices[0]));
	relativeVelocity = body1.velocity - body2.velocity;
	if(dot(relativeVelocity, triangleNormal) > 0)
		return false;

	// baricentric coordinates
    real_t u, v; 
    real_t dist;
	real_t a, b, c, d, e, f, g, h, i, j, k ,l,M;
    a = body2.vertices[0].x - body2.vertices[1].x;
    b = body2.vertices[0].y - body2.vertices[1].y;
    c = body2.vertices[0].z - body2.vertices[1].z;
    d = body2.vertices[0].x - body2.vertices[2].x;
    e = body2.vertices[0].y - body2.vertices[2].y;
    f = body2.vertices[0].z - body2.vertices[2].z;
    g = -triangleNormal.x;
    h = -triangleNormal.y;
    i = -triangleNormal.z;
    j = body2.vertices[0].x -  body1.position.x;
    k = body2.vertices[0].y -  body1.position.y;
    l = body2.vertices[0].z -  body1.position.z;
    M = a*(e*i-h*f)+b*(g*f-d*i)+c*(d*h-e*g);
    dist = -(f*(a*k-j*b)+e*(j*c-a*l)+d*(b*l-k*c))/M;
    u = (j*(e*i-h*f)+k*(g*f-d*i)+l*(d*h-e*g))/M;
	v = (i*(a*k-j*b)+h*(j*c-a*l)+g*(b*l-k*c))/M;

	if((u<0) || (u>1) || (v<0) || (v>(1-u))){
	// the projection of the center of the sphere is out of the triangle
	// compute the distance from the center to each of edges the triangle
	// if the distance is less than the radius, then collision happens
		Vector3 projectionPoint = body1.position - triangleNormal * dist;
		for( int i = 0; i < 3; i++ ){
			Vector3 vertice1 = body2.vertices[i];
			for( int j = 0; (j < 3) && (j > i); j++){
				Vector3 vertice2 = body2.vertices[j];
				Vector3 projectionOnEdge = dot(projectionPoint - vertice1, vertice2 - vertice1)* ( vertice2 - vertice1)/length(vertice2 -vertice1) + vertice1;
				real_t distanceToEdge = length(projectionOnEdge - body1.position);
				if(distanceToEdge < body1.radius){ 
					body1.velocity = body1.velocity - 2* dot(body1.velocity, triangleNormal) * triangleNormal * (1 - collision_damping);
					if(length(body1.velocity) < epsilon)
						body1.velocity = Vector3(0,0,0);
					return true;
				}
			}
		}
		return false;			
	}
	else{
	// the projection of the center of the sphere is within the triangle
	// compute the distance from the sphere center to the plane
	// if less than radius, then collides
		dist = (dist>0)?dist:(-dist);
		if(dist < body1.radius){
			body1.velocity = body1.velocity - 2* dot(body1.velocity, triangleNormal) * triangleNormal * (1 - collision_damping);
			if(length(body1.velocity) < epsilon)
				body1.velocity = Vector3(0,0,0);
			return true;
		}
		else{
			return false;
		}
	}
	
}

bool collides( SphereBody& body1, PlaneBody& body2, real_t collision_damping )
{
    // TODO detect collision. If there is one, update velocity
	relativeVelocity = body1.velocity - body2.velocity;
	direction = body2.position - body1.position;
	if(dot(relativeVelocity, body2.normal) > 0) //not moving towards each other
		return false;
	else{
		//project the center of sphere on to the plane
		real_t distance = dot(body1.position - body2.position, body2.normal);
		if(abs(distance) < body1.radius){ //collision
			body1.velocity =( body1.velocity - 2 * dot(body1.velocity, body2.normal) * body2.normal) * ( 1 - collision_damping);
			if(length(body1.velocity) < epsilon)
				body1.velocity = Vector3(0,0,0);
			return true;
		}
		else 
			return false;
	}
}



}
