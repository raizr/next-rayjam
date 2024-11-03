#include "car.h"

#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "raylib.h"
#include "raymath.h"
#include "utils.h"
#include "resource.h"

#include <assert.h>

Car::Car()
{
	m_chassisId = {};
	m_rearWheelId = {};
	m_frontWheelId = {};
	m_rearAxleId = {};
	m_frontAxleId = {};
	m_isSpawned = false;
}

void Car::Spawn( b2WorldId worldId, b2Vec2 position, float scale, float hertz, float dampingRatio, float torque)
{
	assert( m_isSpawned == false );

	assert( B2_IS_NULL( m_chassisId ) );
	assert( B2_IS_NULL( m_frontWheelId ) );
	assert( B2_IS_NULL( m_rearWheelId ) );

	boxExtent = { 8 * scale, 4 * scale };
	b2Polygon chassis = b2MakeBox(boxExtent.x / 2.0f, boxExtent.y / 2.0f);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 0.01f * scale;
	shapeDef.friction = 0.2f;

	circle = { { 0.0f, 0.0f }, 1.0f * scale };

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = b2Add( { 0.0f, 1.0f * scale }, position );
	m_chassisId = b2CreateBody( worldId, &bodyDef );
	b2CreatePolygonShape( m_chassisId, &shapeDef, &chassis );

	shapeDef.density = 0.02f * scale;
	shapeDef.friction = 1.5f;

	bodyDef.position = b2Add( {25.0f -boxExtent.x / 2.0f, 0.8f * boxExtent.y }, position );
	bodyDef.allowFastRotation = true;
	m_rearWheelId = b2CreateBody( worldId, &bodyDef );
	b2CreateCircleShape( m_rearWheelId, &shapeDef, &circle );

	bodyDef.position = b2Add( { -7.0f + boxExtent.x / 2.0f, 0.8f * boxExtent.y }, position );
	bodyDef.allowFastRotation = true;
	m_frontWheelId = b2CreateBody( worldId, &bodyDef );
	b2CreateCircleShape( m_frontWheelId, &shapeDef, &circle );

	b2Vec2 axis = { 0.0f, 1.0f };
	b2Vec2 pivot = b2Body_GetPosition( m_rearWheelId );

	b2WheelJointDef jointDef = b2DefaultWheelJointDef();

	jointDef.bodyIdA = m_chassisId;
	jointDef.bodyIdB = m_rearWheelId;
	jointDef.localAxisA = b2Body_GetLocalVector( jointDef.bodyIdA, axis );
	jointDef.localAnchorA = b2Body_GetLocalPoint( jointDef.bodyIdA, pivot );
	jointDef.localAnchorB = b2Body_GetLocalPoint( jointDef.bodyIdB, pivot );
	jointDef.motorSpeed = 0.0f;
	jointDef.maxMotorTorque = torque;
	jointDef.enableMotor = true;
	jointDef.hertz = hertz;
	jointDef.dampingRatio = dampingRatio;
	jointDef.lowerTranslation = -0.25f * scale;
	jointDef.upperTranslation = 0.25f * scale;
	jointDef.enableLimit = true;
	m_rearAxleId = b2CreateWheelJoint( worldId, &jointDef );

	pivot = b2Body_GetPosition( m_frontWheelId );
	jointDef.bodyIdA = m_chassisId;
	jointDef.bodyIdB = m_frontWheelId;
	jointDef.localAxisA = b2Body_GetLocalVector( jointDef.bodyIdA, axis );
	jointDef.localAnchorA = b2Body_GetLocalPoint( jointDef.bodyIdA, pivot );
	jointDef.localAnchorB = b2Body_GetLocalPoint( jointDef.bodyIdB, pivot );
	jointDef.motorSpeed = 0.0f;
	jointDef.maxMotorTorque = torque;
	jointDef.enableMotor = true;
	jointDef.hertz = hertz;
	jointDef.dampingRatio = dampingRatio;
	jointDef.lowerTranslation = -0.25f * scale;
	jointDef.upperTranslation = 0.25f * scale;
	jointDef.enableLimit = true;
	m_frontAxleId = b2CreateWheelJoint( worldId, &jointDef );
	m_isSpawned = true;
}

void Car::Despawn()
{
	assert( m_isSpawned == true );

	b2DestroyJoint( m_rearAxleId );
	b2DestroyJoint( m_frontAxleId );
	b2DestroyBody( m_rearWheelId );
	b2DestroyBody( m_frontWheelId );
	b2DestroyBody( m_chassisId );
	m_chassisId = {};
	m_rearWheelId = {};
	m_frontWheelId = {};
	m_rearAxleId = {};
	m_frontAxleId = {};
	m_isSpawned = false;
	position = {};
}

void Car::Draw()
{
	if (!m_isSpawned) {
		return;
	}
	position = b2Body_GetWorldPoint(m_chassisId, b2Vec2{ -boxExtent.x / 2.0f, -boxExtent.y / 2.0f + 4.0f });
	b2Rot bodyRotation = b2Body_GetRotation(m_chassisId);
	float bodyDeg = RAD2DEG * b2Rot_GetAngle(bodyRotation);

	auto drawCircle = [this](b2BodyId id, Color color) {
		b2Vec2 frontWheelVec = b2Body_GetWorldPoint(id, b2Vec2{  });
		b2Rot fronWheelRotation = b2Body_GetRotation(id);
		float fronWheelDeg = RAD2DEG * b2Rot_GetAngle(fronWheelRotation);

		Rectangle source = { 0.0f, 0.0f, (float)Resources::wheel.width, (float)Resources::wheel.height };
		Rectangle dest = { frontWheelVec.x, frontWheelVec.y,
			(float)Resources::wheel.width, (float)Resources::wheel.height };
		Vector2 origin = { (float)Resources::wheel.width / 2.0f, (float)Resources::wheel.height / 2.0f };

		DrawTexturePro(Resources::wheel, source, dest, origin, fronWheelDeg, WHITE);
	};
	drawCircle(m_frontWheelId, RED);
	drawCircle(m_rearWheelId, BROWN);
	DrawTextureEx(Resources::car, Vector2{ position.x, position.y }, bodyDeg, 1.0f, WHITE);
}

void Car::SetSpeed( float speed )
{
	b2WheelJoint_SetMotorSpeed( m_rearAxleId, speed );
	b2WheelJoint_SetMotorSpeed( m_frontAxleId, speed );
	b2Joint_WakeBodies( m_rearAxleId );
}

void Car::SetTorque( float torque )
{
	b2WheelJoint_SetMaxMotorTorque( m_rearAxleId, torque );
	b2WheelJoint_SetMaxMotorTorque( m_frontAxleId, torque );
}

void Car::SetHertz( float hertz )
{
	b2WheelJoint_SetSpringHertz( m_rearAxleId, hertz );
	b2WheelJoint_SetSpringHertz( m_frontAxleId, hertz );
}

void Car::SetDampingRadio( float dampingRatio )
{
	b2WheelJoint_SetSpringDampingRatio( m_rearAxleId, dampingRatio );
	b2WheelJoint_SetSpringDampingRatio( m_frontAxleId, dampingRatio );
}

Vector2 Car::GetPosition()
{
	return { position.x, position.y };
}
