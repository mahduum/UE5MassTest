#pragma once
#include <string>

#include "NoiseMath.h"

namespace NoiseGen
{
	using NoiseMath::VecN;
	
	class Grid
	{
	private:
		int width;
		int height;
		int** grid2DArray;
		float cellSize;

	public:
		Grid(int width, int height, float cellSize);
		~Grid();

		std::string GridInfo() const;

		int GetGridValue(int x, int y) const;

		VecN GetGridCellOriginFromIndices(int x, int y) const;

		VecN GetGridCellCenterFromIndices(int x, int y) const;
		
		/** Produces internal grid coordinates in scaled grid units based on IN world coordinates (x, y). */
		VecN GetGridUnitsPosition(float x, float y) const;

	private:
		VecN GetGridCellOriginWorldPositionInternal(int x, int y) const;
	};
}
