#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <map>
#include <string>
#include <spdlog/spdlog.h>

class AssetStore
{
	std::map<std::string, SDL_Texture*> mapTextures;
public:

	AssetStore() {}
	~AssetStore();

	void AddTexture(SDL_Renderer* mRenderer, std::string strID, const char* szPath);
	SDL_Texture* GetTexture(std::string strID);

};
