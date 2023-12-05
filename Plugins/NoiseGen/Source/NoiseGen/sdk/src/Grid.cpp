#include "../include/Grid.h"

#include <cassert>

namespace NoiseGen
{
	Grid::Grid(int _width, int _height, float _cellSize)
	{
		width = _width;
		height = _height;
		cellSize = _cellSize;
	
		grid2DArray = static_cast<int**>(malloc(width * sizeof(int*)));
		for(size_t s = 0; s < width; s++)
		{
			grid2DArray[s] = static_cast<int*>(malloc(height * sizeof(int)));
		}

		// grid2DArray = new int*[width * sizeof(int*)];
		// for(size_t s = 0; s < width; s++)
		// {
		// 	grid2DArray[s] = new int[height * sizeof(int)];
		// }
	}
	Grid::~Grid()
	{
		for(size_t s = 0; s < width; s++)
		{
			//free(grid2DArray[s]);//todo apparently freeing here is not necessary unreal takes care
		}
		//free(grid2DArray);

		// for(size_t s = 0; s < width; s++)
		// {
		// 	delete grid2DArray[s];
		// }
		// delete[] grid2DArray;
	}

	std::string Grid::GridInfo() const
	{
		return "Grid width: " + std::to_string(width) + ", height: " + std::to_string(height);
	}

	int Grid::GetGridValue(int x, int y) const
	{
		return grid2DArray[x][y];
	}

	VecN Grid::GetGridCellOriginFromIndices(int x, int y) const
	{
		assert (x < width && y < height);
		return Grid::GetGridCellOriginWorldPositionInternal(x, y);
	}

	VecN Grid::GetGridCellCenterFromIndices(int x, int y) const
	{
		assert (x < width && y < height);
		return Grid::GetGridCellOriginWorldPositionInternal(x, y) + VecN(cellSize/2, cellSize/2, cellSize/2);
	}

	VecN Grid::GetGridUnitsPosition(float x, float y) const
	{
		return VecN{x, y, 0.f} / cellSize;
	}

	auto Grid::GetGridCellOriginWorldPositionInternal(int x, int y) const -> VecN
	{
		return VecN{static_cast<float>(x), static_cast<float>(y), 1.f} * cellSize;
	}
}
