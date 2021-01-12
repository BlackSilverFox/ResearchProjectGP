#include "stdafx.h"
#include "SteeringBehaviour.h"

SteeringPlugin_Output  ISteeringBehaviour::Movement(Elite::Vector2 target, float deltaT, AgentInfo& agentInfo) const
{
	SteeringPlugin_Output steering{};

	steering.LinearVelocity = target - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	return steering;
}


Seek::Seek(IExamInterface* pInterface)
	:m_pInterface{ pInterface }
{
}

SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	Elite::Vector2 nextPathPoint{ m_pInterface->NavMesh_GetClosestPathPoint(m_Target) }; // put point on the navmesh as to not run into walls
	
	return Movement(nextPathPoint, deltaT, agentInfo);
}


Wander::Wander(IExamInterface* pInterface)
	:m_pInterface{pInterface}
{
}

SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	m_CurrentDelay += deltaT;

	if (m_CurrentDelay > m_MaxDelay) // Calculate a new target only when x amount of seconds has passed
	{
		m_CurrentDelay -= m_MaxDelay;
	
		Elite::Vector2 currentOrientation{ cos(agentInfo.Orientation - Elite::ToRadians(90.f)),
										   sin(agentInfo.Orientation - Elite::ToRadians(90.f)) };

		Elite::Vector2 circleCenter{ agentInfo.Position + m_Offset * currentOrientation };
		
		float randomAngle{ Elite::randomFloat(m_RandomAngleMax) - m_RandomAngleMax / 2.f };

		Elite::Vector2 circleVector{ cos(agentInfo.Orientation + randomAngle - Elite::ToRadians(90.f) ) ,
									 sin(agentInfo.Orientation + randomAngle - Elite::ToRadians(90.f) )  };

		m_Target = circleCenter + m_Radius * circleVector;
	}

	Elite::Vector2 nextPathPoint{ m_pInterface->NavMesh_GetClosestPathPoint(m_Target) }; // put point on the navmesh as to not run into walls
	
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 0, 1, 0 }); // DEBUG
	
	SteeringPlugin_Output steering{};

	steering.LinearVelocity = nextPathPoint - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	return steering;
}

Face::Face()
	:m_LookAt{}
{
}

SteeringPlugin_Output Face::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	float errorOffset{ 0.02f };
	Elite::Vector2 desiredOrientation{ m_LookAt - agentInfo.Position };
	Elite::Vector2 currentOrientation{ cos(agentInfo.Orientation - Elite::ToRadians(90.f)), sin(agentInfo.Orientation - Elite::ToRadians(90.f)) };

	float angleBetweenOrients{ atan2(Elite::Cross(desiredOrientation, currentOrientation), Elite::Dot(desiredOrientation, currentOrientation)) };

	SteeringPlugin_Output steering{};
	if (angleBetweenOrients > errorOffset)
	{
		steering.AngularVelocity = -agentInfo.MaxAngularSpeed;
	}
	else if (angleBetweenOrients < -errorOffset)
	{
		steering.AngularVelocity = agentInfo.MaxAngularSpeed;
	}

	return steering;
}