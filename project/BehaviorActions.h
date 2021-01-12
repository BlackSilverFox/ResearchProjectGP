#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

#include "SteeringBehaviour.h"
#include "EBlackboard.h"
#include "EBehaviorTree.h"

#include "Utils.h"
#include "ResourceManagement.h"

#include <iostream>


Elite::BehaviorState ChangeToWander_Action(Elite::Blackboard* pBlackboard)
{
	SteeringBehavior steering{};

	bool dataAvailable = pBlackboard->GetData("Steering", steering);

	if (!dataAvailable)
	{
		std::cout << "Steering not found in blackboard, Behavioractions => ChangeToWander" << std::endl;
		return Elite::Failure;
	}
	
	steering = SteeringBehavior::wander;
	pBlackboard->ChangeData("Steering", steering);

	return Elite::Success;
}

Elite::BehaviorState ChangeToSeek_Action(Elite::Blackboard* pBlackboard)
{
	SteeringBehavior steering{};

	bool dataAvailable = pBlackboard->GetData("Steering", steering);

	if (!dataAvailable)
	{
		std::cout << "Steering/target not found in blackboard, Behavioractions => ChangeToSeek" << std::endl;
		return Elite::Failure;
	}

	steering = SteeringBehavior::seek;
	pBlackboard->ChangeData("Steering", steering);

	return Elite::Success;
}

Elite::BehaviorState SearchHouse_Action(Elite::Blackboard* pBlackboard)
{
	//GET DATA
	std::pair<HouseCorner, int> searchInfoPair{};
	HouseInfo currentHouse{};
	IExamInterface* pInterface{};
	bool dataAvailable = pBlackboard->GetData("Interface", pInterface)
						 && pBlackboard->GetData("SearchingHouseInfo", searchInfoPair)
						 && pBlackboard->GetData("CurrentHouse", currentHouse);
	
	if (!dataAvailable)
	{
		std::cout << "interface/searchingHouseInfo/currentHouse not found in blackboard, Behavioractions => SearchHouseAction.\n";
		return Elite::Failure;
	}

	//HANDLE LEAVING HOUSE (finished search)
	if (searchInfoPair.second == 3)
	{
		std::vector<Elite::Vector2> visitedHouses{};
		pBlackboard->GetData("VisitedHouses", visitedHouses);
		visitedHouses.push_back(currentHouse.Center);
		pBlackboard->ChangeData("VisitedHouses", visitedHouses);
		pBlackboard->ChangeData("HasTargetedHouse", false);

		return Elite::Failure;
	}

	//MAKE SEARCHING PATH (rectangle)
	Rectf searchRect{ MakeRect(currentHouse.Center, currentHouse.Size - Elite::Vector2{15.f, 15.f}) };
	
	//HANDLE ENTERING HOUSE
	//If just entering the house, this will be true
	if (searchInfoPair.first == HouseCorner::NONE)
	{
		//Get all distances to the corners
		float distances[4]{};
		for (int i{}; i < 4; ++i)
		{
			distances[i] = Elite::DistanceSquared(pInterface->Agent_GetInfo().Position, searchRect.GetCornerByInt(i));
		}
		
		//Look for the closest corner
		HouseCorner closestCornerByInt{ HouseCorner::leftBottom };
		for (int i{1}; i < 4; ++i)
		{
			if (distances[i] < distances[(int)closestCornerByInt]) closestCornerByInt = (HouseCorner)i;
		}

		//set this corner in the search info in the blackboard
		searchInfoPair.first = closestCornerByInt;
		pBlackboard->ChangeData("SearchingHouseInfo", searchInfoPair);
	}

	//LOOK IF GOAL CORNER HAS BEEN REACHED => CHANGE GOAL CORNER
	if (AgentIsCloseToPoint(pInterface->Agent_GetInfo().Position, searchRect.GetCornerByInt((int)searchInfoPair.first)))
	{
		searchInfoPair.first = HouseCorner((int(searchInfoPair.first) + 1) % 4);
		searchInfoPair.second++;
		pBlackboard->ChangeData("SearchingHouseInfo", searchInfoPair);
	}

	//SET TARGET TO GOAL CORNER
	Elite::Vector2 target{ searchRect.GetCornerByInt((int)searchInfoPair.first) };
	pBlackboard->ChangeData("Target", target);

	return Elite::Success;
}

