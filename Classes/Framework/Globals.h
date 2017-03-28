#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "NullPtr.h"

#define SAFE_DELETE_POINTER(__VAR__) \
if(__VAR__) \
{ \
	delete __VAR__; \
	__VAR__ = nullptr; \
} \

//#define PHYSICS_DEBUG 1

//#define STATE_MACHINE	Framework::StateMachine::GetInstance()

#endif