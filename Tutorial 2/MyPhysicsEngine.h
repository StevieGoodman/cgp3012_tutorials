#pragma once

#include "BasicActors.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	using namespace std;

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = {
		PxVec3(
			46.0f / 255.0f,
			9.0f  / 255.0f,
			39.0f / 255.0f),
		PxVec3(
			217.0f / 255.0f,
			0.0f   / 255.0f,
			0.0f   / 255.0f),
		PxVec3(
			255.0f / 255.0f,
			45.0f  / 255.0f,
			0.0f   / 255.0f),
		PxVec3(
			255.0f / 255.0f,
			140.0f / 255.0f,
			54.0f  / 255.0f),
		PxVec3(
			4.0f   / 255.0f,
			117.0f / 255.0f,
			111.0f / 255.0f)
	};

	//pyramid vertices
	static PxVec3 pyramid_verts[] = {
		PxVec3(0,1,0), 
		PxVec3(1,0,0), 
		PxVec3(-1,0,0),
		PxVec3(0,0,1), 
		PxVec3(0,0,-1)};
	//pyramid triangles: a list of three vertices for each triangle e.g. the first triangle consists of vertices 1, 4 and 0
	//vertices have to be specified in a counter-clockwise order to assure the correct shading in rendering
	static PxU32 pyramid_trigs[] = {
		1, 4, 0,
		3, 1, 0, 
		2, 3, 0, 
		4, 2, 0, 
		3, 2, 1, 
		2, 4, 1};

	class Pyramid : public ConvexMesh
	{
	public:
		Pyramid(PxTransform pose=PxTransform(PxIdentity), PxReal density=1.f) :
			ConvexMesh(vector<PxVec3>(begin(pyramid_verts),end(pyramid_verts)), pose, density)
		{
		}
	};

	class PyramidStatic : public TriangleMesh
	{
	public:
		PyramidStatic(PxTransform pose=PxTransform(PxIdentity)) :
			TriangleMesh(vector<PxVec3>(begin(pyramid_verts),end(pyramid_verts)), vector<PxU32>(begin(pyramid_trigs),end(pyramid_trigs)), pose)
		{
		}
	};

	///Custom scene class
	class MyScene : public Scene
	{
		Plane* plane;
		Box* box;

	public:
		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eBODY_LIN_VELOCITY, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit() 
		{
			SetVisualisation();			

			GetMaterial()->setDynamicFriction(.2f);

			plane = new Plane();
			plane->Color(PxVec3(210.f/255.f,210.f/255.f,210.f/255.f));
			Add(plane);

			Box* boxes[20]{};
			for (auto i = 0; i < 20; i++)
			{
				auto box = new Box(
					PxTransform{
						PxVec3{ i * 1.5f, 10.0f, 0.0f },
						PxQuat{ 0.25 * PxPi, PxVec3{ 0.0f, 1.0f, 1.0f }.getNormalized() }
					}
				);
				auto pose = box->GetShape()->getActor()->getGlobalPose();
				pose.q += PxQuat{ i * (PxPi), PxVec3{0, 1, 0} };
				box->GetShape()->getActor()->setGlobalPose(pose);
				boxes[i] = box;
				Add(box);
			}
		}

		//Custom update function
		virtual void CustomUpdate() 
		{
		}
	};
}
