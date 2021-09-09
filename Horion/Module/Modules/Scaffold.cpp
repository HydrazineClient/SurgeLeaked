﻿#include "Scaffold.h"

#include "../../../Utils/Logger.h"

Scaffold::Scaffold() : IModule(VK_NUMPAD1, Category::WORLD, "Automatically build blocks beneath you") {
	registerBoolSetting("Spoof", &this->spoof, this->spoof);
	registerBoolSetting("AirPlace", &this->airplace, this->airplace);
	registerBoolSetting("Auto Select", &this->autoselect, this->autoselect);
	registerBoolSetting("Entity Scaffold", &this->entityscaff, this->entityscaff);
	registerBoolSetting("Predict", &this->predict, this->predict);
	registerBoolSetting("Staircase Mode", &this->staircaseMode, this->staircaseMode);
}

Scaffold::~Scaffold() {
}

const char* Scaffold::getModuleName() {
	return "Scaffold";
}

bool Scaffold::tryScaffold(vec3_t blockBelow) {
	blockBelow = blockBelow.floor();

	C_Block* block = g_Data.getLocalPlayer()->region->getBlock(vec3_ti(blockBelow));
	C_BlockLegacy* blockLegacy = (block->blockLegacy);
	if (blockLegacy->material->isReplaceable) {
		vec3_ti blok(blockBelow);
		int i = 0;
		if (airplace) {
			g_Data.getCGameMode()->buildBlock(&blok, i);
			return true;
		} else {
			static std::vector<vec3_ti*> checklist;
			bool foundCandidate = false;
			if (checklist.empty()) {
				checklist.push_back(new vec3_ti(0, -1, 0));
				checklist.push_back(new vec3_ti(0, 1, 0));

				checklist.push_back(new vec3_ti(0, 0, -1));
				checklist.push_back(new vec3_ti(0, 0, 1));

				checklist.push_back(new vec3_ti(-1, 0, 0));
				checklist.push_back(new vec3_ti(1, 0, 0));
			}

			for (auto current : checklist) {
				vec3_ti calc = blok.sub(*current);
				bool Y = ((g_Data.getLocalPlayer()->region->getBlock(calc)->blockLegacy))->material->isReplaceable;
				if (!((g_Data.getLocalPlayer()->region->getBlock(calc)->blockLegacy))->material->isReplaceable) {
					// Found a solid block to click
					foundCandidate = true;
					blok = calc;
					break;
				}
				i++;
			}
			if (foundCandidate) {
				g_Data.getCGameMode()->buildBlock(&blok, i);
				return true;
			}
			//g_Data.getCGameMode()->buildBlock(&blok, i);
		}
	}
	return false;
}

bool Scaffold::findBlock() {
	C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();
	C_Inventory* inv = supplies->inventory;
	auto prevSlot = supplies->selectedHotbarSlot;
	for (int n = 0; n < 9; n++) {
		C_ItemStack* stack = inv->getItemStack(n);
		if (stack->item != nullptr) {
			if (stack->getItem()->isBlock()) {
				if (prevSlot != n)
					supplies->selectedHotbarSlot = n;
				return true;
			}
		}
	}
	return false;
}

