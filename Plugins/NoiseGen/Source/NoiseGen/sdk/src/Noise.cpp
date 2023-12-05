#pragma once
#include "../include/Noise.h"

namespace NoiseGen
{
	//std::map<uint8, std::function<void()>> NoiseGen::Noise::someMap = {{0, static_cast<std::function<void()>>(NoiseGen::Noise::SomeFunc)}};
	const VecN Noise::s_gradients2D[8] = {
		{ 1.f, 0.f},
		{-1.f, 0.f},
		{ 0.f, 1.f},
		{ 0.f,-1.f},
		VecN{ DIAGONAL_VECTOR_2D_NORMALIZED, DIAGONAL_VECTOR_2D_NORMALIZED},//.GetSafeNormal(),
		VecN{-DIAGONAL_VECTOR_2D_NORMALIZED, DIAGONAL_VECTOR_2D_NORMALIZED},//.GetSafeNormal(),
		VecN{ DIAGONAL_VECTOR_2D_NORMALIZED,-DIAGONAL_VECTOR_2D_NORMALIZED},//.GetSafeNormal(),
		VecN{-DIAGONAL_VECTOR_2D_NORMALIZED,-DIAGONAL_VECTOR_2D_NORMALIZED},//.GetSafeNormal()
	};
	
	Noise::Noise()
	{
		bIsInitialized = true;
	}

	Noise::~Noise()
	{
		bIsInitialized = false;
	}

	void Noise::Reset() const
	{
		//bIsInitialized = false;
		//clear any data if needed:
	}

	bool Noise::IsInitialized() const
	{
		return true; //Instance->IsInitialized();
	}

	NoiseSample Noise::Value1D(VecN point, float frequency) const
	{
		point *= frequency;
		int i0 = FMath::Floor(point.X);
		float t = point.X - i0;
		i0 &= hashMask;
		int i1 = i0 + 1;

		int h0 = hash[i0];
		int h1 = hash[i1];

		float dt = SmoothDerivative(t);
		t = Smooth(t);

		float a = h0;
		float b = h1 - h0;

		NoiseSample sample;
		sample.value = a + b * t;
		sample.derivative.X = b * dt;
		sample.derivative.Y = 0.f;
		sample.derivative.Z = 0.f;
		sample.derivative *= frequency;
		return sample * (2.f / hashMask) - 1.f;
	}
	//point is clamped within one unit value, that's why the resolution must be established, and cell size
	//otherwise there will be no interpolation, tx and ty will always evaluate to 0,000000some_small_number
	NoiseSample Noise::Value2D(VecN point, float frequency) const
	{
		// if(std::abs(point.X - FMath::Floor(point.X)) < 0.1f && std::abs(point.Y - FMath::Floor(point.Y)) < 0.1f)
		// {
		// 	point = VecN{point.X += 0.1f,
		// 	point.Y += 0.1f, point.Z};
		// }
		
		point *= frequency;
		int ix0 = FMath::Floor(point.X);
		int iy0 = FMath::Floor(point.Y);
		float tx = point.X - ix0;
		float ty = point.Y - iy0;
		ix0 &= hashMask;
		iy0 &= hashMask;
		int ix1 = ix0 + 1;
		int iy1 = iy0 + 1;

		int h0 = hash[ix0];
		int h1 = hash[ix1];
		int h00 = hash[h0 + iy0];
		int h10 = hash[h1 + iy0];
		int h01 = hash[h0 + iy1];
		int h11 = hash[h1 + iy1];

		float dtx = SmoothDerivative(tx);
		float dty = SmoothDerivative(ty);
		tx = Smooth(tx);
		ty = Smooth(ty);

		float a = h00;
		float b = h10 - h00;
		float c = h01 - h00;
		float d = h11 - h01 - h10 + h00;

		NoiseSample sample;
		sample.value = a + b * tx + (c + d * tx) * ty;
		sample.derivative.X = (b + d * ty) * dtx;
		sample.derivative.Y = (c + d * tx) * dty;
		sample.derivative.Z = 0.f;
		sample.derivative *= frequency;//todo if derivative is zero maybe sample adjacent 
		return sample * (2.f / hashMask) - 1.f;;
	}

