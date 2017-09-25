#pragma once

#include <maya/MPxToolCommand.h>
#include <maya/MPxContext.h>
#include <maya/MPxContextCommand.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>

class KevinLetBrush : public MPxToolCommand
{
public:
	KevinLetBrush() { setCommandString("KevinLetBrush"); }
	virtual ~KevinLetBrush() {}

	static void* creator() { return new KevinLetBrush; }
	MStatus doIt(const MArgList& args) { return redoIt(); }
	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable()const { return true; }

	MStatus finalize()
	{
		MArgList command;
		command.addArg(commandString());
		return MPxToolCommand::doFinalize(command);
	}
};







