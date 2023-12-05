#include "../include/NoiseMath.h"

#include <math.h>
#include <algorithm>

namespace NoiseMath
{

	//
	// Vec3 implementation
	//

	VecN::VecN() : X(0), Y(0), Z(0)
	{
	}

	VecN::VecN(const VecN& V)
	{
		X = V.X;
		Y = V.Y;
		Z = V.Z;
	}

	VecN::VecN(float X1, float Y1)
	{
		X = X1;
		Y = Y1;
		Z = 0;
	}

	VecN::VecN(float X1, float Y1, float Z1)
	{
		X = X1;
		Y = Y1;
		Z = Z1;
	}

	float VecN::operator[](int Index) const
	{
		Index = std::min<int>(std::max<int>(0, Index), 2);
		return (&X)[Index];
	}

	VecN VecN::operator+(const VecN& V) const
	{
		return VecN(X + V.X, Y + V.Y, Z + V.Z);
	}

	VecN& VecN::operator+=(const VecN& V)
	{
		X += V.X;
		Y += V.Y;
		Z += V.Z;
		return *this;
	}

	VecN VecN::operator-(const VecN& V) const
	{
		return VecN(X - V.X, Y - V.Y, Z - V.Z);
	}

	VecN& VecN::operator-=(const VecN& V)
	{
		X -= V.X;
		Y -= V.Y;
		Z -= V.Z;
		return *this;
	}

	VecN VecN::operator*(float Scalar) const
	{
		return VecN(X * Scalar, Y * Scalar, Z * Scalar);
	}

	VecN& VecN::operator*=(float Scalar)
	{
		X *= Scalar;
		Y *= Scalar;
		Z *= Scalar;
		return *this;
	}

	VecN VecN::operator/(float Scalar) const
	{
		return VecN(X / Scalar, Y / Scalar, Z / Scalar);
	}

	VecN& VecN::operator/=(float Scalar)
	{
		float Multiplier = 1.0f / Scalar;
		X *= Multiplier;
		Y *= Multiplier;
		Z *= Multiplier;
		return *this;
	}

	VecN& VecN::operator=(const VecN& V)
	{
		X = V.X;
		Y = V.Y;
		Z = V.Z;
		return *this;
	}

	void VecN::Set(float X1, float Y1, float Z1)
	{
		X = X1;
		Y = Y1;
		Z = Z1;
	}

	float VecN::Dot(const VecN& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z;
	}

	VecN VecN::Cross(const VecN& V) const
	{
		float X1 = Y * V.Z - Z * V.Y;
		float Y1 = Z * V.X - X * V.Z;
		float Z1 = X * V.Y - Y * V.X;
		return VecN(X1, Y1, Z1);
	}

	VecN VecN::Project(const VecN& A, const VecN& OnNormal)
	{
		float LenSq = Dot(OnNormal, OnNormal);
		if (LenSq <= NEAR_ZERO)
		{
			return VecN::Zero();
		}

		float ADotNorm = Dot(A, OnNormal);
		float Scale = ADotNorm / LenSq;
		return OnNormal * Scale;
	}

	VecN VecN::ProjectPointOnPlane(
		const VecN& A, 
		const VecN& PlaneOrigin, 
		const VecN& PlaneNormal)
	{
		VecN OriginToA = A - PlaneOrigin;
		VecN AOnNormal = Project(OriginToA, PlaneNormal);
		return PlaneOrigin + (A - (PlaneOrigin + AOnNormal));
	}

	float VecN::AngleBetweenNormals(const VecN& A, const VecN& B)
	{
		return acosf(VecN::Dot(A, B)) * RAD_TO_DEG;
	}

	float VecN::Length() const
	{
		return sqrtf(X * X + Y * Y + Z * Z);
	}

	float VecN::LengthSq() const
	{
		return X * X + Y * Y + Z * Z;
	}

	float VecN::Distance(const VecN& V) const
	{
		float X1 = X - V.X;
		float Y1 = Y - V.Y;
		float Z1 = Z - V.Z;
		return sqrtf(X1 * X1 + Y1 * Y1 + Z1 * Z1);
	}