	NoiseSample Noise::Value3D(VecN point, float frequency) const
	{
		point *= frequency;
		int ix0 = FMath::Floor(point.X);
		int iy0 = FMath::Floor(point.Y);
		int iz0 = FMath::Floor(point.Z);
		float tx = point.X - ix0;
		float ty = point.Y - iy0;
		float tz = point.Z - iz0;
		ix0 &= hashMask;
		iy0 &= hashMask;
		iz0 &= hashMask;
		int ix1 = ix0 + 1;
		int iy1 = iy0 + 1;
		int iz1 = iz0 + 1;

		int h0 = hash[ix0];
		int h1 = hash[ix1];
		int h00 = hash[h0 + iy0];
		int h10 = hash[h1 + iy0];
		int h01 = hash[h0 + iy1];
		int h11 = hash[h1 + iy1];
		int h000 = hash[h00 + iz0];
		int h100 = hash[h10 + iz0];
		int h010 = hash[h01 + iz0];
		int h110 = hash[h11 + iz0];
		int h001 = hash[h00 + iz1];
		int h101 = hash[h10 + iz1];
		int h011 = hash[h01 + iz1];
		int h111 = hash[h11 + iz1];

		float dtx = SmoothDerivative(tx);
		float dty = SmoothDerivative(ty);
		float dtz = SmoothDerivative(tz);
		tx = Smooth(tx);
		ty = Smooth(ty);
		tz = Smooth(tz);

		float a = h000;
		float b = h100 - h000;
		float c = h010 - h000;
		float d = h001 - h000;
		float e = h110 - h010 - h100 + h000;
		float f = h101 - h001 - h100 + h000;
		float g = h011 - h001 - h010 + h000;
		float h = h111 - h011 - h101 + h001 - h110 + h010 + h100 - h000;

		NoiseSample sample;
		sample.value = a + b * tx + (c + e * tx) * ty + (d + f * tx + (g + h * tx) * ty) * tz;
		sample.derivative.X = (b + e * ty + (f + h * ty) * tz) * dtx;
		sample.derivative.Y = (c + e * tx + (g + h * tx) * tz) * dty;
		sample.derivative.Z = (d + f * tx + (g + h * tx) * ty) * dtz;
		sample.derivative *= frequency;
		return sample * (2.f / hashMask) - 1.f;;
	}

	NoiseSample Noise::Perlin1D(VecN point, float frequency) const
	{
		point *= frequency;
		int i0 = FMath::Floor(point.X);
		float t0 = point.X - i0;
		float t1 = t0 - 1.f;
		i0 &= hashMask;
		int i1 = i0 + 1;

		float g0 = gradients1D[hash[i0] & gradientsMask1D];
		float g1 = gradients1D[hash[i1] & gradientsMask1D];

		float v0 = g0 * t0;
		float v1 = g1 * t1;

		float dt = SmoothDerivative(t0);
		float t = Smooth(t0);

		float a = v0;
		float b = v1 - v0;

		float da = g0;
		float db = g1 - g0;

		NoiseSample sample;
		sample.value = a + b * t;
		sample.derivative.X = da + db * t + b * dt;
		sample.derivative.Y = 0.f;
		sample.derivative.Z = 0.f;
		sample.derivative *= frequency;
		return sample * 2.f;
	}

