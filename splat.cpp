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
    cApp::init (width, height, fullScreen, multiSample);

    setDisplayCallback ([this] {
      //{{{  display lambda
      mRender->display (mModel);
      });
      //}}}
    setGuiCallback ([this] {
      //{{{  gui lambda
      ImGui::Begin ("splat", nullptr);
      ImGui::PushItemWidth (0.7f * ImGui::GetContentRegionAvail().x);

      if (ImGui::Checkbox ("splatRender", &mUseSplatRender)) {
        if (mUseSplatRender)
          mRender = mSplatRender;
        else
          mRender = mMeshRender;
        mRender->use (mMultiSample, mBackFaceCull);
        }

      if (mMultiSample)
        if (ImGui::Checkbox ("multiSample", &mMultiSample))
          mRender->setMultiSample (mMultiSample);

      if (ImGui::Checkbox ("backfaceCull", &mBackFaceCull))
        mRender->setBackFaceCull (mBackFaceCull);

      ImGui::Text ("%.1f fps", ImGui::GetIO().Framerate);

      ImGui::SetNextItemOpen (true, ImGuiCond_Once);
      if (ImGui::CollapsingHeader ("scene"))
        if (ImGui::Combo ("model", &mModelIndex, "dragonLo\0dragonHi\0checker\0cube\0piccy\0\0"))
          mModel->load (mModelIndex);

      mRender->gui();
      ImGui::End();
      });
      //}}}
    setCloseCallback ([this] {
      //{{{  close lambda
      delete mMeshRender;
      delete mSplatRender;
      delete mModel;
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
      getCamera().set_perspective(60.0f, aspect, 0.005f, 5.0f);
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
  void setSplatRender (cSplatRender* splatRender) {
    mSplatRender = splatRender;
    mRender = splatRender;
    }
  //}}}
  //{{{
  void setMeshRender (cMeshRender* meshRender) {
    mMeshRender = meshRender;
    mRender = meshRender;
    }
  //}}}
  void setModelIndex (int modelIndex) { mModelIndex = modelIndex; }
  void getUseSplatRender (bool useSplatRender) { mUseSplatRender = useSplatRender; }

  // vars
  cModel* mModel;

private:
  //{{{  vars
  bool mMultiSample = false;
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

  if (fileName.empty()) {
    splatApp.mModel = new cSurfelModel();
    splatApp.getModel()->load (splatApp.getModelIndex());
    splatApp.setSplatRender (new cSplatRender (splatApp));
    splatApp.setMeshRender (new cMeshRender (splatApp));
    }
  else {
    splatApp.mModel = new cSurfelModel (fileName);
    splatApp.setModelIndex (4);
    splatApp.setMeshRender (new cMeshRender (splatApp));
    splatApp.setSplatRender (new cSplatRender (splatApp));
    }

  return splatApp.mainUILoop();
  }
