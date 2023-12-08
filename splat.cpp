// splat.cpp
//{{{  includes
#ifdef _WIN32
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdlib>
#include <memory>
#include <vector>
#include <array>
#include <exception>
#include <cmath>

#include <Eigen/Core>

#include "../common/cLog.h"

#include "../imgui/imgui.h"

#include "../app/cApp.h"

#include "cModel.h"
#include "cSurfelModel.h"
#include "cMeshRender.h"
#include "cSplatRender.h"

using namespace std;
//}}}

//{{{
class cSplatApp : public cApp {
public:
  cSplatApp() : cApp() {}
  virtual ~cSplatApp() = default;

  //{{{
  void init (const string& name, int width, int height, bool fullScreen, bool hasMultiSample) {

    mHasMultiSample = hasMultiSample;
    cApp::init (name, width, height, fullScreen, hasMultiSample);

    setDisplayCallback ([this] {
      mRender->display (mModel);
      });

    setGuiCallback ([this] {
      gui();
      });

    setResizeCallback ([this](int width, int height) {
      const float aspect = static_cast<float>(width) / static_cast<float>(height);
      glViewport (0, 0, width, height);
      getCamera().setPerspective (60.0f, aspect, 0.005f, 5.0f);
      });
    mResizeCallback (width, height);

    setTimerCallback ([this] {
      if (!mUseSplatRender && mRipple)
        mModel->ripple();
      });

    setCloseCallback ([this] {
      delete mMeshRender;
      delete mSplatRender;
      delete mModel;
      });

    setKeyboardCallback ([this](SDL_Keycode key) {
      if (!mRender->keyboard (key))
        switch (key) {
          case SDLK_f: toggleFullScreen(); break;
          case SDLK_SPACE: mRipple = !mRipple; break;
          case SDLK_q:
          case SDLK_ESCAPE: exit (EXIT_SUCCESS); break;
          default: break;
          }
      });
    }
  //}}}

  cModel* getModel() { return mModel; }
  int getModelIndex() { return mModelIndex; }
  bool getRipple() const { return mRipple; }

  bool getMultiSample() const { return mMultiSample; }
  bool getBackFaceCull() const { return mBackFaceCull; }

  bool getUseSplatRender() const { return mUseSplatRender; }
  cSplatRender* getSplatRender() { return mSplatRender; }
  cMeshRender* getMeshRender() { return mMeshRender; }
  cRender* getRender() { return mRender; }

  void setModel (cModel* model) { mModel = model; }
  //{{{
  void setMeshRender (cMeshRender* meshRender) {
    mMeshRender = meshRender;
    mRender = meshRender;
    mRender->use (mMultiSample, mBackFaceCull);
    }
  //}}}
  //{{{
  void setSplatRender (cSplatRender* splatRender) {
    mSplatRender = splatRender;
    mRender = splatRender;
    mRender->use (mMultiSample, mBackFaceCull);
    }
  //}}}

private:
  //{{{
  void gui() {

    ImGui::Begin ("splat", nullptr);
    ImGui::PushItemWidth (0.7f * ImGui::GetContentRegionAvail().x);

    if (mMeshRender && mSplatRender) {
      // have choice of render
      if (ImGui::Checkbox ("splatRender", &mUseSplatRender)) {
        if (mUseSplatRender)
          mRender = mSplatRender;
        else
          mRender = mMeshRender;
        mRender->use (mMultiSample, mBackFaceCull);
        }
      }

    if (mHasMultiSample)
      if (ImGui::Checkbox ("multiSample", &mMultiSample))
        mRender->setMultiSample (mMultiSample);

    if (ImGui::Checkbox ("backfaceCull", &mBackFaceCull))
      mRender->setBackFaceCull (mBackFaceCull);

    ImGui::Text ("%.1f:fps %d:vertices %d:faces %3.2f:scaled",
                 ImGui::GetIO().Framerate,
                 (int)mModel->getNumVertices(), (int)mModel->getNumFaces(),
                 mModel->getScale());

    if (mModel->isSelectable()) {
      ImGui::SetNextItemOpen (true, ImGuiCond_Once);
      if (ImGui::CollapsingHeader ("scene"))
        if (ImGui::Combo ("##", &mModelIndex, "dragonLo\0dragonHi\0checker\0cube\0\0"))
          mModel->loadIndex (mModelIndex);
      }


    mRender->gui();
    ImGui::End();
    }
  //}}}

  // vars
  cModel* mModel;

  bool mHasMultiSample = false;
  bool mBackFaceCull = false;

  int mModelIndex = 0;
  bool mRipple = false;

  bool mUseSplatRender = false;
  cSplatRender* mSplatRender;
  cMeshRender* mMeshRender;
  cRender* mRender;
  };
//}}}

int main (int numArgs, char* args[]) {

  string fileName;
  bool fullScreen = false;
  bool hasMultiSample = false;
  bool backFaceCull = false;
  eLogLevel logLevel = LOGINFO;
  //{{{  parse commandLine to params
  // parse params
  for (int i = 1; i < numArgs; i++) {
    string param = args[i];
    if (param == "log1")
      logLevel = LOGINFO1;
    else if (param == "log2")
      logLevel = LOGINFO2;
    else if (param == "log3")
      logLevel = LOGINFO3;
    else if (param == "full")
      fullScreen = true;
    else if (param == "multi")
      hasMultiSample = true;
    else
      // assume filename
      fileName = param;
    }
  //}}}

  cLog::init (logLevel);
  cLog::log (LOGNOTICE, "splat");

  cSplatApp splatApp;
  splatApp.init ("splatApp", 960, 540, fullScreen, hasMultiSample);
  splatApp.getCamera().translate (Eigen::Vector3f(0.0f, 0.0f, -2.0f));

  if (fileName.empty()) {
    //{{{  surfel drop downs
    splatApp.setModel (new cSurfelModel());
    splatApp.getModel()->loadIndex (splatApp.getModelIndex());
    splatApp.setSplatRender (new cSplatRender (splatApp, hasMultiSample, backFaceCull));
    splatApp.setMeshRender (new cMeshRender (splatApp, hasMultiSample, backFaceCull));
    }
    //}}}
  else if (fileName.substr (fileName.size() - 4, 4) == ".obj") {
    //{{{  .obj file
    cSurfelModel* surfelModel = new cSurfelModel();
    surfelModel->loadObjFile (fileName);
    splatApp.setModel (surfelModel);
    splatApp.setSplatRender (new cSplatRender (splatApp, hasMultiSample, backFaceCull));
    splatApp.setMeshRender (new cMeshRender (splatApp, hasMultiSample, backFaceCull));
    }
    //}}}
  else if (fileName.substr (fileName.size() - 4, 4) == ".raw") {
    //{{{  .raw file
    cSurfelModel* surfelModel = new cSurfelModel();
    surfelModel->loadRawFile (fileName);
    splatApp.setModel (surfelModel);
    splatApp.setSplatRender (new cSplatRender (splatApp, hasMultiSample, backFaceCull));
    splatApp.setMeshRender (new cMeshRender (splatApp, hasMultiSample, backFaceCull));
    }
    //}}}
  else if ((fileName.substr (fileName.size() - 4, 4) == ".png") ||
           (fileName.substr (fileName.size() - 4, 4) == ".jpg")) {
    //{{{  .png, .jpg files
    cSurfelModel* surfelModel = new cSurfelModel();
    surfelModel->loadPiccyFile (fileName);
    splatApp.setModel (surfelModel);
    splatApp.setSplatRender (new cSplatRender (splatApp, hasMultiSample, backFaceCull));
    }
    //}}}

  return splatApp.mainUILoop();
  }
