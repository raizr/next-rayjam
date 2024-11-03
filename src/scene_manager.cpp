#include "scene_manager.h"

#include <string>
#include <cassert>
#include <LDtkLoader/Project.hpp>
#include <LDtkLoader/World.hpp>
#include "core.h"
#include "raymath.h"
#include "resource.h"
#include "utils.h"
#include "car.h"

using namespace scene;
using namespace std::string_literals;

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

SceneManager* SceneManager::getInstance()
{
    if (!instance)
    {
        instance = new SceneManager();
    }
    return instance;
}

void SceneManager::cleanup()
{
    delete instance;
}

SceneManager::SceneManager()
{
    std::string dir = GetWorkingDirectory();
    ldtkProject.loadFromFile((dir + "/test2.ldtk"s).c_str());
	ldtkWorld = &ldtkProject.getWorld();
	createB2World();
}

void SceneManager::createB2World()
{
	//b2SetLengthUnitsPerMeter(220.0f);

	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity.y = 9.8f * 50;
	worldId = b2CreateWorld(&worldDef);
}

Rectangle& SceneManager::ScreenInWorld()
{
    return screenInWorld;
}

bool SceneManager::IsLevelClear()
{
    return state == LevelState::PASSED;
}

void SceneManager::NextLevel()
{
    Reset();
	setLevel(currentLevel + 1);
    Load();
}

void scene::SceneManager::setLevel(int level)
{
	currentLevel = level;
}

void scene::SceneManager::MoveCar()
{
	m_car.SetSpeed(5.0f);
}

