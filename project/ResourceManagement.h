#pragma once
#include "Exam_HelperStructs.h"
#include "stdafx.h"
#include "Utils.h"

static std::vector<int> PlacesInInventoryOfType(const eItemType& type, const std::vector<ItemInfo>& items)
{
	std::vector<int> matchingIndices{};

	for (int i{}; i < items.size(); ++i)
	{
		if (items[i].Type == type)
		{
			matchingIndices.push_back(i);
		}
	}
	return matchingIndices;
}

static int HandleMultiplePistols(ItemInfo& newPistol, const std::vector<int>& pistolIndices, std::vector<ItemInfo> inventory, IExamInterface* pInterface)
{
	int currentItem{ pInterface->Weapon_GetAmmo(newPistol) };
	int lowestChargesIndex{ INVALID };

	for (int i{}; i < pistolIndices.size(); ++i)
	{
		if (pInterface->Weapon_GetAmmo(inventory[pistolIndices[i]]) <= currentItem)//new item better then old item
		{
			lowestChargesIndex = pistolIndices[i];
			currentItem = pInterface->Weapon_GetAmmo(inventory[pistolIndices[i]]);
		}
	}

	if (lowestChargesIndex != INVALID) pInterface->Inventory_RemoveItem(lowestChargesIndex);

	return lowestChargesIndex;
}

static int HandleMultipleFood(ItemInfo& newFood, const std::vector<int>& foodIndices, std::vector<ItemInfo> inventory, IExamInterface* pInterface)
{
	int currentItem{ pInterface->Food_GetEnergy(newFood) };
	int lowestChargesIndex{ INVALID };

	for (int i{}; i < foodIndices.size(); ++i)
	{
		if (pInterface->Food_GetEnergy(inventory[foodIndices[i]]) <= currentItem)//new item better then old item
		{
			lowestChargesIndex = foodIndices[i];
			currentItem = pInterface->Food_GetEnergy(inventory[foodIndices[i]]);
		}
	}

	if (lowestChargesIndex != INVALID)
	{
		pInterface->Inventory_UseItem(lowestChargesIndex);
		pInterface->Inventory_RemoveItem(lowestChargesIndex);
	}

	return lowestChargesIndex;
}

static int HandleMultipleMedkits(ItemInfo& newMedkit, const std::vector<int>& medkitIndices, std::vector<ItemInfo> inventory, IExamInterface* pInterface)
{
	int currentItemCharges{ pInterface->Medkit_GetHealth(newMedkit) };
	int lowestChargesIndex{ INVALID };

	for (int i{}; i < medkitIndices.size(); ++i)
	{
		if (pInterface->Medkit_GetHealth(inventory[medkitIndices[i]]) <= currentItemCharges)//new item better then old item
		{
			lowestChargesIndex = medkitIndices[i];
			currentItemCharges = pInterface->Medkit_GetHealth(inventory[medkitIndices[i]]);
		}
	}

	if (lowestChargesIndex != INVALID)
	{
		pInterface->Inventory_UseItem(lowestChargesIndex);
		pInterface->Inventory_RemoveItem(lowestChargesIndex);
	}

	return lowestChargesIndex;
}

static int HandleNoPistols(ItemInfo& newPistol, const std::vector<int>& foodIndices, const std::vector<int>& medkitIndices, std::vector<ItemInfo> inventory, IExamInterface* pInterface)
{
	size_t nrOfFood{ foodIndices.size() };
	size_t nrOfMedkits{ medkitIndices.size() };

	if (nrOfFood > nrOfMedkits)
	{
		//use one food
		int lowestFood{ foodIndices[0] };
		for (int i{ 1 }; i < nrOfFood; ++i)
		{
			if (pInterface->Food_GetEnergy(inventory[foodIndices[i]]) <= pInterface->Food_GetEnergy(inventory[foodIndices[lowestFood]]))
			{
				lowestFood = foodIndices[i];
			}
		}
		pInterface->Inventory_UseItem(lowestFood);
		pInterface->Inventory_RemoveItem(lowestFood);
		return lowestFood;
	}
	else
	{
		//use one medkit
		int lowestMedkit{ medkitIndices[0] };
		for (int i{ 1 }; i < nrOfMedkits; ++i)
		{
			if (pInterface->Medkit_GetHealth(inventory[medkitIndices[i]]) <= pInterface->Medkit_GetHealth(inventory[medkitIndices[lowestMedkit]]))
			{
				lowestMedkit = medkitIndices[i];
			}
		}
		pInterface->Inventory_UseItem(lowestMedkit);
		pInterface->Inventory_RemoveItem(lowestMedkit);
		return lowestMedkit;
	}
}

