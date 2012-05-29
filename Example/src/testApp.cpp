
#include "testApp.h"


//--------------------------------------------------------------

void testApp::setup(){

	ofSetFrameRate(24);
	ofEnableAlphaBlending();
	ofEnableSmoothing();
	
	// create our ofxCairoTexture 
	cairoTexture.setup(ofGetWidth(), ofGetHeight());
	cairoTexture.setupGraphicDefaults();
	//cairoTexture.setBlendMode(OF_BLENDMODE_SUBTRACT);
	
	// do a little drawing
	//cairoTexture.background(ofColor(33, 255, 123));
	cairoTexture.setLineWidth(35);
	cairoTexture.setFillMode(OF_FILLED);
	cairoTexture.setColor(ofColor(255, 123, 33));
	cairoTexture.drawCircle(ofGetWidth()/2, ofGetHeight()/2, 0, ofGetHeight()/3);
	
	// update the texture (copy pixels from cairo to texture)
	cairoTexture.update();
}

//--------------------------------------------------------------
void testApp::update(){

	
} 

//--------------------------------------------------------------
void testApp::draw(){
	
	ofBackground(255, 255, 255);
	
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	ofSetColor(255, 255, 255);
	cairoTexture.draw(0, 0);
	
	/*
	 // you can also copy the pixels from the texture as you would with an ofImage
	 image.setFromPixels(cairoTexture.getPixels(), cairoTexture.getWidth(), cairoTexture.getHeight(), OF_IMAGE_COLOR_ALPHA, false);	
	 image.draw(0, 0);
	 */
}

//--------------------------------------------------------------

void testApp::keyPressed(int key){
	
	if( key == 's' ){
	
		cairoTexture.save("cairoTexture.png");
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	
}

