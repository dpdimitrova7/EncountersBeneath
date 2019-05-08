#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

   //synchronize the number of the frames displayed per second
	ofSetVerticalSync(true);
	smallFont.loadFont("selena.otf", 16); //http://openfontlibrary.org/en/font/selena
	largeFont.loadFont("selena.otf", 48);

	//sending all messages
	ofSetLogLevel(OF_LOG_VERBOSE);

	//load a sample image to work with
	image.loadImage("seabed005.png");

	
	//START BLURRING
	blur.setup(ofGetWidth(), ofGetHeight(), 10, 0.2, 2);
	//set the dimensions of the cone for radius and height
	cone.set(52, 450);
	//
	last = ofGetElapsedTimeMillis();
	colTransformation.setHsb(0, 80, 255);

	//SETUP FOR KINECT
	//the code for Kinect here just initializes OSC receiver and parser, and the paramaters are port number and font for the debugging text
	kinect.setup(12345, smallFont);
	//a pointer for the number of skeletons
	skeletons = kinect.getSkeletons();
	//passing the list of skeletons to the default renderer class
	renderer.setup(skeletons, largeFont);
	//if you enable depth it obscures drawing the cone lights
    //ofEnableDepthTest();
	ofDisableArbTex();

	//turn the camera
	myCamera.tiltRad(1.6);

	//initializing coordinates for the mesh (the waves)
	scale = 10; // set Scale of grids
	terrainWidth = 800;
	terrainHeight = 600;
	columns = terrainWidth / scale; // set rows per scl
	rows = terrainHeight / scale; // set columns per scl
   	mesh.setMode(OF_PRIMITIVE_TRIANGLES);

	//loop the rows and columns to create terrain of noise
	for (int j = 0; j < rows; j++) {
		xoff = 1;
		for (int i = 0; i < columns; i++) {
			float x = i * scale;
			float y = j * scale;
			//float z = ofRandom(-10,10);
			float z = ofMap(ofNoise(yoff, xoff), 0, 1, -50, 20);
			mesh.addVertex(ofVec3f(x, y, z));
			mesh.addColor(ofFloatColor(0.4, 1.2, 1.9));
			mesh.addColor(ofFloatColor(ofMap(z, 50, 50, 0, 0.3), ofMap(z, -50, 180, 0, 0.4), ofMap(z, -50, 60, 0, 1), 0.07));
			xoff += 0.1;
		}
		yoff += 0.1;
	}
	// Generate order of indices to set triangles per rows and column
	for (int j = 0; j < rows - 1; j++) {
		for (int i = 0; i < columns - 1; i++) {
			mesh.addIndex(i + j * columns);         // 0
			mesh.addIndex((i + 1) + j * columns);     // 1
			mesh.addIndex(i + (j + 1)*columns);     // 6

			mesh.addIndex((i + 1) + j * columns);     // 1
			mesh.addIndex((i + 1) + (j + 1)*columns); // 7
			mesh.addIndex(i + (j + 1)*columns);     // 6
		}
	}

	//setup arduino communication
	serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
	serial.setup("COM4", 9600);

    //  load png sequence
		loadFrames(125);
    	count = 0;

	//vector of particles for the background
	const int NUM_P = 500;
	for (int i = 0; i < NUM_P; i++) {
		Particle particle;
		particle.setup();
		particles.push_back(particle);
	}
	
	//set the parameters of the 3d large box which functions as the surface above the sea
	ofFloatColor(0, 230, 40);
	sky.set(1000,600, 3);
	sky.setPosition(100, -500, 10);
}

float ofApp::getZNoiseValue(int _x, int _y, float n1_freq, float n2_freq, float n1_amp, float n2_amp) {
	int x = _x;
	int y = _y + ofGetFrameNum();
	float n1 = ofNoise(x * n1_freq, y * n1_freq) * n1_amp;
	float n2 = ofNoise(x * n2_freq, y * n2_freq) * n2_amp;
	float ret = n1 + n2;
	return ret;
}
//--------------------------------------------------------------
void ofApp::update() {

	//scale corresponds to how much you stretch the blur
	//rotation corresponds to the two directions the blur is calculated in
	blur.setScale(20.5);
   //blur.setRotation(ofMap(mouseY, 0, ofGetHeight(), -PI, PI));


	//KINECT
	//Each frame check for new Kinect OSC messages
	kinect.update();

	light.setPosition(ofVec3f(0, sin(ofGetElapsedTimef()) * 150, 0));

	for (int i = 0; i < mesh.getNumVertices(); i++) {
		ofVec3f vec;
		vec = mesh.getVertex(i);
		vec.z = getZNoiseValue(vec.x, vec.y, 0.02, 0.02, 0.7, 10);
		mesh.setVertex(i, vec);
	}

	//send information from arduino serial to Openframeworks program
	while (serial.available()) {
		char byteReturned;
		byteReturned = serial.readByte();
		if (byteReturned != '.') {
			recieved += byteReturned;
		}
		else {
			currentVal = ofToInt(recieved);
			//smooth the received value
			currentValSmooth += (currentVal - currentValSmooth)*0.05;
			recieved = "";
			//std::cout << currentVal << std::endl;
		}
	}

	//background particles update
	for (int i = 0; i < particles.size(); i++) {
		particles[i].update();
	}
}


