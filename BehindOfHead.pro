#-------------------------------------------------
#
# Project created by QtCreator 2012-10-22T04:48:53
#
#-------------------------------------------------

QT       += core gui

TARGET = BehindOfHead
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    kinectcontrol.cpp

HEADERS  += mainwindow.h \
    kinectcontrol.h

FORMS    += mainwindow.ui

# OpenNI
macx: LIBS += -L/usr/local/Cellar/sensor/stable-5.1.0.41/lib/ \
  -lXnCore \
  -lXnDDK \
  -lXnDeviceFile \
  -lXnDeviceSensorV2 \
  -lXnFormats
macx: LIBS += -L/usr/local/Cellar/openni/stable-1.5.2.23/lib/ \
  -lOpenNI \
  -lOpenNI.jni \
  -lnimCodecs \
  -lnimMockNodes \
  -lnimRecorder
INCLUDEPATH += /usr/local/Cellar/openni/stable-1.5.2.23/include/ni

# boost
macx: LIBS += -L/usr/local/opt/boost/lib/ \
  -lboost_chrono-mt \
  -lboost_date_time-mt \
  -lboost_filesystem-mt \
  -lboost_graph-mt \
  -lboost_iostreams-mt \
  -lboost_locale-mt \
  -lboost_math_c99-mt \
  -lboost_math_c99f-mt \
  -lboost_math_c99l-mt \
  -lboost_math_tr1-mt \
  -lboost_math_tr1f-mt \
  -lboost_math_tr1l-mt \
  -lboost_prg_exec_monitor-mt \
  -lboost_program_options-mt \
  -lboost_python-mt \
  -lboost_random-mt \
  -lboost_regex-mt \
  -lboost_serialization-mt \
  -lboost_signals-mt \
  -lboost_system-mt \
  -lboost_thread-mt \
  -lboost_timer-mt \
  -lboost_unit_test_framework-mt \
  -lboost_wave-mt \
  -lboost_wserialization-mt
INCLUDEPATH += /usr/local/opt/boost/include

# OpenCV
macx: LIBS += -L/usr/local/lib \
  -lopencv_calib3d.2.4.2 \
  -lopencv_ml.2.4.2 \
  -lopencv_contrib.2.4.2 \
  -lopencv_nonfree.2.4.2 \
  -lopencv_core.2.4.2 \
  -lopencv_objdetect.2.4.2 \
  -lopencv_features2d.2.4.2 \
  -lopencv_photo.2.4.2 \
  -lopencv_flann.2.4.2 \
  -lopencv_stitching.2.4.2 \
  -lopencv_gpu.2.4.2 \
  -lopencv_ts.2.4.2 \
  -lopencv_highgui.2.4.2 \
  -lopencv_video.2.4.2 \
  -lopencv_imgproc.2.4.2 \
  -lopencv_videostab.2.4.2 \
  -lopencv_legacy.2.4.2
INCLUDEPATH += /usr/local/Cellar/opencv/2.4.2/include
INCLUDEPATH += /usr/local/Cellar/opencv/2.4.2/include/opencv
INCLUDEPATH += /usr/local/Cellar/opencv/2.4.2/include/opencv2
