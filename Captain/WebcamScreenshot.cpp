//#include "WebcamScreenshot.h"
//
//#include <iostream>
//#include "include/opencv/include/opencv2/opencv.hpp"
//
//using namespace std;
//
//int WebcamScreenshot::GetConnectedCamerasCount()
//{
//    int cameraCount = 0; 
//    cv::VideoCapture cap; 
//
//    for (int i = 0; i <= 10; i++)
//    {
//        cap.open(i); 
//        if (!cap.isOpened()) 
//        {
//            break;
//        }
//        cameraCount++; 
//        cap.release(); 
//    }
//
//    return cameraCount;
//}
//
//std::string WebcamScreenshot::getNowTime()
//{
//    time_t now = time(nullptr); 
//    char buf[80];
//    strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", localtime(&now)); 
//    return std::string(buf);
//}
//
//bool WebcamScreenshot::Make(std::string sSavePath)
//{
//
//    std::vector<cv::VideoCapture> captures;
//    int num_cameras = GetConnectedCamerasCount();;
//
//    for (int i = 0; i < num_cameras; i++) {
//        cv::VideoCapture cap(i);
//        if (!cap.isOpened()) {
//            std::cerr << "Failed to open camera " << i << ".\n";
//            continue;
//        }
//        captures.push_back(cap);
//    }
//
//    for (int i = 0; i < captures.size(); i++) {
//        cv::Mat frame;
//        captures[i] >> frame; 
//        if (frame.empty()) {
//            std::cerr << "Failed to capture frame from camera " << i << ".\n";
//            continue;
//        }
//
//        std::string filePath = sSavePath + "\\" + WebcamScreenshot::getNowTime() + "Webcam" + std::to_string(i) + ".png"; 
//        cv::imwrite(filePath, frame); 
//
//        std::cout << "Image saved to: " << filePath << std::endl;
//    }
//
//    return true;
//}
//
