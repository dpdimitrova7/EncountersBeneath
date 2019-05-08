#include "particles.h"


void Particle::setup(){ 
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	radius = 1;

	// assign it's starting position
	pos.x = ofRandom(radius, ofGetWidth());
	pos.y = ofRandom(radius, ofGetHeight()-300);

	// assign it's velocity
	velocity.x = ofRandom(0, 0.1);
	velocity.y = ofRandom(-1, 0);

//	 give it a randomised colour
	colour.r = ofRandom(220, 255);
	colour.g = ofRandom(230, 255);
	colour.b = ofRandom(250, 255);
	
}


void Particle::update(){
	pos.x += velocity.x;
	pos.y += velocity.y;

	// calculate min and max for both x and y
	// to keep the maths simple, ignore the radius
	float minX = 0;
	float minY = 300;
	float maxX = ofGetWidth();
	float maxY = ofGetHeight();

	// need to detect whether the particle has reached the
	// edge of the container by comparing the particles
	// position with one edge at a time. Each time an edge
	// is reached the velocity

	ofVec2f reverseX(-1, 1);
	ofVec2f reverseY(1, -1);

	// right
	if (pos.x > maxX || pos.x < minX) {
		velocity *= reverseX;
	}

	if (pos.y > maxY || pos.y < minY) {
		velocity *= reverseY;
	}
}

void Particle::draw() {
	ofEnableBlendMode(OF_BLENDMODE_SCREEN);
	ofSetColor(colour,100);
	ofDrawCircle(pos.x, pos.y, (ofGetFrameNum() % 2) + 0.02);

}


