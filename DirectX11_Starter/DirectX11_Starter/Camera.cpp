#include "Camera.h"


Camera::Camera()
{
	position.x = 0;
	position.y = 0;
	//set the z distance at -5 so our camera is looking toward our scene
	position.z = -5;

	distanceX = 0;
	distanceY = 0;
	//set the z distance at -5 so our camera is looking toward our scee
	distanceZ = -5;

	rotation.x = 0;
	rotation.y = 0;
	rotation.z = +10;

	rotDistanceX = 0;
	rotDistanceY = 0;
	rotDistanceZ = +10;
}

Camera::~Camera()
{
}

//resets the camera back to original position
void Camera::reset()
{
	distanceX = 0;
	distanceY = 0;
	distanceZ = -5;
	
	position.x = 0;
	position.y = 0;
	position.z = -5;

	rotDistanceX = 0;
	rotDistanceY = 0;
	rotDistanceZ = +10;

	rotation.x = 0;
	rotation.y = 0;
	rotation.z = +10;
}

//zooms the camera in and out
void Camera::setDistanceZ(float z)
{
	distanceZ += z;
}

float Camera::getDistanceZ() { return distanceZ; }

//moves camera left and right
void Camera::setDistanceX(float x)
{
	distanceX += x;
}

float Camera::getDistanceX() { return distanceX; }

//Moves camera up and down
void Camera::setDistanceY(float y)
{
	distanceY += y;
}

float Camera::getDistanceY(){ return distanceY; }

//rotates camera around X axis
void Camera::setRotationDistanceX(float x)
{
	rotDistanceX += x;
}

float Camera::getRotationDistanceX() { return rotDistanceX; }

//rotates camera around y axis
void Camera::setRotationDistanceY(float y)
{
	rotDistanceY += y;
}

float Camera::getRotationDistanceY() { return rotDistanceY; }

//rotates camera around z axis
void Camera::setRotationDistanceZ(float z)
{
	rotDistanceZ += z;
}

float Camera::getRotationDistanceZ() { return rotDistanceZ; }

//gets position of where camera currently is
float Camera::getPositionX() {return position.x;}
float Camera::getPositionY() {return position.y;}
float Camera::getPositionZ() {return position.z;}

//gets rotation measurements of where camera currently is
float Camera::getRotationX() { return rotation.x;}
float Camera::getRotationY() { return rotation.y; }
float Camera::getRotationZ() { return rotation.z; }

//sets the overall position of camera all at once
void Camera::setPosition(float x, float y, float z) 
{
	position.x = x;
	position.y = y;
	position.z = z;
}

//sets the overall rotation of camera all at once
void Camera::setRotation(float x, float y, float z)
{
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
}