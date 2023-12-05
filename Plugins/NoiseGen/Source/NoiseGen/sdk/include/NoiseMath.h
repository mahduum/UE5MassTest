#pragma once
#include <limits>

#include "../gcem/gcem.hpp"

#define NEAR_ZERO (1.e-4f)
#define RAD_TO_DEG (57.2957801818848f)
#define DEG_TO_RAD (0.01745329252f)
#define DIAGONAL_VECTOR_2D_NORMALIZED 0.7071067811865475
#define DIAGONAL_VECTOR_3D_NORMALIZED 0.5773502691896258

namespace NoiseMath
{
	
	namespace Detail
	{
		double constexpr sqrtNewtonRaphson(double x, double curr, double prev)
		{
			return curr == prev
				? curr
				: sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
		}
	}

	/*
	* Constexpr version of the square root
	* Return value:
	*	- For a finite and non-negative value of "x", returns an approximation for the square root of "x"
	*   - Otherwise, returns NaN
	*/

	double constexpr sqrtConstexpr(double x)
	{
		return x >= 0 && x < std::numeric_limits<double>::infinity()
			? Detail::sqrtNewtonRaphson(x, x, 0)
			: std::numeric_limits<double>::quiet_NaN();
	}

	struct Vec2_constexpr
	{
		float _X;
		float _Y;

		constexpr Vec2_constexpr(float X, float Y) : _X(X), _Y(Y){}
		
		constexpr float Length() const
		{
			return sqrtConstexpr(_X * _X + _Y * _Y);
		}
		
		constexpr Vec2_constexpr Normalized() const
		{
			const float InvLength = 1.0f / Length();
			return Vec2_constexpr(_X * InvLength, _Y * InvLength);
		}
	};

	struct Vec3_constexpr
	{
		float _X;
		float _Y;
		float _Z;

		constexpr Vec3_constexpr(float X, float Y, float Z) : _X(X), _Y(Y), _Z(Z){}
		
		constexpr float Length() const
		{
			return sqrtConstexpr(_X * _X + _Y * _Y + _Z * _Z);
		}
		
		constexpr Vec3_constexpr Normalized() const
		{
			const float InvLength = 1.0f / Length();
			return Vec3_constexpr(_X * InvLength, _Y * InvLength, _Z * InvLength);
		}
	};

	struct VecN
	{
		float X;
		float Y;
		float Z;

		/**default construction, zero*/
		VecN();
		/** construct from other Vec3 */
	    VecN(const VecN& V);
		
		VecN(float X1, float Y1);
		/** construct from 3-floats */
	    VecN(float X1, float Y1, float Z1);

		/** index into it */
	    float operator[](int Index)const;
		/** addition */
	    VecN operator+(const VecN& V)const;
		/** addition in-place */
	    VecN& operator+=(const VecN& V);
		/** subtraction */
	    VecN operator-(const VecN& V)const;
		/** subtraction in-place */
	    VecN& operator-=(const VecN& V);
		/** multiplication */
	    VecN operator*(float Scalar)const;
		/** multiplication in-place */
	    VecN& operator*=(float Scalar);
		/** division */
	    VecN operator/(float Scalar)const;
		/** division in-place */
	    VecN& operator/=(float Scalar);
		/** assignment */
	    VecN& operator=(const VecN& V);
		
		/** set component Scalars */
	    void Set(float X1, float Y1, float Z1);
		/** dot product w/ other vector */
	    float Dot(const VecN& V)const;
		/** cross product w/ other vector */
	    VecN Cross(const VecN& V)const;
		/** magnitude of the vector */
	    float Length()const;
		/** squared magnitude */
	    float LengthSq()const;
		/** distance to other vector */
	    float Distance(const VecN& V)const;
		/** get normalized vector (fast/unsafe, no Zero length check!) */
	    VecN Normalized()const;
		/** get normalized vector, checks for zero length */
	    VecN NormalizedSafe(float Tolerance = NEAR_ZERO)const;
		/** normalize in-place, checks for zero length */
	    VecN& NormalizeSafe(float Tolerance = NEAR_ZERO);
		/** get a normalized direction vector and length */
		void GetDirectionAndLength(VecN& OutDirection, float& OutLength, float Tolerance = NEAR_ZERO);
		
		/** lerp between two vectors */
		static VecN Lerp(const VecN& A, const VecN& B, const float t);
		/** dot product of two vectors */
		static float Dot(const VecN& A, const VecN& B);
		/** cross product of two vectors */
		static VecN Cross(const VecN& A, const VecN& B);
		/** create new zero vector */
		static VecN Zero() { return VecN(0.0f, 0.0f, 0.0f); }
		/** create new one vector */
		static VecN One() { return VecN(1.0f, 1.0f, 1.0f); }
		/** project A onto a normal vector */
		static VecN Project(const VecN& A, const VecN& OnNormal);
		/** project A onto a plane */
		static VecN ProjectPointOnPlane(const VecN& A, const VecN& PlaneOrigin, const VecN& PlaneNormal);
		/** get angle in degrees between A and B (assumed to be normalized) */
		static float AngleBetweenNormals(const VecN& A, const VecN& B);
	};

	VecN operator*(const float Scalar, const VecN& A);


	struct Quat
	{
		float X;
		float Y;
		float Z;
		float W;

		/** default construction, uninitialized for speed */
	    Quat();
		/** construct from other Quat */
	    Quat(const Quat& Q);
		/** construct from 4-floats */
	    Quat(float InX, float InY, float InZ, float InW);
		
