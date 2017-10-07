#include <Eigen> 
#include "KeviLetBrushCtx.h"
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MPoint.h>
#include <maya/MFloatVectorArray.h>


MStatus KevinLetBrushCtx::doPress(MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	MGlobal::displayInfo("doPress!");

	// eye_position
	// look_direction
	MDagPath cameraDag;
	view.getCamera(cameraDag);
	MFnCamera fnCam(cameraDag);
	cameraPosition = fnCam.eyePoint(MSpace::kWorld);
	MVector viewDir = fnCam.viewDirection(MSpace::kWorld);
	viewDir.normalize();
	isOrtho = fnCam.isOrtho();

	short x, y;
	event.getPosition(x, y);

	pressScreenPos = MPoint(x, y);

	MPoint raySource, rayEnd;
	view.viewToWorld(x, y, raySource, rayEnd);
	pressRay=rayEnd-raySource;
	pressRay.normalize();

	if (isOrtho)
		cout<<"ortho:"<<pressRay.x<<","<<pressRay.y<<","<<pressRay.z<<endl;
	else
		cout<<"persp:"<<pressRay.x<<","<<pressRay.y<<","<<pressRay.z<<endl;

	if(isOrtho && is2D)
		cameraPosition = raySource;
	int hitFace;
	bool hitted = false;
	float minHitParam = 100000;
	for(size_t i=0; i<fnMesh.size(); i++)
	{
		MFloatPoint hitPoint;
		float hitParam;
	 	hitted = fnMesh[i]->closestIntersection(MFloatPoint(raySource), MFloatVector(rayEnd - raySource), NULL, NULL, false, MSpace::kWorld, 2, false, NULL, hitPoint, &hitParam, &hitFace, NULL, NULL, NULL);
		if(hitted&&hitParam<minHitParam)
			x0 = hitPoint;
	}
	if (hitted == false)
	{
		int concentratedPointsCount=0;
		MPoint concentratedPointAverage(0,0,0);
		for(size_t i=0; i<fnMesh.size(); i++)
		{
			MFloatPointArray allPoints;
			fnMesh[i]->getPoints(allPoints, MSpace::kObject);
			MFloatVectorArray allNormals;
			fnMesh[i]->getNormals(allNormals, MSpace::kObject);
			for (size_t j = 0; j < allPoints.length(); j++)
			{
				if ( allNormals[i]*viewDir < 0 )
					continue;
				short x, y;
				//bool isVisable = 
				view.worldToView(allPoints[j], x, y);
				MPoint screenPos(x, y);
				if ( screenPos.distanceTo(pressScreenPos) <= brushRadius )
				{
					concentratedPointAverage=concentratedPointAverage+allPoints[j];
					concentratedPointsCount++;
				}
			}
			
		}
		x0 = concentratedPointAverage*(1.0/concentratedPointsCount);
	}
	

	drawBrush(x,y,drawMgr,context);
	return MS::kSuccess;
}
MStatus KevinLetBrushCtx::doDrag(MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	MGlobal::displayInfo("doDrag!");
	short x, y;
	event.getPosition(x, y);
	drawBrush(x,y,drawMgr,context);
	//lastScreenPos = currentScreenPos;
	currentScreenPos = MPoint(x, y);

	if ( brushMode == k_BRUSHSIZE )
	{
		if (currentScreenPos.x < pressScreenPos.x)
			brushRadius--;
		else
			brushRadius++;
		brushRadius=std::max(0.0, brushRadius);
		return MS::kSuccess;
	}


	//grab force
	MVector f = getForce(pressScreenPos, currentScreenPos);
	f.normalize();
	Eigen::Vector3f f_e(f.x, f.y, f.z);
	// MString print, temp;
	// temp.set(f.x);
	// print += temp;
	// print += ",";
	// temp.set(f.y);
	// print += temp;
	// print += ",";
	// temp.set(f.z);
	// print += temp;
	// MGlobal::displayInfo("force:" + print);
	// cout<<lastScreenPos.x<<","<<lastScreenPos.y<<","<<currentScreenPos.x<<","<<currentScreenPos.y<<endl;

	//twist force
	MPoint tp;
	MVector q;
	view.viewToWorld((int)pressScreenPos.x, (int)pressScreenPos.y, tp, q);
	q.normalize();
	if (currentScreenPos.x < pressScreenPos.x)
		q = -q;
	Eigen::Vector3f q_e(q.x, q.y, q.z);

	//scale direction
	float scaleDir=1;
	if (currentScreenPos.x < pressScreenPos.x)
		scaleDir=-1;

	//pinch matrix
	Eigen::Matrix3f pinchF;
	pinchF(0, 0) = 0;
	pinchF(1, 1) = 0;
	pinchF(2, 2) = 0;
	pinchF(0, 1) = 1;
	pinchF(1, 0) = 1;
	pinchF(0, 2) = 0;
	pinchF(2, 0) = 0;
	pinchF(1, 2) = 0;
	pinchF(2, 1) = 0;
	Eigen::Matrix3f I;
	I=I.Identity();


	for(size_t m=0; m<fnMesh.size(); m++)
	{
		MFloatPointArray allPoints;
		fnMesh[m]->getPoints(allPoints, MSpace::kObject);
		MPointArray newPoints;
		for (size_t i = 0; i < allPoints.length(); i++)
		{
			short x, y;
			view.worldToView(allPoints[i], x, y);
			MPoint screenPos(x, y);
			//if (screenPos.distanceTo(lastScreenPos) <= brushRadius)
			{
				MVector vr;
				if( !is2D ) 
					vr = allPoints[i] - x0;
				else
				{
					if(!isOrtho)
					{
						float projOnRay = MVector(allPoints[i]-cameraPosition)*(pressRay);
						vr = (allPoints[i]-cameraPosition) - pressRay*projOnRay;
					}
					else
					{
						float projOnRay = MVector(allPoints[i]-cameraPosition)*(pressRay);
						vr = (allPoints[i]-cameraPosition) - pressRay*projOnRay;
					}
					
				}
				// float projOnRay = MVector(allPoints[i]-cameraPosition)*(lastPointHitRay);
				// MVector vr = (allPoints[i]-cameraPosition) - lastPointHitRay*projOnRay;
				Eigen::Vector3f r_e(vr.x, vr.y, vr.z);
				float r = vr.length();
				float re = sqrt(r*r + radiusScale_1*radiusScale_1);

				MVector dis;
				if (brushMode == k_GRAB)
				{
					float u = (a - b) / re + a*radiusScale_1*radiusScale_1 / (2 * re*re*re) + b*r*r / (re*re*re);
					u*=c;
					dis = f*u*pressure;
				}
				else if (brushMode == k_GRAB_BISCALE)
				{
					float re_1 = re;
					float re_2 = sqrt(r*r + radiusScale_2*radiusScale_2);
					float u_1 = (a - b) / re_1 + a*radiusScale_1*radiusScale_1 / (2 * re_1*re_1*re_1) + b*r*r / (re_1*re_1*re_1);
					float u_2 = (a - b) / re_2 + a*radiusScale_2*radiusScale_2 / (2 * re_2*re_2*re_2) + b*r*r / (re_2*re_2*re_2);
					float u = u_1 - u_2;
					u*=(c/(1.0/radiusScale_1-1/radiusScale_2));
					dis = f*u*pressure;
				}
				else if (brushMode == k_GRAB_TRISCALE)
				{
					float re_1 = re;
					float re_2 = sqrt(r*r + radiusScale_2*radiusScale_2);
					float re_3 = sqrt(r*r + radiusScale_3*radiusScale_3);
					float u_1 = (a - b) / re_1 + a*radiusScale_1*radiusScale_1 / (2 * re_1*re_1*re_1) + b*r*r / (re_1*re_1*re_1);
					float u_2 = (a - b) / re_2 + a*radiusScale_2*radiusScale_2 / (2 * re_2*re_2*re_2) + b*r*r / (re_2*re_2*re_2);
					float u_3 = (a - b) / re_3 + a*radiusScale_3*radiusScale_3 / (2 * re_3*re_3*re_3) + b*r*r / (re_3*re_3*re_3);
					float w1=1;
					float w2=-(radiusScale_3*radiusScale_3-radiusScale_1*radiusScale_1+0.0)/(radiusScale_3*radiusScale_3-radiusScale_2*radiusScale_2);
					float w3=(radiusScale_2*radiusScale_2-radiusScale_1*radiusScale_1+0.0)/(radiusScale_3*radiusScale_3-radiusScale_2*radiusScale_2);
					float u = u_1*w1+u_2*w2+u_3*w3;
					u*=(c/(w1/radiusScale_1+w2/radiusScale_2+w3/radiusScale_3));
					dis = f*u*pressure;
				}
				else if (brushMode == k_TWIST)
				{
					float u = -a*(1 / (re*re*re) + 3 * radiusScale_1*radiusScale_1 / (2 * re*re*re*re*re));
					
					u*=c;
					Eigen::Vector3f qr = q_e.cross(r_e);
					Eigen::Vector3f dis_e = qr*u*pressure;
					dis = MVector(dis_e.x(), dis_e.y(), dis_e.z());
				}
				else if (brushMode == k_SCALE)
				{
					float u = (2 * b - a)*(1 / (re*re*re) + 3 * radiusScale_1*radiusScale_1 / (2 * re*re*re*re*re));
					u*=(c*scaleDir);
					dis = -vr*u*pressure;
				}
				else if (brushMode == k_PINCH)
				{
					Eigen::Vector3f v1 = pinchF*r_e;
					Eigen::Vector3f ttt = (r_e.transpose()*pinchF).transpose();
					Eigen::Matrix3f ttt1 = ttt*r_e.transpose();
					Eigen::Vector3f v2 = ttt1*r_e;
					MVector v1_v(v1.x(), v1.y(), v1.z());
					MVector v2_v(v2.x(), v2.y(), v2.z());
					dis = v1_v*(2 * b - a) / (re*re*re) - v2_v * 3 * b / (re*re*re*re*re) - v1_v * 3 * a*radiusScale_1*radiusScale_1 / (2 * re*re*re*re*re);
					dis = -dis*pressure;
					dis*=c;
				}
					
				newPoints.append(allPoints[i] + dis);
			}
			//else
			//	newPoints.append(allPoints[i]);
		}

		fnMesh[m]->setPoints(newPoints);
	}
	return MS::kSuccess;
}
MStatus KevinLetBrushCtx::doRelease(MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	MGlobal::displayInfo("doRelease!");
	// short x, y;
	// event.getPosition(x, y);
	//drawBrush(x,y,drawMgr,context);
	//view.refresh();
	return MS::kSuccess;
}
