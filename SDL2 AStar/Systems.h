#pragma once
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>
#include "AssetStore.h"
#include "Components.h"
#include "WorldGrid.h"
#include "Events.h"


//creates a path using the a* algo loads it in the Pathfinding component
class AStarPathfindingSystem
{
	struct EntityPath
	{
		entt::entity entity;
		std::set<PathfindingNode> ClosedList;
		std::set<PathfindingNode> OpenList;
		PathfindingNode startNode, targetNode;
		EntityPath(entt::entity entity, std::set<PathfindingNode> ClosedList, std::set<PathfindingNode> OpenList, PathfindingNode startNode, PathfindingNode targetNode) :
			entity(entity), ClosedList(ClosedList), OpenList(OpenList), startNode(startNode), targetNode(targetNode) {}
	};

	std::vector<EntityPath> vecEntityPath;
	std::set<PathfindingNode> OpenList;
	std::set<PathfindingNode> ClosedList;
	//all valid path nodes are stored here
	std::set<PathfindingNode> AllPathNodes;

public:
	AStarPathfindingSystem() {}

	void InsertNode(glm::ivec2 ivGridPos)
	{
		AllPathNodes.insert(PathfindingNode(ivGridPos));
	}

	void ProcessPathNodes(const TargetPositionEvent& targetPositionEvent)
	{
		//now init the start node
		PathfindingNode startNode = PathfindingNode(targetPositionEvent.ivStartPos, 0.0f);
		PathfindingNode targetNode = PathfindingNode(targetPositionEvent.ivTargetPos, 0.0f);
		startNode.H = Heuristic(startNode.ivGridPos, targetNode.ivGridPos);
		startNode.F = startNode.H;

		//a vector is useful here instead of set since there will be modifications to the values
		std::vector<PathfindingNode> NeighborNodes;
		NeighborNodes.resize(8);								//8 neighbors

		//add the starting node to the openlist
		OpenList.insert(startNode);

		//start the loop for traversal
		//if the openlist gets empty that means automatically the path is invalid and is not traveresed in ConstructPath
		PathfindingNode currentNode;
		while (OpenList.size())
		{
			currentNode = *OpenList.begin();
			//get the node with the least F from the openlist
			for (std::set<PathfindingNode>::iterator it = OpenList.begin(); it != OpenList.end(); it++)
			{
				if (it->F <= currentNode.F)
					currentNode = (*it);
			}

			//or if we have finally reached our goal
			if (currentNode == targetNode)			
			{
				vecEntityPath.push_back(EntityPath(targetPositionEvent.entity, ClosedList, OpenList, startNode, currentNode));
				break;
			};

			//remove it from the openlist and put in closedlist
			ClosedList.insert(currentNode);
			OpenList.erase(currentNode);

			//now get the valid neighbors
			Neighbors(currentNode, NeighborNodes);
			for (auto neighborNode : NeighborNodes)
			{
				//first check if the neighbor is not in closed list
				if (ClosedList.find(neighborNode) == ClosedList.end())
				{
					//if it isnt then calculate its F
					neighborNode.H = Heuristic(neighborNode.ivGridPos, targetNode.ivGridPos);
					neighborNode.F = neighborNode.G + neighborNode.H;
					//now check if it isnt in OpenList
					if (OpenList.find(neighborNode) == OpenList.end())
					{
						//add it if it isnt
						OpenList.insert(neighborNode);
					}
					else
					{
						auto openNode = OpenList.extract(neighborNode);
						if (neighborNode.G < openNode.value().G)
						{
							openNode.value().G = openNode.value().G;
							openNode.value().ivParentPos = neighborNode.ivParentPos;
						}
						OpenList.insert(std::move(openNode));

					}
				}
			}

		}

		OpenList.clear();
		ClosedList.clear();

	}

	float Heuristic(const glm::ivec2& ivCurrentPos, const glm::ivec2& ivTargetPos)
	{
		float absX = static_cast<float>(glm::abs(ivCurrentPos.x - ivTargetPos.x));
		float absY = static_cast<float>(glm::abs(ivCurrentPos.y - ivTargetPos.y));
		float D = 1.0f, D2 = 1.414f;
		return D * (absX + absY) + (D2 - 2.0f * D) * glm::min(absX, absY);
	}