//--------------------------------------------------------------
void ofApp::draw() {

	//std::cout<<"particles"<<endl;
	//SETTING BLUR VALUES
	//if the time time passed is more than a second
	if (ofGetElapsedTimeMillis() - last > 1000)
	{
		colTransformation.setHue(counter % 256);
		counter++;
	}

	//DRAW the particles
	blur.begin();
	for (int i = 0; i < particles.size(); i++) {
		particles[i].draw();
		ofPopMatrix();
	}
	//set color of the cones
	ofSetColor(0, 155, 255, 20);

	//rotate the direction that all cones move in
	ofRotateX(30);
	//blendmode add can be changed to alpha to make it softer or not use a blendmode at all, just to use the blur effect
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	cone.draw();

	std::cout << skeletons->size() << std::endl;
	for (int i = 0; i < skeletons->size(); i++) {
		ofSetColor(ofGetFrameNum()*ofColor::fromHsb(8, 240, 25));
		Joint handLeft = skeletons->at(i).getHandLeft();
		//
		
	
		Joint handRight = skeletons->at(i).getHandRight();
		std::cout << i << " : " << handRight.z() << std::endl;
		std::cout << "is this running?" << std::endl;
		if (handRight.z() > 12) {
			cone.setPosition(handRight.x(), ofMap(handRight.y(), 0, ofGetHeight(), 500, 800), 30);
			ofNoFill();
			ofCircle(ofRandom((handRight.x() - 20), (handRight.x() - 20)), handRight.y() - 50, (ofGetFrameNum() % 30) + 0.9);
			ofDisableBlendMode();
		}
		if(handLeft.z() > 12) {
			cone.setPosition(handLeft.x(), ofMap(handLeft.y(), 0, ofGetHeight(), 500, 800), 30);
			ofPushMatrix();
			ofNoFill();
			ofCircle(ofRandom((handLeft.x() - 20), (handLeft.x() + 20)), handLeft.y() - 50, (ofGetFrameNum() % 40) + 0.4);
			ofPopMatrix();
			
		}
	}

	blur.end();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	//ofBackground(0);
	ofSetColor(colTransformation);
	//uncomment the following statement to draw the png sequence
	
	if (count == frames.size()) {
		count = 0;
	}
	frames[count].draw(0, 0, ofGetWidth(), ofGetHeight());
	if (ofGetFrameNum() % 2 == 0) {
		count++;
	}

	//image.draw(0, 0);
	blur.draw();

	//seabed.draw(0, 0, ofGetWidth(), ofGetHeight());
	//kinect.clearStaleSkeletons;
	//if (kinect.hasSkeletons) {
		kinect.drawDebug();
		//We passed the skeleton list pointer to the renderer earlier,
		//now we tell it to draw them
		renderer.draw();
	//}

	//If you want to stop using the default renderer and start
	//DRAW KINECT CORRESPODANCE HERE

	/*
	//Print out commands and text
	string commands = "COMMANDS\n\n";
	commands.append("d = debug\n");
	commands.append("j = joints\n");
	commands.append("b = bones\n");
	commands.append("h = hands\n");
	commands.append("r = ranges\n");

	ofSetColor(ofColor::white);
	smallFont.drawString(commands, 20, 40);kay 
	largeFont.drawString("fps:\n" + ofToString(ofGetFrameRate()), 20, ofGetHeight() - 100);
	*/
	//KINECTEND
	
	myCamera.begin();
	ofEnableLighting();
	light.enable();
	ofSetColor(255);
	ofRotateX(ofMap(sin(ofGetFrameNum()*0.03),-1,1,738,752)+ofMap(sin(currentValSmooth),0,currentVal,706,752));
	ofTranslate(-500,0,0);
	ofPushMatrix();
	ofTranslate(ofGetWidth()/2 - terrainWidth + scale, ofGetHeight()- terrainHeight * 0.5 + scale * 0.5);
	ofSetColor(102,175, 215);
	sky.draw();
	ofPopMatrix();
	mesh.draw();
	light.disable();
	ofDisableLighting();
	myCamera.end();

	//display the the mesh without color and texture
	/*
	if (showIndices) {
		for (int j = 0; j < rows; j++) {
			for (int i = 0; i < cols; i++) {
				ofDrawBitmapStringHighlight(ofToString(i + j * cols), ofPoint(mesh.getVertex(i + j * cols).x - 10, mesh.getVertex(i + j * cols).y + 5));
			}
		}
	}
	*/

	//draw a string which presents the value sent from arduino
	ofDrawBitmapString(msg, 50, 20);

}

//FUNCTION TO LOAD PNG

void ofApp::loadFrames(int n) {
	for (int i = 0; i < n; i++) {
		//strings are objects that represent sequences of characters;
		std::ostringstream localOSS;
		localOSS << "seabed" << setw(3) << setfill('0') << i << ".png";
		string path = "seabedAsStills/" + localOSS.str();
		std::cout << path << std::endl;
		ofImage temp;
		//loading a path of all images that end with .gif
		temp.load(path);
		//pushing the images back to the vector
		frames.push_back(temp);
	}

}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	//KINECT
	if (key == 'd') kinect.toggleDebug();
	if (key == 'j') renderer.toggleJoints();
	if (key == 'b') renderer.toggleBones();
	if (key == 'h') renderer.toggleHands();
	if (key == 'r') renderer.toggleRanges();
	if (key == 'f') {
		ofToggleFullscreen();
	}
	//KINECTEND

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	if (key == 'i') {
		showIndices = !showIndices;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
