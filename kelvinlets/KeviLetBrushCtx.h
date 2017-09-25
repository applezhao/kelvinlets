#pragma once

#include "KevinLetBrush.h"
#include <maya/MPoint.h>
#include <maya/MCursor.h>
#include <maya/M3dView.h>
#include <maya/MFnMesh.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>

class KevinLetBrushCtx :public MPxContext
{
public:
	KevinLetBrushCtx(void)
	{
		pressure = 1.f;
		radiusScale = 3.f;
		elasticShearModulus = 1;
		poissonRatio = 0.5;

		brushRadius = 50;

		brushMode = k_GRAB;

		setTitleString("KevinLetBrush");
		setCursor(MCursor::handCursor);
		setImage("KevinLetBrush.xpm", MPxContext::kImage1);

		computeOtherParams();

	}
	virtual ~KevinLetBrushCtx(void) {}

	virtual void toolOnSetup(MEvent & event) {}
	//virtual void toolOffCleanup();
	virtual MStatus doPress(MEvent & event);
	virtual MStatus doDrag(MEvent & event);
	virtual MStatus doRelease(MEvent & event);
	virtual MStatus doEnterRegion(MEvent & event)
	{
		MGlobal::displayInfo("doEnterRegion");
		view = M3dView::active3dView();
		return MS::kSuccess;
	}
	virtual MStatus doHold(MEvent & event)
	{
		MGlobal::displayInfo("doHold");
		return MS::kSuccess;
	}
	virtual void getClassName(MString & name)const
	{
		name.set("KelvinBrushPaint");
	}

	void setPressure(float pressure)
	{
		this->pressure = pressure;
	}
	void setRadiusScale(float radiusScale)
	{
		this->radiusScale = radiusScale;
	}
	void setElasticShearModulus(float elasticShearModulus)
	{
		this->elasticShearModulus = elasticShearModulus;
		computeOtherParams();
	}
	void setPoissonRatio(float poissonRatio)
	{
		this->poissonRatio = poissonRatio;
		computeOtherParams();
	}
	void setBrushMode(int brushMode)
	{
		this->brushMode = BRUSHMODE(brushMode);
	}

	void setMeshToManip(MString meshName)
	{
		this->meshToManip = meshName;
		MSelectionList slist;
		MGlobal::getSelectionListByName(meshToManip, slist);
		MDagPath dag;
		slist.getDagPath(0, dag);
		dag.extendToShape();

		fnMesh.setObject(dag);
		MGlobal::displayInfo("mesh:"+meshName);
	}

	void computeOtherParams()
	{
		a = 1 / (4 * M_PI * elasticShearModulus);
		b = a / (4 * (1 - poissonRatio));
		c = 2 / (3 * a - 2 * b);
	}

	//void doErase(short xpos, short ypos);


public:
	//float radius;//r
	float pressure;//|f|
	float radiusScale;//epsilon

	float elasticShearModulus;//miu
	float poissonRatio;//v

	float brushRadius;

	MString meshToManip;

	enum BRUSHMODE
	{
		k_GRAB,
		k_TWIST,
		k_SCALE,
		k_PINCH
	};

	BRUSHMODE brushMode;

	//other
	float a;//1/4/pi/elasticShearModulus
	float b;//a/4/(1-poissonRatio)
	float c;//2/(3a-2b)

	//test 
	MString result;

	MPoint lastScreenPos;
	MPoint currentScreenPos;

	M3dView view;
	MFnMesh fnMesh;

private:
	MVector getForce(const MPoint& startScreenPos, const MPoint& endScreenPos)
	{
		MPoint pt1, pt2;
		MVector v1, v2;
		view.viewToWorld(startScreenPos.x, startScreenPos.y, pt1, v1);
		view.viewToWorld(endScreenPos.x, endScreenPos.y, pt2, v2);

		return v2.normal() - v1.normal();
	}
};

