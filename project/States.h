#pragma once
#include "stdafx.h"
#include "EFiniteStateMachine.h"
#include "EBlackboard.h"
#include "SteeringBehaviour.h"
#include "WorldGrid.h"

#include "EBehaviorTree.h"

class Explore_State : public Elite::FSMState
{
public:
	Explore_State(WorldGrid* pWorldgrid)
		: m_pWorldgrid{ pWorldgrid }
	{
	};

	~Explore_State() {}; // doesnt own worldgrid!

	virtual void OnEnter(Elite::Blackboard* pBlackboard) override
	{
		m_pWorldgrid->ResetTargetCell(); //simply reset targetcell in the navigation, so the algo doesnt get stuck
		pBlackboard->ChangeData("Steering", SteeringBehavior::seek);
	}

	virtual void OnExit(Elite::Blackboard* pBlackboard) override {}

	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override
	{
		Elite::Vector2 target{ m_pWorldgrid->GetNextTarget_Mapping() }; // get next target on the pathfinding

		bool dataChanged{ pBlackboard->ChangeData("Target", target) };

		if (!dataChanged)
		{
			std::cout << "Blackboard variable not found\n";
			return;
		}
	}

private:
	WorldGrid* m_pWorldgrid;
};

class Search_State : public Elite::FSMState
{
public:
	Search_State(Elite::BehaviorTree* pSearchTree)
		: m_pSearchTree{ pSearchTree }
	{
	}

	~Search_State() {}; // Doesn't own the searchtree

	virtual void OnEnter(Elite::Blackboard* pBlackboard) override
	{
		pBlackboard->ChangeData("Steering", SteeringBehavior::seek);
	}

	virtual void OnExit(Elite::Blackboard* pBlackboard) override
	{
		//pBlackboard->ChangeData("HasTargetedHouse", false);
	}

	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override
	{
		m_pSearchTree->Update(deltaTime);
	}

private:
	Elite::BehaviorTree* m_pSearchTree;
};

class DealWithEnemy_State : public Elite::FSMState
{
public:
	DealWithEnemy_State(Elite::BehaviorTree* pEnemyTree, WorldGrid* pWorldgrid)
		: m_pEnemyTree{ pEnemyTree }
		, m_pWorldgrid{ pWorldgrid }
	{
	}

	~DealWithEnemy_State() {} // Doesn't own the enemyTree

	virtual void OnEnter(Elite::Blackboard* pBlackboard) override
	{
		m_pWorldgrid->ResetTargetCell();
		m_TimerCurrent = 0.f;
		pBlackboard->ChangeData("EnemySpotted", true);
	}

	virtual void OnExit(Elite::Blackboard* pBlackboard) override
	{
		m_TimerCurrent = 0.f;
		pBlackboard->ChangeData("EnemySpotted", false);
	}

	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override
	{
		Elite::Vector2 target{ m_pWorldgrid->GetNextTarget_Mapping() }; // get next target on the pathfinding
		pBlackboard->ChangeData("Target", target);
		
		m_pEnemyTree->Update(deltaTime); // will overwrite target incase of house(search)
		
		m_TimerCurrent += deltaTime;
		if (m_TimerCurrent >= m_TimerMax)
		{
			pBlackboard->ChangeData("EnemySpotted", false);
		}
	}

private:
	Elite::BehaviorTree* m_pEnemyTree;
	float m_TimerCurrent{ 0.f };
	float m_TimerMax{ 2.f };
	WorldGrid* m_pWorldgrid;
};