#pragma once
#include <iostream>
#include <vector>

#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

#include "SteeringBehaviour.h"
#include "EBlackboard.h"

#include "Utils.h"

bool UnvisitedHouseInFOV_Condition(Elite::Blackboard* pBlackboard)
{
	//GET DATA
	IExamInterface* interface {};
	Elite::Vector2 target{};
	std::vector<Elite::Vector2> visitedHouses{};
	bool hasTargetedHouse{};

	bool dataAvailable = pBlackboard->GetData("Interface", interface)
		&& pBlackboard->GetData("Target", target)
		&& pBlackboard->GetData("VisitedHouses", visitedHouses)
		&& pBlackboard->GetData("HasTargetedHouse", hasTargetedHouse);

	if (!dataAvailable)
	{
		std::cout << "Interface/target/visitedHouses not found in blackboard, BehaviorConditions => HouseInFOV" << std::endl;
		return Elite::Failure;
	}

	//ALREADY TARGETING A HOUSE
	if (hasTargetedHouse)
	{
		HouseInfo targetHouse{};
		pBlackboard->GetData("CurrentHouse", targetHouse);
		pBlackboard->ChangeData("Target", targetHouse.Center);
		return true;
	}

	//GET UNVISITED HOUSES IN FOV
	std::vector<HouseInfo> housesToVisit{};
	HouseInfo hi{};
	for (int i{};; i++)
	{
		if (interface->Fov_GetHouseByIndex(i, hi))
		{
			auto it = find_if(visitedHouses.begin(), visitedHouses.end(), [&hi](const Elite::Vector2 visited) { return hi.Center == visited; });
			if (it == visitedHouses.end()) housesToVisit.push_back(hi);

			continue;
		}

		break;
	}

	//TARGET HOUSE
	if (housesToVisit.size() > 0)
	{
		target = housesToVisit[0].Center;
		pBlackboard->ChangeData("Target", target);
		pBlackboard->ChangeData("HasTargetedHouse", true);

		//reset search info
		std::pair<HouseCorner, int> searchInfoPair{ HouseCorner::NONE, 0 };
		bool dataAvailable1{ pBlackboard->ChangeData("SearchingHouseInfo", searchInfoPair) };

		if (!dataAvailable1)
		{
			std::cout << "Unable to change SearchingHouseInfo in blackboard, BehaviorConditions => HouseInFOV\n";
		}

		//Set houseinfo
		dataAvailable1 = pBlackboard->ChangeData("CurrentHouse", housesToVisit[0]);
		if (!dataAvailable1)
		{
			std::cout << "Unable to change CurrentHouse in blackboard, BehaviorConditions => HouseInFOV\n";
		}

		return true;
	}

	return false;
}

bool IsInsideUnvisitedHouse_Condition(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface {};

	bool dataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!dataAvailable)
	{
		std::cout << "Interface not found in blackboard, BehaviorConditions => IsInsideUnvisitedHouse.\n";
		return false;
	}

	if (pInterface->Agent_GetInfo().IsInHouse)
	{
		return true;
	}
	return false;
}

bool ItemInFOV_Condition(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface {};
	bool dataAvailable{ pBlackboard->GetData("Interface", pInterface) };
	if (!dataAvailable)
	{
		std::cout << "Interface not found in blackboard, BehaviorConditions => ItemInsideFOV\n";
		return false;
	}

	EntityInfo ei{};
	for (int i{};; i++)
	{
		if (pInterface->Fov_GetEntityByIndex(i, ei))
		{
			if (ei.Type == eEntityType::ITEM)
			{
				return true;
			}
			continue;
		}

		break;
	}

	return false;
}

bool CanGrabItem_Condition(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	EntityInfo item{};
	bool hasTargetedItem{};
	bool dataAvailable{ pBlackboard->GetData("Interface", pInterface)
						&& pBlackboard->GetData("CurrentItem", item) 
						&& pBlackboard->GetData("HasTargetedItem", hasTargetedItem) };
	if (!dataAvailable)
	{
		std::cout << "Interface/target not found in blackboard, BehaviorConditions => CanGrabItem\n";
		return false;
	}
	
	if (!hasTargetedItem) return false;

	bool closeEnough{ AgentIsCloseToPoint(pInterface->Agent_GetInfo().Position, item.Location) };
	if (closeEnough)
	{
		return true;
	}

	return false;
}

bool HasPistol_Condition(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("Interface", pInterface) };
	if (!dataAvailable)
	{
		std::cout << "Interface not found in blackboard, BehaviorConditions => EnemyInsideFOV\n";
		return false;
	}

	for (int i{}; i < pInterface->Inventory_GetCapacity(); ++i)
	{
		ItemInfo info{};
		if (pInterface->Inventory_GetItem(i, info) && info.Type == eItemType::PISTOL) return true;
	}

	return false;
}