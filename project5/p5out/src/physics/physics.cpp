#include "physics/physics.hpp"

namespace _462 {

Physics::Physics()
{
    reset();
}

Physics::~Physics()
{
    reset();
}


void setPosition(Derivative& previous, Derivative& current, Derivative& initial, real_t dt){
	current.velocity = initial.velocity + previous.velocity * dt;
	current.position = initial.position + previous.position * dt;
	current.angular_velocity = initial.angular_velocity + previous.angular_velocity * dt;
	current.angle = initial.angle + previous.angle * dt;
	if(current.angle != Vector3(0,0,0)){
		current.orientation = initial.orientation * Quaternion(normalize(current.angle), length(current.angle));
	}else{
		current.orientation = previous.orientation;
	}
}

//calculate the differetial 
void Evaluate(Derivative& derivative, SphereBody& sphere, real_t dt){
	derivative.position = derivative.velocity * dt;
	derivative.velocity = sphere.force/sphere.mass * dt;
	
	real_t angular_momentum = (2.0 / 5.0 * sphere.mass * sphere.radius * sphere.radius);
	derivative.angle = derivative.angular_velocity * dt;
	derivative.angular_velocity = sphere.torque / angular_momentum * dt;
	
}

void resetForce(SphereBody &sphere){
	sphere.force = Vector3(0,0,0);
	sphere.torque = Vector3(0,0,0);
}

//update the state of the spherebody for the calculation of spring
void updateState(SphereBody &sphere, Derivative &derivative){
	sphere.angular_velocity = derivative.angular_velocity;
	sphere.position = derivative.position;
	sphere.velocity = derivative.velocity;
	sphere.orientation = derivative.orientation;
}


void Physics::step( real_t dt )
{
    // TODO step the world forward by dt. Need to detect collisions, apply
    // forces, and integrate positions and orientations.
    //
    // Note: put RK4 here, not in any of the physics bodies
    //
    // Must use the functions that you implemented
    //
    // Note, when you change the position/orientation of a physics object,
    // change the position/orientation of the graphical object that represents
    // it
	Derivative initial;
	Derivative k1, k2, k3, k4;
	Vector3 sumAngles;
	for(unsigned int i = 0; i < spheres.size(); i++){
		initial.velocity = spheres[i]->velocity;
		initial.angular_velocity = spheres[i]->angular_velocity;
		initial.position = spheres[i]->position;
		initial.orientation = spheres[i]->orientation;
		initial.angle = Vector3(0,0,0);
		resetForce(*spheres[i]);
		
		/****************************RK4**************************************/
		/* k1 */
		setPosition(initial, k1, initial, 0);
		updateState(*spheres[i], k1);
		spheres[i]->apply_force(gravity, Vector3(0,0,0));
		for(unsigned int j = 0; j < springs.size(); j++){
			if(springs[j]->body1->id == spheres[i]->id || springs[j]->body2->id == spheres[i]->id)
					springs[j]->step(dt);
		}
		Evaluate(k1, *spheres[i], dt);
		resetForce(*spheres[i]);
		

		/* k2 */
		setPosition(k1, k2, initial, dt/2);
		updateState(*spheres[i], k2);
		spheres[i]->apply_force(gravity, Vector3(0,0,0));
		for(unsigned int j = 0; j < springs.size(); j++){
			if(springs[j]->body1->id == spheres[i]->id || springs[j]->body2->id == spheres[i]->id)
					springs[j]->step(dt/2);
		}
		Evaluate(k2, *spheres[i], dt);
		resetForce(*spheres[i]);
		

		/* k3 */
		setPosition(k2, k3, initial, dt/2);
		updateState(*spheres[i], k3);
		spheres[i]->apply_force(gravity, Vector3(0,0,0));
		for(unsigned int j = 0; j < springs.size(); j++){
			if(springs[j]->body1->id == spheres[i]->id || springs[j]->body2->id == spheres[i]->id)
					springs[j]->step(dt/2);
		}
		Evaluate(k3, *spheres[i], dt);
		resetForce(*spheres[i]);
		
		/* k4 */
		setPosition(k3, k4, initial, dt);
		updateState(*spheres[i], k4);
		spheres[i]->apply_force(gravity, Vector3(0,0,0));
		for(unsigned int j = 0; j < springs.size(); j++){
			if(springs[j]->body1->id == spheres[i]->id || springs[j]->body2->id == spheres[i]->id)
					springs[j]->step(dt);
		}
		Evaluate(k4, *spheres[i], dt);
		resetForce(*spheres[i]);
	

		/***************udpate position and orientation after RK4***************/
		spheres[i]->position = initial.position + 1.0 / 6.0 * (k1.position + 2.0 * (k2.position + k3.position) + k4.position);
		spheres[i]->velocity = initial.velocity + 1.0 / 6.0 * (k1.velocity + 2.0 * (k2.velocity + k3.velocity) + k4.velocity);
		spheres[i]->angular_velocity = initial.angular_velocity + 1.0 / 6.0 * (k1.angular_velocity + 2.0 * (k2.angular_velocity + k3.angular_velocity) + k4.angular_velocity);

		sumAngles = 1.0 / 6.0 * (k1.angle + 2.0 * (k2.angle + k3.angle) + k4.angle);
		if(sumAngles != Vector3(0,0,0)){
			spheres[i]->orientation = initial.orientation * Quaternion(normalize(sumAngles), length(sumAngles));	
		}
		else{
			spheres[i]->orientation = initial.orientation;
		}

		spheres[i]->sphere->position = spheres[i]->position;
		spheres[i]->sphere->orientation = spheres[i]->orientation;

		
		/*******collision detection************/
		for(unsigned int j = 0; j < spheres.size(); j++){
			if(spheres[i]->id != spheres[j]->id){
				collides(*spheres[i], *spheres[j], collision_damping);
			}
		}

		for(unsigned int j = 0; j < triangles.size(); j++){
			collides(*spheres[i], *triangles[j], collision_damping);
		}

		for(unsigned int j = 0; j < planes.size(); j++){
			collides(*spheres[i], *planes[j], collision_damping);
		}
		
	}
}

void Physics::add_sphere( SphereBody* b )
{
    spheres.push_back( b );
}

size_t Physics::num_spheres() const
{
    return spheres.size();
}

void Physics::add_plane( PlaneBody* p )
{
    planes.push_back( p );
}

size_t Physics::num_planes() const
{
    return planes.size();
}

void Physics::add_triangle( TriangleBody* t )
{
    triangles.push_back( t );
}

size_t Physics::num_triangles() const
{
    return triangles.size();
}

void Physics::add_spring( Spring* s )
{
    springs.push_back( s );
}

size_t Physics::num_springs() const
{
    return springs.size();
}

void Physics::reset()
{
    for ( SphereList::iterator i = spheres.begin(); i != spheres.end(); i++ ) {
        delete *i;
    }
    for ( PlaneList::iterator i = planes.begin(); i != planes.end(); i++ ) {
        delete *i;
    }
    for ( TriangleList::iterator i = triangles.begin(); i != triangles.end(); i++ ) {
        delete *i;
    }
    for ( SpringList::iterator i = springs.begin(); i != springs.end(); i++ ) {
        delete *i;
    }

    spheres.clear();
    planes.clear();
    triangles.clear();
    springs.clear();
    
    gravity = Vector3::Zero;
	collision_damping = 0.0;
}

}
