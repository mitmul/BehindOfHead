#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <XnCppWrapper.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include <boost/timer.hpp>
#include <opencv.hpp>
#include <sys/types.h>
#include <dirent.h>
#include "kinectcontrol.h"

#define DIR_NAME "/Users/saito/Pictures/Eye-Fi/2012-11-12/"
#define NORMAL_TIME 10
#define FIRST_PART 16
#define SECOND_PART 28

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_SensorStart_clicked();

private:
    Ui::MainWindow *ui;

    cv::Mat display_image;
    int screen_mode;

    boost::mutex thread_sync;
    boost::condition_variable thread_state;

    KinectControl kinect;
    boost::thread* kinect_thread;
    bool kinect_thread_running;
    cv::Mat mask_show;

    cv::Mat garden_image;
    cv::Rect garden_roi;
    cv::Point user_pos;

    boost::thread* dir_thread;
    bool dir_thread_running;
    vector<string> file_names;
    int arrival_pic_number;
    cv::Mat normal_photo;
    cv::Mat diff_photo;

    void getKinectData();
    void getArrivalFile();
    vector<string> split(const string &str, char delim);
    cv::Mat alphaBlend(const cv::Mat& src1, const cv::Mat& src2, const double alpha);
    void showFullScreen(const cv::Mat& image);
    void showMatInfo(const cv::Mat& src);
    void changeGardenROI(const int x, const int y);
    cv::Point getGravCenter(const cv::Mat& mask);
    cv::Point getTopPoint(const cv::Mat& mask);
    bool isAllBlack(const cv::Mat& src);
};

#endif // MAINWINDOW_H
