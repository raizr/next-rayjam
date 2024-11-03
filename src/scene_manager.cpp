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
    ldtkProject.loadFromFile((dir + "/levels.ldtk"s).c_str());
	ldtkWorld = &ldtkProject.getWorld();
	maxLevels = static_cast<int>(ldtkWorld->allLevels().size());
	tutorialPos = {
		{ 512.0f, 204.0f },
		{ 507.0f, 207.0f },
		{ 700.0f, 59.0f }
	};
	for (auto i = 0; i < 3; i++) {
		Texture2D texture = LoadTexture((dir + "/tutorial" + std::to_string(i) + ".png"s).c_str());
		tutorials.emplace_back(texture);
	}
	createB2World();
}

void SceneManager::createB2World()
{
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

bool scene::SceneManager::IsLastLevel()
{
	return currentLevel >= maxLevels - 1;
}

void scene::SceneManager::setLevel(int level)
{
	currentLevel = level;
}

void scene::SceneManager::MoveCar()
{
	m_car.SetSpeed(150.0f);
	PlayMusicStream(Resources::effectCar);
}

void scene::SceneManager::SetTutotrialPassed()
{
	tutorialStep = 3;
	tutorialPassed = true;
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
			float m_torque = 90000.0f;
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
	UpdateMusicStream(Resources::effectCar);
    float deltaTime = GetFrameTime();
    seconds += deltaTime;
	if (worldId) {
		b2World_Step(worldId.value(), 0.016f, 4);
	}
	checkCollisions();
}

void scene::SceneManager::checkCollisions()
{
	checkNodesCollision();
	checkCarCollision();
}

void SceneManager::checkNodesCollision()
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
				PlaySound(Resources::effect4);
				if (selectedNode && focusNode != selectedNode) {
					AddJoint(selectedNode, focusNode);
					selectedNode = nullptr;
					focusNode = nullptr;
					if (!tutorialPassed && tutorialStep == 1) {
						tutorialStep = 2;
					}
					return;
				}

				selectedNode = &node;
				if (selectedNode == &(*nodeEntities.begin()) && !tutorialPassed && tutorialStep == 0) {
					tutorialStep = 1;
				}
				focusNode = nullptr;
			}
			return;
		}

	}
	if (!selectedNode) {
	}
	else {
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			selectedNode = nullptr;
			focusNode = nullptr;
			if (!tutorialPassed && tutorialStep == 1) {
				tutorialStep = 0;
			}
		}
		/*if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			addNode(mousePosition);
			AddJoint(selectedNode, focusNode);
			selectedNode = nullptr;
			focusNode = nullptr;
		}*/
	}
}