	VecN VecN::Normalized() const
	{
		float InvLength = 1.0f / Length();
		return VecN(X * InvLength, Y * InvLength, Z * InvLength);
	}

	VecN VecN::NormalizedSafe(float Tolerance) const
	{
		const float LengthSq = X * X + Y * Y + Z * Z;

		if (LengthSq == 1.f)
		{
			return VecN(X, Y, Z); // already normalized
		}
			
		if (LengthSq < Tolerance)
		{
			return VecN(1.0f, 0.0f, 0.0f);	// avoid divide by zero
		}

		const float Multiplier = 1.0f / sqrtf(LengthSq); // TODO optimize with fast inv sqrt
		return VecN(X * Multiplier, Y * Multiplier, Z * Multiplier);
	}

	VecN& VecN::NormalizeSafe(float Tolerance)
	{
		const float LengthSq = X * X + Y * Y + Z * Z;

		if (LengthSq == 1.f)
		{
			return *this; // already normalized
		}

		if (LengthSq < Tolerance)
		{
			return *this; // avoid divide by zero
		}

		const float Multiplier = 1.0f / sqrtf(LengthSq); // TODO optimize with fast inv sqrt
		X *= Multiplier;
		Y *= Multiplier;
		Z *= Multiplier;

		return *this;
	}

	void VecN::GetDirectionAndLength(VecN& OutDirection, float& OutLength, float Tolerance)
	{
		OutLength = Length();
		if (OutLength > Tolerance)
		{
			float InvLength = 1.0f / OutLength;
			OutDirection = VecN(X * InvLength, Y * InvLength, Z * InvLength);
		}
		else
		{
			OutDirection = VecN::Zero();
		}
	}

	VecN operator*(const float Scalar, const VecN& A)
	{
		return VecN(A.X * Scalar, A.Y * Scalar, A.Z * Scalar);
	}


	VecN VecN::Lerp(const VecN& A, const VecN& B, const float T)
	{
		const float X = A.X + T * (B.X - A.X);
		const float Y = A.Y + T * (B.Y - A.Y);
		const float Z = A.Z + T * (B.Z - A.Z);
		return VecN(X, Y, Z);
	}

	float VecN::Dot(const VecN& A, const VecN& B)
	{
		return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
	}

	VecN VecN::Cross(const VecN& A, const VecN& B)
	{
		const float X1 = A.Y * B.Z - A.Z * B.Y;
		const float Y1 = A.Z * B.X - A.X * B.Z;
		const float Z1 = A.X * B.Y - A.Y * B.X;
		return VecN(X1, Y1, Z1);
	}

	//
	// Quat implementation
	//

	Quat::Quat() : X(0), Y(0), Z(0), W(1)
	{
	}

	Quat::Quat(const Quat& Q)
	{
		X = Q.X;
		Y = Q.Y;
		Z = Q.Z;
		W = Q.W;
	}

	Quat::Quat(float InX, float InY, float InZ, float InW)
	{
		X = InX;
		Y = InY;
		Z = InZ;
		W = InW;
	}

	Quat Quat::operator+(const Quat& Q) const
	{
		return Quat(X + Q.X, Y + Q.Y, Z + Q.Z, W + Q.W);
	}

	void Quat::Invert()
	{
		X = -X;
		Y = -Y;
		Z = -Z;
	}

	Quat Quat::Inverse()
	{
		return Quat(-X, -Y, -Z, W);
	}

	Quat Quat::operator*(const Quat& Q) const
	{
		Quat Result;
		Result.X = (W * Q.X) + (X * Q.W) + (Y * Q.Z) - (Z * Q.Y);
		Result.Y = (W * Q.Y) - (X * Q.Z) + (Y * Q.W) + (Z * Q.X);
		Result.Z = (W * Q.Z) + (X * Q.Y) - (Y * Q.X) + (Z * Q.W);
		Result.W = (W * Q.W) - (X * Q.X) - (Y * Q.Y) - (Z * Q.Z);
		return Result;
	}