	void Neighbors(PathfindingNode currentNode, std::vector<PathfindingNode>& NeighborNodes)
	{
		NeighborNodes.clear();
		glm::ivec2 ivGridPos = currentNode.ivGridPos;
		float G_Ortho = currentNode.G + 1.0f, G_Diag = currentNode.G + 1.414f;

		//first 4 are top left right bottom and rest are diagonal ones
		std::vector<PathfindingNode> vecNeighbors;
		vecNeighbors.push_back(PathfindingNode(glm::ivec2(ivGridPos.x, ivGridPos.y - 1), G_Ortho));
		vecNeighbors.push_back(PathfindingNode(glm::ivec2(ivGridPos.x, ivGridPos.y + 1), G_Ortho));
		vecNeighbors.push_back(PathfindingNode(glm::ivec2(ivGridPos.x + 1, ivGridPos.y), G_Ortho));
		vecNeighbors.push_back(PathfindingNode(glm::ivec2(ivGridPos.x - 1, ivGridPos.y), G_Ortho));
		vecNeighbors.push_back(PathfindingNode(glm::ivec2(ivGridPos.x - 1, ivGridPos.y - 1), G_Diag));
		vecNeighbors.push_back(PathfindingNode(glm::ivec2(ivGridPos.x + 1, ivGridPos.y - 1), G_Diag));
		vecNeighbors.push_back(PathfindingNode(glm::ivec2(ivGridPos.x - 1, ivGridPos.y + 1), G_Diag));
		vecNeighbors.push_back(PathfindingNode(glm::ivec2(ivGridPos.x + 1, ivGridPos.y + 1), G_Diag));

		for (auto it = vecNeighbors.begin(); it != vecNeighbors.end(); it++)
		{
			//check if the node is a path or obstacle and is a valid neighbor to the current Node
			//if it doesnt exist then it automatically is an obstacle and isnt added to the NeighborNodes vector
			if (AllPathNodes.find(*it) != AllPathNodes.end())
			{
				it->ivParentPos = currentNode.ivGridPos;
				NeighborNodes.push_back(*it);
			}
		}
	}


	void Update(std::unique_ptr<entt::registry>& mRegistry, std::unique_ptr<AssetStore>& mAssetStore)
	{
		if (!vecEntityPath.empty())
		{
			auto view = mRegistry->view<PathfindingComponent>();
			auto viewTiles = mRegistry->view<SpriteComponent, TileComponent>();
			for (auto entityPath : vecEntityPath)
			{
				//construct path
				auto& pathfinding = view.get<PathfindingComponent>(entityPath.entity);
				ConstructPath(pathfinding, entityPath);

				//display path on screen with different tiles sprites
				for (auto [entityTile, sprite, tile] : viewTiles.each())
				{
					if (tile.mTileType != FINISH)
					{
						bool bPath = false;
						for (auto& path : pathfinding.deqPath)
						{
							if (tile.ivGridPos == path.ivGridPos)
							{
								sprite.texSprite = mAssetStore->GetTexture("sprite-closedlist");
								tile.mTileType = PATH_CLOSEDLIST;
								bPath = true;
							}
						}
						if (!bPath)
						{
							if (entityPath.ClosedList.find(tile.ivGridPos) != entityPath.ClosedList.end()
								|| entityPath.OpenList.find(tile.ivGridPos) != entityPath.OpenList.end())
							{
								sprite.texSprite = mAssetStore->GetTexture("sprite-openlist");
								tile.mTileType = PATH_OPENLIST;
							}
							else
								if (tile.mTileType == PATH_CLOSEDLIST || tile.mTileType == PATH_OPENLIST)
								{
									sprite.texSprite = mAssetStore->GetTexture("sprite-tile");
									tile.mTileType = PATH;
								}
						}
					}

				}

			}
			vecEntityPath.clear();
		}
	}