	NoiseSample Noise::Perlin2D(VecN point, float frequency) const
	{
		point *= frequency;
		int ix0 = FMath::Floor(point.X);
		int iy0 = FMath::Floor(point.Y);
		float tx0 = point.X - ix0;
		float ty0 = point.Y - iy0;
		float tx1 = tx0 - 1.f;
		float ty1 = ty0 - 1.f;
		ix0 &= hashMask;
		iy0 &= hashMask;
		int ix1 = ix0 + 1;
		int iy1 = iy0 + 1;

		int h0 = hash[ix0];
		int h1 = hash[ix1];
		VecN g00 = gradients2D[hash[h0 + iy0] & gradientsMask2D];
		VecN g10 = gradients2D[hash[h1 + iy0] & gradientsMask2D];
		VecN g01 = gradients2D[hash[h0 + iy1] & gradientsMask2D];
		VecN g11 = gradients2D[hash[h1 + iy1] & gradientsMask2D];

		float v00 = Dot(g00, tx0, ty0);
		float v10 = Dot(g10, tx1, ty0);
		float v01 = Dot(g01, tx0, ty1);
		float v11 = Dot(g11, tx1, ty1);

		float dtx = SmoothDerivative(tx0);
		float dty = SmoothDerivative(ty0);
		float tx = Smooth(tx0);
		float ty = Smooth(ty0);

		float a = v00;
		float b = v10 - v00;
		float c = v01 - v00;
		float d = v11 - v01 - v10 + v00;

		VecN da = g00;
		VecN db = g10 - g00;
		VecN dc = g01 - g00;
		VecN dd = g11 - g01 - g10 + g00;

		NoiseSample sample;
		sample.value = a + b * tx + (c + d * tx) * ty;
		const VecN derivativeXY = da + db * tx + (dc + dd * tx) * ty;
		sample.derivative = VecN{derivativeXY.X, derivativeXY.Y, 0.f};
		sample.derivative.X += (b + d * ty) * dtx;
		sample.derivative.Y += (c + d * tx) * dty;
		sample.derivative.Z = 0.f;
		sample.derivative *= frequency;
		return sample * sqr2;
	}

	NoiseSample Noise::Perlin3D(VecN point, float frequency) const
	{
		point *= frequency;
		int ix0 = FMath::Floor(point.X);
		int iy0 = FMath::Floor(point.Y);
		int iz0 = FMath::Floor(point.Z);
		float tx0 = point.X - ix0;
		float ty0 = point.Y - iy0;
		float tz0 = point.Z - iz0;
		float tx1 = tx0 - 1.f;
		float ty1 = ty0 - 1.f;
		float tz1 = tz0 - 1.f;
		ix0 &= hashMask;
		iy0 &= hashMask;
		iz0 &= hashMask;
		int ix1 = ix0 + 1;
		int iy1 = iy0 + 1;
		int iz1 = iz0 + 1;

		int h0 = hash[ix0];
		int h1 = hash[ix1];
		int h00 = hash[h0 + iy0];
		int h10 = hash[h1 + iy0];
		int h01 = hash[h0 + iy1];
		int h11 = hash[h1 + iy1];
		VecN g000 = gradients3D[hash[h00 + iz0] & gradientsMask3D];
		VecN g100 = gradients3D[hash[h10 + iz0] & gradientsMask3D];
		VecN g010 = gradients3D[hash[h01 + iz0] & gradientsMask3D];
		VecN g110 = gradients3D[hash[h11 + iz0] & gradientsMask3D];
		VecN g001 = gradients3D[hash[h00 + iz1] & gradientsMask3D];
		VecN g101 = gradients3D[hash[h10 + iz1] & gradientsMask3D];
		VecN g011 = gradients3D[hash[h01 + iz1] & gradientsMask3D];
		VecN g111 = gradients3D[hash[h11 + iz1] & gradientsMask3D];

		float v000 = Dot(g000, tx0, ty0, tz0);
		float v100 = Dot(g100, tx1, ty0, tz0);
		float v010 = Dot(g010, tx0, ty1, tz0);
		float v110 = Dot(g110, tx1, ty1, tz0);
		float v001 = Dot(g001, tx0, ty0, tz1);
		float v101 = Dot(g101, tx1, ty0, tz1);
		float v011 = Dot(g011, tx0, ty1, tz1);
		float v111 = Dot(g111, tx1, ty1, tz1);

		float dtx = SmoothDerivative(tx0);
		float dty = SmoothDerivative(ty0);
		float dtz = SmoothDerivative(tz0);
		float tx = Smooth(tx0);
		float ty = Smooth(ty0);
		float tz = Smooth(tz0);

		float a = v000;
		float b = v100 - v000;
		float c = v010 - v000;
		float d = v001 - v000;
		float e = v110 - v010 - v100 + v000;
		float f = v101 - v001 - v100 + v000;
		float g = v011 - v001 - v010 + v000;
		float h = v111 - v011 - v101 + v001 - v110 + v010 + v100 - v000;

		VecN da = g000;
		VecN db = g100 - g000;
		VecN dc = g010 - g000;
		VecN dd = g001 - g000;
		VecN de = g110 - g010 - g100 + g000;
		VecN df = g101 - g001 - g100 + g000;
		VecN dg = g011 - g001 - g010 + g000;
		VecN dh = g111 - g011 - g101 + g001 - g110 + g010 + g100 - g000;

		NoiseSample sample;
		sample.value = a + b * tx + (c + e * tx) * ty + (d + f * tx + (g + h * tx) * ty) * tz;
		sample.derivative = da + db * tx + (dc + de * tx) * ty + (dd + df * tx + (dg + dh * tx) * ty) * tz;
		sample.derivative.X += (b + e * ty + (f + h * ty) * tz) * dtx;
		sample.derivative.Y += (c + e * tx + (g + h * tx) * tz) * dty;
		sample.derivative.Z += (d + f * tx + (g + h * tx) * ty) * dtz;
		sample.derivative *= frequency;
		return sample;
	}

