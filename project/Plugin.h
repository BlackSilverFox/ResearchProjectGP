#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

#include "SteeringBehaviour.h"
#include "EBlackboard.h"

#include "EBehaviorTree.h"
#include "BehaviorConditions.h"
#include "BehaviorActions.h"

#include "EFiniteStateMachine.h"
#include "States.h"
#include "Transitions.h"

#include "WorldGrid.h"

class IBaseInterface;
class IExamInterface;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose



	ISteeringBehaviour* m_pWander;
	ISteeringBehaviour* m_pSeek;
	ISteeringBehaviour* m_pFace;

	Elite::BehaviorTree* m_pBT_ItemHouse;
	Elite::BehaviorTree* m_pBT_Enemy;
	WorldGrid* m_pWorldGrid;

	Elite::FiniteStateMachine* m_pFSM;
	std::vector<Elite::FSMState*> m_pStates{};
	std::vector<Elite::FSMTransition*> m_pTransitions{};

	Elite::Blackboard* CreateBlackboard(AgentInfo* pAgentInfo) const;
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}