void Scaffold::onTick(C_GameMode* gm) {
	if (g_Data.getLocalPlayer() == nullptr)
		return;
	if (!g_Data.canUseMoveKeys())
		return;
	if (autoselect) {
		findBlock();
	}
	auto selectedItem = g_Data.getLocalPlayer()->getSelectedItem();
	if ((selectedItem == nullptr || selectedItem->count == 0 || selectedItem->item == nullptr || !selectedItem->getItem()->isBlock()) && !spoof)  // Block in hand?
		return;

	// Adjustment by velocity
	float speed = g_Data.getLocalPlayer()->velocity.magnitudexz();
	vec3_t vel = g_Data.getLocalPlayer()->velocity;
	vel = vel.normalize();  // Only use values from 0 - 1

	if (this->staircaseMode) {
		vec3_t blockBelow = g_Data.getLocalPlayer()->eyePos0;  // Block 1 block below the player
		blockBelow.y -= g_Data.getLocalPlayer()->height;
		blockBelow.y -= 1.5f;

		vec3_t blockBelowBelow = g_Data.getLocalPlayer()->eyePos0;  // Block 2 blocks below the player
		blockBelowBelow.y -= g_Data.getLocalPlayer()->height;
		blockBelowBelow.y -= 2.0f;

		if (!tryScaffold(blockBelow) && !tryScaffold(blockBelowBelow)) {
			if (speed > 0.05f) {  // Are we actually walking?
				blockBelow.z -= vel.z * 0.4f;
				blockBelowBelow.z -= vel.z * 0.4f;
				if (!tryScaffold(blockBelow) && !tryScaffold(blockBelowBelow)) {
					blockBelow.x -= vel.x * 0.4f;
					blockBelowBelow.x -= vel.x * 0.4f;
					if (!tryScaffold(blockBelow) && !tryScaffold(blockBelowBelow) && g_Data.getLocalPlayer()->isSprinting()) {
						blockBelow.z += vel.z;
						blockBelow.x += vel.x;
						blockBelowBelow.z += vel.z;
						blockBelowBelow.x += vel.x;
						tryScaffold(blockBelow);
						tryScaffold(blockBelowBelow);
					}
				}
			}
		}
	}
	if (!entityscaff || !staircaseMode) {
		vec3_t blockBelow = g_Data.getLocalPlayer()->eyePos0;  // Block below the player
		blockBelow.y -= g_Data.getLocalPlayer()->height;
		blockBelow.y -= 0.5f;

		blockBelow.z -= vel.z * 0.4f;
		if (!tryScaffold(blockBelow)) {
			blockBelow.x -= vel.x * 0.4f;
			blockBelow.z += vel.z;
			blockBelow.x += vel.x;
			tryScaffold(blockBelow);
			blockBelow.z -= vel.z * 0.3f;
			if (predict) {
				if (!tryScaffold(blockBelow)) {
					blockBelow.x -= vel.x * 0.2f;
					blockBelow.z += vel.z;
					blockBelow.x += vel.x;
					tryScaffold(blockBelow);
					blockBelow.z -= vel.z * 0.1f;
				}
			}
		}
	}
	if (this->entityscaff) {
		vec3_t blockBelow = g_Data.getLocalPlayer()->eyePos0;  // Block 1 block below the player
		blockBelow.y -= g_Data.getLocalPlayer()->height;
		blockBelow.y -= 1.5f;

		//vec3_t blockBelowBelow = g_Data.getLocalPlayer()->eyePos0;  // Block 2 blocks below the player
		//blockBelowBelow.y -= g_Data.getLocalPlayer()->height;
		//blockBelowBelow.y -= 2.0f;

		if (!tryScaffold(blockBelow)) {
			if (speed > 0.05f) {  // Are we actually walking?
				blockBelow.z -= vel.z * 0.4f;
				//blockBelowBelow.z -= vel.z * 0.4f;
				if (!tryScaffold(blockBelow)) {
					blockBelow.x -= vel.x * 0.4f;
					//blockBelowBelow.x -= vel.x * 0.4f;
					if (!tryScaffold(blockBelow)) {
						blockBelow.z += vel.z;
						blockBelow.x += vel.x;
						//blockBelowBelow.z += vel.z;
						//blockBelowBelow.x += vel.x;
						tryScaffold(blockBelow);
						//tryScaffold(blockBelowBelow);
					}
				}
			}
		}
		if (!entityscaff || !staircaseMode) {
			vec3_t blockBelow = g_Data.getLocalPlayer()->eyePos0;  // Block below the player
			blockBelow.y -= g_Data.getLocalPlayer()->height;
			blockBelow.y -= 0.5f;

			blockBelow.z -= vel.z * 0.4f;
			if (!tryScaffold(blockBelow)) {
				blockBelow.x -= vel.x * 0.4f;
				blockBelow.z += vel.z;
				blockBelow.x += vel.x;
				tryScaffold(blockBelow);
				blockBelow.z -= vel.z * 0.3f;
				if (predict) {
					if (!tryScaffold(blockBelow)) {
						blockBelow.x -= vel.x * 0.2f;
						blockBelow.z += vel.z;
						blockBelow.x += vel.x;
						tryScaffold(blockBelow);
						blockBelow.z -= vel.z * 0.1f;
					}
				}
			}
		}
	}
	if (this->spoof) {
		C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();
		C_Inventory* inv = supplies->inventory;
		supplies->selectedHotbarSlot = slot;
	}
}

void Scaffold::onDisable() {
	if (g_Data.getLocalPlayer() == nullptr)
		return;
}