static int HandleNoFood(ItemInfo& newFood, const std::vector<int>& pistolIndices, const std::vector<int>& medkitIndices, std::vector<ItemInfo> inventory, IExamInterface* pInterface)
{
	size_t nrOfPistols{ pistolIndices.size() };
	size_t nrOfMedkits{ medkitIndices.size() };

	if (nrOfPistols > nrOfMedkits)
	{
		//remove one pistol
		int lowestPistol{ pistolIndices[0] };
		for (int i{ 1 }; i < nrOfPistols; ++i)
		{
			if (pInterface->Weapon_GetAmmo(inventory[pistolIndices[i]]) < pInterface->Weapon_GetAmmo(inventory[pistolIndices[lowestPistol]]))
			{
				lowestPistol = pistolIndices[i];
			}
		}
		pInterface->Inventory_RemoveItem(lowestPistol);
		return lowestPistol;
	}
	else
	{
		//use one medkit
		int lowestMedkit{ medkitIndices[0] };
		for (int i{ 1 }; i < nrOfMedkits; ++i)
		{
			if (pInterface->Medkit_GetHealth(inventory[medkitIndices[i]]) <= pInterface->Medkit_GetHealth(inventory[medkitIndices[lowestMedkit]]))
			{
				lowestMedkit = medkitIndices[i];
			}
		}
		pInterface->Inventory_UseItem(lowestMedkit);
		pInterface->Inventory_RemoveItem(lowestMedkit);
		return lowestMedkit;
	}
}

static int HandleNoMedkits(ItemInfo& newMedkit, const std::vector<int>& pistolIndices, const std::vector<int>& foodIndices, std::vector<ItemInfo> inventory, IExamInterface* pInterface)
{
	size_t nrOfPistols{ pistolIndices.size() };
	size_t nrOfFood{ foodIndices.size() };

	if (nrOfPistols > nrOfFood)
	{
		//remove one pistol
		int lowestPistol{ pistolIndices[0] };
		for (int i{ 1 }; i < nrOfPistols; ++i)
		{
			if (pInterface->Weapon_GetAmmo(inventory[pistolIndices[i]]) <= pInterface->Weapon_GetAmmo(inventory[pistolIndices[lowestPistol]]))
			{
				lowestPistol = pistolIndices[i];
			}
		}
		pInterface->Inventory_RemoveItem(lowestPistol);
		return lowestPistol;
	}
	else
	{
		//use one food
		int lowestFood{ foodIndices[0] };
		for (int i{ 1 }; i < nrOfFood; ++i)
		{
			if (pInterface->Food_GetEnergy(inventory[foodIndices[i]]) <= pInterface->Food_GetEnergy(inventory[foodIndices[lowestFood]]))
			{
				lowestFood = foodIndices[i];
			}
		}
		pInterface->Inventory_UseItem(lowestFood);
		pInterface->Inventory_RemoveItem(lowestFood);
		return lowestFood;
	}
}

static void ManageHealth(IExamInterface* pInterface)
{
	float maxHealth{ 10.f };
	
	for (int i{}; i < pInterface->Inventory_GetCapacity(); ++i)
	{
		ItemInfo item{};
		if (pInterface->Inventory_GetItem(i, item) && item.Type == eItemType::MEDKIT)
		{
			if (pInterface->Agent_GetInfo().Health + pInterface->Medkit_GetHealth(item) <= maxHealth)
			{
				pInterface->Inventory_UseItem(i);
				pInterface->Inventory_RemoveItem(i);
				return;
			}
		}
	}
}

static void ManageEnergy(IExamInterface* pInterface)
{
	float maxEnergy{ 10.f };

	for (int i{}; i < pInterface->Inventory_GetCapacity(); ++i)
	{
		ItemInfo item{};
		if (pInterface->Inventory_GetItem(i, item) && item.Type == eItemType::FOOD)
		{
			if (pInterface->Agent_GetInfo().Energy + pInterface->Food_GetEnergy(item) <= maxEnergy)
			{
				pInterface->Inventory_UseItem(i);
				pInterface->Inventory_RemoveItem(i);
				return;
			}
		}
	}
}