#pragma once

#include "KevinLetBrush.h"
#include <maya/MPoint.h>
#include <maya/MCursor.h>
#include <maya/M3dView.h>
#include <maya/MFnMesh.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MFloatPoint.h>
#include <maya/MFnCamera.h>

#include <maya/MIOStream.h>
#include <math.h>
#include <stdlib.h>

#include <maya/MAngle.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>
#include <maya/M3dView.h>
#include <maya/MDagPath.h>
#include <maya/MArgList.h>
 
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>

#include <maya/MPxContext.h>
#include <maya/MPxToolCommand.h> 
#include <maya/MEvent.h>

#include <maya/MUiMessage.h>
#include <maya/MUIDrawManager.h>
#include <maya/MFrameContext.h>
#include <maya/MPoint.h>
#include <maya/MColor.h>

#include <GL/gl.h>
#include <GL/glu.h>

//COMPUTE
#include <maya/MFnDagNode.h>

#include <maya/MDagPathArray.h>
#include <maya/MDagPath.h>

#include <maya/MFnNurbsCurve.h>
#include <maya/MPointArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFnTransform.h>
#include <maya/MFnMesh.h>

#include <vector>

class KevinLetBrushCtx :public MPxContext
{
public:
	KevinLetBrushCtx(void)
	{
		pressure = 1.f;
		radiusScale_1 = 3.f;
		radiusScale_2 = radiusScale_1*2;
		radiusScale_3 = radiusScale_2*2;
		elasticShearModulus = 1;
		poissonRatio = 0.4;

		brushRadius = 50;

		brushMode = k_GRAB;
		is2D = true;

		setTitleString("KevinLetBrush");
		setCursor(MCursor::handCursor);
		setImage("KevinLetBrush.xpm", MPxContext::kImage1);

		computeOtherParams();

	}
	virtual ~KevinLetBrushCtx(void) 
	{
		for(size_t i=0; i<fnMesh.size(); i++)
			delete fnMesh[i];
	}