void SceneManager::Load()
{
	if (worldId) {
		createB2World();
	}

	currentLdtkLevel = &ldtkWorld->getLevel("Level_" + std::to_string(currentLevel));

	auto levelSize = currentLdtkLevel->size;
	auto renderTexture = LoadRenderTexture(levelSize.x, levelSize.y);

	BeginTextureMode(renderTexture);

	std::string dir = GetWorkingDirectory();
	if (currentLdtkLevel->hasBgImage())
	{
		auto backgroundPath = currentLdtkLevel->getBgImage();
		auto backgroundTexture = LoadTexture((dir + "/"s + backgroundPath.path.filename()).c_str());
		SetTextureFilter(backgroundTexture, TEXTURE_FILTER_TRILINEAR);

		DrawTextureV(backgroundTexture, { }, WHITE);
	}

	auto& layers = currentLdtkLevel->allLayers();
	auto begin = std::make_reverse_iterator(layers.end());
	auto end = std::make_reverse_iterator(layers.begin());
	for (; begin != end; begin++)
	{
		auto& layer = *begin;
		if (layer.hasTileset())
		{
			currentTilesetTexture = LoadTexture((dir + "/"s + layer.getTileset().path).c_str());
			for (auto&& tile : layer.allTiles())
			{
				auto source_pos = tile.getTextureRect();
				auto tile_size = float(layer.getTileset().tile_size);

				Rectangle source_rect = {
					float(source_pos.x),
					float(source_pos.y),
					tile.flipX ? -tile_size : tile_size,
					tile.flipY ? -tile_size : tile_size,
				};

				Vector2 target_pos = {
					(float)tile.getPosition().x,
					(float)tile.getPosition().y,
				};

				DrawTextureRec(currentTilesetTexture, source_rect, target_pos, WHITE);
			}
		}
	}

	EndTextureMode();
	renderedLevelTexture = renderTexture.texture;

	for (auto&& entity : currentLdtkLevel->getLayer("Entities").allEntities())
	{

		if (entity.getName() == "Car") {
			float m_speed = 35.0f;
			float m_torque = 50000.0f;
			float m_hertz = 25.0f;
			float m_dampingRatio = 0.7f;
			m_car.Spawn(worldId.value(), { static_cast<float>(entity.getPosition().x),
				static_cast<float>(entity.getPosition().y) }, 10.0f, m_hertz, m_dampingRatio, m_torque);
		}
		else if (entity.getName() == "Static") {
			auto b2width = entity.getSize().x / 2.0f;
			auto b2height = entity.getSize().y / 2.0f;

			auto centerX = entity.getPosition().x + b2width;
			auto centerY = entity.getPosition().y + b2height;

			b2BodyDef bodyDef = b2DefaultBodyDef();
			bodyDef.position = { centerX, centerY };

			b2BodyId body = b2CreateBody(worldId.value(), &bodyDef);

			b2BodyId groundId = b2CreateBody(worldId.value(), &bodyDef);

			b2Polygon groundBox = b2MakeBox(b2width, b2height);
			b2ShapeDef groundShapeDef = b2DefaultShapeDef();
			b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
			Entity localEntity;
			localEntity.pos = { static_cast<float>(entity.getPosition().x), static_cast<float>(entity.getPosition().y) };
			localEntity.bodyId = body;
			localEntity.extent = { static_cast<float>(entity.getSize().x), static_cast<float>(entity.getSize().y) };

			groundEntities.emplace_back(localEntity);
		}
		else if (entity.getName() == "Node") {
			auto b2width = entity.getSize().x / 2.0f;
			auto b2height = entity.getSize().y / 2.0f;

			auto centerX = entity.getPosition().x + b2width;
			auto centerY = entity.getPosition().y + b2height;

			b2BodyDef bodyDef = b2DefaultBodyDef();
			bodyDef.position = { centerX, centerY };

			b2BodyId body = b2CreateBody(worldId.value(), &bodyDef);

			b2BodyId groundId = b2CreateBody(worldId.value(), &bodyDef);

			b2Polygon groundBox = b2MakeBox(b2width, b2height);
			b2ShapeDef groundShapeDef = b2DefaultShapeDef();
			b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
			Entity localEntity;
			localEntity.pos = { static_cast<float>(entity.getPosition().x), static_cast<float>(entity.getPosition().y) };
			localEntity.bodyId = body;
			localEntity.extent = { static_cast<float>(entity.getSize().x), static_cast<float>(entity.getSize().y) };

			nodeEntities.emplace_back(localEntity);
		}
		else if (entity.getName() == "Passed") {
			passedEntity.pos = { static_cast<float>(entity.getPosition().x), static_cast<float>(entity.getPosition().y) };
			passedEntity.extent = { static_cast<float>(entity.getSize().x), static_cast<float>(entity.getSize().y) };
		}
		else if (entity.getName() == "Lose") {
			loseEntity.pos = { static_cast<float>(entity.getPosition().x), static_cast<float>(entity.getPosition().y) };
			loseEntity.extent = { static_cast<float>(entity.getSize().x), static_cast<float>(entity.getSize().y) };
		}
	}
	state = LevelState::PLAYING;
}

void SceneManager::Update()
{
    float deltaTime = GetFrameTime();
    seconds += deltaTime;
	if (worldId) {
		b2World_Step(worldId.value(), 0.016f, 4);
	}
	checkCollisions();
}

void scene::SceneManager::checkCollisions()
{
	mousePosition = GetMousePosition();
	float scale = MIN(core::gameScreenWidth / (float)GetScreenWidth(), core::gameScreenHeight / (float)GetScreenHeight());
	mousePosition = Vector2Scale(mousePosition, scale);
	if (focusNode && !checkEntityCollision(focusNode, mousePosition)) {
		focusNode = nullptr;
	}
	for (auto& node : nodeEntities) {
		if (checkEntityCollision(&node, mousePosition))
		{
			focusNode = &node;
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				if (selectedNode && focusNode != selectedNode) {
					AddJoint(selectedNode, focusNode);
					selectedNode = nullptr;
					focusNode = nullptr;
					break;
				}

				selectedNode = &node;
				focusNode = nullptr;
			}
			break;
		}

	}
	if (!selectedNode) {
	}
	else {
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			selectedNode = nullptr;
			focusNode = nullptr;
		}
	}

	if (CheckCollisionPointRec(m_car.GetPosition(), Rectangle {
				passedEntity.pos.x,
				passedEntity.pos.y,
				passedEntity.extent.x,
				passedEntity.extent.y
		}) && state != LevelState::PASSED) {
		state = LevelState::PASSED;
		m_car.SetSpeed(0.0f);
	}

	if (CheckCollisionPointRec(m_car.GetPosition(), Rectangle{
				loseEntity.pos.x,
				loseEntity.pos.y,
				loseEntity.extent.x,
				loseEntity.extent.y
		}) && state != LevelState::LOSE) {
		state = LevelState::LOSE;
		m_car.SetSpeed(0.0f);
		core::Core::getInstance()->OnLose();
	}
}

