#include "AssetStore.h"

AssetStore::~AssetStore()
{
	for (auto it : mapTextures)
		SDL_DestroyTexture(it.second);
	mapTextures.clear();
}

void AssetStore::AddTexture(SDL_Renderer* mRenderer, std::string strID, const char* szPath)
{
	SDL_Surface* surface = IMG_Load(szPath);
	if (!surface)
	{
		spdlog::error(std::string(IMG_GetError()));
		return;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
	//dont need it anymore free it
	SDL_FreeSurface(surface);

	mapTextures[strID] = texture;
}

SDL_Texture* AssetStore::GetTexture(std::string strID)
{
	return mapTextures[strID];
}


