///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017, Carnegie Mellon University and University of Cambridge,
// all rights reserved.
//
// ACADEMIC OR NON-PROFIT ORGANIZATION NONCOMMERCIAL RESEARCH USE ONLY
//
// BY USING OR DOWNLOADING THE SOFTWARE, YOU ARE AGREEING TO THE TERMS OF THIS LICENSE AGREEMENT.  
// IF YOU DO NOT AGREE WITH THESE TERMS, YOU MAY NOT USE OR DOWNLOAD THE SOFTWARE.
//
// License can be found in OpenFace-license.txt

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite at least one of the following works:
//
//       OpenFace: an open source facial behavior analysis toolkit
//       Tadas Baltrušaitis, Peter Robinson, and Louis-Philippe Morency
//       in IEEE Winter Conference on Applications of Computer Vision, 2016  
//
//       Rendering of Eyes for Eye-Shape Registration and Gaze Estimation
//       Erroll Wood, Tadas Baltrušaitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling 
//       in IEEE International. Conference on Computer Vision (ICCV),  2015 
//
//       Cross-dataset learning and person-speci?c normalisation for automatic Action Unit detection
//       Tadas Baltrušaitis, Marwa Mahmoud, and Peter Robinson 
//       in Facial Expression Recognition and Analysis Challenge, 
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015 
//
//       Constrained Local Neural Fields for robust facial landmark detection in the wild.
//       Tadas Baltrušaitis, Peter Robinson, and Louis-Philippe Morency. 
//       in IEEE Int. Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    
//
///////////////////////////////////////////////////////////////////////////////
// FaceTrackingVid.cpp : Defines the entry point for the console application for tracking faces in videos.

// Libraries for landmark detection (includes CLNF and CLM modules)
#include "LandmarkCoreIncludes.h"
#include "GazeEstimation.h"

#include <fstream>
#include <sstream>

// OpenCV includes
#include <opencv2/videoio/videoio.hpp>  // Video write
#include <opencv2/videoio/videoio_c.h>  // Video write
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <SequenceCapture.h>
#include <Visualizer.h>
#include <VisualizationUtils.h>

// Boost includes
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>


#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

static void printErrorAndAbort( const std::string & error )
{
    std::cout << error << std::endl;
    abort();
}

#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )

using namespace std;

// Global Vars :v 

bool detection_success=FALSE;
cv::Vec6d pose_estimate;
cv::Point3f gazeDirection0(0, 0, -1);
cv::Point3f gazeDirection1(0, 0, -1);
double g_doubleArray[140];

// DLL FNs

extern "C" __declspec(dllexport) int __stdcall getXY(void** ppDoubleArrayReceiver)
{
	*ppDoubleArrayReceiver = (void*)g_doubleArray;
	return 0;
}

extern "C"
{
	__declspec(dllexport) int __stdcall getdetection_success() {
		return detection_success;
	}
}

extern "C"
{
	__declspec(dllexport) float __stdcall get_pose1()
	{
		return pose_estimate[3];
	}
}

extern "C"
{
	__declspec(dllexport) float __stdcall get_pose2()
	{
		return pose_estimate[4];
	}
}

extern "C"
{
	__declspec(dllexport) float __stdcall get_pose3()
	{
		return pose_estimate[5];
	}
}

extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze1()
	{
		return gazeDirection0.x;
	}
}
extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze2()
	{
		return gazeDirection0.y;
	}
}
extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze3()
	{
		return gazeDirection0.z;
	}
}
extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze4()
	{
		return gazeDirection1.x;
	}
}
extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze5()
	{
		return gazeDirection1.y;
	}
}
extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze6()
	{
		return gazeDirection1.z;
	}
}

