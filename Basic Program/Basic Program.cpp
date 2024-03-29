#include <iostream> //cout, cerr
#include <iomanip> //stream formatting
#include <windows.h> //delay function
#include <PxPhysicsAPI.h> //PhysX

using namespace std;
using namespace physx;

//PhysX objects
PxPhysics* physics = 0;
PxFoundation* foundation = 0;
#if PX_PHYSICS_VERSION < 0x304000 // SDK 3.3
debugger::comm::PvdConnection* pvd = 0;
#else
PxPvd*  pvd = 0;
#endif

//simulation objects
PxScene* scene;
PxRigidDynamic* droppedBox;
PxRigidStatic* restingBox;
PxRigidStatic* plane;

int time_elapsed{};
bool moved{ false };

///Initialise PhysX objects
bool PxInit()
{
	//default error and allocator callbacks
	static PxDefaultErrorCallback gDefaultErrorCallback;
	static PxDefaultAllocator gDefaultAllocatorCallback;

	//Init PhysX
	//foundation
#if PX_PHYSICS_VERSION < 0x304000 // SDK 3.3
	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
#else
	foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
#endif

	if(!foundation)
		return false;

	//connect to an external visual debugger (if exists)
#if PX_PHYSICS_VERSION < 0x304000 // SDK 3.3
	pvd = PxVisualDebuggerExt::createConnection(physics->getPvdConnectionManager(), "localhost", 5425, 100,
		PxVisualDebuggerExt::getAllConnectionFlags());
#else
	pvd = PxCreatePvd(*foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
	pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif

	//physics
#if PX_PHYSICS_VERSION < 0x304000 // SDK 3.3
	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale());
#else
	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true, pvd);
#endif

	if(!physics)
		return false;

	//create a default scene
	PxSceneDesc sceneDesc(physics->getTolerancesScale());

	if(!sceneDesc.cpuDispatcher)
	{
		PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
		sceneDesc.cpuDispatcher = mCpuDispatcher;
	}

	if(!sceneDesc.filterShader)
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	scene = physics->createScene(sceneDesc);

	if (!scene)
		return false;

	return true;
}

/// Release all allocated resources
void PxRelease()
{
	if (scene)
		scene->release();
	if (physics)
		physics->release();
	if (pvd)
		pvd->release();
	if (foundation)
		foundation->release();
}

///Initialise the scene
void InitScene()
{
	//default gravity
	scene->setGravity(PxVec3(0.f, -9.81f, 0.f));

	//materials
 	PxMaterial* default_material = physics->createMaterial(0.f, 0.2f, 0.f);   //static friction, dynamic friction, restitution

	//create a static plane (XZ)
	plane = PxCreatePlane(*physics, PxPlane(PxVec3(0.f, 1.f, 0.f), 0.f), *default_material);
	scene->addActor(*plane);

	// Create the dropped box
	droppedBox = physics->createRigidDynamic(PxTransform(PxVec3(0.f, 100.0f, 0.f)));
	droppedBox->createShape(PxBoxGeometry(.5f, .5f, .5f), *default_material);
	PxRigidBodyExt::updateMassAndInertia(*droppedBox, 1.f); //density of 1kg/m^3
	scene->addActor(*droppedBox);

	// Create the resting box
	restingBox = physics->createRigidStatic(PxTransform(PxVec3(0.f, .5f, 0.f)));
	restingBox->createShape(PxBoxGeometry(.5f, .5f, .5f), *default_material);
	scene->addActor(*restingBox);
}

/// Perform a single simulation step
void Update(PxReal delta_time, int steps)
{
	scene->simulate(delta_time);
	scene->fetchResults(true);
	time_elapsed += delta_time;
	if (time_elapsed > 10 && !moved) {
		moved = true;
		droppedBox->setLinearVelocity(PxVec3{ 0, 0, 0});
	}
}

/// The main function
int main()
{
	//initialise PhysX	
	if (!PxInit())
	{
		cerr << "Could not initialise PhysX." << endl;
		return 0;
	}

	//initialise the scene
	InitScene();

	//set the simulation step to 1/60th of a second
	PxReal delta_time = 1.f/60.f;

	int steps{};

	Sleep(2000);

	//simulate until the 'Esc' is pressed
	while (!GetAsyncKeyState(VK_ESCAPE))
	{
		steps++;
		//'visualise' position and velocity of the box
		PxVec3 position = droppedBox->getGlobalPose().p;
		PxVec3 velocity = droppedBox->getLinearVelocity();
		cout << setiosflags(ios::fixed) << setprecision(2) << "x=" << position.x << 
			", y=" << position.y << ", z=" << position.z << ",  ";
		cout << setiosflags(ios::fixed) << setprecision(2) << "vx=" << velocity.x << 
			", vy=" << velocity.y << ", vz=" << velocity.z << endl;
		cout << "isSleeping: " << droppedBox->isSleeping() << endl;
		//perform a single simulation step
		Update(delta_time, steps);

		//introduce 100ms delay for easier visual analysis of the results
		Sleep(50);
	}

	//Release all resources
	PxRelease();

	return 0;
}