#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // 乱数初期化
  srand((unsigned int)time(NULL));

  // 庭写真
  garden_image = cv::imread("5.jpg");
  garden_roi = cv::Rect(0, 500, garden_image.cols, (double)garden_image.cols / 16.0 * 11.0);

  // ユーザ初期位置
  user_pos = cv::Point(0, 0);

  // 初期枚数
  arrival_pic_number = 1;

  // 表示される画像
  display_image.create(800, 1280, CV_8UC3);

  // 0:庭写真、1:撮影写真、2:遷移中
  screen_mode = 0;

  // Kinect開始
  on_pushButton_SensorStart_clicked();

  // ディレクトリ監視
  dir_thread_running = true;
  dir_thread = new boost::thread(boost::bind(&MainWindow::getArrivalFile, this));
}

MainWindow::~MainWindow()
{
  kinect_thread_running = false;
  kinect_thread->join();
  delete kinect_thread;

  dir_thread_running = false;
  dir_thread->join();
  delete dir_thread;

  cv::destroyAllWindows();
  cvDestroyAllWindows();
  kinect.~KinectControl();

  delete ui;
}

void MainWindow::getKinectData()
{
  while(kinect_thread_running)
  {
    try
    {
      kinect.updateKinect();

      // ユーザ領域
      XnLabel* user_md = kinect.getUserMaskData();
      cv::Mat mask(480, 640, CV_16SC1, user_md);
      mask.convertTo(mask_show, CV_8U, 255.0);

      if(arrival_pic_number == FIRST_PART)
      {
        garden_image = cv::imread("7.jpg");
        garden_roi = cv::Rect(0, 0, garden_image.cols, garden_image.rows);
      }

      if(arrival_pic_number == SECOND_PART)
      {
        garden_image = cv::imread("3.jpg");
        garden_roi = cv::Rect(0, 0, garden_image.cols, garden_image.rows);
      }

      // ユーザがいて遷移モードじゃなければ
      if(!isAllBlack(mask_show) && screen_mode != 2)
      {
        // 撮影写真モードから戻る場合
        if(screen_mode == 1)
        {
          // 遷移モードにする
          screen_mode = 2;

          // 目標画像
          cv::Mat resize_garden;
          cv::Mat roi(garden_image, garden_roi);
          cv::resize(roi, resize_garden, cv::Size(display_image.cols, display_image.rows));

          // 元画像
          cv::Mat arrival_photo = display_image.clone();

          // 10秒掛けて表示する
          boost::timer t;
          int fps;

          // １幕はゆっくり戻す
          if(arrival_pic_number <= FIRST_PART)
          {
            fps = 20;
          }
          // ２幕以降は速く戻す
          else
          {
            fps = 4;
          }
          for(int i = 1; i <= fps * NORMAL_TIME; ++i)
          {
            // ブレンド
            arrival_photo = alphaBlend(display_image, resize_garden, (double)i / (double)(fps * NORMAL_TIME));

            showFullScreen(arrival_photo);
          }
          cout << "Elapsed time: " << t.elapsed() << endl;

          // 庭写真モードにする
          screen_mode = 0;
        }

        // 庭写真モードにする
        if(screen_mode == 1)
          screen_mode = 0;

        //        changeGardenROI(new_x, new_y);
        cv::Mat roi(garden_image, garden_roi);

        // 庭写真モードなら表示
        if(screen_mode == 0)
        {
          cv::resize(roi, display_image, cv::Size(display_image.cols, display_image.rows));
          showFullScreen(display_image);
        }
      }

      // ユーザがいなかったら
      else
      {
        // 撮影写真モードにする
        screen_mode = 1;
      }
    }
    catch(std::exception &e)
    {
      cerr << "getDepth" << endl
           << e.what() << endl;
    }
  }
}

