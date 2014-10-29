#pragma once
#include "Global.h"

//Camera class that will allow us to debug in 3D space.
//In our game our camera will stay in place while the scene scrolls left to right, I have implemented camera movement though just for debugging purposes
class Camera
{
private:
	//vector for rotation around X,Y,Z axis
	Vector rotation;

	//vector for position of camera(During game it will stay still)
	Vector position;

	//distances to alter each dimension by
	float distanceX;
	float distanceY;
	float distanceZ;

	//distaces to rotate each dimension by
	float rotDistanceX;
	float rotDistanceY;
	float rotDistanceZ;
public:
	Camera();
	~Camera();

	//get individual axis's positions
	float getPositionX();
	float getPositionY();
	float getPositionZ();

	//get individual axis's rotation
	float getRotationX();
	float getRotationY();
	float getRotationZ();


	//set overall position or rotation
	void setPosition(float x, float y, float z);
	void setRotation(float x, float y, float z);

	//get and set the distance you want to move in each direction
	float getDistanceX();
	float getDistanceY();
	float getDistanceZ();
	void setDistanceX(float x);
	void setDistanceY(float y);
	void setDistanceZ(float z);

	//get and set the distace you want to rotate in each direction
	float getRotationDistanceX();
	float getRotationDistanceY();
	float getRotationDistanceZ();
	void setRotationDistanceX(float x);
	void setRotationDistanceY(float y);
	void setRotationDistanceZ(float z);

	void reset();

};