	virtual void toolOnSetup(MEvent & event) {}
	//virtual void toolOffCleanup();
	virtual MStatus doPress(MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context);
	virtual MStatus doDrag(MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context);
	virtual MStatus doRelease(MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context);
	virtual MStatus doEnterRegion(MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
	{
		MGlobal::displayInfo("doEnterRegion");
		view = M3dView::active3dView();
		return MS::kSuccess;
	}
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
	void setBrushRadius(double brushRadius)
	{
		this->brushRadius = brushRadius;
	}
	void setRadiusScale(float radiusScale, int index)
	{
		if ( index == 1 )
			radiusScale_1 = radiusScale;
		if ( index == 2 )
			radiusScale_2 = radiusScale;
		if ( index == 3 )
			radiusScale_3 = radiusScale;

		if ( this->radiusScale_2 < this->radiusScale_1 )
			this->radiusScale_2 = this->radiusScale_1*2;
		if ( this->radiusScale_3 < this->radiusScale_2 )
			this->radiusScale_3 = this->radiusScale_2*2;
	}
	// void setRadiusScale(float radiusScale_1, float radiusScale_2=-1, float radiusScale_3=-1)
	// {
	// 	this->radiusScale_1 = radiusScale_1;
	// 	this->radiusScale_2 = radiusScale_2;
	// 	this->radiusScale_3 = radiusScale_3;
	// 	if ( this->radiusScale_2 < this->radiusScale_1 )
	// 		this->radiusScale_2 = this->radiusScale_1*2;
	// 	if ( this->radiusScale_3 < this->radiusScale_2 )
	// 		this->radiusScale_3 = this->radiusScale_2*2;
	// }
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
	void setIs2DBrush(bool is2D)
	{
		this->is2D=is2D;
		cout<<"context2d:"<<this->is2D<<endl;
	}

	void setBrushMode(int brushMode)
	{
		this->brushMode = BRUSHMODE(brushMode);
		switch(brushMode)
		{
		case k_GRAB: 
			brushString="grab";
			break;
		case k_GRAB_BISCALE: 
			brushString="grab_biscale";
			break;
		case k_GRAB_TRISCALE: 
			brushString="grab_triscale";
			break;
		case k_TWIST:
			brushString="twist";
			break;
		case k_SCALE:
			brushString="scale";
			break;
		case k_PINCH:
			brushString="pinch";
			break;
		case k_BRUSHSIZE:
			brushString="size_adjust";
			break;
		}
	}

	void setMeshToManip(MStringArray meshNames)
	{
		this->meshToManip = meshNames;
		size_t mesh_num = meshNames.length();
		fnMesh.resize(mesh_num);
		for( size_t i=0; i<mesh_num; i++ )
		{
			MSelectionList slist;
			MGlobal::getSelectionListByName(meshToManip[i], slist);
			MGlobal::displayInfo(meshToManip[i]);
			MDagPath dag;
			slist.getDagPath(0, dag);
			dag.extendToShape();
			fnMesh[i]=new MFnMesh(dag);
			//fnMesh[i].setObject(dag);
		}
		

		
		//MGlobal::displayInfo("mesh:"+meshName);
	}

	void computeOtherParams()
	{
		a = 1 / (4 * M_PI * elasticShearModulus);
		b = a / (4 * (1 - poissonRatio));
		c = 2 / (3 * a - 2 * b);
	}


	void drawBrush(short mouse_x, short mouse_y, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
	{
		drawMgr.beginDrawable();
		drawMgr.setColor( MColor(1.0f, 0.0f, 0.0f) );
		drawMgr.circle2d( MPoint( mouse_x, mouse_y), brushRadius,false );
		drawMgr.setLineWidth(2.0);
		if( is2D )
			drawMgr.text2d( MPoint( mouse_x, mouse_y ), MString(brushString+"2D") );
		else
			drawMgr.text2d( MPoint( mouse_x, mouse_y ), MString(brushString+"3D") );
		drawMgr.setPointSize(2.0);
		drawMgr.point(x0);
		drawMgr.endDrawable();
		view.refresh();
	}

	//void doErase(short xpos, short ypos);


public:
	//float radius;//r
	float pressure;//|f|
	float radiusScale_1;//epsilon
	float radiusScale_2;//epsilon
	float radiusScale_3;//epsilon

	float elasticShearModulus;//miu
	float poissonRatio;//v

	double brushRadius;

	MStringArray meshToManip;

	enum BRUSHMODE
	{
		k_GRAB,
		k_GRAB_BISCALE,
		k_GRAB_TRISCALE,
		k_TWIST,
		k_SCALE,
		k_PINCH,
		k_BRUSHSIZE
	};

	BRUSHMODE brushMode;
	bool is2D;
	MString brushString;

	//other
	float a;//1/4/pi/elasticShearModulus
	float b;//a/4/(1-poissonRatio)
	float c;//2/(3a-2b)


	MPoint pressScreenPos;
	MPoint currentScreenPos;
	// MFloatPoint lastPointOnMesh;
	MVector pressRay;

	MFloatPoint x0;//concentracted part's center

	M3dView view;
	std::vector< MFnMesh* > fnMesh;

	MFloatPoint cameraPosition;
	bool isOrtho;
	// std::vector< MVectorArray* > xtox0;
	// std::vector< MVectorArray* > rpluse;

private:
	MVector getForce(const MPoint& startScreenPos, const MPoint& endScreenPos)
	{
		MPoint pt1_start, pt2_start, pt1_end, pt2_end;
		MVector v1, v2;
		view.viewToWorld((int)startScreenPos.x, (int)startScreenPos.y, pt1_start, pt2_start);
		view.viewToWorld((int)endScreenPos.x, (int)endScreenPos.y, pt1_end, pt2_end);

		return MVector(pt2_end - pt2_start).normal();
	}
};