		/** add quaternions component-wise */
	    Quat operator+(const Quat& Q) const;
		/** multiply by scalar */
	    Quat operator*(const float Multiplier) const;
		/** quaternion multiplication (represents a composed rotation of this, followed by Q) */
	    Quat operator*(const Quat& Q) const;
		
		/** inverse, in-place */
	    void Invert();
		/** inverse, return copy */
	    Quat Inverse();
		/** normalize in-place */
	    Quat& Normalize(float Tolerance = NEAR_ZERO);
		/** rotate a vector by this */
	    VecN Rotate(const VecN& V) const;
		
		/** set this quaternion based on an axis and angle (in degrees)*/
		static Quat FromAxisAngleDegrees(const VecN& Axis, const float AngleDegrees);
		/** set this quaternion based on an axis and angle (in radians)*/
		static Quat FromAxisAngleRadians(const VecN& Axis, const float AngleRadians);
		/** lerp between two quaternions */
		static Quat Lerp(const Quat& A, const Quat& B, const float Alpha);
		/** create quaternion that rotates from A to B */
		static Quat FromToRotation(const VecN& A, const VecN& B);
		/** dot product of two vectors */
		static float Dot(const Quat& A, const Quat& B);
		/** are two quaternions close to each other? */
		static bool AreClose(const Quat& A, const Quat& B);
		/** create new zero vector */
		static Quat Identity() { return Quat(0.0f, 0.0f, 0.0f, 1.0f); }
		/** angle in degrees between two quaternions */
		static float AngleDegrees(const Quat& A, const Quat& B);
	};

	struct Transform
	{
		VecN Position;
		Quat Rotation;

		Transform()
		{
			Position = VecN::Zero();
			Rotation = Quat::Identity();
		}
	};

	/**
	* Linearly interpolate between two float values
	* 
	* @param: A - start value
	* @param: B - end value
	* @param: T - parameter (0 at A and 1 at B)
	* @return: float - result interpolated from A to B
	*/
	float ScalarLerp(const float A, const float B, const float T);


	/**
	* Move a position towards a target position with linear velocity
	* (will not overshoot target)
	* 
	* @param: Current - current position
	* @param: Target - position to move towards
	* @param: MaxSpeed - speed in cm/second to move towards
	* @param: MaxDistance - clamp maximum distance between current and target
	* @param: DeltaTime - seconds since last tick
	* @return: PowerIK::Vec3 - the new position moved towards target
	*/
	VecN MoveTowards(
		const VecN& Current,
		const VecN& Target,
		const float MaxSpeed,
		const float MaxDistance,
		const float DeltaTime);

	/**
	* Move a position towards a target position with non-linear velocity decreasing towards target
	* (will not overshoot target)
	*
	* @param: Current - current position
	* @param: Target - position to move towards
	* @param: MaxSpeed - speed in cm/second to move towards
	* @param: MaxDistance - clamp maximum distance between current and target
	* @param: DeltaTime - seconds since last tick
	* @return: PowerIK::Vec3 - the new position moved towards target
	*/
	VecN MoveTowardsSmooth(
		const VecN& Current,
		const VecN& Target,
		const float SlowFactor);
		
	/**
	* Rotate a quaternion towards a target quaternion with linear angular velocity
	* (will not overshoot target)
	* 
	* @param: Current - current rotation
	* @param: Target - rotation to rotate towards
	* @param: MaxAngularSpeed - speed in degrees/second to rotate towards target
	* @param: DeltaTime - seconds since last tick
	* @return: PowerIK::Quat - the new rotation, closer to target
	*/
	Quat RotateTowards(
		const Quat& Current,
		const Quat& Target,
		const float MaxAngularSpeed,
		const float DeltaTime);
		
	/**
	* Decompose rotation between two normals into separate pitch/yaw, relative to a forward vector
	* 
	* @param: Fwd - normalized vector pointing in forward direction
	* @param: Up - normalized vector pointing up 
	* @param: Normal - normalized vector pointing away from up (the orientation normal)
	* @param: OutSide - outputs vector perpendicular to normal and forward (sideways vector)
	* @param: OutVertNormal - rotated up vector projected into plane of pitch
	* @param: OutSideNormal - rotated side vector projected into plane of roll
	* @param: OutPitchRotation - quaternion with only pitch
	* @param: OutRollRotation - quaternion with only roll
	* @param: OutPitchedFwdVec - the forward vector rotated by pitch
	* @param: OutRolledSideVec - the side vector rotated by roll
	*/
	void PitchYawFromNormal(
		const VecN& Fwd,
		const VecN& Up,
		const VecN& Normal,
		VecN& OutSide,
		VecN& OutVertNormal,
		VecN& OutSideNormal,
		Quat& OutPitchRotation,
		Quat& OutRollRotation,
		VecN& OutPitchedFwdVec,
		VecN& OutRolledSideVec);

	/**
	* Clamp the input vector to be within MaxAngle of "RelativeTo" vector
	*
	* @param: RelativeTo - normalized vector to measure angle relative to
	* @param: OutVector -normalized  vector to clamp within MaxAngle of RelativeTo
	* @param: MaxAngle - maximum angle, in degrees, that OutVector can deviate from RelativeTo
	*/
	void ClampAngleBetweenNormals(
		const VecN& RelativeTo,
		const float MaxAngle,
		VecN& OutVector);
		


}

