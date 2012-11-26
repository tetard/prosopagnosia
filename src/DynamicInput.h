//
//  DynamicInput.h
//  Prosopagnosia
//
//  Created by Camille Troillard on 26/11/12.
//
//

#ifndef __Prosopagnosia__DynamicInput__
#define __Prosopagnosia__DynamicInput__

#include "ofMain.h"
#include "ofxSyphonClient.h"


class DynamicInput : public ofBaseHasTexture
{
 public:
  ~DynamicInput() {};
  virtual ofTexture & getTextureReference();
  virtual void setUseTexture(bool bUseTex);

  enum _Type {
    Camera,
    Syphon
  };

  typedef enum _Type Type;

  void init(float width, float height);

  void toggle();
  void setType(Type type);
  Type getType() const;

  float getWidth();
  float getHeight();

  void update();
  bool isFrameNew();

  void draw(float x, float y);
  ofPixels const& getPixels();

 private:
  Type _type;
  ofFbo _framebuffer;
  ofPixels _pixels;
  ofPtr<ofVideoGrabber> _inputCamera;
  ofPtr<ofxSyphonClient> _inputSyphon;
};

#endif /* defined(__Prosopagnosia__DynamicInput__) */
