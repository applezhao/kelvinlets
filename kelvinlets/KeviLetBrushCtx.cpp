#include "KeviLetBrushCtx.h"
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <Eigen>
#include <maya/MFnCamera.h>

MStatus KevinLetBrushCtx::doPress(MEvent & event)
{
	MGlobal::displayInfo("doPress!");
	short x, y;
	event.getPosition(x, y);
	currentScreenPos = MPoint(x, y);
	lastScreenPos = currentScreenPos;
	MPoint raySource, rayEnd;
	view.viewToWorld(x, y, raySource, rayEnd);
	pressRay = rayEnd - raySource;
	pressRay.normalize();
	MDagPath cameraDag;
	view.getCamera(cameraDag);
	MFnCamera fnCam(cameraDag);
	cameraPos = fnCam.eyePoint(MSpace::kWorld);
	/*int hitFace;
	bool hitted = fnMesh.closestIntersection(raySource, rayEnd - raySource, NULL, NULL, false, MSpace::kWorld, 10, false, NULL, lastPointOnMesh, NULL, &hitFace, NULL, NULL, NULL);
	if (hitted == false)
		MGlobal::displayError("no hit point!");*/
	return MS::kSuccess;
}
MStatus KevinLetBrushCtx::doDrag(MEvent & event)
{
	MGlobal::displayInfo("doDrag!");
	short x, y;
	event.getPosition(x, y);
	//lastScreenPos = currentScreenPos;
	currentScreenPos = MPoint(x, y);


	MVector f = getForce(lastScreenPos, currentScreenPos);
	f.normalize();
	Eigen::Vector3f f_e(f.x, f.y, f.z);
	MString print, temp;
	temp.set(f.x);
	print += temp;
	print += ",";
	temp.set(f.y);
	print += temp;
	print += ",";
	temp.set(f.z);
	print += temp;
	MGlobal::displayInfo("force:" + print);

	MPoint tp;
	MVector q;
	view.viewToWorld(lastScreenPos.x, lastScreenPos.y, tp, q);
	q.normalize();
	if (currentScreenPos.x < lastScreenPos.x)
		q = -q;
	Eigen::Vector3f q_e(q.x, q.y, q.z);

	Eigen::Matrix3f pinchF;
	pinchF(0, 0) = 0;
	pinchF(1, 1) = 0;
	pinchF(2, 2) = 0;
	pinchF(0, 1) = pinchF(1, 0) = 1;
	pinchF(0, 2) = pinchF(2, 0) = 1;
	pinchF(1, 2) = pinchF(2, 1) = 1;
	Eigen::Matrix3f I;
	I=I.Identity();


	MFloatPointArray allPoints;
	fnMesh.getPoints(allPoints, MSpace::kObject);
	MPointArray newPoints;
	for (size_t i = 0; i < allPoints.length(); i++)
	{
		/*short x, y;
		view.worldToView(allPoints[i], x, y);
		MPoint screenPos(x, y);*/
		//if (screenPos.distanceTo(lastScreenPos) <= brushRadius)
		{
			float proj = MVector(allPoints[i] - cameraPos)*pressRay;
			MVector vr = MVector(allPoints[i] - cameraPos) - pressRay*proj;
			//MVector vr = MVector(allPoints[i] - cameraPos)*pressRay - lastPointOnMesh;
			Eigen::Vector3f r_e(vr.x, vr.y, vr.z);
			float r = vr.length();
			float re = sqrt(r*r + radiusScale*radiusScale);

			MVector dis;
			if (brushMode == k_GRAB)
			{
				float u = (a - b) / re + a*radiusScale*radiusScale / (2 * re*re*re) + b*r*r / (re*re*re);
				dis = f*u*pressure;
			}
			else if (brushMode == k_TWIST)
			{
				float u = -a*(1 / (re*re*re) + 3 * radiusScale*radiusScale / (2 * re*re*re*re*re));
				
				
				Eigen::Vector3f qr = q_e.cross(r_e);
				Eigen::Vector3f dis_e = qr*u*pressure;
				dis = MVector(dis_e.x(), dis_e.y(), dis_e.z());
			}
			else if (brushMode == k_SCALE)
			{
				float u = (2 * b - a)*(1 / (re*re*re) + 3 * radiusScale*radiusScale / (2 * re*re*re*re*re));

				dis = -vr*u*pressure;
			}
			else if (brushMode == k_PINCH)
			{
				Eigen::Vector3f v1 = pinchF*r_e;
				Eigen::Vector3f v2 = r_e.transpose()*pinchF*r_e*I*r_e;
				MVector v1_v(v1.x(), v1.y(), v1.z());
				MVector v2_v(v2.x(), v2.y(), v2.z());
				dis = v1_v*(2 * b - a) / (re*re*re) - v2_v * 3 * b / (re*re*re*re*re) - v1_v * 3 * a*radiusScale*radiusScale / (2 * re*re*re*re*re);
				dis = -dis*pressure;
			}
				
			newPoints.append(allPoints[i] + dis);
		}
		//else
		//	newPoints.append(allPoints[i]);
	}

	fnMesh.setPoints(newPoints);
	return MS::kSuccess;
}
MStatus KevinLetBrushCtx::doRelease(MEvent & event)
{
	MGlobal::displayInfo("doRelease!");
	view.refresh();
	return MS::kSuccess;
}