	NoiseSample Noise::SimplexValue1D(VecN point, float frequency) const
	{
		point *= frequency;
		int ix = FMath::Floor(point.X);
		NoiseSample sample = SimplexValue1DPart(point, ix);
		sample += SimplexValue1DPart(point, ix + 1);
		sample.derivative *= frequency;
		return sample * (2.f / hashMask) - 1.f;
	}

	NoiseSample Noise::Sum(uint8 noiseType, uint8 dimensions, VecN point, float frequency, int octaves,
						float lacunarity, float persistence)
	{
		const auto method = GetNoiseMethod(noiseType, dimensions);
		NoiseSample sum = method(*this, point, frequency);
		float amplitude = 1.f;
		float range = 1.f;
		for (int o = 1; o < octaves; o++)
		{
			frequency *= lacunarity;
			amplitude *= persistence;
			range += amplitude;
			sum += method(*this, point, frequency) * amplitude;
		}
		return sum * (1.f / range);
	}

	// auto Noise::SomeFunc() -> void
	// {
	// 	printf("Some func called.");
	// }

	NoiseSample Noise::SimplexValue1DPart(VecN point, int ix)
	{
		float x = point.X - ix;
		//falloff function for measured x distance: (1 - x^2)^2 
		float f = 1.f - x * x;
		float f2 = f * f;
		float f3 = f * f2;
		float h = hash[ix & hashMask];
		NoiseSample sample;
		sample.value = f3 * h; //hash values are factored in falloff function
		sample.derivative.X = -6.f * h * x * f2; //derivative is -6x(1-x^2)^2 with hash value factored in
		return sample;
	}

	NoiseSample Noise::SimplexValue2D(VecN point, float frequency) const
	{
		point *= frequency;
		//we assume that we have triangle grid imposed, because it is how we want our points to be located
		//we than skew them as though it was deformed square grid only to define more easily inside which triangle we are interpolating
		float skew = (point.X + point.Y) * trianglesToSquares;
		//to determine which pair of triangles we are inside square, we skew it out
		//here each singular point is skewed out to determine where it lies inside square, which square triangle it belongs to
		float sx = point.X + skew;
		float sy = point.Y + skew;
		//now we take the grid sector in which the points are located (according to the skewed to square grid)
		int ix = FMath::Floor(sx);
		int iy = FMath::Floor(sy);
		//and we combine our two halves of the function as usual
		NoiseSample sample = SimplexValue2DPart(point, ix, iy);
		sample += SimplexValue2DPart(point, ix + 1, iy + 1);

		//determine if we are in left top triangle, on in right bottom one:
		if (sx - ix >= sy - iy)
		{
			//and having determined this we reinforce towards 1 if we are in lower bottom triangle
			//with high x value we push it towards near zero value which in our function will give us
			//higher result (combined with small y): 1 - near zero x - near zero y will give result near 1
			sample += SimplexValue2DPart(point, ix + 1, iy);
		}
		else
		{
			//and for top left triangle we push y value back instead to obtain the same result:
			sample += SimplexValue2DPart(point, ix, iy + 1);
		}

		sample.derivative *= frequency;
		return sample * (8.f * 2.f / hashMask) - 1.f; //scaling the falloff for skewed square (or unskewed triangles)
	}

