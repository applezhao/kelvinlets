#include "KevinLetBrushCtxCmd.h"



MStatus         KevinLetBrushCtxCmd::doEditFlags()
{
	MGlobal::displayInfo("doEditFlags!");
	MStatus status = MS::kSuccess;
	MArgParser argData = parser();

	if (argData.isFlagSet("-rs"))
	{
		double radiusScale;
		argData.getFlagArgument("-rs", 0, radiusScale);
		m_KevinLetBrushCtx->setRadiusScale(radiusScale);
	}

	if (argData.isFlagSet("-ps"))
	{
		double pressure;
		argData.getFlagArgument("-ps", 0, pressure);
		m_KevinLetBrushCtx->setPressure(pressure);
	}

	if (argData.isFlagSet("-esm"))
	{
		double elasticShearModulus;
		argData.getFlagArgument("-esm", 0, elasticShearModulus);
		m_KevinLetBrushCtx->setElasticShearModulus(elasticShearModulus);
	}

	if (argData.isFlagSet("-pr"))
	{
		double poissonRatio;
		argData.getFlagArgument("-pr", 0, poissonRatio);
		m_KevinLetBrushCtx->setPoissonRatio(poissonRatio);
	}

	if (argData.isFlagSet("-m"))
	{
		MString meshName;
		argData.getFlagArgument("-m", 0, meshName);
		m_KevinLetBrushCtx->setMeshToManip(meshName);
	}

	if (argData.isFlagSet("-bm"))
	{
		MString brushMode;
		argData.getFlagArgument("-bm", 0, brushMode);
		if (brushMode == "Grab")
			m_KevinLetBrushCtx->setBrushMode(KevinLetBrushCtx::k_GRAB);
		else if (brushMode == "Twist")
			m_KevinLetBrushCtx->setBrushMode(KevinLetBrushCtx::k_TWIST);
		else if (brushMode == "Scale")
			m_KevinLetBrushCtx->setBrushMode(KevinLetBrushCtx::k_SCALE);
		else if (brushMode == "Pinch")
			m_KevinLetBrushCtx->setBrushMode(KevinLetBrushCtx::k_PINCH);
		
	}

	return MS::kSuccess;
}
MStatus         KevinLetBrushCtxCmd::doQueryFlags()
{
	MArgParser argData = parser();
	if (argData.isFlagSet("-rs")) {
		setResult(m_KevinLetBrushCtx->radiusScale);
	}

	if (argData.isFlagSet("-ps")) {
		setResult(m_KevinLetBrushCtx->pressure);
	}

	if (argData.isFlagSet("-esm")) {
		setResult(m_KevinLetBrushCtx->elasticShearModulus);
	}

	if (argData.isFlagSet("-pr")) {
		setResult(m_KevinLetBrushCtx->poissonRatio);
	}

	if (argData.isFlagSet("-m")) {
		setResult(m_KevinLetBrushCtx->meshToManip);
	}

	if (argData.isFlagSet("-bm")) {
		if(m_KevinLetBrushCtx->brushMode == KevinLetBrushCtx::k_GRAB)
			setResult("Grab");
		else if (m_KevinLetBrushCtx->brushMode == KevinLetBrushCtx::k_TWIST)
			setResult("Twist");
		else if (m_KevinLetBrushCtx->brushMode == KevinLetBrushCtx::k_SCALE)
			setResult("Scale");
		else if (m_KevinLetBrushCtx->brushMode == KevinLetBrushCtx::k_PINCH)
			setResult("Pinch");
	}

	return MS::kSuccess;
}
MStatus         KevinLetBrushCtxCmd::appendSyntax()
{
	MSyntax mySyntax = syntax();

	mySyntax.addFlag("-rs", "-radiusScale", MSyntax::kDouble);
	mySyntax.addFlag("-ps", "-pressure", MSyntax::kDouble);
	mySyntax.addFlag("-esm", "-elasticShearModulus", MSyntax::kDouble);
	mySyntax.addFlag("-pr", "-poissonRatio", MSyntax::kDouble);
	mySyntax.addFlag("-m", "-meshToManipulate", MSyntax::kString);
	mySyntax.addFlag("-bm", "-brushMode", MSyntax::kString);
	return MS::kSuccess;
}
