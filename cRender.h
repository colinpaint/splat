#pragma once
#include <memory>
#include <string>
#include <array>
#include <vector>
#include <map>
#include <Eigen/Core>

#include "app/cApp.h"
#include "app/cCamera.h"
class cModel;

class cRender {
public:
  cRender (cApp& app) : mApp(app) {}
  virtual ~cRender() {}

  bool getMultiSample() const { return mMultiSample; }
  bool getBackFaceCull() const { return mBackFaceCull; }

  bool getMaterialColored() const { return mMaterialColored; };
  void setMaterialColored (bool enable = true) { mMaterialColored = enable; }

  Eigen::Vector3f getMaterialColor() const { return mMaterialColor; }
  void setMaterialColor (Eigen::Vector3f const color) { mMaterialColor = color; }

  float getMaterialShine() const { return mMaterialShine; }
  void setMaterialShine (float shine) { mMaterialShine = shine; };

  // overides
  virtual void setMultiSample (bool enable = true) { mMultiSample = enable; }
  virtual void setBackFaceCull (bool enable = true) { mBackFaceCull = enable; }

  virtual void use (bool multiSample, bool backFaceCull) = 0;

  // abstract
  virtual void display (cModel* model) = 0;
  virtual void gui() {}
  virtual bool keyboard (SDL_Keycode key) { return false; }
  virtual void resize (int width, int height) {};

protected:
  cApp& mApp;
  cUniformCamera mUniformCamera;

  bool mMultiSample = false;
  bool mBackFaceCull = false;

  bool mMaterialColored = false;
  Eigen::Vector3f mMaterialColor;
  float mMaterialShine = 1.f;
  };