void MainWindow::getArrivalFile()
{
  // ディレクトリを監視する
  while(dir_thread_running)
  {
    try
    {
      DIR* dp = opendir(DIR_NAME);

      if (dp!=NULL)
      {
        struct dirent* dent;
        do
        {
          dent = readdir(dp);
          if (dent!=NULL)
          {
            string file_name = string(dent->d_name);

            int dup_num = 0;
            for(int i = 0; i < file_names.size(); ++i)
            {
              if(file_names.at(i) == file_name)
              {
                ++dup_num;
              }
            }

            // 新しいファイルがあれば
            if(dup_num == 0)
            {
              file_names.push_back(file_name);
              string new_file = DIR_NAME + string(dent->d_name);

              cout << new_file << endl;

              // それがjpgファイルなら
              vector<string> splited = split(new_file, '.');
              if(splited.back() == "JPG")
              {
                // 遷移モードにする
                screen_mode = 2;

                // 奇数枚目ならば
                if(arrival_pic_number % 2 == 1)
                {
                  cout << "start showing normal photo" << endl;

                  // 普通の写真
                  normal_photo = cv::imread(new_file);

                  // 10秒掛けて表示する
                  boost::timer t;
                  int fps = 4;
                  for(int i = 1; i <= fps * NORMAL_TIME; ++i)
                  {
                    // 到着した写真を庭写真の大きさにリサイズ
                    cv::Mat resize_normal, resize_garden;
                    cv::resize(normal_photo, resize_normal, cv::Size(display_image.cols, display_image.rows));
                    cv::Mat garden_show(garden_image, garden_roi);
                    cv::resize(garden_show, resize_garden, cv::Size(display_image.cols, display_image.rows));

                    // ブレンド
                    display_image = alphaBlend(resize_garden, resize_normal, (double)i / (double)(fps * NORMAL_TIME));

                    showFullScreen(display_image);
                  }
                  cout << "Garden -> Normal Elapsed time: " << t.elapsed() << endl;
                }

                // 偶数枚目ならば
                else
                {
                  // 差分画像
                  diff_photo = cv::imread(new_file);
                  cv::absdiff(normal_photo, diff_photo, diff_photo);

                  // 10秒掛けて表示する
                  boost::timer t;
                  int fps;

                  // 第１幕ではゆっくり
                  if(arrival_pic_number <= FIRST_PART)
                  {
                    fps = 20;
                  }

                  // 第２幕以降は速く表示
                  else
                  {
                    fps = 4;
                  }
                  int pixel_num = display_image.cols * display_image.rows;
                  for(int i = 0; i < fps * NORMAL_TIME; ++i)
                  {
                    // 到着した写真を庭写真にリサイズ
                    cv::Mat resize_diff, resize_garden;
                    cv::resize(diff_photo, resize_diff, cv::Size(display_image.cols, display_image.rows));
                    cv::Mat garden_show(garden_image, garden_roi);
                    cv::resize(garden_show, resize_garden, cv::Size(display_image.cols, display_image.rows));

                    // ブレンド
                    display_image = alphaBlend(resize_garden, resize_diff, (double)i / (double)(fps * NORMAL_TIME));

                    showFullScreen(display_image);
                  }
                  cout << "Garden -> Diff Elapsed time: " << t.elapsed() << endl;

                  time_t current;
                  struct tm *local;
                  time(&current);
                  local = localtime(&current);
                  std::stringstream ss;
                  ss << "diff/";
                  ss << local->tm_mon + 1;
                  ss << "-";
                  ss << local->tm_mday;
                  ss << "-";
                  ss << local->tm_hour;
                  ss << "_";
                  ss << local->tm_min;
                  ss << "_";
                  ss << local->tm_sec;
                  ss << "_";
                  ss << arrival_pic_number;
                  ss << ".jpg";
                  cv::imwrite(ss.str(), display_image);
                }

                ++arrival_pic_number;

                // 撮影写真表示モードにする
                screen_mode = 1;
              }
            }
          }
        } while(dent != NULL);

        closedir(dp);
      }
    }
    catch(std::exception &e)
    {
      cerr << "getArrivalFile" << endl
           << e.what() << endl;
    }
  }
}

vector<string> MainWindow::split(const string& str, char delim)
{
  vector<string> res;
  size_t current = 0, found;
  while((found = str.find_first_of(delim, current)) != string::npos)
  {
    res.push_back(string(str, current, found - current));
    current = found + 1;
  }
  res.push_back(string(str, current, str.size() - current));
  return res;
}

