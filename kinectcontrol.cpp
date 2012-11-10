#include "KinectControl.h"

KinectControl::KinectControl()
  : skeleton(0),
    pose(0)
{
}

KinectControl::~KinectControl()
{
  image_generator.Release();
  depth_generator.Release();
  user_generator.Release();
  context.Release();
}

void KinectControl::init()
{
  try
  {
    XnStatus nRetVal;

    nRetVal = context.Init();
    errorCheck(nRetVal, "Context Init");

    XnMapOutputMode map_mode;
    map_mode.nFPS = 30;
    map_mode.nXRes = XN_VGA_X_RES;
    map_mode.nYRes = XN_VGA_Y_RES;

    // デプスを初期化
    nRetVal = depth_generator.Create(context);
    errorCheck(nRetVal, "Find depth generator");
    depth_generator.SetMapOutputMode(map_mode);

    // イメージを初期化
    nRetVal = image_generator.Create(context);
    errorCheck(nRetVal, "Find image generator");
    image_generator.SetMapOutputMode(map_mode);

    // ユーザを初期化
    nRetVal = user_generator.Create(context);
    errorCheck(nRetVal, "Find user generator");

    // コールバック登録
    nRetVal = user_generator.RegisterUserCallbacks(NewUser, LostUser, this, user_callbacks);
    errorCheck(nRetVal, "Register User Callbacks");

    nRetVal = user_generator.GetSkeletonCap().RegisterToCalibrationStart(CalibrationStart, this, calib_start);
    errorCheck(nRetVal, "Register Calibration Start Callback");

    nRetVal = user_generator.GetSkeletonCap().RegisterToCalibrationComplete(CalibrationEnd, this, calib_end);
    errorCheck(nRetVal, "Register Calibration End Callback");

    // スケルトンを初期化
    if(!user_generator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
      cerr << "Supplied user generator doesn't suppoert skeleton tracking" << endl;
    else
    {
      skeleton = user_generator.GetSkeletonCap();
      skeleton.SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
    }

    if(!user_generator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
      cerr << "Supplied user generator doesn't suppoert user detection" << endl;
    else
    {
      pose = user_generator.GetPoseDetectionCap();
    }
    nRetVal = context.StartGeneratingAll();
    errorCheck(nRetVal, "Start Generating All");
  }
  catch(std::exception &e)
  {
    cerr << "init" << endl
         << e.what() << endl;
  }
}

void KinectControl::updateKinect()
{
  XnStatus nRetVal = context.WaitAndUpdateAll();
  errorCheck(nRetVal, "Update Data");

  depth_generator.GetMetaData(depth_md);
  image_generator.GetMetaData(image_md);
  user_generator.GetUserPixels(0, scene_md);
}

void KinectControl::NewUser(UserGenerator &generator, XnUserID user)
{
  cout << "New User: " << user << endl;

  user_generator.GetPoseDetectionCap().StartPoseDetection(str_pose, user);
  XnStatus nRetVal = skeleton.StartTracking(user);
  errorCheck(nRetVal, "Start Tracking");
}

void KinectControl::LostUser(UserGenerator &generator, XnUserID user)
{
  cout << "Lost User: " << user << endl;

  XnStatus nRetVal = skeleton.StopTracking(user);
  errorCheck(nRetVal, "End Tracking");
}

void KinectControl::CalibrationStart(SkeletonCapability &skeleton, XnUserID user)
{
  cout << "Calibration Start: " << user << endl;
}

void KinectControl::CalibrationEnd(SkeletonCapability &skeleton, XnUserID user, XnCalibrationStatus calibrationError)
{
  if(calibrationError == XN_CALIBRATION_STATUS_OK)
  {
    // Calibration成功
    cout << "Calibration complete, start tracking user " << user << endl;
    user_generator.GetSkeletonCap().StartTracking(user);
  }
  else
  {
    // Calibration失敗
    cerr << "Calibration failed for user " << user << endl;
  }
}

void KinectControl::errorCheck(XnStatus nRetVal, const char *what)
{
  if(nRetVal != XN_STATUS_OK)
  {
    cerr << what << " failed " << xnGetStatusString(nRetVal) << endl;
  }
}

XnDepthPixel *KinectControl::getDepth()
{
  return depth_md.WritableData();
}

XnUInt8 *KinectControl::getImage()
{
  return image_md.WritableData();
}

vector<vector<XnPoint3D> > KinectControl::getSkeleton()
{
  XnUserID user_id[15];
  XnUInt16 user_count = sizeof(user_id) / sizeof(user_id[0]);

  XnStatus nRetVal = user_generator.GetUsers(user_id, user_count);
  errorCheck(nRetVal, "Get Users");

  // 全ユーザ
  std::vector<std::vector<XnPoint3D> > skeletons;

  for(int i = 0; i < user_count; ++i)
  {
    // i番目のユーザがトラッキングされていたら
    if(skeleton.IsTracking(user_id[i]))
    {
      // 個別ユーザ
      std::vector<XnPoint3D> user_skeleton;

      // 関節は全てで24個
      XnSkeletonJointPosition joint[24];

      // [0]はCenter of Mass
      XnPoint3D pt[25];

      for(int j = 1; j < 25; ++j)
      {
        skeleton.GetSkeletonJointPosition(user_id[i], (XnSkeletonJoint)(j), joint[j - 1]);
        pt[j] = joint[j - 1].position;
      }

      user_generator.GetCoM(user_id[i], pt[0]);
      depth_generator.ConvertRealWorldToProjective(25, pt, pt);

      for(int j = 0; j < 25; ++j)
        user_skeleton.push_back(pt[j]);

      skeletons.push_back(user_skeleton);
    }
  }

  return skeletons;
}

XnLabel* KinectControl::getUserMaskData()
{
  return scene_md.WritableData();
}
