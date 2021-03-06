#include "testApp.h"

ofMesh texturedRectMesh;
void texturedRect(float width, float height) {
	if(texturedRectMesh.getNumVertices() == 0) {
		texturedRectMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
		texturedRectMesh.addTexCoord(ofVec2f(0, 0));
		texturedRectMesh.addVertex(ofVec2f(0, 0));
		texturedRectMesh.addTexCoord(ofVec2f(0, 1));
		texturedRectMesh.addVertex(ofVec2f(0, 1));
		texturedRectMesh.addTexCoord(ofVec2f(1, 0));
		texturedRectMesh.addVertex(ofVec2f(1, 0));
		texturedRectMesh.addTexCoord(ofVec2f(1, 1));
		texturedRectMesh.addVertex(ofVec2f(1, 1));
	}
	ofPushMatrix();
	ofScale(width, height);
	texturedRectMesh.drawFaces();
	ofPopMatrix();
}

// this is important for avoiding slightl discrepencies when the mesh is
// projected, or processed by GL transforms vs OF transforms
void testApp::normalizeMesh(ofMesh& mesh) {
	vector<ofVec3f>& vertices = mesh.getVertices();
	for(int i = 0; i < vertices.size(); i++) {
		vertices[i] *= normalizedMeshScale / normalizedWidth;
		vertices[i] += ofVec2f(normalizedWidth, normalizedHeight) / 2.;
		vertices[i].z = 0;
	}
}

void testApp::drawNormalized(ofxFaceTracker& tracker) {
	ofClear(0, 0);
    ofMesh mesh = tracker.getMeanObjectMesh();
	normalizeMesh(mesh);
	mesh.draw();
}

void testApp::drawNormalized(ofxFaceTracker& tracker, ofBaseHasTexture& tex, ofFbo& result) {
	result.begin();
	tex.getTextureReference().bind();
	drawNormalized(tracker);
	tex.getTextureReference().unbind();
	result.end();
}

void testApp::maskBlur(ofBaseHasTexture& tex, ofFbo& result) {
	int k = 128;
    //    bypass the mouse blur
//        int k = ofMap(mouseX, 0, ofGetWidth(), 1, 128, true);

	halfMaskBlur.begin();
	ofClear(0, 0);
	maskBlurShader.begin();
	maskBlurShader.setUniformTexture("tex", tex, 1);
	maskBlurShader.setUniformTexture("mask", faceMask, 2);
	maskBlurShader.setUniform2f("direction", 1, 0);
	maskBlurShader.setUniform1i("k", k);
	tex.getTextureReference().draw(0, 0);
	maskBlurShader.end();
	halfMaskBlur.end();

	result.begin();
	ofClear(0, 0);
	maskBlurShader.begin();
	maskBlurShader.setUniformTexture("tex", halfMaskBlur, 1);
	maskBlurShader.setUniformTexture("mask", faceMask, 2);
	maskBlurShader.setUniform2f("direction", 0, 1);
	maskBlurShader.setUniform1i("k", k);
	halfMaskBlur.draw(0, 0);
	maskBlurShader.end();
	result.end();
}

void testApp::radialBlur(ofBaseHasTexture& tex, ofFbo& result) {
}

void testApp::alphaBlur(ofBaseHasTexture& tex, ofFbo& result) {
//	int k = ofMap(mouseY, 0, ofGetHeight(), 1, 25, true);
    int k = 25;


	halfAlphaBlur.begin();
	ofClear(0, 0);
	blurAlphaShader.begin();
	blurAlphaShader.setUniformTexture("tex", tex, 1);
	blurAlphaShader.setUniform2f("direction", 1, 0);
	blurAlphaShader.setUniform1i("k", k);
	tex.getTextureReference().draw(0, 0);
	blurAlphaShader.end();
	halfAlphaBlur.end();

	result.begin();
	ofClear(0, 0);
	blurAlphaShader.begin();
	blurAlphaShader.setUniformTexture("tex", halfAlphaBlur, 1);
	blurAlphaShader.setUniform2f("direction", 0, 1);
	blurAlphaShader.setUniform1i("k", k);
	halfAlphaBlur.draw(0, 0);
	blurAlphaShader.end();
	result.end();
}

void testApp::normalizeImage(ofImage& refImg, ofImage& img, ofImage& normalized) {
	srcTracker.reset();
	srcTracker.update(ofxCv::toCv(refImg));
	if(srcTracker.getFound()) {
		drawNormalized(srcTracker, img, srcNormalized);
		normalized.allocate(normalizedWidth, normalizedHeight, OF_IMAGE_COLOR);
		srcNormalized.readToPixels(normalized.getPixelsRef());
		normalized.update();
	} else {
		ofLogWarning() << "couldn't find the face" << endl;
	}
}

