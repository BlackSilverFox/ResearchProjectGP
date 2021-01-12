#pragma once
#include "IExamInterface.h"
#include "Exam_HelperStructs.h"

class ISteeringBehaviour
{
public:
	ISteeringBehaviour() = default;
	virtual ~ISteeringBehaviour() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaTime, AgentInfo& agentInfo) = 0;
	
	void SetTarget(const Elite::Vector2& targetPos) { m_Target = targetPos; }

protected:
	Elite::Vector2 m_Target;

	SteeringPlugin_Output Movement(Elite::Vector2 target, float deltaT, AgentInfo& agentInfo) const;
};

class Seek : public ISteeringBehaviour
{
public:
	Seek(IExamInterface* pInterface);
	virtual ~Seek() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo& agentInfo) override;

private:
	IExamInterface* m_pInterface;
};

class Wander : public ISteeringBehaviour
{
public:
	Wander(IExamInterface* interface);
	virtual ~Wander() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo& agentInfo) override;

	void SetOffset(float offset) { m_Offset = offset; };
	void SetRandomAngleMax(float angleDegrees) { m_RandomAngleMax = Elite::ToRadians(angleDegrees); };
	void SetRadius(float radius) { m_Radius = radius; };
	void SetMaxDelay(float delay) { m_MaxDelay = delay; };

private:
	float m_Offset{ 10.f };
	float m_RandomAngleMax{ 0.f };
	float m_Radius{ 20.f };

	float m_MaxDelay{ 0.5f };
	float m_CurrentDelay{ 0.5001f };

	IExamInterface* m_pInterface;
};

class Face : public ISteeringBehaviour
{
public:
	Face();
	virtual ~Face() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo& pAgent) override;
	void SetLookAt(const Elite::Vector2 lookAt) { m_LookAt = lookAt; };

private:
	Elite::Vector2 m_LookAt{};
};