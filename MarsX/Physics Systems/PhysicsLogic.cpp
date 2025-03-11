#include "PhysicsLogic.h"
#include "../Components/Physics.h"
#include "../Components/Identification.h"
#include "../Components/Gameplay.h"
#include "../Events/GameStates.h"

bool MX::PhysicsLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator& _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	eventPusher = _eventPusher;

	onPaused.Create([&](const GW::GEvent e) {
		MX::PAUSE_STATE currState; MX::PAUSE_STATE_DATA pauseData;
		if (+e.Read(currState, pauseData) && currState == MX::PAUSE_STATE::PAUSED || currState == MX::PAUSE_STATE::UNPAUSED)
		{
			isPaused = pauseData.isPaused;
		}
		});
	eventPusher.Register(onPaused);

	onGameStart.Create([&](const GW::GEvent e) {
		MX::MENU_STATE currState; MX::MENU_STATE_DATA menuData;
		if (+e.Read(currState, menuData) && currState == MX::MENU_STATE::MAINMENU)
		{
			isPaused = menuData.mainMenu;
		}
		});
	eventPusher.Register(onGameStart);

	// **** MOVEMENT ****
	// update velocity by acceleration
	game->system<RigidBody::Velocity, const RigidBody::Acceleration>("Acceleration System")
		.each([this](flecs::entity e, RigidBody::Velocity& v, const RigidBody::Acceleration& a) {
		if (!isPaused)
		{
			GW::MATH::GVECTORF accel;
			GW::MATH::GVector::ScaleF(a.value, e.delta_time(), accel);
			GW::MATH::GVector::AddVectorF(accel, v.value, v.value);
		}
			});
	// update position by velocity
	game->system<Transform::Position, const RigidBody::Velocity>("Translation System")
		.each([this](flecs::entity e, Transform::Position& p, const RigidBody::Velocity& v) {
		if (!isPaused)
		{
			GW::MATH::GVECTORF speed;
			GW::MATH::GVector::ScaleF(v.value, e.delta_time(), speed);
			// adding is simple but doesn't account for orientation
			GW::MATH::GVector::AddVectorF(speed, p.value, p.value);
		}
			});
	// **** CLEANUP ****
	// clean up any objects that end up offscreen

	game->system<const Transform::Position>("Cleanup System")
		.each([this](flecs::entity e, const Transform::Position& p) {
		if (!isPaused)
		{
			if (p.value.z > 1255.0f || p.value.z < -1255.0f ||
				p.value.y > 180.f || p.value.y < 5.0f) {

				e.destruct();
#if _DEBUG
				std::cout << "Crashed in the Mountains (Boundary Collision)" << '\n';
#endif
			}
		}
			});
	// **** COLLISIONS ****
	// due to wanting to loop through all collidables at once, we do this in two steps:
	// 1. A System will gather all collidables into a shared std::vector
	// 2. A second system will run after, testing/resolving all collidables against each other

	queryCache = game->query<Collidable, Transform::Position, Transform::Orientation, BlenderInformation::BlenderName, BlenderInformation::ModelBoundary>();
	// only happens once per frame at the very start of the frame
	struct CollisionSystem {}; // local definition so we control iteration count (singular)
	game->entity("Detect-Collisions").add<CollisionSystem>();
	game->system<CollisionSystem>()
		.each([this](CollisionSystem& s) {
		// This the base shape all objects use & draw, this might normally be a component collider.(ex:sphere/box)
		// collect any and all collidable objects
		if (!isPaused)
		{
			queryCache.each([this](flecs::entity e, Collidable& c, Transform::Position& p, Transform::Orientation& o, BlenderInformation::BlenderName& _blendName, BlenderInformation::ModelBoundary& col) {


				SHAPE polygon; // compute buffer for this objects polygon
				// This is critical, if you want to store an entity handle it must be mutable
				polygon.owner = e; // allows later changes
				polygon.nameOfEntity = _blendName.name;
				polygon.collider.extent = col.obb.extent;
				polygon.collider.rotation = col.obb.rotation;
				polygon.collider.center = col.obb.center;
				// add to vector
				testCache.push_back(polygon);
				});
			// loop through the testCahe resolving all collisions
			for (int i = 0; i < testCache.size(); ++i) {
				// the inner loop starts at the entity after you so you don't double check collisions
				for (int j = i + 1; j < testCache.size(); ++j) {


					// test the two world space polygons for collision
					// possibly make this cheaper by leaving one of them local and using an inverse matrix
					GW::MATH::GCollision::GCollisionCheck result;
					GW::MATH::GOBBF left = testCache[i].collider;
					GW::MATH::GOBBF right = testCache[j].collider;
					if (testCache[i].owner.has<Bullet>())
					{
						left.extent = { left.extent.x, left.extent.y, 10 };
					}
					else if (testCache[j].owner.has<Bullet>())
					{
						right.extent = { right.extent.x, right.extent.y, 10 };
					}

					GW::MATH::GVECTORF left_position = testCache[i].owner.get < Transform::Position>()->value;
					GW::MATH::GVECTORF right_position = testCache[j].owner.get < Transform::Position>()->value;
					GW::MATH::GMATRIXF left_matrix = testCache[i].owner.get<Transform::Orientation>()->value;
					GW::MATH::GMATRIXF right_matrix = testCache[j].owner.get<Transform::Orientation>()->value;
					left_matrix.row4 = left_position;
					right_matrix.row4 = right_position;
					left_matrix.row4.x = right_matrix.row4.x = 0.0f;
					GW::MATH::GVector::VectorXMatrixF(left.center, left_matrix, left.center);
					GW::MATH::GVector::VectorXMatrixF(right.center, right_matrix, right.center);



					//GW::MATH::GCollision::TestOBBToOBBF(
					//	testCache[i].collider, testCache[j].collider, result);
					GW::MATH::GCollision::TestOBBToOBBF(
						left, right, result);
					if (result == GW::MATH::GCollision::GCollisionCheck::COLLISION) {
						// Create an ECS relationship between the colliders
						// Each system can decide how to respond to this info independently

						//if (testCache[j].owner.has<Player>())
						//	std::cout << "Player" << "\n";
						//if (testCache[j].owner.has<StaticObjects>())
						//	std::cout << "StaticObjects" << "\n";
						//else if (testCache[j].owner.has<Enemy>())
						//	std::cout << "Enemy" << "\n";
						//else if (testCache[j].owner.has<Bullet>())
						//	std::cout << "Bullet" << "\n";
						//else if (testCache[j].owner.has<Pickup>())
						//	std::cout << "Pickup" << "\n";

						//if (testCache[i].owner.has<Player>())
						//	std::cout << "Player" << "\n";
						//if (testCache[i].owner.has<StaticObjects>())
						//	std::cout << "StaticObjects" << "\n";
						//else if (testCache[i].owner.has<Enemy>())
						//	std::cout << "Enemy" << "\n";
						//else if (testCache[i].owner.has<Bullet>())
						//	std::cout << "Bullet" << "\n";
						//else if (testCache[i].owner.has<Pickup>())
						//	std::cout << "Pickup" << "\n";

						//if (testCache[i].owner.has<Player>() && testCache[j].owner.has<Enemy>())
						//	std::cout << "Player has Collided with Enemy!" << '\n';

						testCache[j].owner.add<CollidedWith>(testCache[i].owner);
						testCache[i].owner.add<CollidedWith>(testCache[j].owner);
					}
					else if (result == GW::MATH::GCollision::GCollisionCheck::NO_COLLISION)
					{

						if (testCache[j].owner.has<CollidedWith>(testCache[i].owner))
							testCache[j].owner.remove<CollidedWith>(testCache[i].owner);
						else if (testCache[i].owner.has<CollidedWith>(testCache[j].owner))
							testCache[i].owner.remove<CollidedWith>(testCache[j].owner);
					}
					// Create an ECS relationship between the collide)
				}
			}
		}
		// wipe the test cache for the next frame (keeps capacity intact)
		testCache.clear();
			});
	return true;
}

bool MX::PhysicsLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Acceleration System").enable();
		game->entity("Translation System").enable();
		game->entity("Cleanup System").enable();
	}
	else {
		game->entity("Acceleration System").disable();
		game->entity("Translation System").disable();
		game->entity("Cleanup System").disable();
	}
	return true;
}

bool MX::PhysicsLogic::Shutdown()
{
	queryCache.destruct(); // fixes crash on shutdown
	game->entity("Acceleration System").destruct();
	game->entity("Translation System").destruct();
	game->entity("Cleanup System").destruct();
	return true;
}