void testApp::setup() {
#ifdef TARGET_OSX
	ofSetDataPathRoot("../../../data/");
#endif

	ofSetVerticalSync(true);
    ofSetFrameRate(FRAME_RATE);

	maskBlurShader.load("", "MaskBlur.frag");
	cloneShader.load("", "Clone.frag");
	blurAlphaShader.load("", "BlurAlpha.frag");
	voronoiShader.load("", "Voronoi.frag");
    radialBlurShader.load("", "RadialBlur.frag");

#if USE_DYNAMIC_INPUT
	input.init(640, 480);
	input.setType(DynamicInput::Camera);
#else
    input.loadMovie(MOVIE_FILENAME);
//    input.play();
    oscReceiver.setup(OSC_INPUT_PORT);
#endif

    ofSetWindowShape(input.getWidth(), input.getHeight());

    srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);
	dstTracker.setup();

	ofFbo::Settings settings;
	settings.width = normalizedWidth;
	settings.height = normalizedHeight;
	srcNormalized.allocate(settings);
	dstNormalized.allocate(settings);
	halfMaskBlur.allocate(settings);
	srcBlur.allocate(settings);
	dstBlur.allocate(settings);
	faceMask.allocate(settings);

	settings.width = input.getWidth();
	settings.height = input.getHeight();
	cloned.allocate(settings);
	halfAlphaBlur.allocate(settings);
	final.allocate(settings);
    output.allocate(settings);
    radialBlurOutput.allocate(settings);
    radialBlurOrigin = ofVec2f(0.5, 0.5);
    radialBlurWidth = ofVec2f(0.5, 0.5);

    faceDirectory.setShowHidden(false);
    faceDirectory.allowExt("jpg");
    faceDirectory.allowExt("jpeg");
    faceDirectory.allowExt("png");
	faceDirectory.listDir("faces");
	faceDirectory.sort();
    grimaceDirectory.setShowHidden(false);
    grimaceDirectory.allowExt("jpg");
    grimaceDirectory.allowExt("jpeg");
    grimaceDirectory.allowExt("png");
    grimaceDirectory.listDir("grimaces");
	grimaceDirectory.sort();

    syphonOutput.setName("P");

	faces.resize(5);
	for(int i = 0; i < 5; i++) {
		ofImage curFace;
		ofImage curGrimace;
		curFace.loadImage(faceDirectory[i]);
		curGrimace.loadImage(grimaceDirectory[i]);
		normalizeImage(curFace, curGrimace, faces[i]);
	}

	pointsImage.allocate(faces.size(), 1, OF_IMAGE_COLOR);

	faceMask.begin();
	drawNormalized(srcTracker);
	faceMask.end();

	referenceMeanMesh = srcTracker.getMeanObjectMesh();
	normalizeMesh(referenceMeanMesh);

	useVoronoi = true;
	debug = false;
}

void testApp::receiveOSC()
{
    int nextFrame = -1;
	while (oscReceiver.hasWaitingMessages()) {
		ofxOscMessage msg;
		oscReceiver.getNextMessage(&msg);
    
#if !USE_DYNAMIC_INPUT
        if (msg.getAddress() == "/video/speed") {
            float speed = msg.getArgAsFloat(0);
            input.setSpeed(speed);
        } else if (msg.getAddress() == "/video/volume") {
            float volume = msg.getArgAsFloat(0);
            input.setVolume(volume);
        } else if (msg.getAddress() == "/video/play") {
            input.play();
        } else if (msg.getAddress() == "/video/stop") {
            input.stop();
        } else if (msg.getAddress() == "/video/position") {
            float position = msg.getArgAsFloat(0);
            int totalNumFrames = 0;
            totalNumFrames = input.getTotalNumFrames();
            nextFrame = position * totalNumFrames - 1;
        } else if (msg.getAddress() == "/blur/origin") {
            float x = msg.getArgAsFloat(0);
            float y = msg.getArgAsFloat(1);
            radialBlurOrigin = ofVec2f(x, y);
        } else if (msg.getAddress() == "/blur/width") {
            float x = msg.getArgAsFloat(0);
            float y = msg.getArgAsFloat(1);
            radialBlurWidth = ofVec2f(x, y);
        }
#endif
        if (msg.getAddress() == "/reset") {
            dstTracker.reset();
        }
    }

#if !USE_DYNAMIC_INPUT
    if (nextFrame > -1) {
        input.setFrame(nextFrame);
    }
#endif
}