	//construct the path for the entity finally
	void ConstructPath(PathfindingComponent& pathfinding, EntityPath& entityPath)
	{
		//construct path using reverse traversal
		pathfinding.deqPath.clear();
		PathfindingNode node = entityPath.targetNode;
		while (node != entityPath.startNode)
		{
			pathfinding.deqPath.push_back(node);
			//the nodes with their parents are in the ClosedList
			node = *entityPath.ClosedList.find(node.ivParentPos);
		}

		//only allow all this if an actual path is found and the start node is simply not the target node
		if (!pathfinding.deqPath.empty())
		{
			//set the first target for the entity to follow
			pathfinding.bSetTargetNode = true;
			//and dont forget to set this true so that the entity can follow
			pathfinding.bFollowPath = true;
		}
	}



	void Clear()
	{
		AllPathNodes.clear();
	}
};


//after the path is created by the Astarpathfindingsystem, the movement on that path occurs
//this systems just sets the rigidbody values of velocity and the direction for entities that have a path generated
class PathFollowingSystem
{
	//load the next level if the current node ends up bieng the same as nodeNextLevel
	PathfindingNode nodeNextLevel, currentNode;

public:

	//tells Core whether to load the next level if the player reaches the stairs
	bool Update(std::unique_ptr<entt::registry>& mRegistry, float& fDeltaTime)
	{
		auto view = mRegistry->view<TransformComponent, RigidBodyComponent, PathfindingComponent>();
		for (auto [entity, transform, rigid, pathfinding] : view.each())
		{
			if (pathfinding.bFollowPath)
			{
				//have to call this to set the very first target
				if (pathfinding.bSetTargetNode)
				{
					SetEntityDirection(transform, pathfinding);
					rigid.bMove = true;
					pathfinding.bSetTargetNode = false;
				}
				else
				{
					pathfinding.fTargetNodeDistance -= (rigid.fVelocity * fDeltaTime);
					//just keep track of the distance the entity is making
					if (pathfinding.fTargetNodeDistance <= 0.0f)
					{
						//first set the entity at that exact location of the node 
						transform.vPosition = pathfinding.vTargetNodePosition;
						//if the queue is empty then stop movement since the player has reached their target
						if (pathfinding.deqPath.empty())
						{
							pathfinding.bFollowPath = false;
							rigid.bMove = false;				//stop the movement the target has been reached or not set
							//if the player has reached the stairs
							if (currentNode == nodeNextLevel)
								return true;
						}
						else
						{
							//set the direction for the next node
							SetEntityDirection(transform, pathfinding);
						}
					}
				}
			}
			else
				rigid.bMove = false;				//stop the movement the target has been reached or not set

		}

		return false;
	}

	void SetEntityDirection(TransformComponent& transform, PathfindingComponent& pathfinding)
	{
		//then set the next node as its target by popping it from the queue
		currentNode = pathfinding.deqPath.back();
		glm::vec2 vGridPos = WorldGrid::GetGridPos(pathfinding.deqPath.back().ivGridPos);
		float fDeltaY = vGridPos.y - transform.vPosition.y, fDeltaX = vGridPos.x - transform.vPosition.x;
		//pathfinding.fTargetNodeDistance = glm::sqrt((fDeltaY * fDeltaY) + (fDeltaX * fDeltaX));
		pathfinding.fTargetNodeDistance = glm::distance(vGridPos, transform.vPosition);						//just use glm::distance
		pathfinding.vTargetNodePosition = WorldGrid::GetGridPos(pathfinding.deqPath.back().ivGridPos);
		pathfinding.deqPath.pop_back();
		transform.dRotation = atan2(fDeltaY, fDeltaX);
	}

	void SetNodeNextLevel(glm::ivec2 ivGridPos)
	{
		nodeNextLevel.ivGridPos = ivGridPos;
	}
};

class CameraFollowingSystem
{
	int mMapHeight, mMapWidth;

public:
	void Update(std::unique_ptr<entt::registry>& mRegistry, SDL_Rect& rectCamera)
	{
		auto viewEntity = mRegistry->view<TransformComponent, CameraFollowComponent>();
		for (auto [entity, transform] : viewEntity.each())
		{
			glm::vec2 vPosition = transform.vPosition;
			//change camera x and y based on the entity position
			//player entity is at the center of the camera
		//	if (vPosition.x + rectCamera.w / 2 < mMapWidth)
				rectCamera.x = vPosition.x - rectCamera.w / 2;
		//	if (vPosition.y + rectCamera.h / 2 < mMapHeight)
				rectCamera.y = transform.vPosition.y - rectCamera.h / 2;

			//keep camera rect view within the screen limits
			/*rectCamera.x = rectCamera.x < 0 ? 0 : rectCamera.x;
			rectCamera.y = rectCamera.y < 0 ? 0 : rectCamera.y;
			rectCamera.x = rectCamera.x > rectCamera.w ? rectCamera.w : rectCamera.x;
			rectCamera.y = rectCamera.y > rectCamera.h ? rectCamera.h : rectCamera.y;*/
		}
	}


