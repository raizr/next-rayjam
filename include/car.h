#pragma once
#include "box2d/types.h"
#include "raylib.h"

class Car
{
	public:
		Car();

		void Spawn( b2WorldId worldId, b2Vec2 position, float scale, float hertz, float dampingRatio, float torque);
		void Despawn();
		void Draw();
		void SetSpeed( float speed );
		void SetTorque( float torque );
		void SetHertz( float hertz );
		void SetDampingRadio( float dampingRatio );
		Vector2 GetPosition();

	private:
		b2BodyId m_chassisId;
		b2BodyId m_rearWheelId;
		b2BodyId m_frontWheelId;
		b2JointId m_rearAxleId;
		b2JointId m_frontAxleId;
		bool m_isSpawned;
		b2Vec2 boxExtent;
		b2Circle circle;
		b2Vec2 position;
};
