#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxFaceTracker.h"
#include "DynamicInput.h"
#include "ofxSyphonServer.h"
#include "ofxOsc.h"


#define USE_DYNAMIC_INPUT 0
#define FRAME_RATE 30
#define MOVIE_FILENAME "P.MOV"
#define OSC_INPUT_PORT 8339


class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void alphaBlur(ofBaseHasTexture& tex, ofFbo& result);
	void maskBlur(ofBaseHasTexture& tex, ofFbo& result);
	void radialBlur(ofBaseHasTexture& tex, ofFbo& result);
	void buildVoronoiFace();
	void normalizeImage(ofImage& refImg, ofImage& img, ofImage& normalized);
	void normalizeMesh(ofMesh& mesh);
	void drawNormalized(ofxFaceTracker& tracker);
	void drawNormalized(ofxFaceTracker& tracker, ofBaseHasTexture& tex, ofFbo& result);
	void keyPressed(int key);

	static const int normalizedWidth = 256;
	static const int normalizedHeight = 256;
	
	// this (approximately) makes the mesh hit the edges of the fbos
    const float normalizedMeshScale = 1400;

    void receiveOSC();
    ofxOscReceiver oscReceiver;

#if USE_DYNAMIC_INPUT
	DynamicInput input;
#else
    ofVideoPlayer input;
#endif

	ofxFaceTracker dstTracker;
	ofFbo dstNormalized, dstBlur, output, radialBlurOutput;
	ofShader radialBlurShader;
    ofVec2f radialBlurOrigin;
    ofVec2f radialBlurWidth;

    ofxSyphonServer syphonOutput;
	
	ofxFaceTracker srcTracker;
	ofImage src;
	ofFbo srcNormalized, srcBlur;
	
	ofMesh referenceMeanMesh;
	ofShader maskBlurShader;
	ofFbo faceMask, halfMaskBlur;
	
	ofShader cloneShader;
	ofFbo cloned;
	
	ofShader blurAlphaShader;
	ofFbo halfAlphaBlur, final;
	
	ofShader voronoiShader;
	ofFloatImage pointsImage;
	
	ofDirectory faceDirectory;
	ofDirectory grimaceDirectory;
	vector<ofImage> faces;

	bool useVoronoi;
	bool debug;
};
