#include "StateManager.h"
#include "Windows.h"

StateManager::StateManager()
{
	currentState = 0;

	//initialize states
	states[0] = L"Menu";
	states[1] = L"Game";
	states[2] = L"Instructions";
	states[3] = L"Pause";
	states[4] = L"Win";
	states[5] = L"Lose";

	//initialize edges
	for (int x = 0; x < 6; x++){
		for (int y = 0; y < 6; y++){
			adj[x][y] = 0;
		}
	}
	adj[0][1] = 1; //Menu to Game
	adj[0][2] = 1; //Menu to Instructions
	adj[1][3] = 1; //Game to Pause
	adj[1][4] = 1; //Game to Win
	adj[1][5] = 1; //Game to Lose
	adj[2][0] = 1; //Instructions to Menu
	adj[3][1] = 1; //Pause to Game
	adj[3][0] = 1; //Pause to Menu
	adj[4][1] = 1; //Win to Game
	adj[4][0] = 1; //Win to Menu
	adj[5][1] = 1; //Lose to Game
	adj[5][0] = 1; //Lose to Menu
}

StateManager::~StateManager(void){
	
}

wchar_t* StateManager::setState(int index){
	currentState = max(0, min(index, 5));
	return states[currentState];
}

wchar_t* StateManager::returnState(){
	return states[currentState];
}

// Simple getter function allowing functions in other classes to get a state name from a state index
wchar_t* StateManager::getStateFromIndex(int index)
{
	return states[index];
}

wchar_t* StateManager::changeState()
{
	if (GetAsyncKeyState('0') & 0x8000){
		if (adj[currentState][0]){
			currentState = 0;
		}
	}
	else if (GetAsyncKeyState('1') & 0x8000){
		if (adj[currentState][1]){
			currentState = 1;
		}
	}
	else if (GetAsyncKeyState('2') & 0x8000){
		if (adj[currentState][2]){
			currentState = 2;
		}
	}
	else if (GetAsyncKeyState('P') & 0x8000){
		if (adj[currentState][3]){
			currentState = 3;
		}
		if (adj[currentState][3]){
			currentState = 1;
		}
	}
	else if (GetAsyncKeyState('4') & 0x8000){
		if (adj[currentState][4]){
			currentState = 4;
		}
	}
	else if (GetAsyncKeyState('5') & 0x8000){
		if (adj[currentState][5]){
			currentState = 5;
		}
	}
	return states[currentState];
}