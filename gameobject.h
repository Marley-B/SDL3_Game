#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>	
#include "animation.h"

enum class PlayerState {
	idle, running, jumping
	// idle, flying,no stamina, hit, dead
};

enum class CoinState { // coind and stamina recharge behave the same... FUTURE
	colliding, inactive
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
	Timer weaponTimer; // REMOVE
	int staminaPoints;
	PlayerData() : weaponTimer(0.1f){
		state = PlayerState::idle;
		staminaPoints = 100;
	}
};

struct LevelData {};

struct CoinData {};

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
	CoinData coin;
	//EnemyData enemy;
	//BulletData bullet;
};

enum class ObjectType {
	player, level, coin,
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
	bool grounded; //REMOVE
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
		grounded = false;  // REMOVE
		shouldFlash = false;
		spriteFrame = 0;

		colliding = false;
	}
};