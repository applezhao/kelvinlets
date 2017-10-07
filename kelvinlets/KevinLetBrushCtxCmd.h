#pragma once
#include "KeviLetBrushCtx.h"
#include <maya/MFnPlugin.h>

class KevinLetBrushCtxCmd :public MPxContextCommand
{
public:
	KevinLetBrushCtxCmd(){}
	virtual MStatus         doEditFlags();
	virtual MStatus         doQueryFlags();
	virtual MPxContext* makeObj()
	{
		MGlobal::displayInfo("makeobj!");
		m_KevinLetBrushCtx = new KevinLetBrushCtx();
		return m_KevinLetBrushCtx;
	}
	virtual MStatus         appendSyntax();
	static void*            creator()
	{
		return new KevinLetBrushCtxCmd;
	}

protected:
	KevinLetBrushCtx*               m_KevinLetBrushCtx;
};


//
MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerContextCommand("KevinLetBrushCtx",
		KevinLetBrushCtxCmd::creator,
		"KevinLetBrush",
		KevinLetBrush::creator);
	if (!status) {
		status.perror("registerContextCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);
	status = plugin.deregisterContextCommand("KevinLetBrushCtx",
		"KevinLetBrush");
	if (!status) {
		status.perror("deregisterContextCommand");
		return status;
	}

	return status;
}


