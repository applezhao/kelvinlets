#include "KevinLetBrush.h"


MStatus KevinLetBrush::redoIt()
{
	MGlobal::displayInfo("redoIt!");
	return MS::kSuccess;
}

MStatus KevinLetBrush::undoIt()
{
	return MS::kSuccess;
}
