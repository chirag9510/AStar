//the node for the astar algo for pathfinding system
#pragma once
#include <memory>
#include <glm.hpp>

struct PathfindingNode
{
	//position of the node on the world grid based on the tile
	glm::ivec2 ivGridPos;
	glm::ivec2 ivParentPos;

	//the F G H values for the astar algo node
	//F = G + H
	float F, G, H;

	PathfindingNode(glm::ivec2 ivGridPos = glm::ivec2(0), float G = 0.0f)
	{
		this->ivGridPos = ivGridPos;
		this->F = 0.0f;
		this->G = G;
		this->H = 0.0f;
	}

	//op overloading so that they can be stored in std::set with its fast searching
	bool operator==(const PathfindingNode& other) const
	{
		return (ivGridPos.x == other.ivGridPos.x) && (ivGridPos.y == other.ivGridPos.y);
	}

	bool operator<(const PathfindingNode& other) const
	{
		//{1,3} is less than {2,2} according to the world grid
		//comparing 2 dimensional object with compatible comparator
		if (ivGridPos.y < other.ivGridPos.y)
			return true;
		else if (ivGridPos.y > other.ivGridPos.y)
			return false;
		else
			return ivGridPos.x < other.ivGridPos.x;
	}

	bool operator!=(const PathfindingNode& other) const
	{
		return (ivGridPos.x != other.ivGridPos.x) || (ivGridPos.y != other.ivGridPos.y);
	}

};