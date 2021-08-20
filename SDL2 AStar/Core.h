#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <cstdint>
#include <memory>
#include <entt/entt.hpp>
#include "Systems.h"

class Core
{
	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
	int iTicksLastFrame;
	bool bRunning;
	SDL_Rect rectCamera;
	int iMaxLevels, iLevel;

	std::unique_ptr<entt::registry> mRegistry;
	std::unique_ptr<entt::dispatcher> mDispatcher;
	std::unique_ptr<AssetStore> mAssetStore;
	std::unique_ptr<AStarPathfindingSystem> mAStarSystem;
	std::unique_ptr<PathFollowingSystem> mPathfollowingSystem;
	std::unique_ptr<RenderingSystem> mRenderingSystem;
	std::unique_ptr<MouseInputSystem> mMouseInputSystem;
	std::unique_ptr<MovementSystem> mMovementSystem;
	std::unique_ptr<CameraFollowingSystem> mCameraFollowingSystem;

	void LoadAssets();
	void LoadLevel();
	void ProcessInput();
	void Update();
	void Render();

public:
	int mWidth, mHeight;

	Core();
	~Core();

	void Init();
	void Run();
};

