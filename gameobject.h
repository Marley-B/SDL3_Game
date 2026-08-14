#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>	
#include "animation.h"

enum class PlayerState {
	idle, running, noStamina, hit, dead
	// idle, flying,no stamina, hit, dead
};

enum class PickUpState { 
	idle, colliding, inactive
};

/*
enum class BulletState {
	moving, colliding, inactive
};

enum class EnemyState {
	shambaling, damaged, dead
};
*/

struct PlayerData {
	PlayerState state;
	int collectedCoins;
	int staminaPoints;
	PlayerData() {
		state = PlayerState::idle;
		staminaPoints = 100;
		collectedCoins = 0;
	}
};

struct LevelData {};

struct PickUpData {
	PickUpState state;
	PickUpData() : state(PickUpState::idle) {
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
	player, level, coin, juice,
	//enemy, bullet
};

struct GameObject {
	ObjectType type;
	ObjectData data;
	glm::vec2 position, velocity, acceleration;
	float direction;
	float maxSpeedX;
	std::vector<Animation> animations;
	int currentAnimation;
	SDL_Texture* texture;
	bool dynamic;
	SDL_FRect collider;
	Timer flashTimer;
	bool shouldFlash;
	int spriteFrame;

	bool colliding; //?

	GameObject() : data{ .level = LevelData() }, collider{ 0 }, flashTimer(0.05f) {
		type = ObjectType::level;
		direction = 1;
		maxSpeedX = 0;
		position = velocity = acceleration = glm::vec2(0);
		currentAnimation = -1;
		texture = nullptr;
		dynamic = false;
		shouldFlash = false;
		spriteFrame = 0;

		colliding = false;
	}
};