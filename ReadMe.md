# RoboticArmFruitNinja

# Project Objective:
The object of the project is to be the ultimate fruit ninja player with the hand robot on “XBOX Kinect Fruit Ninja”,
or in simple words, play fruit ninja using the robotic arm in the RDB lab using the Kinect device on the XBOX 360.
In this project, we aimed in creating a working proof of concept of the fruit ninja player,
while making a solid workspace and easy to use code for following projects to continue our ninja/play other games with its algorithms, and improving it, to finally be the worlds best fruit ninja.


# Our solution and workflow
Assumptions:
1. The game is played in classic mode – no bombs are allowed.

# Solution:

Our solution to playing fruit ninja is as follows:
We first connected to our XBOX 360 an easy-cap hauppauge device to capture its view and connected it into our Windows computer, in that computer we have installed easyDevice which turns the easy-cap device video into a available video device in Open CV.
in our Client – the computer which is connected to the EasyCap, we have created an algorithm to detect the fruits with image
processing and created an algorithm to make a good slice of them, with sockets we send each time the best slice to the Linux computer
which is connected to the robotic arm, and it makes the move and asks for a nice slice, and that continues until the game is over, we have made the hand wrist keep rotating, so in the time it goes to slice fruits, it will slice all the fruits surrounding it.
In the end, we have made the kinecket device recognize the robot by dressing it with a jacket and a fake head, it took about 5 seconds each game for the kinecket to recognize the robot after it started moving.

# Workflow:
We have first created our fruit detecting algorithm through image processing on static images, and then on videos to see if it
works, then we have created a basic algorithm which slices the fruits, and then we have made it better with real time
support, after that we have worked on the hand movement on the Robot, then worked on getting the video from the XBOX device, and
then connected our detection and slice algorithm on the real video of the game, we connected using sockets our windows
client which gives the slices to our linux computer which controls the arm and tells it where to move.
and in the end calibrated the robot and dressed it with a jacket and a fake head so the kinecket device will recognize it, and made the wrist turn and the real robot movements so they will move accordingly to the coordinates that the computer generates.

# set up
Client:
The code needs to run on c++ when opencv is installed, 
and needs to be connected to the same wifi of the server running.
You will need to change the background picture according to your game/mode so 
that the IP will work correctly, and change the socket IP according to the server ip.

Server:
the server code needs to run on a c++ opencv support system, and it needs to be connected
to the Robotic arm in the lab.

# Running the programs
First open the linux computer handling the arm and start up the server sockets code.
after that you will need to open a game of fruit ninja on the xbox while all the devices are connected as explained above – and the
kinecket device pointing start to the robotic arm, on the windows client computer you will need to open easyDevice and connect to the
easy cap hauppauge device, after doing that start the client program and the hand will start playing.
Please notice to put the current IP of the Linux computer to the Client windows computer so the sockets will work correctly, you can find it with the ifconfig command on the linux bash.

# Algorithms description:

# Detecting the Fruits

The algorithm was created based on Research done and error and success.:
decrement from the current image an background image with no fruits (save from the start)
by that you get mostly only the fruits
delete lives and time from the image by changing those pixels to black
so this wont interrupt getting the fruits
gray scale and blur the image
easier to detect area with a lot of pixels this way and contours
do the gradient of x,y and add the weights of it to the picture
this gives more strength to the edges of the fruits
do the algorithm of canny on it,
this helps us recognize the fruits edges
find contours of the image
find smallest circle which fits on all the points of the contours – this circles each fruit
by that we get circles around each fruit
Now with this information, we have center (x,y) and radius, and we can pass it to our best slice algorithm





# Best slice algorithm

The algorithm gets all the circles -center(x,y) and radius of current frame:
create all lines from current robot hand position to the circles center
calculate how many circles each line cuts
take the maximum line which cuts the most
update the current hand position the where it needs to cut
now we can pass the coordinate to the hand so it can slice to there.

# more information
For more information and full documentation please refere to:
https://sites.hevra.haifa.ac.il/rbd/2017/11/27/fruitninja/?lang=en



—————————————————————————————————
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

see <http://www.gnu.org/licenses/>.