bool SceneManager::checkEntityCollision(Entity* entity, Vector2 point)
{
	constexpr auto MOUSE_SCALE_MARK_SIZE = 12;
	return CheckCollisionPointRec(point, Rectangle {
			entity->pos.x - MOUSE_SCALE_MARK_SIZE,
			entity->pos.y - MOUSE_SCALE_MARK_SIZE,
			entity->extent.x + MOUSE_SCALE_MARK_SIZE,
			entity->extent.y + MOUSE_SCALE_MARK_SIZE
		});
}

void SceneManager::Draw()
{
    Vector2 center = Vector2Scale({ (float)core::gameScreenWidth , (float)core::gameScreenHeight }, 1.0f);
	worldCamera.offset = {};
    worldCamera.target = {};
    worldCamera.zoom = 1.0f;
    float screenSize[2] = { (float)core::gameScreenWidth, (float)core::gameScreenHeight };

    BeginMode2D(worldCamera);
        Rectangle screen = { 0, 0,  center.x * 2,  center.y * 2 };
        Vector2 screenOriginInWorld = GetScreenToWorld2D(Vector2Zero(), worldCamera);
        Vector2 screenEdgeInWorld = GetScreenToWorld2D(Vector2{ screen.width, screen.height }, worldCamera);
        screenInWorld = Rectangle{ screenOriginInWorld.x, screenOriginInWorld.y, screenEdgeInWorld.x - screenOriginInWorld.x,screenEdgeInWorld.y - screenOriginInWorld.y };

		DrawTextureRec(renderedLevelTexture,
			{ 0, 0, (float)renderedLevelTexture.width, (float)-renderedLevelTexture.height },
			{ 0, 0 }, WHITE);


		if (selectedNode) {
			DrawEntity(*selectedNode, R_D_BLUE);
			DrawLineEx(Vector2 {
				selectedNode->pos.x + selectedNode->extent.x / 2.0f,
				selectedNode->pos.y + selectedNode->extent.y / 2.0f
			}, Vector2{
				mousePosition.x + selectedNode->extent.x / 2.0f,
				mousePosition.y + selectedNode->extent.y / 2.0f
			}, 5.0f, R_RED);
		}
		for (auto& entity : nodeEntities) {
			DrawEntity(entity, R_DDBLUE);
		}

		if (focusNode) {
			DrawEntity(*focusNode, R_RED);
		}

#ifdef _DEBUG
		/*for (auto& entity : groundEntities) {
			DrawEntity(entity, WHITE);
		}
		for (auto& entity : nodeEntities) {
			DrawEntity(entity, RED);
		}
		for (auto& entity : jointBodyEntities) {
			DrawEntity(entity, RED);
			DrawRectangleLines(entity.pos.x, entity.pos.y, entity.extent.x, entity.extent.y, RED);
		}*/
#endif
	m_car.Draw();
    EndMode2D();
}

