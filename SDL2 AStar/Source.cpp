#include "Core.h"

int main(int argv, char** argc)
{
	std::unique_ptr<Core> core(std::make_unique<Core>());
	core->Init();
	core->Run();
	return 0;
}