	NoiseSample Noise::SimplexValue2DPart(VecN point, int ix, int iy)
	{
		float unskew = (ix + iy) * squaresToTriangles;
		//unskew to the back to equilateral triangles and determine the actual distances within them, NOTE:
		//if our current ix or iy or both are 0, especially both, we don't need to unskew anything because the point we are measuring our point float
		//distance from is fixed and was not affected by trianglesToSquares factor, and we are only measuring against "nexus" grid points

		//and now we calculate where out nexus point (which we referred to transformed to a square) if it was again compressed along diagonal
		float x = point.X - ix + unskew;
		//subtract form x component x grid point plus the factor by which it gets diminished along the diagonal
		//previously we just subtracted grid point, so we knew how distant from that grid point our x was inside current grid square, but
		//now we subtracting it plus its compression distance along main diagonal 
		float y = point.Y - iy + unskew;
		float f = 0.5f - x * x - y * y;
		//0.5f instead of 1f for scaling the falloff, its is squared distance of the height of the triangle (sqrt(2)/2), which is the new measure unit
		NoiseSample sample{};
		//clipping negative values:
		if (f > 0.f)
		{
			float f2 = f * f;
			float f3 = f * f2;
			float h = hash[hash[ix & hashMask] + iy & hashMask];
			float h6f2 = -6.f * h * f2;
			sample.value = h * f3;
			sample.derivative.X = h6f2 * x;
			sample.derivative.Y = h6f2 * y;
		}

		return sample;
	}

	NoiseSample Noise::SimplexValue3D(VecN point, float frequency) const
	{
		point *= frequency;
		float skew = (point.X + point.Y + point.Z) * (1.f / 3.f);
		float sx = point.X + skew;
		float sy = point.Y + skew;
		float sz = point.Z + skew;
		int ix = FMath::Floor(sx);
		int iy = FMath::Floor(sy);
		int iz = FMath::Floor(sz);

		NoiseSample sample = SimplexValue3DPart(point, ix, iy, iz);
		sample += SimplexValue3DPart(point, ix + 1, iy + 1, iz + 1);

		float x = sx - ix;
		float y = sy - iy;
		float z = sz - iz;
		if (x >= y)
		{
			if (x >= z)
			{
				sample += SimplexValue3DPart(point, ix + 1, iy, iz);
				if (y >= z)
				{
					sample += SimplexValue3DPart(point, ix + 1, iy + 1, iz);
				}
				else
				{
					sample += SimplexValue3DPart(point, ix + 1, iy, iz + 1);
				}
			}
			else
			{
				sample += SimplexValue3DPart(point, ix, iy, iz + 1);
				sample += SimplexValue3DPart(point, ix + 1, iy, iz + 1);
			}
		}
		else
		{
			if (y >= z)
			{
				sample += SimplexValue3DPart(point, ix, iy + 1, iz);
				if (x >= z)
				{
					sample += SimplexValue3DPart(point, ix + 1, iy + 1, iz);
				}
				else
				{
					sample += SimplexValue3DPart(point, ix, iy + 1, iz + 1);
				}
			}
			else
			{
				sample += SimplexValue3DPart(point, ix, iy, iz + 1);
				sample += SimplexValue3DPart(point, ix, iy + 1, iz + 1);
			}
		}

		sample.derivative *= frequency;
		return sample * (8.f * 2.f / hashMask) - 1.f;
	}

