#pragma once
#include "EFiniteStateMachine.h"
#include "SteeringBehaviour.h"
#include "IExamInterface.h"

// helper functions
bool NoticedEnemy_Transition_Helper(Elite::Blackboard* pBlackboard) // bitten or in FOV
{
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("Interface", pInterface)};

	if (!dataAvailable)
	{
		std::cout << "Interface was not found in blackboard, Transitions, EnemyInFOV\n";
	}
	

	EntityInfo ei{};

	for (int i{}; ; ++i)
	{
		if (pInterface->Fov_GetEntityByIndex(i, ei))
		{
			if (ei.Type == eEntityType::ENEMY) return true;
			continue;
		}
		break;
	}

	if (pInterface->Agent_GetInfo().Bitten || pInterface->Agent_GetInfo().WasBitten) return true;

	return false;
}

bool HouseInFOV_Transition_Helper(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	std::vector<Elite::Vector2> visitedHouses{};
	bool dataAvailable{ pBlackboard->GetData("Interface", pInterface) &&
						pBlackboard->GetData("VisitedHouses", visitedHouses) };

	if (!dataAvailable)
	{
		std::cout << "Interface/VisitedHouses was not found in blackboard, Transitions, HouseInFOV\n";
	}

	HouseInfo hi{};
	for (int i{};; i++)
	{
		if (pInterface->Fov_GetHouseByIndex(i, hi))
		{
			auto it = find_if(visitedHouses.begin(), visitedHouses.end(), [&hi](const Elite::Vector2 visited) { return hi.Center == visited; });
			if (it == visitedHouses.end()) return true;

			continue;
		}

		break;
	}
	return false;
}

bool ItemInFOV_Transition_Helper(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("Interface", pInterface) };

	if (!dataAvailable)
	{
		std::cout << "Interface was not found in blackboard, Transitions, EnemyInFOV\n";
	}

	EntityInfo ei{};

	for (int i{}; ; ++i)
	{
		if (pInterface->Fov_GetEntityByIndex(i, ei))
		{
			if (ei.Type != eEntityType::ENEMY) return true;
			continue;
		}
		return false;
	}
}

class EnemySpotted_Transition : public Elite::FSMTransition
{
public:
	EnemySpotted_Transition() {};
	~EnemySpotted_Transition() {};

	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override
	{
		return NoticedEnemy_Transition_Helper(pBlackboard);
	}
};

class ItemHouseSpotted_Transition : public Elite::FSMTransition
{
public:
	ItemHouseSpotted_Transition() = default;
	~ItemHouseSpotted_Transition() = default;

	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override
	{
		bool dealingWithEnemy{};
		pBlackboard->GetData("EnemySpotted", dealingWithEnemy);
		
		return !NoticedEnemy_Transition_Helper(pBlackboard) &&
			   !dealingWithEnemy &&
			   (ItemInFOV_Transition_Helper(pBlackboard) || HouseInFOV_Transition_Helper(pBlackboard));
	}
};

class NoneSpotted_Transition : public Elite::FSMTransition
{
public:
	NoneSpotted_Transition() = default;
	~NoneSpotted_Transition() = default;

	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override
	{
		bool hasTargetedHouse{};
		bool dealingWithEnemy{};
		pBlackboard->GetData("EnemySpotted", dealingWithEnemy);
		pBlackboard->GetData("HasTargetedHouse", hasTargetedHouse);
		
		return !ItemInFOV_Transition_Helper(pBlackboard) &&
			   !HouseInFOV_Transition_Helper(pBlackboard) &&
			   !NoticedEnemy_Transition_Helper(pBlackboard) &&
			   !dealingWithEnemy &&
			   !hasTargetedHouse;
	}
};