Elite::BehaviorState GoToItem_Action(Elite::Blackboard* pBlackboard)
{
	//GET DATA
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("Interface", pInterface) };
	if (!dataAvailable) return Elite::Failure;
	
	//GET ALL ITEMS IN FOV
	std::vector<EntityInfo> allItems{};
	EntityInfo ei{};
	for (int i{};; i++)
	{
		if (pInterface->Fov_GetEntityByIndex(i, ei))
		{
			if (ei.Type == eEntityType::ITEM) allItems.push_back(ei);

			continue;
		}

		break;
	}

	if (allItems.size() == 0) return Elite::Failure;
	
	//TARGET CLOSEST ITEM
	float distance{1000.f};
	int itemIndex{};
	for (int i{}; i < allItems.size(); ++i)
	{
		float temp{ Elite::DistanceSquared(pInterface->Agent_GetInfo().Position, allItems[i].Location) };
		if (temp < distance)
		{
			distance = temp;
			itemIndex = i;
		}
	}

	Elite::Vector2 target = allItems[itemIndex].Location;
	pBlackboard->ChangeData("Target", target);
	pBlackboard->ChangeData("CurrentItem", allItems[itemIndex]);
	pBlackboard->ChangeData("HasTargetedItem", true);
	
	return Elite::Success;
}

Elite::BehaviorState GrabItem_Action(Elite::Blackboard* pBlackboard)
{
	//GET DATA
	EntityInfo ei{};
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("CurrentItem", ei) && pBlackboard->GetData("Interface", pInterface) };

	if (!dataAvailable) return Elite::Failure;
	
	//GET ITEMINFO FROM ENTITY
	ItemInfo itemInfo{};
	pInterface->Item_GetInfo(ei, itemInfo);

	//DEAL WITH GARBAGE
	if (itemInfo.Type == eItemType::GARBAGE)
	{
		pInterface->Item_Destroy(ei);
		pBlackboard->ChangeData("HasTargetedItem", false);
		return Elite::Success;
	}

	//GET INVENTORY INFO
	std::vector<int> freeSpotIndices{};
	std::vector<ItemInfo> inventoryInfo{};

	ItemInfo temp{};
	for (int i{}; i < pInterface->Inventory_GetCapacity(); ++i)
	{
		if (!pInterface->Inventory_GetItem(i, temp)) freeSpotIndices.push_back(i);
		inventoryInfo.push_back(temp);
	}

	//FREE SPACE
	if (freeSpotIndices.size() > 0)
	{
		bool grabbedItem{ pInterface->Item_Grab(ei, itemInfo) };
		
		if (grabbedItem)
		{
			pInterface->Inventory_AddItem(freeSpotIndices[0], itemInfo);
			pBlackboard->ChangeData("HasTargetedItem", false);
			return Elite::Success;
		}
		return Elite::Failure;
	}

	//NO FREE SPACE
	std::vector<int> pistolIndices{ PlacesInInventoryOfType(eItemType::PISTOL, inventoryInfo) };
	std::vector<int> foodIndices{ PlacesInInventoryOfType(eItemType::FOOD, inventoryInfo) };
	std::vector<int> medkitIndices{ PlacesInInventoryOfType(eItemType::MEDKIT, inventoryInfo) };

	int freeIndex{ };

	//DEAL WITH ITEMS PER TYPE
	switch (itemInfo.Type)
	{
	case eItemType::PISTOL:
		if (pistolIndices.size() > 0) freeIndex = HandleMultiplePistols(itemInfo, pistolIndices, inventoryInfo, pInterface);
		else freeIndex = HandleNoPistols(itemInfo, foodIndices, medkitIndices, inventoryInfo, pInterface);
		if (freeIndex == INVALID)
		{
			pInterface->Item_Destroy(ei);
			pBlackboard->ChangeData("HasTargetedItem", false);
			return Elite::Success;
		}
		break;
	case eItemType::FOOD:
		if (foodIndices.size() > 0) freeIndex = HandleMultipleFood(itemInfo, foodIndices, inventoryInfo, pInterface);
		else freeIndex = HandleNoFood(itemInfo, pistolIndices, medkitIndices, inventoryInfo, pInterface);
		if (freeIndex == INVALID)
		{
			pInterface->Item_Destroy(ei);
			pBlackboard->ChangeData("HasTargetedItem", false);
			return Elite::Success;
		}
		break;
	case eItemType::MEDKIT:
		if (medkitIndices.size() > 0) freeIndex = HandleMultipleMedkits(itemInfo, medkitIndices, inventoryInfo, pInterface);
		else freeIndex = HandleNoMedkits(itemInfo, pistolIndices, foodIndices, inventoryInfo, pInterface);
		if (freeIndex == INVALID)
		{
			pInterface->Item_Destroy(ei);
			pBlackboard->ChangeData("HasTargetedItem", false);
			return Elite::Success;
		}
		break;
	}	
	
	//TRY AND GRAB ITEM
	bool grabbedItem{ pInterface->Item_Grab(ei, itemInfo) };
	if (grabbedItem)
	{
		pInterface->Inventory_AddItem(freeIndex, itemInfo);
		pBlackboard->ChangeData("HasTargetedItem", false);
		return Elite::Success;
	}

	return Elite::Failure;
}