vector<string> get_arguments(int argc, char **argv)
{

	vector<string> arguments;

	for(int i = 0; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;
}

// The Main Function 

extern "C"
{
	__declspec(dllexport) int __stdcall main ()
{
	int argcc = 3;
	char* argvv[4];

	// FaceLandmarkVid.exe Need to be changed : I hope I don't forget about it :v
	// Device is 0 for laptop webcam, it can get 1...

	argvv[0] = "E:\\Projects\\Face\\FaceLandmarkVid.exe";
	argvv[1] = "-device";
	argvv[2] = "0";
	argvv[3] = NULL;

	vector<string> arguments = get_arguments(argcc, argvv);

	// no arguments: output usage
	if (arguments.size() == 1)
	{
		cout << "For command line arguments see:" << endl;
		cout << " https://github.com/TadasBaltrusaitis/OpenFace/wiki/Command-line-arguments";
		return 0;
	}

	LandmarkDetector::FaceModelParameters det_parameters(arguments);

	// The modules that are being used for tracking
	LandmarkDetector::CLNF face_model(det_parameters.model_location);

	// Open a sequence
	Utilities::SequenceCapture sequence_reader;

	// A utility for visualizing the results (show just the tracks)
	Utilities::Visualizer visualizer(true, false, false);

	// Tracking FPS for visualization
	Utilities::FpsTracker fps_tracker;
	fps_tracker.AddFrame();

	int sequence_number = 0;

	while (true) // this is not a for loop as we might also be reading from a webcam
	{

		// The sequence reader chooses what to open based on command line arguments provided
		if (!sequence_reader.Open(arguments))
			break;

		INFO_STREAM("Device or file opened");
		cv::Mat captured_image = sequence_reader.GetNextFrame();
		INFO_STREAM("Starting tracking");
		while (!captured_image.empty()) // this is not a for loop as we might also be reading from a webcam
		{

			// Reading the images
			cv::Mat_<uchar> grayscale_image = sequence_reader.GetGrayFrame();

			// The actual facial landmark detection / tracking
			detection_success = LandmarkDetector::DetectLandmarksInVideo(grayscale_image, face_model, det_parameters);

			// Gaze tracking, absolute gaze direction
			// If tracking succeeded and we have an eye model, estimate gaze
			if (detection_success && face_model.eye_model)
			{
				GazeAnalysis::EstimateGaze(face_model, gazeDirection0, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, true);
				GazeAnalysis::EstimateGaze(face_model, gazeDirection1, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, false);

				// Fill the XY position to send it to C# :D

				for (int i = 0; i < 136; i++) {
					g_doubleArray[i] = face_model.detected_landmarks.at<double>(i);
				}

				// --> those lines for debuging purpose for later :v

				//cout << "t[0] : " << g_doubleArray[0] << "\n";
				//cout << "t[10] : " << g_doubleArray[10] << "\n";
				//cout << "t[100] : " << g_doubleArray[100] << "\n";
				//cout << "pts : " << face_model.detected_landmarks.at<double>(0) << "\n";
			}

			// Work out the pose of the head from the tracked model
			pose_estimate = LandmarkDetector::GetPose(face_model, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy);

			// Keeping track of FPS
			fps_tracker.AddFrame();

			// Displaying the tracking visualizations
			visualizer.SetImage(captured_image, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy);
			visualizer.SetObservationLandmarks(face_model.detected_landmarks, face_model.detection_certainty, face_model.GetVisibilities());
			visualizer.SetObservationPose(pose_estimate, face_model.detection_certainty);
			visualizer.SetObservationGaze(gazeDirection0, gazeDirection1, LandmarkDetector::CalculateAllEyeLandmarks(face_model), LandmarkDetector::Calculate3DEyeLandmarks(face_model, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy), face_model.detection_certainty);
			visualizer.SetFps(fps_tracker.GetFPS());
			// detect key presses (due to pecularities of OpenCV, you can get it when displaying images)
			char character_press = visualizer.ShowObservation();

			// restart the tracker
			if (character_press == 'r')
			{
				face_model.Reset();
			}
			// quit the application
			else if (character_press == 'q')
			{
				face_model.Reset();
				return(0);
			}

			// Grabbing the next frame in the sequence
			captured_image = sequence_reader.GetNextFrame();

		}
		
		// Reset the model, for the next video
		face_model.Reset();
		sequence_number++;

	}
	return 0;
}
}