	NoiseSample Noise::SimplexValue3DPart(VecN point, int ix, int iy, int iz)
	{
		float unskew = (ix + iy + iz) * (1.f / 6.f);
		float x = point.X - ix + unskew;
		float y = point.Y - iy + unskew;
		float z = point.Z - iz + unskew;
		float f = 0.5f - x * x - y * y - z * z;
	
		NoiseSample sample{};
		if (f > 0.f)
		{
			float f2 = f * f;
			float f3 = f * f2;
			float h = hash[hash[hash[ix & hashMask] + iy & hashMask] + iz & hashMask];
			float h6f2 = -6.f * h * f2;
			sample.value = h * f3;
			sample.derivative.X = h6f2 * x;
			sample.derivative.Y = h6f2 * y;
			sample.derivative.Z = h6f2 * z;
		}
		return sample;
	}

	NoiseSample Noise::Simplex1D(VecN point, float frequency) const
	{
		point *= frequency;
		int ix = FMath::Floor(point.X);
		NoiseSample sample = Simplex1DPart(point, ix);
		sample += Simplex1DPart(point, ix + 1);
		sample.derivative *= frequency;
		return sample; // * (2f/hashMask) - 1f;
	}

	NoiseSample Noise::Simplex1DPart(VecN point, int ix)
	{
		float x = point.X - ix;
		float f = 1.f - x * x;
		float f2 = f * f;
		float f3 = f * f2;
		//float h = hash[ix & hashMask];

		//As with Perlin noise, we compute the gradient value by taking the
		//dot product of the gradient vector and the vector from the corner
		//to our sample point.
		//Just as with Perlin noise, the gradient now has to include the falloff multiplied by the gradient vector.
		//again 1d gradients are just -1 or 1, and hash randomly selects one or the other, it is just randomly selected directional vector (in 1d it can be only -1 or 1)
		//against which we take the dot product of our function
		float g = gradients1D[hash[ix & hashMask] & gradientsMask1D];
		float v = g * x; //"dot" product is taken of the gradient and x value distance from measuring point;
		//in two part simplex function we measure distance from ix and ix + 1 in separate function calls and then we add the results
		NoiseSample sample{};
		sample.value = f3 * v;
		//instead of a random hash value the gradient is selected based on that hash value and its dot product with x distance is used
		sample.derivative.X = g * f3 - 6.f * v * x * f2;
		//just reminder derivative is: -6f * x * f2, here like in perlin noise
		//the gradient must include the falloff multiplied by gradient vector, it must be added to the function
		//because of the product rule, and because of v that has g dependent on x, we must also 
		//take it's own derivative, which will be simply "g" and multiply it with "non-derivative" f3
		//NOTE: in Perlin functions that "x" was written as "t"
		//and derivative of g*x = (g*x)' = g*(f(x)') = g*(1) = g;
		return sample * (64.f / 27.f);
		//scaling the maximum value, that is when the gradients on both ends are pointing towards themselves
		//the maximum value is halfway along that segment, 2x(1 - x^2)^3 where x = ½, which is 27 / 64.
		//2x comes from "v" by which it is multiplied two times in two calls?
	}

	NoiseSample Noise::Simplex2D(VecN point, float frequency) const
	{
		point *= frequency;
		float skew = (point.X + point.Y) * trianglesToSquares;
		float sx = point.X + skew;
		float sy = point.Y + skew;
		int ix = FMath::Floor(sx);
		int iy = FMath::Floor(sy);
		NoiseSample sample = Simplex2DPart(point, ix, iy);
		sample += Simplex2DPart(point, ix + 1, iy + 1);

		if (sx - ix >= sy - iy)
		{
			sample += Simplex2DPart(point, ix + 1, iy);
		}
		else
		{
			sample += Simplex2DPart(point, ix, iy + 1);
		}

		sample.derivative *= frequency;
		return sample; // * (8f * 2f / hashMask) - 1f;
	}