void scene::SceneManager::checkCarCollision()
{
	if (CheckCollisionPointRec(m_car.GetPosition(), Rectangle{
				passedEntity.pos.x,
				passedEntity.pos.y,
				passedEntity.extent.x,
				passedEntity.extent.y
		}) && state != LevelState::PASSED) {
		state = LevelState::PASSED;
		m_car.SetSpeed(0.0f);
		StopMusicStream(Resources::effectCar);
	}

	if (CheckCollisionPointRec(m_car.GetPosition(), Rectangle{
				loseEntity.pos.x,
				loseEntity.pos.y,
				loseEntity.extent.x,
				loseEntity.extent.y
		}) && state != LevelState::LOSE) {
		state = LevelState::LOSE;
		m_car.SetSpeed(0.0f);
		StopMusicStream(Resources::effectCar);
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
		if (!tutorialPassed) {
			DrawTexture(tutorials[tutorialStep], tutorialPos[tutorialStep].x, tutorialPos[tutorialStep].y, WHITE);
		}
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
		for (auto& entity : jointBodyEntities) {
			DrawJointBodies(entity, R_DDBLUE);
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
	if (entity.bodyId) {
		b2Vec2 p = b2Body_GetWorldPoint(entity.bodyId.value(), b2Vec2{-entity.extent.x / 2.0f, -entity.extent.y / 2.0f});
		b2Rot rotation = b2Body_GetRotation(entity.bodyId.value());
		float radians = b2Rot_GetAngle(rotation);
		Vector2 ps = { p.x, p.y };
		DrawRectanglePro(Rectangle { ps.x, ps.y, entity.extent.x, entity.extent.y }, { 0.0f, 0.0f }, RAD2DEG* radians, color);
		p = b2Body_GetWorldPoint(entity.bodyId.value(), b2Vec2{0.0f, 0.0f});
		ps = { p.x, p.y };
		DrawCircleV(ps, 2.0f, R_GOLD);
	}
	DrawRectangleLines(entity.pos.x, entity.pos.y, entity.extent.x, entity.extent.y, color);
}

void scene::SceneManager::DrawJointBodies(const Entity& entity, Color color)
{
	b2Vec2 p = b2Body_GetWorldPoint(entity.bodyId.value(), b2Vec2{ -entity.extent.x / 2.0f, -entity.extent.y / 2.0f });
	b2Rot rotation = b2Body_GetRotation(entity.bodyId.value());
	float radians = b2Rot_GetAngle(rotation);
	Vector2 ps = { p.x, p.y };
	DrawRectanglePro(Rectangle{ ps.x, ps.y, entity.extent.x, entity.extent.y }, { 0.0f, 0.0f }, RAD2DEG * radians, color);
	p = b2Body_GetWorldPoint(entity.bodyId.value(), b2Vec2{ 0.0f, 0.0f });
	ps = { p.x, p.y };
	DrawCircleV(ps, 2.0f, R_GOLD);
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
	if (entityA->pos.x > entityB->pos.x) {
		std::swap(entityA, entityB);
	}
	auto width = Vector2Distance(entityA->pos, entityB->pos);
	auto boxWidth = width / 2.0f - 8.0f;
	auto boxHeight = 5.f;
	b2Polygon box = b2MakeBox(boxWidth, boxHeight);
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { entityA->pos.x + width / 2.0f + entityA->extent.x / 2.0f + 2.0f, entityA->pos.y + boxHeight - entityA->extent.y + 2.0f};
	auto angle = RAD2DEG * Vector2Angle(entityA->pos, entityB->pos);
	bodyDef.enableSleep = false;
	auto bodyId = b2CreateBody(worldId.value(), &bodyDef);
	b2CreatePolygonShape(bodyId, &shapeDef, &box);
	Entity localEntity;
	localEntity.pos = { entityA->pos.x + 7.0f, entityA->pos.y };
	localEntity.bodyId = bodyId;
	localEntity.extent = { boxWidth * 2.0f, boxHeight * 2.0f };
	
	auto jointDef = b2DefaultWeldJointDef();
	b2Vec2 pivot = { 0.0f, 0.5f };
	jointDef.bodyIdA = entityA->bodyId.value();
	jointDef.bodyIdB = bodyId;
	jointDef.localAnchorA = b2Body_GetLocalPoint(jointDef.bodyIdA, {0.5f, 0.5f});
	jointDef.localAnchorB = b2Body_GetLocalPoint(jointDef.bodyIdB, pivot);
	jointDef.angularHertz = 20.0f;
	jointDef.angularDampingRatio = 10.0f;
	jointDef.linearHertz = 10.0f;
	jointDef.linearDampingRatio = 15.0f;
	jointDef.collideConnected = true;
	auto id = b2CreateWeldJoint(worldId.value(), &jointDef);
	Joint joint { entityA, &localEntity, id };

	pivot = { 1.0f, 0.5f };
	jointDef.bodyIdA = bodyId;
	jointDef.bodyIdB = entityB->bodyId.value();
	jointDef.localAnchorA = b2Body_GetLocalPoint(jointDef.bodyIdA, pivot);
	jointDef.localAnchorB = b2Body_GetLocalPoint(jointDef.bodyIdB, {0.5f, 0.5f});
	jointDef.angularHertz = 20.0f;
	jointDef.angularDampingRatio = 10.0f;
	jointDef.linearHertz = 10.0f;
	jointDef.linearDampingRatio = 15.0f;
	jointDef.collideConnected = true;
	id = b2CreateWeldJoint(worldId.value(), &jointDef);
	Joint joint2{ &localEntity, entityB, id };

	jointEntities.emplace_back(joint);
	jointEntities.emplace_back(joint2);
	jointBodyEntities.emplace_back(localEntity);

}

void SceneManager::addNode(Vector2 position)
{
	Entity localEntity;
	localEntity.extent = { 8.0f, 8.0f };
	auto b2width = localEntity.extent.x / 2.0f;
	auto b2height = localEntity.extent.y / 2.0f;

	auto centerX = position.x + b2width;
	auto centerY = position.y + b2height;

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.position = { centerX, centerY };

	b2BodyId body = b2CreateBody(worldId.value(), &bodyDef);

	b2BodyId groundId = b2CreateBody(worldId.value(), &bodyDef);

	b2Polygon groundBox = b2MakeBox(b2width, b2height);
	b2ShapeDef groundShapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
	localEntity.pos = position;
	localEntity.bodyId = groundId;
	focusNode = &nodeEntities.emplace_back(localEntity);
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