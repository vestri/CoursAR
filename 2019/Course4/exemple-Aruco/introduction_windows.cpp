//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <aruco/aruco.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <Windows.h>

using namespace cv;
using namespace std;

static cv::Size		sBoardSize			= cvSize(7, 5);
static cv::Size2f	sBoardSquareSize	= Size2f(0.03f, 0.03f);

static void calcChessboardCorners(vector<Point3f>& pCorners)
{
	pCorners.resize(0);
	for (int i = 0; i < sBoardSize.height; i++)
		for (int j = 0; j < sBoardSize.width; j++)
		{
			Point3f lP(float(j) * sBoardSquareSize.width, float(i) * sBoardSquareSize.height, 0.0f);
			pCorners.push_back(lP);
			std::cout << lP << std::endl;
		}
}

int main(int argc, char* argv[])
{
	static std::vector<std::vector<Point3f>> chessBoardObjectCorners(1);
	
	calcChessboardCorners(chessBoardObjectCorners[0]);

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
	
	//Open the default video camera
	VideoCapture cap(0);

	// if not success, exit program
	if (cap.isOpened() == false)
	{
		cout << "Cannot open the video camera" << endl;
		cin.get(); //wait for any key press
		return -1;
	}

	cap.set(CV_CAP_PROP_FRAME_WIDTH, 800);		//set the width of frames of the video
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 600);	//set the height of frames of the video

	double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);		//get the width of frames of the video
	double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);		//get the height of frames of the video

	cout << "Resolution of the video : " << dWidth << " x " << dHeight << endl;

	string window_name = "My Camera Feed";
	namedWindow(window_name); //create a window called "My Camera Feed"

	bool bCameraIsCalibrated = false;
	bool bCalibrationFrameRequested = false;
	bool bContinuousFrameRequested = false;

	std::vector<std::vector<cv::Point2f>> calibrationCorners;

	float markerSize = 0.05f;
	std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
	std::vector<cv::Point2f> chessBoardCorners;

	aruco::CameraParameters					mCameraParameters;
	aruco::MarkerDetector					mMarkerDetector;
	std::map<int, aruco::MarkerPoseTracker>	mMarkerPoseTrackerMap;

	mMarkerDetector.setDetectionMode(aruco::DetectionMode::DM_NORMAL);
	
	std::vector<aruco::Marker>	mMarkerVector;

	std::vector<cv::Mat>		imagesVector;
	std::vector<std::string>	posesStringVector;

	bool						bNewFrame = false;	

	Mat							previousGrayFrame;

	while (true)
	{
		Mat frame, frameCopy, frameGrayCopy;
		while (!bNewFrame)
		{
			bool bSuccess = cap.read(frame); // read a new frame from video 
			
			//Breaking the while loop if the frames cannot be captured
			if (bSuccess == false)
			{
				cout << "Video camera is disconnected" << endl;
				cin.get(); //Wait for any key press
				break;
			}

			if(frame.size() != previousGrayFrame.size())
				bNewFrame = true;
			else
			{
				Mat matGray;
				cv::cvtColor(frame, matGray, CV_BGR2GRAY);
				Mat matDiff = matGray - previousGrayFrame;
				if (cv::countNonZero(matDiff) > 0)
					bNewFrame = true;
			}
		}
		
		bNewFrame = false;

		frame.copyTo(frameCopy);
		cv::cvtColor(frame, frameGrayCopy, CV_BGR2GRAY);

		frameGrayCopy.copyTo(previousGrayFrame);

		mMarkerVector = mMarkerDetector.detect(frame);
		for (int i = 0; i < mMarkerVector.size(); i++)
		{
			mMarkerVector.at(i).draw(frameCopy);
		}

		std::vector<cv::Vec3d> rvecs, tvecs;

		if (bCameraIsCalibrated)
		{
			std::string lMessage = std::to_string(mMarkerVector.size()) + " mires visibles.";
			
			for (auto &lMarker : mMarkerVector)
			{
				//if (!mMarkerPoseTrackerMap[lMarker.id].estimatePose(lMarker, mCameraParameters, markerSize, 4))
					lMarker.calculateExtrinsics(markerSize, mCameraParameters);
				aruco::CvDrawingUtils::draw3dAxis(frameCopy, lMarker, mCameraParameters);
			}
			
			if (mMarkerVector.size() == 2)
			{
				auto lMat = mMarkerVector.at(1).Tvec - mMarkerVector.at(0).Tvec;
				lMessage += "Distance = " + std::to_string(cv::norm(lMat));
			}

			cv::putText(frameCopy, lMessage, cv::Point(5, 45), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 1, cvScalar(250, 250, 250));
		}
		if (bContinuousFrameRequested)
		{
			imagesVector.push_back(Mat(frameGrayCopy));
			std::string lMessage = "Acquisition en cours. " + std::to_string(imagesVector.size()) + " images acquises.";
			cv::putText(frameCopy, lMessage, cv::Point(5, 15), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 1, cvScalar(250, 250, 250));

			std::stringstream lstrstream;
			lstrstream << imagesVector.size() - 1 << ":" << mMarkerVector.size() << ":";
			for (auto &lMarker : mMarkerVector)
			{
				lstrstream << lMarker.id << ":";
				lstrstream << std::fixed << std::setprecision(16) << lMarker.Rvec.at<double>(0, 0) << ',' << lMarker.Rvec.at<double>(0, 1) << ',' << lMarker.Rvec.at<double>(0, 2) << ';';
				lstrstream << std::fixed << std::setprecision(16) << lMarker.Tvec.at<double>(0, 0) << ',' << lMarker.Tvec.at<double>(0, 1) << ',' << lMarker.Tvec.at<double>(0, 2) << ';';
			}
			posesStringVector.push_back(lstrstream.str());
		}

		else if(bCalibrationFrameRequested)
		{ 
			bool bPatternFound = cv::findChessboardCorners(frameGrayCopy, sBoardSize, chessBoardCorners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
			if (bPatternFound)
			{
				cv::cornerSubPix(frameGrayCopy, chessBoardCorners, cvSize(11, 11), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
				calibrationCorners.push_back(chessBoardCorners);
			}
			drawChessboardCorners(frameCopy, sBoardSize, Mat(chessBoardCorners), bPatternFound);
			
			bCalibrationFrameRequested = false;
		}
		else
		{
			std::string lMessage;
			bool bPatternFound = cv::findChessboardCorners(frameGrayCopy, sBoardSize, chessBoardCorners, CALIB_CB_FAST_CHECK);
			if (bPatternFound)
				drawChessboardCorners(frameCopy, sBoardSize, Mat(chessBoardCorners), bPatternFound);
			if (!bCameraIsCalibrated)
				lMessage = "Camera non calibree. Pressez \'g\' pour demarrer celui-ci ou \'l\' pour charger le dernier.";
			else
				lMessage = "Camera en cours de calibrage " + std::to_string(calibrationCorners.size()) + " images acquises.";
			cv::putText(frameCopy, lMessage, cv::Point(5, 15), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 1, cvScalar(250, 250, 250));
		}

		std::string lMessage = "Taille de Mire : " + std::to_string(markerSize);
		cv::putText(frameCopy, lMessage, cv::Point(5, 30), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 1, cvScalar(250, 250, 250));
			
		//show the frame in the created window
		imshow(window_name, frameCopy);

		//wait for for 10 ms until any key is pressed.  
		//If the 'Esc' key is pressed, break the while loop.
		//If the any other key is pressed, continue the loop 
		//If any key is not pressed withing 10 ms, continue the loop 
		int keyPressed = waitKey(10);
		if (keyPressed == 27)
		{
			cout << "Esc key is pressed by user. Stoppig the video" << endl;
			break;
		}
		else if (keyPressed == '3' || keyPressed == '5' || keyPressed == '7')
		{
			markerSize = float(keyPressed - '0') / 100.0f;
		}
		else if (keyPressed == 'c')
		{
			//fin de la sequence de capture on calibre maintenant
			chessBoardObjectCorners.resize(calibrationCorners.size(), chessBoardObjectCorners[0]);
			vector<Mat> rvecs, tvecs;
			double lRms = calibrateCamera(chessBoardObjectCorners, calibrationCorners, cvSize(dWidth, dHeight), cameraMatrix, distCoeffs, rvecs, tvecs);
			cout << "Fin du calibrage de la camera. RMS = " << lRms << endl;
			bCameraIsCalibrated = true;
		}
		else if (keyPressed == 'g')
		{
			bCalibrationFrameRequested = true;
		}
		else if (keyPressed == 'l')
		{
			std::string lFilename = "camera_calib.yaml";
			cv::FileStorage fs(lFilename, FileStorage::READ);

			fs["camera_matrix"] >> cameraMatrix;
			fs["distortion_coefficients"] >> distCoeffs;

			mCameraParameters.setParams(cameraMatrix, distCoeffs, cvSize(dWidth, dHeight));

			fs.release();
			bCameraIsCalibrated = true;
		}
		else if (keyPressed == 's')
		{
			std::string lFilename = "camera_calib.yaml";
			cv::FileStorage fs(lFilename, FileStorage::WRITE);

			auto t = std::time(nullptr);
			auto tm = *std::localtime(&t);
			
			//fs << "calibration_time" << std::put_time(&tm, "-%Y%m%d-%H-%M-%S");
			fs << "camera_matrix" << cameraMatrix;
			fs << "distortion_coefficients" << distCoeffs;

			fs.release();

			std::stringstream lFilenameBackup;
			lFilenameBackup << lFilename << std::put_time(&tm, "-%Y%m%d-%H-%M-%S") << ".yaml";

			std::ifstream src(lFilename,				std::ios::binary);
			std::ofstream dst(lFilenameBackup.str(),	std::ios::binary);
			dst << src.rdbuf();
		}
		else if (keyPressed == 't')
		{
			if (bContinuousFrameRequested == true)
			{
				bContinuousFrameRequested = false;
				std::stringstream lFoldername;
				auto t = std::time(nullptr);
				auto tm = *std::localtime(&t);
				lFoldername << "data" << std::put_time(&tm, "-%Y%m%d-%H-%M-%S");
				if (CreateDirectory(lFoldername.str().c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
				{
					// On sauve les images dans le folder 
					for (int i = 0; i < imagesVector.size(); i++)
					{
						std::stringstream lfilenamestream;
						lfilenamestream << lFoldername.str() << "\\" << "img" << std::setw(5) << std::setfill('0') << i << ".tiff";
						try 
						{
							cv::imwrite(lfilenamestream.str(), imagesVector.at(i));
						}
						catch (runtime_error & pException)
						{
							std::cerr << "Exception converting image to tiff format: %s\n", pException.what();
						}
					}
					// Puis les données de pose
					std::fstream fs;
					std::string lPosesFilename = lFoldername.str() + "\\" + lFoldername.str() + ".csv";
					fs.open(lPosesFilename.c_str(), std::fstream::out);
					if (fs.is_open())
					{
						for (int i = 0; i < posesStringVector.size(); i++)
						{
							std::cout << posesStringVector[i] << endl;
							fs << posesStringVector[i] << endl;
						}
					}
					fs.close();
				}
				else
					;
			}
			else
			{
				imagesVector.clear();
				posesStringVector.clear();
				bContinuousFrameRequested = true;
			}
		}
	}
	return 0;
}