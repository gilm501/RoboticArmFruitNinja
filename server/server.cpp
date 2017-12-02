
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

#include <iostream>
#include <robotarm.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <deque>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>

using namespace std;

int cntWaits = 0;

//error function, will jump when an error appeir
void error(const char *msg) {
	perror(msg);
	exit(1);
}


//the function
int main(int argc, char *argv[])
{

	//Robot initialization
	robotarm myRobot("192.168.100.10", 80, "thisTraceFile.log", "thisCommLog.log");// DLR robot APIos
	int connectReturnCode = myRobot.Connect(true);  //true - llong conn, false - onetime


													//powering up our robot
	myRobot.svon(true);

	//gets the state now
	RPOSC myRobPosition = { 0 };
	myRobPosition = myRobot.rposc(0);


	//prints the state now
	cout << "Robot position: " << endl;
	cout << "X:  " << myRobPosition.Coord.x << endl;
	cout << "Y:  " << myRobPosition.Coord.y << endl;
	cout << "Z:  " << myRobPosition.Coord.z << endl;
	cout << "X1:  " << myRobPosition.R.x << endl;
	cout << "Y1:  " << myRobPosition.R.y << endl;
	cout << "Z1:  " << myRobPosition.R.z << endl;
	cout << "***********************" << endl;



	// Initialize start position coordinates

	JOINTMOTION start = { 0 };
	start.Pos.Coord.x = 712.991;
	start.Pos.Coord.y = 0.033;
	start.Pos.Coord.z = 439.993;
	start.Pos.R.x = -141.115;
	start.Pos.R.y = -89.9983;
	start.Pos.R.z = -38.8818;
	start.MotionSpeedPerc = 80;

	//moves the robot to the start position
	myRobot.movj(start);

	//waits until the movment will finish
	sleep(1);




	//server code

	//initialize the socket parameters
	socklen_t clilen;
	int sockfd, newsockfd, portno;
	struct sockaddr_in serv_addr, cli_addr;

	//create new socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	//if the opening went wrong send an error
	if (sockfd < 0)
		error("ERROR opening socket");


	//resets bits of server address in struct
	bzero((char *)&serv_addr, sizeof(serv_addr));
	//initialize the port that the sockets will "talk" on it
	portno = 8887;

	//set so the server will get joining request from each client
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	//set the server port with endians so there will be no formatting error
	serv_addr.sin_port = htons(portno);

	//bind server
	//if an error accure when binding the servers
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	cout << "waiting for socket connection" << endl;

	//listen on the socket
	listen(sockfd, 5); //allowing 5 requests to be in request queue

					   //receiving current client
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	//send error if binding went wrong
	if (newsockfd < 0)
		error("ERROR on accept");

	cout << "conection accepted" << endl;

	//read data fron socket
	int n;
	int buffer[4] = { 0 };
	bzero(buffer, 4);

	//keep reading
	while (1) {

		bzero(buffer, 4);

		//read from socket
		n = read(newsockfd, buffer, 4 * sizeof(int));

		//send error if failed reading from socket
		if (n < 0)
			error("ERROR reading from socket");

		//robot section

		//our coords
		int robotX = buffer[0];
		int robotY = buffer[1];
		//needed coords
		int moveToX = buffer[2];
		int moveToY = buffer[3];

		//make the movment

		//create linear movment
		LINEARMOTION play = { 0 };

		//gets the state now
		RPOSC currPos = { 0 };
		currPos = myRobot.rposc(0);

		//move y - set limits to movment
		int movY = currPos.Coord.y + robotX - moveToX;
		if (movY > 400 || movY <-400)
			movY = 0;
		else {
			movY -= currPos.Coord.y;

		}

		//move z  - set limits to movment
		int movZ = currPos.Coord.z + robotY - moveToY;
		if (movZ > 400 || movZ<-400)
			movZ = 0;
		else
			movZ -= currPos.Coord.z;


		//sets movment of y to next coord
		play.Pos.Coord.y = movY;


		//rotate wrist
		//-150 < R.x < 150
		if (play.Pos.R.x >= 0) {
			play.Pos.R.x = -50 - play.Pos.R.x;
		}
		if (play.Pos.R.x < 0) {
			play.Pos.R.x = 50 - play.Pos.R.x;
		}

		//set movment of x next coord
		play.Pos.Coord.z = movZ;
		//set speed movment
		play.Speed = 1000;


		//make the move
		myRobot.imov(play);
		//wait until movment finished
		sleep(0.6);

		//prints the position
		RPOSC myRobPosition = { 0 };
		myRobPosition = myRobot.rposc(0);
		cout << "Robot position: " << endl;
		cout << "X:  " << myRobPosition.Coord.x << endl;
		cout << "Y:  " << myRobPosition.Coord.y << endl;
		cout << "Z:  " << myRobPosition.Coord.z << endl;
		cout << "***********************" << endl;

		printf("moving from (%i,%i) to (%i,%i) \n", robotX, robotY, moveToX, moveToY);


		//finish socket code
		n = write(newsockfd, "moved", 5);

		//if error writing to socket
		if (n < 0)
			error("ERROR writing to socket");

	}

	//closing everything
	//closing socket
	close(newsockfd);
	close(sockfd); //or goto accept to wait for another clients
				   //closing robot
	sleep(2);
	myRobot.svon(false);

	myRobot.Disconnect();
	return 0;

}
