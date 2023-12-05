#pragma once

#include <functional>
#include <map>
#include <array>

#include "NoiseMath.h"
#include "NoiseSample.h"

namespace NoiseGen
{
	using namespace NoiseMath;

	class Noise
	{
		using Action = std::function<NoiseSample(const Noise&, VecN, float)>;//is the same as:
		using Action2 = NoiseSample (Noise::*)(VecN, float) const;

	public:
		Noise();
		~Noise();

		void Reset() const;

		bool IsInitialized() const;

		enum class NoiseMethodType {
			Value,
			Perlin,
			SimplexValue,
			Simplex
		};

		NoiseSample Value1D(VecN point, float frequency) const;

		NoiseSample Value2D(VecN point, float frequency) const;

		NoiseSample Value3D(VecN point, float frequency) const;

		// static std::array<Action, 3> valueMethods = {
		// 	&Noise::Value1D,
		// 	&Noise::Value2D,
		// 	&Noise::Value3D,
		// };

		NoiseSample Perlin1D(VecN point, float frequency) const;

		NoiseSample Perlin2D(VecN point, float frequency) const;

		NoiseSample Perlin3D(VecN point, float frequency) const;

		// static std::array<Action, 3> perlinMethods = {
		// 	&Noise::Perlin1D,
		// 	&Noise::Perlin2D,
		// 	&Noise::Perlin3D,
		// };

		NoiseSample SimplexValue1D(VecN point, float frequency) const;
	
		NoiseSample SimplexValue2D(VecN point, float frequency) const;

		NoiseSample SimplexValue3D(VecN point, float frequency) const;

		// static std::array<Action, 3> simplexValueMethods = {
		// 	&Noise::SimplexValue1D,
		// 	&Noise::SimplexValue2D,
		// 	&Noise::SimplexValue3D,
		// };

		NoiseSample Simplex1D(VecN point, float frequency) const;

		NoiseSample Simplex2D(VecN point, float frequency) const;

		NoiseSample Simplex3D(VecN point, float frequency) const;

		// static std::array<Action, 3> simplexMethods  = {
		// 	&Noise::Simplex1D,
		// 	&Noise::Simplex2D,
		// 	&Noise::Simplex3D
		// };

		NoiseSample Sum(uint8 noiseType, uint8 dimensions, VecN point, float frequency,
							int octaves, float lacunarity, float persistence);//todo try convert to static

		std::map<uint8, std::array<std::function<NoiseSample(const Noise&, VecN, float)>, 3>> methods =
		{
			{0, {&Noise::Value1D, &Noise::Value2D, &Noise::Value3D}},
			{1, {&Noise::Perlin1D, &Noise::Perlin2D, &Noise::Perlin3D,}},
			{2, {&Noise::SimplexValue1D, &Noise::SimplexValue2D, &Noise::SimplexValue3D}},
			{3, {&Noise::Simplex1D, &Noise::Simplex2D, &Noise::Simplex3D}},
		};

		std::function<NoiseSample(const Noise&, VecN, float)> GetNoiseMethod(uint8 noiseType, uint8 dimensions)
		{
			return methods.find(noiseType)->second.at(dimensions);
		}

		static auto SomeFunc() -> void
		{};

		static std::map<uint8, std::function<void()>> someMap;
	
	private:
		
		bool bIsInitialized = false;
		
		static NoiseSample SimplexValue1DPart(VecN point, int ix);

		static NoiseSample SimplexValue2DPart(VecN point, int ix, int iy);

		static NoiseSample SimplexValue3DPart(VecN point, int ix, int iy, int iz);

		static NoiseSample Simplex1DPart(VecN point, int ix);

		NoiseSample Simplex2DPart(VecN point, int ix, int iy) const;

		NoiseSample Simplex3DPart(VecN point, int ix, int iy, int iz) const;

		/*
		* The question is now where the maximum lies. Along the edge of a triangle, or at its center? Let's compute them both.
		* Remember that the edge length is √⅔ or √6 / 3. So halfway along an edge we get 2x(½ - x2)3 where x = √6 / 6, which is √6 / 81.
		* Next, the distance from a corner to the center of an equilateral triangle is equal to its edge length multiplied by √3 / 3 or √⅓.
		* So at the center we get 3x(½ - x2)3 where x = √2 / 3, which gives us 125√2 / 5832. As this value is a tiny bit larger than the other one,
		* it is our theoretical maximum. Its multiplicative inverse can be written as 2916√2 / 125, so that's our final scale.
		* the "1/2" is taken from the scaled value for "f" 
		*/
	
		static constexpr float simplexScale2D = 2916.f * /*sqrtConstexpr*/ gcem::sqrt(2) / 125.f;

		static constexpr float simplexScale3D = 8192.f * sqrtConstexpr(3.f) / 375.f;
	
		static constexpr float squaresToTriangles = (3.f - sqrtConstexpr(3.f)) / 6.f;

		static constexpr float trianglesToSquares = (sqrtConstexpr(3.f) - 1.f) / 2.f;

		static constexpr float sqr2 = sqrtConstexpr(2.f);
	
		static constexpr int hashMask = 255;
	
		static constexpr float gradients1D[2] = {
			1.f, -1.f
		};
	
		static constexpr int gradientsMask1D = 1;

