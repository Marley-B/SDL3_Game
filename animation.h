#pragma once
#include "timer.h"

class Animation {
	Timer timer;
	int frameCount;

public:
	Animation() : timer(0),  frameCount(0){}
	Animation(int frameCount, float lenght) : frameCount(frameCount), timer(lenght){}

	float getLength() const { return timer.getLenght(); }
	int currentFrame() const {
		// return the frame of the animation it sould be playing as an int 
		return static_cast<int>(timer.getTime() / timer.getLenght() * frameCount);
	}

	void step(float deltaTime) {
		timer.step(deltaTime);
	}

	bool isDone() const { return timer.isTimeout(); }
};