cv::Mat MainWindow::alphaBlend(const cv::Mat& src1, const cv::Mat& src2, const double alpha)
{
  cv::Mat dst(src1.size(), src1.type());
  for(int y = 0; y < dst.rows; ++y)
  {
    for(int x = 0; x < dst.cols; ++x)
    {
      for(int c = 0; c < dst.channels(); ++c)
      {
        dst.at<cv::Vec3b>(y, x)[c] = (uchar)((1.0 - alpha) * (double)src1.at<cv::Vec3b>(y, x)[c] + alpha * (double)src2.at<cv::Vec3b>(y, x)[c]);
      }
    }
  }
  return dst;
}

void MainWindow::showFullScreen(const cv::Mat& image)
{
  //    cv::resize(mask_show, mask_show, cv::Size(display_image.cols, display_image.rows));

  //    cv::Mat show(display_image.size(), display_image.type());
  //    show = cv::Mat::zeros(show.size(), show.type());
  //    image.copyTo(show, mask_show);

  cvNamedWindow("arrival", 0);
  cvMoveWindow("arrival", 0, -800);

  IplImage new_image = image;
  cvShowImage("arrival", &new_image);
  cvSetWindowProperty("arrival", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

  //  cv::imshow("display", image);
}

void MainWindow::showMatInfo(const cv::Mat& src)
{
  cout << "size: " << src.cols << " x " << src.rows << endl
       << "type: " << src.type() << endl;
}

void MainWindow::changeGardenROI(const int x, const int y)
{
  garden_roi.x = x;
  garden_roi.y = y;
  if(garden_roi.x < 0)
    garden_roi.x = 0;
  if(garden_roi.y < 0)
    garden_roi.y = 0;
  if(garden_roi.x + garden_roi.width > garden_image.cols)
    garden_roi.x = garden_image.cols - garden_roi.width;
  if(garden_roi.y + garden_roi.height > garden_image.rows)
    garden_roi.y = garden_image.rows - garden_roi.height;
}

cv::Point MainWindow::getGravCenter(const cv::Mat& mask)
{
  cv::Point grav(0, 0);
  if(mask.type() == CV_8UC1)
  {
    int user_pixel_num = 0;
    for(int y = 0; y < mask.rows; ++y)
    {
      for(int x = 0; x < mask.cols; ++x)
      {
        // 画素値が0でなければ
        if(mask.at<uchar>(y, x) != 0)
        {
          grav.x += x;
          grav.y += y;
          ++user_pixel_num;
        }
      }
    }
    if(user_pixel_num != 0)
    {
      grav.x /= user_pixel_num;
      grav.y /= user_pixel_num;
    }
  }
  return grav;
}

cv::Point MainWindow::getTopPoint(const cv::Mat& mask)
{
  cv::Mat tmp = mask.clone();
  vector<vector<cv::Point> > contours;
  cv::findContours(tmp, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

  cv::Point max(mask.cols, mask.rows);
  if(contours.size() > 0)
  {
    for(int i = 0; i < contours.at(0).size(); ++i)
    {
      cv::Point contour_point = contours.at(0).at(i);
      if(contour_point.y < max.y)
      {
        max = contour_point;
      }
    }
  }

  return max;
}

bool MainWindow::isAllBlack(const cv::Mat& src)
{
  bool ret = true;
  if(src.type() == CV_8UC1)
  {
    for(int y = 0; y < src.rows; ++y)
    {
      for(int x = src.cols / 3; x < src.cols / 3 * 2; ++x)
      {
        if(src.at<uchar>(y, x) != 0)
        {
          ret = false;
        }
      }
    }
  }
  else
  {
    ret = false;
  }
  return ret;
}

void MainWindow::on_pushButton_SensorStart_clicked()
{
  // Kinect Start
  kinect.init();
  kinect_thread_running = true;
  kinect_thread = new boost::thread(boost::bind(&MainWindow::getKinectData, this));
}

