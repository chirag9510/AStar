#pragma once
#include <glm.hpp>
#include <entt/entt.hpp>

//setting a target position for player entity when the mouse clicks on a tile
//then through astar algo the player goes
//subscribed by AStarPathfindingSystem and emitted by MouseInputSystem
struct TargetPositionEvent
{
public:
	entt::entity entity;
	//current / start pos of the entity
	glm::ivec2 ivStartPos;
	//target position for the entity to go to
	glm::ivec2 ivTargetPos;

	TargetPositionEvent(entt::entity entity, glm::ivec2 ivStartPos, glm::ivec2 ivTargetPos) : 
				entity(entity), ivStartPos(ivStartPos), ivTargetPos(ivTargetPos) {}
};
