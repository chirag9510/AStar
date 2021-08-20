#include "Core.h"
#include <SDL_image.h>
#include <string>
#include <fstream>
#include <spdlog/spdlog.h>


Core::Core()
{
	mWidth = 800;
	mHeight = 600;
	iTicksLastFrame = 0;
	iMaxLevels = 3;
	iLevel = 1;
	bRunning = true;
	mRegistry = std::make_unique<entt::registry>();
	mDispatcher = std::make_unique<entt::dispatcher>();
	mAssetStore = std::make_unique<AssetStore>();
	mAStarSystem = std::make_unique<AStarPathfindingSystem>();
	mPathfollowingSystem = std::make_unique<PathFollowingSystem>();
	mRenderingSystem = std::make_unique<RenderingSystem>();
	mMouseInputSystem = std::make_unique<MouseInputSystem>();
	mMovementSystem = std::make_unique<MovementSystem>();
	mCameraFollowingSystem = std::make_unique<CameraFollowingSystem>();
}

Core::~Core()
{
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);

	IMG_Quit();
	SDL_Quit();
}

void Core::Init()
{
	//SDL Init
	SDL_Init(SDL_INIT_VIDEO);
	mWindow = SDL_CreateWindow("Map", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mWidth, mHeight, SDL_WINDOW_RESIZABLE);
	if (!mWindow)
		spdlog::error("mWindow : " + std::string(SDL_GetError()));
	mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
	if (!mRenderer)
		spdlog::error("mRenderer : " + std::string(SDL_GetError()));
	IMG_Init(IMG_INIT_PNG);

	//systems subscribing to events 
	mDispatcher->sink<TargetPositionEvent>().connect<&AStarPathfindingSystem::ProcessPathNodes>(mAStarSystem);

	LoadAssets();
}

void Core::LoadAssets()
{
	mAssetStore->AddTexture(mRenderer, "sprite-tile", "./assets/tile.png");
	mAssetStore->AddTexture(mRenderer, "sprite-cursor", "./assets/cursor.png");
	mAssetStore->AddTexture(mRenderer, "sprite-wall", "./assets/wall.png");
	mAssetStore->AddTexture(mRenderer, "sprite-player", "./assets/player.png");
	mAssetStore->AddTexture(mRenderer, "sprite-closedlist", "./assets/tile-closedlist.png");
	mAssetStore->AddTexture(mRenderer, "sprite-openlist", "./assets/tile-openlist.png");
	mAssetStore->AddTexture(mRenderer, "sprite-stairs", "./assets/tile-stairs.png");

	LoadLevel();
}

