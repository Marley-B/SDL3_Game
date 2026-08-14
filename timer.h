#pragma once

class Timer {
	float lenght, time;
	bool timeout;
public:
	Timer(float lenght) : lenght(lenght), time(0), timeout(false) {

	}

	bool step(float deltaTime) {
		time += deltaTime;
		if (time >= lenght) {
			time -= lenght; 
			// this way we dont lose the seconds it went over the lenght by
			timeout = true;
			return true;
		}
		return false;
	}

	bool isTimeout() const { return timeout;  }
	float getTime() const { return time; }
	float getLenght() const { return lenght; }
	void reset() { time = 0, timeout = false; }
};