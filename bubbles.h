#pragma once
#include "ofMain.h"

enum particleMode {
	PARTICLE_MODE_ATTRACT = 0,

};

class demoParticle {

public:
	demoParticle();

	void setMode(particleMode newMode);
	void setAttractPoints(vector <ofPoint> * attract);

	void reset();
	void update(ofPoint(kinX),ofPoint(kinY));
	void draw();

	ofPoint pos;
	ofPoint vel;
	ofPoint frc;

	float drag;
	float uniqueVal;
	float scale;

	particleMode mode;

	vector <ofPoint> * attractPoints;
};