		//TODO change all vector arrays for custom constexpr arrays and constexpr math lib like here:
		static constexpr Vec2_constexpr s_gradients2DVec2[8] = {
			Vec2_constexpr{ 1.f, 0.f},
			Vec2_constexpr{-1.f, 0.f},
			Vec2_constexpr{ 0.f, 1.f},
			Vec2_constexpr{ 0.f,-1.f},
			Vec2_constexpr{ 1.f, 1.f}.Normalized(),
			Vec2_constexpr{-1.f, 1.f}.Normalized(),
			Vec2_constexpr{ 1.f,-1.f}.Normalized(),
			Vec2_constexpr{-1.f,-1.f}.Normalized(),
		};

		const VecN gradients2D[8] = {
			{ 1.f, 0.f},
			{-1.f, 0.f},
			{ 0.f, 1.f},
			{ 0.f,-1.f},
			VecN{ 1.f, 1.f}.NormalizedSafe(),
			VecN{-1.f, 1.f}.NormalizedSafe(),
			VecN{ 1.f,-1.f}.NormalizedSafe(),
			VecN{-1.f,-1.f}.NormalizedSafe()
		};

		static const VecN s_gradients2D[8];

		static constexpr int gradientsMask2D = 7;

		const VecN gradients3D[16] = {
			{ 1.f, 1.f, 0.f},
			{-1.f, 1.f, 0.f},
			{ 1.f,-1.f, 0.f},
			{-1.f,-1.f, 0.f},
			{ 1.f, 0.f, 1.f},
			{-1.f, 0.f, 1.f},
			{ 1.f, 0.f,-1.f},
			{-1.f, 0.f,-1.f},
			{ 0.f, 1.f, 1.f},
			{ 0.f,-1.f, 1.f},
			{ 0.f, 1.f,-1.f},
			{ 0.f,-1.f,-1.f},
			{ 1.f, 1.f, 0.f},
			{-1.f, 1.f, 0.f},
			{ 0.f,-1.f, 1.f},
			{ 0.f,-1.f,-1.f}
		};

		static constexpr int gradientsMask3D = 15;

		const VecN simplexGradients3D[32] = {
			VecN{ 1.f, 1.f, 0.f}.NormalizedSafe(),
			VecN{-1.f, 1.f, 0.f}.NormalizedSafe(),
			VecN{ 1.f,-1.f, 0.f}.NormalizedSafe(),
			VecN{-1.f,-1.f, 0.f}.NormalizedSafe(),
			VecN{ 1.f, 0.f, 1.f}.NormalizedSafe(),
			VecN{-1.f, 0.f, 1.f}.NormalizedSafe(),
			VecN{ 1.f, 0.f,-1.f}.NormalizedSafe(),
			VecN{-1.f, 0.f,-1.f}.NormalizedSafe(),
			VecN{ 0.f, 1.f, 1.f}.NormalizedSafe(),
			VecN{ 0.f,-1.f, 1.f}.NormalizedSafe(),
			VecN{ 0.f, 1.f,-1.f}.NormalizedSafe(),
			VecN{ 0.f,-1.f,-1.f}.NormalizedSafe(),
			VecN{ 1.f, 1.f, 0.f}.NormalizedSafe(),
			VecN{-1.f, 1.f, 0.f}.NormalizedSafe(),
			VecN{ 1.f,-1.f, 0.f}.NormalizedSafe(),
			VecN{-1.f,-1.f, 0.f}.NormalizedSafe(),
			VecN{ 1.f, 0.f, 1.f}.NormalizedSafe(),
			VecN{-1.f, 0.f, 1.f}.NormalizedSafe(),
			VecN{ 1.f, 0.f,-1.f}.NormalizedSafe(),
			VecN{-1.f, 0.f,-1.f}.NormalizedSafe(),
			VecN{ 0.f, 1.f, 1.f}.NormalizedSafe(),
			VecN{ 0.f,-1.f, 1.f}.NormalizedSafe(),
			VecN{ 0.f, 1.f,-1.f}.NormalizedSafe(),
			VecN{ 0.f,-1.f,-1.f}.NormalizedSafe(),
			VecN{ 1.f, 1.f, 1.f}.NormalizedSafe(),
			VecN{-1.f, 1.f, 1.f}.NormalizedSafe(),
			VecN{ 1.f,-1.f, 1.f}.NormalizedSafe(),
			VecN{-1.f,-1.f, 1.f}.NormalizedSafe(),
			VecN{ 1.f, 1.f,-1.f}.NormalizedSafe(),
			VecN{-1.f, 1.f,-1.f}.NormalizedSafe(),
			VecN{ 1.f,-1.f,-1.f}.NormalizedSafe(),
			VecN{-1.f,-1.f,-1.f}.NormalizedSafe()
		};

		static const int simplexGradientsMask3D = 31;

		//TODO make func const and const VecN&
		static float Dot (VecN g, float x, float y) {
			return g.X * x + g.Y * y;
		}

		static float Dot (VecN g, float x, float y, float z) {
			return g.X * x + g.Y * y + g.Z * z;
		}
	
		static float Smooth (float t) {
			return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
		}

		static float SmoothDerivative (float t) {
			return 30.f * t * t * (t * (t - 2.f) + 1.f);
		}
	
		static constexpr int hash[] = {
			151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
			140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
			247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
			57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
			74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
			60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
			65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
			200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
			52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
			207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
			119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
			129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
			218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
			81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
			184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
			222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180,

			151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
			140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
			247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
			57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
			74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
			60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
			65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
			200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
			52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
			207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
			119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
			129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
			218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
			81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
			184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
			222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180
		};
	};
}