	Quat Quat::operator*(const float Multiplier) const
	{
		return Quat(Multiplier * X, Multiplier * Y, Multiplier * Z, Multiplier * W);
	}

	Quat& Quat::Normalize(float Tolerance)
	{
		const float LengthSq = X * X + Y * Y + Z * Z + W * W;

		if (LengthSq >= Tolerance)
		{
			const float Multiplier = 1.0f / sqrtf(LengthSq); // TODO optimize with fast inv sqrt
			X *= Multiplier;
			Y *= Multiplier;
			Z *= Multiplier;
			W *= Multiplier;
		}
		else
		{
			// set to identity
			X = 0;
			Y = 0;
			Z = 0;
			W = 1.0f;
		}

		return *this;
	}

	VecN Quat::Rotate(const VecN& V) const
	{
		// http://people.csail.mit.edu/bkph/articles/Quaternions.pdf
		// V' = V + 2w(Q x V) + (2Q x (Q x V))
		// refactor:
		// V' = V + w(2(Q x V)) + (Q x (2(Q x V)))
		// T = 2(Q x V);
		// V' = V + w*(T) + (Q x T)

		const VecN Q(X, Y, Z);
		const VecN T = 2.f * VecN::Cross(Q, V);
		const VecN Result = V + (W * T) + VecN::Cross(Q, T);
		return Result;
		
		/*
		Quat Vec(V.X, V.Y, V.Z, 0.0f);
		Quat A = *this * Vec;
		Quat B = A * Quat(-X, -Y, -Z, W);
		return Vec3(B.X, B.Y, B.Z);*/
	}

	Quat Quat::FromAxisAngleDegrees(const VecN& Axis, const float AngleDegrees)
	{
		return FromAxisAngleRadians(Axis, AngleDegrees * DEG_TO_RAD);
	}

	Quat Quat::FromAxisAngleRadians(const VecN& Axis, const float AngleRadians)
	{
		float HalfAngle = 0.5f * AngleRadians;
		float SinHalfAngle = sinf(HalfAngle);
		Quat Q;
		Q.W = cosf(HalfAngle);
		Q.X = (SinHalfAngle * Axis.X);
		Q.Y = (SinHalfAngle * Axis.Y);
		Q.Z = (SinHalfAngle * Axis.Z);
		return Q;
	}

	Quat Quat::Lerp(const Quat& A, const Quat& B, const float Alpha)
	{
		// a "good enough" approximation of SLERP but much faster.
		//
		// output is NOT normalized for efficiency, 
		// but if A and B are normalized then output will be too

		// take dot product to see if Quat are pointing in same or opposite direction
		// use dot to flip quaternion to take shortest path
		const float Bias = Quat::Dot(A, B) >= 0.0f ? 1.0f : -1.0f;
		return (B * Alpha) + (A * (Bias * (1.f - Alpha)));
	}

	Quat Quat::FromToRotation(const VecN& A, const VecN& B)
	{
		// Based on:
		// http://lolengine.net/blog/2014/02/24/quaternion-from-two-vectors-final
		// http://www.euclideanspace.com/maths/algebra/vectors/angleBetween/index.htm

		Quat Result;
		const float NormAB = sqrtf(A.LengthSq() * B.LengthSq());
		float W = NormAB + VecN::Dot(A, B);
		if (W >= 1e-6f * NormAB)
		{
			//Axis = CrossProduct(A, B);
			Result.X = A.Y * B.Z - A.Z * B.Y;
			Result.Y = A.Z * B.X - A.X * B.Z;
			Result.Z = A.X * B.Y - A.Y * B.X;
			Result.W = W;
		}
		else
		{
			// A and B point in opposite directions
			W = 0.f;
			if (fabs(A.X) > fabs(A.Y))
			{
				Result = Quat(-A.Z, 0.f, A.X, W);
			}
			else
			{
				Result = Quat(0.f, -A.Z, A.Y, W);
			}
		}

		Result.Normalize();
		return Result;
	}