void SceneManager::DrawEntity(const Entity& entity, Color color)
{
    b2Vec2 p = b2Body_GetWorldPoint(entity.bodyId, b2Vec2{ -entity.extent.x / 2.0f, -entity.extent.y / 2.0f });
    b2Rot rotation = b2Body_GetRotation(entity.bodyId);
    float radians = b2Rot_GetAngle(rotation);
	Vector2 ps = { p.x, p.y };
	DrawRectanglePro(Rectangle { ps.x, ps.y, entity.extent.x, entity.extent.y }, { 0.0f, 0.0f }, RAD2DEG* radians, color);
	p = b2Body_GetWorldPoint(entity.bodyId, b2Vec2{0.0f, 0.0f});
	ps = { p.x, p.y };
	DrawCircleV(ps, 2.0f, BLUE);
	DrawRectangleLines(entity.pos.x, entity.pos.y, entity.extent.x, entity.extent.y, color);
}

void SceneManager::DrawJoint(const Joint& joint)
{
	DrawLineEx(Vector2{
				joint.nodeA->pos.x + joint.nodeA->extent.x / 2.0f,
				joint.nodeA->pos.y + joint.nodeA->extent.y / 2.0f
		}, Vector2{
			joint.nodeB->pos.x + joint.nodeB->extent.x / 2.0f,
			joint.nodeB->pos.y + joint.nodeB->extent.y / 2.0f
		}, 5.0f, R_RED);
}

void SceneManager::AddJoint(Entity* entityA, Entity* entityB)
{
	auto width = Vector2Distance(entityA->pos, entityB->pos);
	b2Polygon box = b2MakeBox(width / 2.0f - 10.0f, 10.0f);
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 0.1f;
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { entityA->pos.x + width / 2.0f + 5.0f, entityA->pos.y + 10.0f };
	bodyDef.enableSleep = false;
	auto bodyId = b2CreateBody(worldId.value(), &bodyDef);
	b2CreatePolygonShape(bodyId, &shapeDef, &box);
	Entity localEntity;
	localEntity.pos = entityA->pos;
	localEntity.bodyId = bodyId;
	localEntity.extent = { width, 5.0f };
	
	auto jointDef = b2DefaultWeldJointDef();
	b2Vec2 pivot = { 1.0f, 0.0f };
	jointDef.bodyIdA = entityA->bodyId;
	jointDef.bodyIdB = bodyId;
	jointDef.localAnchorA = b2Body_GetLocalPoint(jointDef.bodyIdA, pivot);
	jointDef.localAnchorB = b2Body_GetLocalPoint(jointDef.bodyIdB, pivot);
	jointDef.angularHertz = 20.0f;
	jointDef.angularDampingRatio = 10.0f;
	jointDef.linearHertz = 25.0f;
	jointDef.linearDampingRatio = 1.0f;
	auto id = b2CreateWeldJoint(worldId.value(), &jointDef);
	Joint joint { entityA, &localEntity, id };

	pivot = { 0.0f, 0.0f };
	jointDef.bodyIdA = bodyId;
	jointDef.bodyIdB = entityB->bodyId;
	jointDef.localAnchorA = b2Body_GetLocalPoint(jointDef.bodyIdA, pivot);
	jointDef.localAnchorB = b2Body_GetLocalPoint(jointDef.bodyIdB, pivot);
	jointDef.angularHertz = 20.0f;
	jointDef.angularDampingRatio = 10.0f;
	jointDef.linearHertz = 25.0f;
	jointDef.linearDampingRatio = 1.0f;
	id = b2CreateWeldJoint(worldId.value(), &jointDef);
	Joint joint2{ &localEntity, entityB, id };

	jointEntities.emplace_back(joint);
	jointEntities.emplace_back(joint2);
	nodeEntities.emplace_back(localEntity);

}

void SceneManager::Reset()
{
	m_car.Despawn();
	b2DestroyWorld(worldId.value());
	groundEntities.clear();
	nodeEntities.clear();
	jointEntities.clear();
	jointBodyEntities.clear();
	UnloadTexture(currentTilesetTexture);
	passedEntity = {};
	loseEntity = {};
}