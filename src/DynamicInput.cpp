//
//  DynamicInput.cpp
//  Prosopagnosia
//
//  Created by Camille Troillard on 26/11/12.
//
//

#include "DynamicInput.h"


void DynamicInput::init(float width, float height)
{
    ofFbo::Settings settings;
    settings.width = width;
    settings.height = height;
    _framebuffer.allocate(settings);
}

ofTexture& DynamicInput::getTextureReference()
{
    return _framebuffer.getTextureReference();
}

void DynamicInput::setUseTexture(bool bUseTex)
{
    _framebuffer.setUseTexture(bUseTex);
}

void DynamicInput::toggle()
{
    Type nextType;
    switch (getType()) {
        case Camera:
            nextType = Syphon;
            break;

        case Syphon:
            nextType = Camera;
            break;
    }
    setType(nextType);
}


void DynamicInput::setType(DynamicInput::Type type)
{
    switch (type) {
        case Camera:
            _inputSyphon.reset();
            _inputCamera.reset(new ofVideoGrabber);
            _inputCamera->initGrabber(getWidth(), getHeight());
            break;

        case Syphon:
            _inputCamera.reset();
            _inputSyphon.reset(new ofxSyphonClient);
            _inputSyphon->setup();
            _inputSyphon->setServerName("cam-input");
            break;
    }

    _type = type;
}

DynamicInput::Type DynamicInput::getType() const
{
    return _type;
}

float DynamicInput::getWidth()
{
    return _framebuffer.getWidth();
}

float DynamicInput::getHeight()
{
    return _framebuffer.getHeight();
}

void DynamicInput::update()
{
    switch (getType()) {
        case Camera:
            _inputCamera->update();
            if (_inputCamera->isFrameNew()) {
                _framebuffer.begin();
                _inputCamera->draw(0, 0, getWidth(), getHeight());
                _framebuffer.end();
            }
            break;

        case Syphon:
            _framebuffer.begin();
            _inputSyphon->draw(0, 0, getWidth(), getHeight());
            _framebuffer.end();
            break;
    }

    _framebuffer.readToPixels(_pixels);
}

bool DynamicInput::isFrameNew()
{
    switch (getType()) {
        case Camera:
            return _inputCamera->isFrameNew();

        case Syphon:
            return true;
    }
}

ofPixelsRef const& DynamicInput::getPixelsRef()
{
    return _pixels;
}

void DynamicInput::draw(float x, float y)
{
    _framebuffer.draw(x, y);
}
