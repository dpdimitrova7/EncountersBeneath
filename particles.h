#pragma once
#include "ofMain.h"

class Particle  {
public:
	void setup();
	void update();
	void draw();
	ofVec2f pos;
    ofVec2f velocity;
	ofColor colour;
	float radius;

	vector<Particle>particles;
};

