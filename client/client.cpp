/*
© Copy Right 2017 - Gil Maman, Dani Rubin

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

see <http://www.gnu.org/licenses/>.
*/

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <sys/types.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/features2d/features2d.hpp"

#include <math.h>
#include <cmath>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib for sockets
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 5
#define DEFAULT_PORT "8887"

//functions
vector< vector<float> > thresh_callback(int, void*, Mat image, Mat originImage);
int getNumCutsOfLine(vector< vector<int> > line, vector< vector<float> > circles);
int detectBestSlice(vector< vector<float> > circles);


//vars
Mat drawing;
vector< vector<int> > maxLine;
Point currentHandLocation;


int __cdecl main(int argc, char **argv)
{
	//init IP
	//init data
	currentHandLocation = Point(0, 0); //sets hand first location
	VideoCapture capture(2);
	Mat frame;

	
	if (!capture.isOpened())
		throw "Error when reading steam_avi";


	//init sockets

	//CONNECT TO SERVER through sockets
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;


	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);

		waitKey(0);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("132.75.55.196", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();

		waitKey(0);

		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();

			waitKey(0);

			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;

			waitKey(0);

			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!!!!\n");

		WSACleanup();

		waitKey(0);
		return 1;
	}
	//END CONNETING TO SERVER

	//connecting to server
	cout << "connected to server" << endl;

	

	// Send an initial coordinate so the hand will start moving
	int coord[4] = { 0 };

	iResult = send(ConnectSocket, (char *)coord, sizeof(int) * 4, 0);
	if (iResult == SOCKET_ERROR) { //on failure
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();

		waitKey(0);

		return 1;
	}

	printf("Bytes Sent: %ld\n", iResult);


	int counter = 0;
	// keep sending coordinates of movment untill the server is closed
	do {

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

        //waint until the robot will finish its movment, when it finishes it sends moved and then we keep reading data
		if (strncmp(recvbuf, "moved", 5) != 0)
			continue;

		//-------algorithem : get next coordinate based on current frame


		//get image
		counter++;
        capture.read(frame);

		//imwrite(to_string(counter)+".jpg", frame);
		//printf("writing : %s", to_string(counter) + ".jpg");

		if (frame.empty())
			break;



		//----------remove bg


		//save data
		Mat savedOrigImage = frame;
		Mat origImage = frame;

		String bgImageSource("bg.png"); // by default
		Mat bgImage = imread(bgImageSource, IMREAD_COLOR);


		//resize them
		Size size(656, 400);
		resize(origImage, origImage, size);
		resize(bgImage, bgImage, size);

		resize(savedOrigImage, savedOrigImage, size);


		//reduce images
		//change some pixel value

		for (int i = 0; i<origImage.rows; i++)
		{
			for (int j = 0; j<origImage.cols; j++)
			{
				origImage.at<cv::Vec3b>(i, j)[0] = (int)abs((int)origImage.at<cv::Vec3b>(i, j)[0] - (int)bgImage.at<cv::Vec3b>(i, j)[0]);
				origImage.at<cv::Vec3b>(i, j)[1] = (int)abs((int)origImage.at<cv::Vec3b>(i, j)[1] - (int)bgImage.at<cv::Vec3b>(i, j)[1]);
				origImage.at<cv::Vec3b>(i, j)[2] = (int)abs((int)origImage.at<cv::Vec3b>(i, j)[2] - (int)bgImage.at<cv::Vec3b>(i, j)[2]);


			}
		}


		//-----remove score and lives

		//score
		for (int i = 0; i<100; i++)
		{
			for (int j = 0; j<100; j++)
			{
				origImage.at<cv::Vec3b>(i, j)[0] = 0;
				origImage.at<cv::Vec3b>(i, j)[1] = 0;
				origImage.at<cv::Vec3b>(i, j)[2] = 0;
			}
		}

		//lives
		for (int i = 0; i<50; i++)
		{
			for (int j = 0; j<100; j++)
			{
				origImage.at<cv::Vec3b>(i, origImage.cols - j)[0] = 0;
				origImage.at<cv::Vec3b>(i, origImage.cols - j)[1] = 0;
				origImage.at<cv::Vec3b>(i, origImage.cols - j)[2] = 0;
			}
		}


		//-----convert to gray scale
		cvtColor(origImage, origImage, CV_BGR2GRAY);

		//gussian blur the image
		GaussianBlur(origImage, origImage, Size(5, 5), 0, 0);


		//get gradient
		Mat grad;


		/// Generate grad_x and grad_y
		Mat grad_x, grad_y;
		Mat abs_grad_x, abs_grad_y;

		/// Gradient X
		Sobel(origImage, grad_x, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
		convertScaleAbs(grad_x, abs_grad_x);

		/// Gradient Y
		Sobel(origImage, grad_y, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
		convertScaleAbs(grad_y, abs_grad_y);

		/// Total Gradient (approximate)
		addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);


		Mat contours;
		Canny(origImage, contours, 15, 240);


		//imshow("detected", contours);

		//imshow("original", savedOrigImage);


        //get circles based on IP done on frame - circles representing the fruits
		vector< vector<float> > circles = thresh_callback(0, 0, contours, savedOrigImage);


		//------------------get slices

		//get robot location
		int robotX = currentHandLocation.x;
		int robotY = currentHandLocation.y;
        
        //based on the circles, get the best slice that the robot will do
		int numCuts = detectBestSlice(circles);
		currentHandLocation = Point(maxLine[0][0], maxLine[0][1]);



		//update coordinates =
		int moveToX = currentHandLocation.x;
		int moveToY = currentHandLocation.y;




		//------------end algorithem



		//write data that is inputed

		cout << "sending data : (" << robotX << "," << robotY << ") to ";
		cout << " (" << moveToX << "," << moveToY << ")  " << endl;
        
        
        //send new coordinates to robot to move
		int coord[4] = { robotX,robotY,moveToX,moveToY }; // sending: (current robot location x, current robot location y, move to x, move to y)

		iResult = send(ConnectSocket, (char *)coord, sizeof(int) * 4, 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			
		}






	} while (1);
	
	// cleanup - close sockets
	closesocket(ConnectSocket);
	WSACleanup();

	waitKey(0);

	return 0;
}