void testApp::buildVoronoiFace() {
	ofSeedRandom(0);
	float* pixels = pointsImage.getPixels();
	for(int i = 0; i < faces.size(); i++) {
		float speed = .4;
		pixels[i * 3 + 0] = ofNoise(ofGetElapsedTimef() * speed + ofRandom(1024));
		pixels[i * 3 + 1] = ofNoise(ofGetElapsedTimef() * speed + ofRandom(1024));
		pixels[i * 3 + 2] = 0;
	}
	pointsImage.update();

	srcNormalized.begin();
	voronoiShader.begin();
	voronoiShader.setUniform1i("count", faces.size());
	for(int i = 0; i < faces.size(); i++) {
		string texName = "tex" + ofToString(i);
		voronoiShader.setUniformTexture(texName.c_str(), faces[i], i);
	}
	voronoiShader.setUniformTexture("points", pointsImage, faces.size());
	texturedRect(normalizedWidth, normalizedHeight);
	voronoiShader.end();
	srcNormalized.end();

	ofTranslate(0, normalizedHeight);
	int y = 0, x = 0;
	float gridScale = .5;
	ofPushMatrix();
	ofScale(gridScale, gridScale);
	for(int i = 0; i < faces.size(); i++) {
		faces[i].draw(x, y);
		x += faces[i].getWidth();
		if(x + faces[i].getWidth() > input.getWidth() / gridScale) {
			x = 0;
			y += faces[i].getHeight();
		}
	}
	ofPopMatrix();
}

void testApp::update() {
    receiveOSC();
	input.update();

    ofEnableAlphaBlending();

    ofPixelsRef pixels = input.getPixelsRef();
    dstTracker.update(ofxCv::toCv(pixels));
    drawNormalized(dstTracker, input, dstNormalized);

    buildVoronoiFace();
    maskBlur(srcNormalized, srcBlur);
    maskBlur(dstNormalized, dstBlur);

    ofMesh dstMesh = dstTracker.getImageMesh();
    dstMesh.clearTexCoords();
    vector<ofVec3f>& vertices = referenceMeanMesh.getVertices();
    for(int i = 0; i < vertices.size(); i++) {
        dstMesh.addTexCoord(ofVec2f(vertices[i].x, vertices[i].y));
    }

    cloned.begin();
    ofClear(0, 0);
    cloneShader.begin();
    cloneShader.setUniformTexture("src", srcNormalized, 1);
    cloneShader.setUniformTexture("srcBlur", srcBlur, 2);
    cloneShader.setUniformTexture("dstBlur", dstBlur, 3);
    dstMesh.draw();
    cloneShader.end();
    cloned.end();

    // alpha blur causes black fringes right now..
    //alphaBlur(cloned, final);

    output.begin();
    ofPushMatrix();
    ofSetupScreenOrtho(output.getWidth(), output.getHeight(), OF_ORIENTATION_180, true);
    ofClear(0);
    input.draw(0, 0, output.getWidth(), output.getHeight());
    cloned.draw(0, 0, output.getWidth(), output.getHeight());
    ofPopMatrix();
    output.end();

    radialBlurOutput.begin();
	radialBlurShader.begin();
	radialBlurShader.setUniform2f("origin", radialBlurOrigin.x * radialBlurOutput.getWidth(), radialBlurOrigin.y * radialBlurOutput.getHeight());
	radialBlurShader.setUniform2f("width", radialBlurWidth.x, radialBlurWidth.y);
    output.draw(0, 0, radialBlurOutput.getWidth(), radialBlurOutput.getHeight());
	radialBlurShader.end();
    radialBlurOutput.end();

    ofDisableAlphaBlending();
}

void testApp::draw() {
    ofPushMatrix();
    ofSetupScreenOrtho(radialBlurOutput.getWidth(), radialBlurOutput.getHeight(), OF_ORIENTATION_180, false);
    radialBlurOutput.draw(0, 0, radialBlurOutput.getWidth(), radialBlurOutput.getHeight());
    ofPopMatrix();

	if(debug) {
        ofSetColor(255);
		dstTracker.draw();
		ofPushMatrix();
		ofScale(.5, .5);
		srcNormalized.draw(0, 0);
		dstNormalized.draw(normalizedWidth, 0);
		srcBlur.draw(0, normalizedHeight);
		dstBlur.draw(normalizedWidth, normalizedHeight);
		ofPopMatrix();
        ofDisableAlphaBlending();
        //	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
	}

    ofTexture& texture = radialBlurOutput.getTextureReference();
    syphonOutput.publishTexture(&texture);
}

void testApp::keyPressed(int key) {
	if(key == 'r') {
		maskBlurShader.load("", "MaskBlur.frag");
		cloneShader.load("", "Clone.frag");
		blurAlphaShader.load("", "BlurAlpha.frag");
		voronoiShader.load("", "Voronoi.frag");
	}
	if(key == 'v') {
		useVoronoi = !useVoronoi;
	}
	if(key == 'd') {
		debug = !debug;
	}

    if(key == ' ') {
    dstTracker.reset();
    }
#if USE_DYNAMIC_INPUT
	if(key == 'i') {
        input.toggle();
	}
#endif
}
