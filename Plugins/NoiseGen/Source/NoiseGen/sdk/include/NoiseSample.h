#pragma once
#include "NoiseMath.h"

using NoiseMath::VecN;

namespace  NoiseGen
{
	class NoiseSample
	{
	public:
		float value;
		VecN derivative;

		friend NoiseSample operator+(const NoiseSample& a, float b)
		{
			return {a.value + b, a.derivative};
		}

		friend NoiseSample operator+ (float a, const NoiseSample& b) {
			return {a + b.value, b.derivative};
		}

		friend NoiseSample operator+ (const NoiseSample& a, const NoiseSample& b) {
			return {a.value + b.value, a.derivative + b.derivative};
		}

		friend NoiseSample operator- (const NoiseSample& a, float b) {
			return {a.value - b, a.derivative};
		}
		
		friend NoiseSample operator- (float a, const NoiseSample& b) {
			return {a - b.value, VecN{-b.derivative.X, -b.derivative.Y, -b.derivative.Z}};
		}
		
		friend NoiseSample operator- (const NoiseSample& a, const NoiseSample& b) {
			return {a.value - b.value, a.derivative - b.derivative};
		}

		friend NoiseSample operator* (const NoiseSample& a, float b) {
			return {a.value * b, a.derivative * b};
		}

		friend NoiseSample operator* (float a, const NoiseSample& b) {
			return {b.value * a, b.derivative * a};
		}

		friend NoiseSample operator* (const NoiseSample& a, const NoiseSample& b) {
			return {a.value * b.value, a.derivative * b.value + b.derivative * a.value};
		}

		NoiseSample& operator+=(const NoiseSample& other)
		{
			this->value += other.value;
			this->derivative += other.derivative;
			return *this;
		};

		NoiseSample& operator*=(const NoiseSample& other)
		{
			this->derivative = derivative * other.value + other.derivative * value;
			this->value *= other.value;
			return *this;
		};
	};
}
