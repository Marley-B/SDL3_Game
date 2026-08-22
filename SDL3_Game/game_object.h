#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>	
#include "animation.h"
#include "timer.h"

//enum class PlayerState {
//	idle, running, flying, noStamina, hit, dead
//	// idle, flying,no stamina, hit, dead
//};
//
//enum class PickUpState { 
//	idle, colliding, inactive
//};

/*
enum class BulletState {
	moving, colliding, inactive
};

enum class EnemyState {
	shambaling, damaged, dead
};
*/

struct PlayerData {
	//PlayerState state;
	int collectedCoins;
	float staminaPoints;
	Timer damagedTimer;
	bool invincible;
	int maxStamina;
	PlayerData() : damagedTimer(0.5f) {
		//state = PlayerState::idle;
		staminaPoints = 100;
		maxStamina = 100;
		collectedCoins = 0;
		invincible = false;
	}
};

struct LevelData {};

struct PickUpData {
	//PickUpState state;
	float value;
	//PickUpData() : state(PickUpState::idle)
	PickUpData() {
		value = 0;
	}
};


/*
struct EnemyData {
	EnemyState state;
	Timer damagedTimer;
	int healthPoints;
	EnemyData() : state(EnemyState::shambaling), damagedTimer(0.5f) {
		healthPoints = 100;
	}
};
struct BulletData {
	BulletState state;
	BulletData() : state(BulletState::moving) {

	}
};
*/

union ObjectData {
	PlayerData player;
	LevelData level;
	PickUpData pickUp;
	//EnemyData enemy;
	//BulletData bullet;
};

enum class ObjectType {
	player, level, coin, juice, win
};

struct GameObject {
	ObjectType type;
	ObjectData data;
	glm::vec2 position, velocity, acceleration;
	float directionH;
	float directionV;
	float maxSpeedX;
	float maxSpeedY;
	std::vector<Animation> animations;
	int currentAnimation;
	SDL_Texture* texture;
	bool dynamic;
	SDL_FRect collider;
	Timer flashTimer;
	bool shouldFlash;
	int spriteFrame;
	float visualDirectionH = 1;
	bool invisible;

	GameObject() : data{ .level = LevelData() }, collider{ 0 }, flashTimer(0.5f) {
		type = ObjectType::level;
		directionH = 1;
		directionV = 1;
		maxSpeedX = 0;
		maxSpeedY = 0;
		position = velocity = acceleration = glm::vec2(0);
		currentAnimation = -1;
		texture = nullptr;
		dynamic = false;
		shouldFlash = false;
		spriteFrame = 0;
		invisible = false;
	}
};