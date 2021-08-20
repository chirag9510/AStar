#pragma once
#include <deque>
#include <glm.hpp>
#include <SDL.h>
#include "PathfindingNode.h"

//contains the path they will traverse on
struct PathfindingComponent
{
	//the actual path with the nodes is stored here after the a* is applied by the PathFindingSystem
	//move the entity along this path then using MovementSystem
	std::deque<PathfindingNode> deqPath;

	//the node on the back of the queue is targeted by entity and the cycle repeats until all the entities are popped
	//distance is calculated from the entity to this node and that distance is used to check if the entity has reached the path node 
	glm::ivec2 vTargetNodePosition;
	//the distance of the entity to the path node its going towards, if it reaches the node then pop in the next node
	float fTargetNodeDistance;
	bool bSetTargetNode;
	//check if the target has to move along the path or if it has reached its target
	bool bFollowPath;

	PathfindingComponent()
	{
		vTargetNodePosition = glm::ivec2(0);
		bFollowPath = false;
		bSetTargetNode = false;
		fTargetNodeDistance = 0.0f;
	}
};

struct TransformComponent
{
	glm::vec2 vPosition;
	double dRotation;
	TransformComponent(glm::vec2 vPosition = glm::vec2(0.0f), double dRotation = 0.0f) : vPosition(vPosition), dRotation(dRotation) {}
};

struct SpriteComponent
{
	SDL_Texture* texSprite;
	glm::ivec2 ivDim;
	SpriteComponent(SDL_Texture* texSprite = nullptr, glm::ivec2 ivDim = glm::ivec2(0, 0)) :texSprite(texSprite), ivDim(ivDim) {}
};


enum TileType { BLANK = -1, WALL, PATH, FINISH, SPAWN, PATH_CLOSEDLIST, PATH_OPENLIST };
struct TileComponent
{
	TileType mTileType;
	glm::ivec2 ivGridPos;
	TileComponent(TileType mTileType = BLANK, glm::ivec2 ivGridPos = glm::ivec2(0)) : mTileType(mTileType), ivGridPos(ivGridPos) {}
};

//all the physics stuff
struct RigidBodyComponent
{
	//only use this velocity if bMove is true
	bool bMove;
	float fVelocity;

	RigidBodyComponent(float velocity = 0.0f, bool bMove = true)
	{
		//increase velocity if the fTileSize is increased 
		this->fVelocity = velocity;
		this->bMove = bMove;
	}
};

//blank components useful for seperating certain types of entities
struct CameraFollowComponent
{
	CameraFollowComponent() {}
};

struct MouseInputComponent
{
	MouseInputComponent() {}
};