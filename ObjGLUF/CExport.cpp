#include "stdafx.h"
#include "ObjGLUF.h"
#include "GLUFGui.h"
#include "CExport.h"

extern _declspec(dllexport) void __cdecl SimulateGameDLL(int num_games, int rand_in) {

	// This is part of the DLL, so we can call any function we want
	// in the C++. The parameters can have any names we want to give
	// them and they don't need to match the extern declaration.
}