	float Quat::Dot(const Quat& A, const Quat& B)
	{
		return A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
	}

	bool Quat::AreClose(const Quat& A, const Quat& B)
	{
		return Quat::Dot(A, B) > 0.0f;
	}

	float Quat::AngleDegrees(const Quat& A, const Quat& B)
	{
		float DotAB = Quat::Dot(A, B);
	    float AbsClampedDot = std::min((float)fabs(DotAB), 1.0f);
		return (acosf(AbsClampedDot) * 2.0f) * RAD_TO_DEG;
	}

	float ScalarLerp(const float A, const float B, const float T)
	{
		return A + T * (B - A);
	}


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
		VecN& OutRolledSideVec)
	{
		// decompose normal into forward/back
		OutSide = VecN::Cross(Up, Fwd);
		OutVertNormal = Normal - VecN::Project(Normal, OutSide);
		OutVertNormal.NormalizeSafe();
		OutPitchRotation = Quat::FromToRotation(Up, OutVertNormal);
		OutPitchedFwdVec = OutPitchRotation.Rotate(Fwd).Normalized();

		// decompose normal into side-to-side
		OutSideNormal = Normal - VecN::Project(Normal, Fwd);
		OutSideNormal.NormalizeSafe();
		OutRollRotation = Quat::FromToRotation(Up, OutSideNormal);
		OutRolledSideVec = OutRollRotation.Rotate(OutSide).Normalized();
	}

	void ClampAngleBetweenNormals( 
		const VecN& RelativeTo, 
		const float MaxAngle,
		VecN& OutVector)
	{
		if (VecN::AngleBetweenNormals(RelativeTo, OutVector) <= MaxAngle)
			return;

		VecN Axis = VecN::Cross(RelativeTo, OutVector).NormalizedSafe();
		Quat Rotation = Quat::FromAxisAngleDegrees(Axis, MaxAngle);
		OutVector = Rotation.Rotate(RelativeTo);
	}

	VecN MoveTowards(
		const VecN& Current,
		const VecN& Target,
		const float MaxSpeed,
		const float MaxDistance,
		const float DeltaTime)
	{
		VecN ToTgt = Target - Current;
		float DistToTgt;
		ToTgt.GetDirectionAndLength(ToTgt, DistToTgt);

		// clamp current position to always be within 'max delta ever' of target
		if (DistToTgt > MaxDistance)
		{
			VecN Start = Target - (ToTgt * MaxDistance);
			ToTgt = Target - Start;
			ToTgt.GetDirectionAndLength(ToTgt, DistToTgt);
		}

		// if we are within "max delta per frame" of target, just move there
		const float MaxDelta = DeltaTime * MaxSpeed;
		if (DistToTgt <= MaxDelta || DistToTgt == 0.0f)
		{
			return Target;
		}

		// move towards target "max delta" amount
		return Current + ToTgt * MaxDelta;
	}

	VecN MoveTowardsSmooth(const VecN& Current, const VecN& Target, const float SlowFactor)
	{
		//http://sol.gfxile.net/interpolation/index.html#s5
		//v = ( (v * (N - 1) ) + w) / N; 
		VecN Result;
		Result = ((Current * (SlowFactor - 1)) + Target) / SlowFactor;
		return Result;
	}

	Quat RotateTowards(
		const Quat& Current,
		const Quat& Target,
		const float MaxAngularSpeed,
		const float DeltaTime)
	{
		// check if already at or very near Target
		float AngleBetween = Quat::AngleDegrees(Current, Target);
		if (AngleBetween <= 0.1f)
		{
			return Target;
		}

		float MaxDegrees = MaxAngularSpeed * DeltaTime;
		float T = std::min(1.0f, MaxDegrees / AngleBetween);
		Quat Result = Quat::Lerp(Current, Target, T);
		Result.Normalize();
		return Result;
	}

}