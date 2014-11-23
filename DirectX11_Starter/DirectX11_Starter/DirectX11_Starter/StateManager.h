#ifndef _STATEMANAGER_H
#define _STATEMANAGER_H

#include <string>
using namespace std;
class StateManager{
public:
	StateManager();
	~StateManager(void);
	wchar_t* changeState();
	wchar_t* setState(int index);
	wchar_t* returnState();
	wchar_t* getStateFromIndex(int index);
private:
protected:
	wchar_t* states[6];
	int adj[6][6];
	int currentState;
};
#endif