	NoiseSample Noise::Simplex2DPart(VecN point, int ix, int iy) const
	{
		float unskew = (ix + iy) * squaresToTriangles;
		float x = point.X - ix + unskew;
		float y = point.Y - iy + unskew;
		float f = 0.5f - x * x - y * y;
		NoiseSample sample{};
		if (f > 0.f)
		{
			float f2 = f * f;
			float f3 = f * f2;
			//float h = hash[hash[ix & hashMask] + iy & hashMask];
			VecN g = gradients2D[hash[hash[ix & hashMask] + iy & hashMask] & gradientsMask2D];
			float v = Dot(g, x, y);
			float v6f2 = -6.f * v * f2;
			sample.value = v * f3;
			sample.derivative.X = g.X * f3 + v6f2 * x;
			sample.derivative.Y = g.Y * f3 + v6f2 * y;
		}

		return sample * simplexScale2D;
		//scaling the maximum value when at the center with all the vectors pointing towards each other
	}

	NoiseSample Noise::Simplex3D(VecN point, float frequency) const
	{
		point *= frequency;
		float skew = (point.X + point.Y + point.Z) * (1.f / 3.f);
		float sx = point.X + skew;
		float sy = point.Y + skew;
		float sz = point.Z + skew;
		int ix = FMath::Floor(sx);
		int iy = FMath::Floor(sy);
		int iz = FMath::Floor(sz);

		NoiseSample sample = Simplex3DPart(point, ix, iy, iz);
		sample += Simplex3DPart(point, ix + 1, iy + 1, iz + 1);

		float x = sx - ix;
		float y = sy - iy;
		float z = sz - iz;
		if (x >= y)
		{
			if (x >= z)
			{
				sample += Simplex3DPart(point, ix + 1, iy, iz);
				if (y >= z)
				{
					sample += Simplex3DPart(point, ix + 1, iy + 1, iz);
				}
				else
				{
					sample += Simplex3DPart(point, ix + 1, iy, iz + 1);
				}
			}
			else
			{
				sample += Simplex3DPart(point, ix, iy, iz + 1);
				sample += Simplex3DPart(point, ix + 1, iy, iz + 1);
			}
		}
		else
		{
			if (y >= z)
			{
				sample += Simplex3DPart(point, ix, iy + 1, iz);
				if (x >= z)
				{
					sample += Simplex3DPart(point, ix + 1, iy + 1, iz);
				}
				else
				{
					sample += Simplex3DPart(point, ix, iy + 1, iz + 1);
				}
			}
			else
			{
				sample += Simplex3DPart(point, ix, iy, iz + 1);
				sample += Simplex3DPart(point, ix, iy + 1, iz + 1);
			}
		}

		sample.derivative *= frequency;
		return sample; // * (8f * 2f/hashMask) - 1f;
	}

	NoiseSample Noise::Simplex3DPart(VecN point, int ix, int iy, int iz) const
	{
		float unskew = (ix + iy + iz) * (1.f / 6.f);
		float x = point.X - ix + unskew;
		float y = point.Y - iy + unskew;
		float z = point.Z - iz + unskew;
		float f = 0.5f - x * x - y * y - z * z;
		NoiseSample sample{};
		if (f > 0.f)
		{
			float f2 = f * f;
			float f3 = f * f2;
			//float h = hash[hash[hash[ix & hashMask] + iy & hashMask] + iz & hashMask];
			VecN g = simplexGradients3D[hash[hash[hash[ix & hashMask] + iy & hashMask] + iz & hashMask] &
				simplexGradientsMask3D];
			float v = Dot(g, x, y, z);
			float v6f2 = -6.f * v * f2;
			sample.value = v * f3;
			sample.derivative.X = g.X * f3 + v6f2 * x;
			sample.derivative.Y = g.Y * f3 + v6f2 * y;
			sample.derivative.Z = g.Z * f3 + v6f2 * z;
		}

		return sample * simplexScale3D;
	}
}