	void SetMapDimensions(int mMapWidth, int mMapHeight)
	{
		this->mMapWidth = mMapWidth;
		this->mMapHeight = mMapHeight;
	}
};


class MovementSystem
{
public:

	void Update(std::unique_ptr<entt::registry>& mRegistry, float& fDeltaTime)
	{
		auto view = mRegistry->view<TransformComponent, RigidBodyComponent>();
		for (auto [entity, transform, rigid] : view.each())
		{
			if (rigid.bMove)
			{
				float fVelocity = rigid.fVelocity;
				transform.vPosition.x += (fVelocity * glm::cos(transform.dRotation) * fDeltaTime);
				transform.vPosition.y += (fVelocity * glm::sin(transform.dRotation) * fDeltaTime);
			}
		}
	}

};


//sets the target position for the player entity when clicked
//obviously checks if the target position is possible and the tile is not an obstacle or anything
class MouseInputSystem
{
	bool bMouseLPressed;

public:
	MouseInputSystem()
	{
		bMouseLPressed = false;
	}

	void ProcessInput(std::unique_ptr<entt::registry>& mRegistry, std::unique_ptr<entt::dispatcher>& mDispatcher, SDL_Event& e, SDL_Rect& rectCamera)
	{
		bool bEmitEvent = false;
		//update the location of the cursor in accordance with the grid system and mouse movements
		int iMouseX, iMouseY;
		SDL_GetMouseState(&iMouseX, &iMouseY);
		switch (e.type)
		{
		case SDL_MOUSEBUTTONDOWN:
			if (!bMouseLPressed)
				bMouseLPressed = true;
			break;

		case SDL_MOUSEBUTTONUP:
			//trigger the pathfinding event TargetPositionEvent
			if (bMouseLPressed)
			{
				bEmitEvent = true;
				bMouseLPressed = false;
			}
			break;
		}

		//update mouse position wrt camera
		auto view = mRegistry->view<TransformComponent, MouseInputComponent>();
		for (auto [entity, transform] : view.each())					
		{
			glm::ivec2 ivGridPos = WorldGrid::GetGridPos(static_cast<float>(iMouseX + rectCamera.x), static_cast<float>(iMouseY + rectCamera.y));
			transform.vPosition = glm::vec2(static_cast<float>(ivGridPos.x) * WorldGrid::fTileSize, static_cast<float>(ivGridPos.y) * WorldGrid::fTileSize);
			
			//trigger the Astar path event 
			if (bEmitEvent)
			{
				auto viewPlayer = mRegistry->view<TransformComponent, PathfindingComponent>();
				for (auto [entityPlayer, transform, pathfinding] : viewPlayer.each())
				{
					mDispatcher->trigger<TargetPositionEvent>(entityPlayer, WorldGrid::GetGridPos(transform.vPosition.x, transform.vPosition.y), ivGridPos);
				}
			}
		}
	}
};




struct RenderingSystem
{
	void Update(SDL_Renderer* mRenderer, std::unique_ptr<entt::registry>& registry, SDL_Rect& rectCamera)
	{
		SDL_Rect rectDest;
		auto view = registry->view<TransformComponent, SpriteComponent>();
		for (auto [entity, transform, sprite] : view.each())					
		{
			rectDest = { static_cast<int>(transform.vPosition.x - rectCamera.x), static_cast<int>(transform.vPosition.y - rectCamera.y), static_cast<int>(WorldGrid::fTileSize), static_cast<int>(WorldGrid::fTileSize) };
			SDL_RenderCopy(mRenderer, sprite.texSprite, NULL, &rectDest);
		}
	}
};