Elite::BehaviorState FleeFromEnemy_Action(Elite::Blackboard* pBlackboard)
{
	//DATA
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("Interface", pInterface) };
	if (!dataAvailable) return Elite::Failure;

	AgentInfo& agentInfo{ pInterface->Agent_GetInfo() };
	WorldInfo& worldInfo{ pInterface->World_GetInfo() };

	//TARGETING
	pBlackboard->ChangeData("Steering", SteeringBehavior::flee);

	Elite::Vector2 forwardVector{ cos(agentInfo.Orientation - Elite::ToRadians(90.f)), sin(agentInfo.Orientation - Elite::ToRadians(90.f)) };
	Elite::Vector2 distance{};
	Elite::Vector2 target{};

	EntityInfo ei{};
	if (pInterface->Fov_GetEntityByIndex(0, ei) && ei.Type == eEntityType::ENEMY)
	{
		forwardVector = { cos(agentInfo.Orientation), sin(agentInfo.Orientation) };
	}

	distance = forwardVector * 10.f;
	target = agentInfo.Position + distance;

	//is flee target still in world
	float offset{ 15.f };
	if (!IsInsideBounds(target, worldInfo.Center, Elite::Vector2{ worldInfo.Dimensions.x - offset, worldInfo.Dimensions.y - offset }))
	{
		//bounce off world edges, choose direction that says in world
		forwardVector = { cos(agentInfo.Orientation), sin(agentInfo.Orientation) };
		target = agentInfo.Position + distance;
		if (!IsInsideBounds(target, worldInfo.Center, Elite::Vector2{ worldInfo.Dimensions.x - offset, worldInfo.Dimensions.y - offset }))
		{
			forwardVector = { cos(agentInfo.Orientation + Elite::ToRadians(180.f)), sin(agentInfo.Orientation + Elite::ToRadians(180.f)) };
			target = agentInfo.Position + distance;
		}
	}

	pBlackboard->ChangeData("Target", target);
	return Elite::Success;
}

Elite::BehaviorState ShootEnemy_Action(Elite::Blackboard* pBlackboard)
{
	//DATA
	IExamInterface* pInterface{};
	pBlackboard->GetData("Interface", pInterface);
	AgentInfo& agentInfo{ pInterface->Agent_GetInfo() };
	
	//AIMING
	pBlackboard->ChangeData("Steering", SteeringBehavior::shoot);
	Elite::Vector2 forwardVector{ cos(agentInfo.Orientation - Elite::ToRadians(90.f)), sin(agentInfo.Orientation - Elite::ToRadians(90.f)) };

	EntityInfo ei{};
	if (pInterface->Fov_GetEntityByIndex(0, ei))
	{
		if (ei.Type != eEntityType::ENEMY) return Elite::Success;
		
		//ENEMY ALREADY IN FOV
		//compare forward vector direction to agentToEnemy vector direction
		Elite::Vector2 agentToEnemy{ ei.Location - agentInfo.Position };
		float dot{ Elite::Dot(agentToEnemy, forwardVector) / (agentToEnemy.Magnitude() * forwardVector.Magnitude()) };

		if (dot <= 1.001f && dot >= 0.999f) //=> means angle is (almost) 0 between two vectors
		{
			for (int i{}; i < pInterface->Inventory_GetCapacity(); ++i)
			{
				ItemInfo itemInfo{};
				if (pInterface->Inventory_GetItem(i, itemInfo) && itemInfo.Type == eItemType::PISTOL)
				{
					pInterface->Inventory_UseItem(i);
					if (pInterface->Weapon_GetAmmo(itemInfo) <= 0) pInterface->Inventory_RemoveItem(i);
					return Elite::Success;
				}
			}
		}

		pBlackboard->ChangeData("Target", agentInfo.Position + agentToEnemy * 10.f);
		pBlackboard->ChangeData("LookAt", ei.Location);
	}
	else
	{
		//ATTACKED FROM OUTSIDE FOV, SEARCH ATTACKER
		Elite::Vector2 sideVector{ cos(agentInfo.Orientation + Elite::ToRadians(45.f)), sin(agentInfo.Orientation + Elite::ToRadians(45.f)) };
		Elite::Vector2 target{ agentInfo.Position + sideVector * 10.f };

		pBlackboard->ChangeData("Target", target);// target with seek instead of face, to make turning faster
		pBlackboard->ChangeData("Steering", SteeringBehavior::seek);// basically only here to make the turn really abrupt
	}

	return Elite::Success;
}


Elite::BehaviorState SetToFlee_Action(Elite::Blackboard* pBlackboard)
{
	pBlackboard->ChangeData("Steering", SteeringBehavior::flee);
	return Elite::Success;
}