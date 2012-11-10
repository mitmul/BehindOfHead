#ifndef KINECTCONTROL_H
#define KINECTCONTROL_H

#include <iostream>
#include <vector>
#include <XnCppWrapper.h>

using namespace std;
using namespace xn;

class KinectControl
{
  public:
    KinectControl();
    ~KinectControl();

    void init();
    void updateKinect();

    XnDepthPixel* getDepth();
    XnUInt8* getImage();
    vector<vector<XnPoint3D> > getSkeleton();
    XnLabel* getUserMaskData();

  private:
    // コールバック関数
    static void NewUser(UserGenerator& generator, XnUserID user, void* pCookie)
    {
      ((KinectControl*)pCookie)->NewUser(generator, user);
    }
    void NewUser(UserGenerator &generator, XnUserID user);

    static void LostUser(UserGenerator& generator, XnUserID user, void* pCookie)
    {
      ((KinectControl*)pCookie)->LostUser(generator, user);
    }
    void LostUser(UserGenerator &generator, XnUserID user);

    static void CalibrationStart(SkeletonCapability& skeleton, XnUserID user, void* pCookie)
    {
      ((KinectControl*)pCookie)->CalibrationStart(skeleton, user);
    }
    void CalibrationStart(SkeletonCapability &skeleton, XnUserID user);

    static void CalibrationEnd(SkeletonCapability& skeleton, XnUserID user, XnCalibrationStatus calibrationError, void* pCookie)
    {
      ((KinectControl*)pCookie)->CalibrationEnd(skeleton, user, calibrationError);
    }
    void CalibrationEnd(SkeletonCapability& skeleton, XnUserID user, XnCalibrationStatus calibrationError);

    void errorCheck(XnStatus nRetVal, const char* what);

    Context context;

    DepthGenerator depth_generator;
    DepthMetaData depth_md;
    ImageGenerator image_generator;
    ImageMetaData image_md;
    UserGenerator user_generator;
    SceneMetaData scene_md;

    SkeletonCapability skeleton;
    PoseDetectionCapability pose;

    XnCallbackHandle user_callbacks;
    XnCallbackHandle calib_start;
    XnCallbackHandle calib_end;

    XnChar str_pose[20];
};

#endif // KINECTCONTROL_H
