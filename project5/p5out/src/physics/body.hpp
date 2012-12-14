#ifndef _462_PHYSICS_BODY_HPP_
#define _462_PHYSICS_BODY_HPP_

#include "math/vector.hpp"
#include "math/quaternion.hpp"
#include <exception>
#include <iostream>

namespace _462 {
struct Derivative
	{
		Vector3 velocity;
		Vector3 angular_velocity;
		Vector3 angle;
		Vector3 position;
		Quaternion orientation;

		Derivative& operator*( real_t dt ) {
		   velocity *= dt;
		   angular_velocity *= dt;
		   angle *= dt;
		   position *= dt;
		   orientation = orientation;
		   return *this;
		}

		Derivative& operator/( real_t dt ) {
		   velocity /= dt;
		   angular_velocity /= dt;
		   angle /= dt;
		   position /= dt;
		   orientation = orientation;
           return *this;
		}

		Derivative& operator+(Derivative &rhs) {
			velocity += rhs.velocity;
			angular_velocity += rhs.angular_velocity;
			angle += rhs.angle;
			position += rhs.angle;
			orientation = orientation;
            return *this;
		}
	};
class Body
{
public:
    int id;
    int type;
    Vector3 position;
    Quaternion orientation;
    Vector3 velocity;
    Vector3 angular_velocity;

    virtual ~Body() { }
    virtual Vector3 step_position( real_t dt, real_t motion_damping ) = 0;
    virtual Vector3 step_orientation( real_t dt, real_t motion_damping ) = 0;
    virtual void apply_force( const Vector3& f, const Vector3& offset ) = 0;
};

}

#endif