void Core::LoadLevel()
{
	//load all entities and components
	entt::entity entityCursor = mRegistry->create();
	mRegistry->emplace<TransformComponent>(entityCursor, glm::vec2(0.0f));
	mRegistry->emplace<SpriteComponent>(entityCursor, mAssetStore->GetTexture("sprite-cursor"), glm::ivec2(32));
	mRegistry->emplace<MouseInputComponent>(entityCursor);

	entt::entity entityPlayer = mRegistry->create();
	auto& transformPlayer = mRegistry->emplace<TransformComponent>(entityPlayer, glm::vec2(0.0f));
	mRegistry->emplace<PathfindingComponent>(entityPlayer);
	mRegistry->emplace<CameraFollowComponent>(entityPlayer);
	mRegistry->emplace<RigidBodyComponent>(entityPlayer, 200.0f);
	mRegistry->emplace<SpriteComponent>(entityPlayer, mAssetStore->GetTexture("sprite-player"), glm::ivec2(32));

	//Load the tilemap
	std::fstream fileTilemap;
	std::string strFilename = "tilemap" + std::to_string(iLevel) + ".csv";
	fileTilemap.open("./assets/" + strFilename);
	if (fileTilemap.is_open())
	{
		char ch;
		int iRow = 0, iColumn = 0;
		while ((ch = fileTilemap.get()) != EOF)
		{
			//check what to do with newline or end of file
			if (ch == '-')
				fileTilemap.ignore();
			else if (ch == '\n')
			{
				iRow++;
				iColumn = 0;
			}
			else if (ch == ',')					//just a hack for endoffile
				iColumn++;
			else
			{
				int8_t index = atoi(&ch);
				//0 in the map is just empty space ignore it
				if (index >= 0)
				{
					glm::vec2 vPosition = glm::vec2(static_cast<float>(iColumn) * WorldGrid::fTileSize, static_cast<float>(iRow) * WorldGrid::fTileSize);
					glm::ivec2 ivGridPos = WorldGrid::GetGridPos(vPosition.x, vPosition.y);
					entt::entity entity = mRegistry->create();
					mRegistry->emplace<TransformComponent>(entity, vPosition);
					switch (index)
					{
					case WALL:
						mRegistry->emplace<SpriteComponent>(entity, mAssetStore->GetTexture("sprite-wall"), glm::ivec2(32));
						mRegistry->emplace<TileComponent>(entity, WALL, ivGridPos);
						break;

					case SPAWN:
						transformPlayer.vPosition = WorldGrid::GetGridPos(ivGridPos);
					case PATH:
						mRegistry->emplace<SpriteComponent>(entity, mAssetStore->GetTexture("sprite-tile"), glm::ivec2(32));
						mRegistry->emplace<TileComponent>(entity, PATH, ivGridPos);
						//only insert valid path nodes to the system
						mAStarSystem->InsertNode(ivGridPos);
						break;
					case FINISH:
						mPathfollowingSystem->SetNodeNextLevel(ivGridPos);
						mRegistry->emplace<SpriteComponent>(entity, mAssetStore->GetTexture("sprite-stairs"), glm::ivec2(32));
						mRegistry->emplace<TileComponent>(entity, FINISH, ivGridPos);
						mAStarSystem->InsertNode(ivGridPos);
						break;
					}
				}
			}
		}

		//init camera 
		rectCamera = { 0,0, mWidth, mHeight };
		mCameraFollowingSystem->SetMapDimensions(iColumn * (WorldGrid::fTileSize), iRow * static_cast<int>(WorldGrid::fTileSize));
	}
	else
		spdlog::error("Tilemap file not found : " + strFilename);

}

void Core::Run()
{
	while (bRunning)
	{
		ProcessInput();
		Update();
		Render();
	}
}


void Core::ProcessInput()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		mMouseInputSystem->ProcessInput(mRegistry, mDispatcher, e, rectCamera);
		switch (e.type)
		{
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				bRunning = false;
				break;
			}

			break;

		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				SDL_GetWindowSize(mWindow, &mWidth, &mHeight);
				rectCamera = { 0,0, mWidth, mHeight };
			}	
			break;
		case SDL_QUIT:
			bRunning = false;
			break;
		}

	}
}

void Core::Update()
{
	//frame cap
	int iFrameLimit = 16 - (SDL_GetTicks() - iTicksLastFrame);
	if (iFrameLimit <= 16 && iFrameLimit >= 0)
		SDL_Delay(iFrameLimit);
	float fDeltaTime = static_cast<float>(SDL_GetTicks() - iTicksLastFrame) / 1000.0f;
	iTicksLastFrame = SDL_GetTicks();

	mAStarSystem->Update(mRegistry, mAssetStore);
	if (mPathfollowingSystem->Update(mRegistry, fDeltaTime))
	{
		//load the next level
		auto view = mRegistry->view<TransformComponent>();
		mRegistry->destroy(view.begin(), view.end());
		iLevel = iLevel == iMaxLevels ? 1 : ++iLevel;
		mAStarSystem->Clear();
		LoadLevel();
	}
	mMovementSystem->Update(mRegistry, fDeltaTime);
	mCameraFollowingSystem->Update(mRegistry, rectCamera);
}

void Core::Render()
{
	SDL_RenderClear(mRenderer);

	mRenderingSystem->Update(mRenderer, mRegistry, rectCamera);

	SDL_RenderPresent(mRenderer);
}
