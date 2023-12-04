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

#include "cMeshRender.h"
#include "cSplatRender.h"
#include "models.h"

using namespace std;
//}}}

//{{{
class cSplatApp : public cApp {
public:
  cSplatApp() : cApp() {}
  virtual ~cSplatApp() = default;

  void init (int width, int height, bool fullScreen, bool multiSample) {
    mHasMultiSample = multiSample;
    cApp::init (width, height, fullScreen, multiSample);
    cLog::log (LOGINFO, fmt::format ("splatApp init {}:{}", multiSample ? "multi" : "", mMultiSample ? "multi" : ""));

    setDisplayCallback ([this] {
      //{{{  display lambda
      mRender->display (mModel);
      });
      //}}}
    setGuiCallback ([this] {
      //{{{  gui lambda
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

      ImGui::Text ("%.1f fps %d vertices %d faces", ImGui::GetIO().Framerate,
                                 (int)mModel->getNumVertices(), (int)mModel->getNumFaces());

      if (mModel->isSelectable()) {
        ImGui::SetNextItemOpen (true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader ("scene"))
          if (ImGui::Combo ("##", &mModelIndex, "dragonLo\0dragonHi\0checker\0cube\0piccy\0\0"))
            mModel->load (mModelIndex);
        }

      mRender->gui();
      ImGui::End();
      });
      //}}}
    setTimerCallback ([this] {
      //{{{  timer lambda
      if (!mUseSplatRender && mRipple)
        mModel->ripple();
      });
      //}}}
    setKeyboardCallback ([this](SDL_Keycode key) {
      //{{{  keyboard lambda
      if (!mRender->keyboard (key))
        switch (key) {
          case SDLK_f: toggleFullScreen(); break;
          case SDLK_SPACE: mRipple = !mRipple; break;
          case SDLK_q:
          case SDLK_ESCAPE: exit (EXIT_SUCCESS); break;
          default: break;
          }
        });
      //}}}
    setResizeCallback ([this](int width, int height) {
      //{{{  resize lambda
      const float aspect = static_cast<float>(width) / static_cast<float>(height);
      glViewport (0, 0, width, height);
      getCamera().setPerspective (60.0f, aspect, 0.005f, 5.0f);
      });
      //}}}
    setCloseCallback ([this] {
      //{{{  close lambda
      delete mMeshRender;
      delete mSplatRender;
      delete mModel;
      });
      //}}}

    mResizeCallback (width, height);
    }

  cModel* getModel() { return mModel; }
  int getModelIndex() { return mModelIndex; }
  bool getRipple() const { return mRipple; }

  bool getUseSplatRender() const { return mUseSplatRender; }
  bool getMultiSample() const { return mMultiSample; }
  bool getBackFaceCull() const { return mBackFaceCull; }

  cSplatRender* getSplatRender() { return mSplatRender; }
  cMeshRender* getMeshRender() { return mMeshRender; }
  cRender* getRender() { return mRender; }

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

  // vars
  cModel* mModel;

private:
  //{{{  vars
  bool mHasMultiSample = false;
  bool mBackFaceCull = false;

  int mModelIndex = 0;
  bool mRipple = false;

  cSplatRender* mSplatRender;
  cMeshRender* mMeshRender;
  cRender* mRender;
  bool mUseSplatRender = false;
  //}}}
  };
//}}}

int main (int numArgs, char* args[]) {

  string fileName;
  bool fullScreen = false;
  bool multiSample = false;
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
      multiSample = true;
    else
      // assume filename
      fileName = param;
    }
  //}}}

  cLog::init (logLevel);
  cLog::log (LOGNOTICE, "splat");

  cSplatApp splatApp;
  splatApp.init (960, 540, fullScreen, multiSample);

  splatApp.mCamera.translate (Eigen::Vector3f(0.0f, 0.0f, -2.0f));

  splatApp.mModel = new cSurfelModel (fileName);
  splatApp.getModel()->load (splatApp.getModelIndex());
  splatApp.setSplatRender (new cSplatRender (splatApp));
  splatApp.setMeshRender (new cMeshRender (splatApp));

  return splatApp.mainUILoop();
  }