//gets a line and a point, and calculates the distance between the line and the point;
double distance_to_Line(Point line_start, Point line_end, Point point)
{
	double normalLength = hypot(line_end.x - line_start.x, line_end.y - line_start.y);
	double distance = (double)((point.x - line_start.x) * (line_end.y - line_start.y) - (point.y - line_start.y) * (line_end.x - line_start.x)) / normalLength;
	return distance;
}

//gets a line, and all circles, and returns how many circles it cuts
//line : ((x,y),(x,y))
//circles : ((radius,x,y),(radius,x,y)....)
int getNumCutsOfLine(vector< vector<int> > line, vector< vector<float> > circles) {
	Point startLine = Point(line[0][0], line[0][1]);
	Point endLine = Point(line[1][0], line[1][1]);

	int counter = 0;

	//run on all circles
	for (int i = 0; i < circles.size(); i++) {
		Point center = Point(circles[i][1], circles[i][2]);

		double distance = abs(distance_to_Line(startLine, endLine, center));
        
        //if the distance is smaller then the radius of the circle - cutting it
		if (distance <= circles[i][0])
			counter++;

	}

	return counter;

}


//returns all circles based on current frame data
vector< vector<float> > thresh_callback(int, void*, Mat image, Mat originImage)
{
    //arguments of threshold - based on trial and error
	int thresh = 100;
	int max_thresh = 255;
	RNG rng(12345);

    //init data
	vector< vector<float> > totalCirclesData;
	Mat threshold_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

    //get contours of image
	threshold(image, threshold_output, thresh, 255, THRESH_BINARY);
	findContours(threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	
    //set up circles around areas with a lot of pixels
    vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Point2f>center(contours.size());
	vector<float>radius(contours.size());

    //do the circle
	for (size_t i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
		boundRect[i] = boundingRect(Mat(contours_poly[i]));
		minEnclosingCircle(contours_poly[i], center[i], radius[i]);
	}

	drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
	for (size_t i = 0; i< contours.size(); i++)
	{
		if (radius[i] < 60 && radius[i] > 20) {
			Scalar color = Scalar(0, 0, 204); //set up color

            //draw the circle
			circle(drawing, center[i], (int)radius[i], color, 2, 8, 0);


			//add data
			vector<float> circleData;
			circleData.push_back((int)radius[i]);
			circleData.push_back(center[i].x);
			circleData.push_back(center[i].y);

			totalCirclesData.push_back(circleData);

		}



	}


	return totalCirclesData;
}


//detect the best slice algorithem
int detectBestSlice(vector< vector<float> > circles) {

	//set up all lines - from current robot location to all circles center
	vector< vector< vector<int> > > lines;

	for (int i = 0; i < circles.size(); i++) {
		vector< vector<int> > currentPoint;

		//point - one circle center
		vector<int> point1;
		point1.push_back(circles[i][1]); //x
		point1.push_back(circles[i][2]); //y
        
        //current hand location
		vector<int> point2;
		point2.push_back(currentHandLocation.x);
		point2.push_back(currentHandLocation.y);

		//add points
		currentPoint.push_back(point1);
		currentPoint.push_back(point2);

		lines.push_back(currentPoint);


	}

	//run on all circles and get line the cuts most of circles
	int maxCuts = -1;
	for (int i = 0; i < lines.size(); i++) { // run on all lines
		int numCuts = getNumCutsOfLine(lines[i], circles); //get num of cuts
        
        //get max cuts line
		if (numCuts > maxCuts) {
			maxLine = lines[i];
			maxCuts = numCuts;
		}

	}

	return maxCuts;
}








