#pragma once

#include "../ModuleManager.h"
#include "Module.h"

class InfiniteReach : public IModule {
private:
	int delay = 15;
	int Odelay = 0;
	bool isMulti = false;

public:
	bool isMobAura = false;
	float range = 15;
	InfiniteReach();
	~InfiniteReach();
	// Inherited via IModule
	virtual const char* getModuleName() override;
	virtual void onTick(C_GameMode* gm) override;
};
