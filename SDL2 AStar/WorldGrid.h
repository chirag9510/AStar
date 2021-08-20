#pragma once
#include <glm.hpp>
#include "Components.h"

namespace WorldGrid
{
	//global tile size 
	const float fTileSize = 32.0f;

	static glm::ivec2 GetGridPos(const float fXPos, const float fYPos)
	{
		return glm::ivec2(static_cast<int>(fXPos / fTileSize), static_cast<int>(fYPos / fTileSize));
	}
	static glm::ivec2 GetGridPos(const glm::vec2 vPosition)
	{
		return glm::ivec2(static_cast<int>(vPosition.x / fTileSize), static_cast<int>(vPosition.y / fTileSize));
	}
	static glm::vec2 GetGridPos(const glm::ivec2 ivPosition)
	{
		return glm::vec2(static_cast<float>(ivPosition.x) * fTileSize, static_cast<float>(ivPosition.y) * fTileSize);
	}
};
