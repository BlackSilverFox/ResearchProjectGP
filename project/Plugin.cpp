#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include <iostream>
#include <random>
#include "Utils.h"

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//--------
	//GENERAL
	//--------
	srand(unsigned int(time(nullptr)));
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	info.BotName = "Dumb-o";
	info.Student_FirstName = "Judith";
	info.Student_LastName = "Verdonck";
	info.Student_Class = "2DAE02";
	
	//---------
	//STEERING
	//---------
	m_pWander = new Wander{ m_pInterface };
	static_cast<Wander*>(m_pWander)->SetRandomAngleMax(90.f);

	m_pSeek = new Seek{ m_pInterface };

	m_pFace = new Face{};

	//-----------
	//NAVIGATION
	//-----------
	m_pWorldGrid = new WorldGrid{ m_pInterface, 15 };
	
	//=============================================================
	//DECISIONMAKING
	//=============================================================
	Elite::Blackboard* pBlackboard{ CreateBlackboard(&(m_pInterface->Agent_GetInfo()))};

	//--------------
	//BEHAVIOR TREE
	//--------------
	m_pBT_ItemHouse = new Elite::BehaviorTree{ pBlackboard,
		new Elite::BehaviorSelector{
			{
				new Elite::BehaviorSelector//Deals with items
				{{
					new Elite::BehaviorSequence
					{{
						new Elite::BehaviorConditional{CanGrabItem_Condition},
						new Elite::BehaviorAction{GrabItem_Action}
					}}
					, new Elite::BehaviorSequence
					{{
						new Elite::BehaviorConditional{ItemInFOV_Condition},
						new Elite::BehaviorAction{GoToItem_Action},
					}}
				}}
				, new Elite::BehaviorSequence//Deals with searching inside a house
				{{
					new Elite::BehaviorConditional{IsInsideUnvisitedHouse_Condition},
					new Elite::BehaviorAction{SearchHouse_Action}
				}}
				, new Elite::BehaviorSequence//Deals with getting into a house
				{{
					new Elite::BehaviorConditional{UnvisitedHouseInFOV_Condition},
					new Elite::BehaviorAction{ChangeToSeek_Action}
				}}
				, new Elite::BehaviorAction{ChangeToWander_Action}
			}
		}
	};

	m_pBT_Enemy = new Elite::BehaviorTree{ pBlackboard,

		new Elite::BehaviorSelector
		{{
			new Elite::BehaviorSequence // pistol, fight
			{{
				new Elite::BehaviorConditional{HasPistol_Condition}
				, new Elite::BehaviorAction{ShootEnemy_Action}
			}}				
			, new Elite::BehaviorSequence // no pistol, flee
			{{
				new Elite::BehaviorAction{SetToFlee_Action}
				, new Elite::BehaviorSelector
				{{
					new Elite::BehaviorSequence//Deals with searching inside a house
					{{
						new Elite::BehaviorConditional{IsInsideUnvisitedHouse_Condition},
						new Elite::BehaviorAction{SearchHouse_Action}
					}}
					, new Elite::BehaviorSequence//Deals with getting into a house
					{{
						new Elite::BehaviorConditional{UnvisitedHouseInFOV_Condition},
						new Elite::BehaviorAction{ChangeToSeek_Action}
					}}
				}}
				, new Elite::BehaviorSequence // still try and grab items when passing over them
				{{
					new Elite::BehaviorConditional{CanGrabItem_Condition},
					new Elite::BehaviorAction{GrabItem_Action}
				}}
			}}
		}}
	};

	//---------------------
	//FINITE STATE MACHINE
	//---------------------
	//STATES
	Explore_State* pExplore_State{ new Explore_State{ m_pWorldGrid } };
	Search_State* pSearch_State{ new Search_State{ m_pBT_ItemHouse} };
	DealWithEnemy_State* pDealWithEnemy_State{ new DealWithEnemy_State{ m_pBT_Enemy, m_pWorldGrid } };
	m_pStates.push_back(pExplore_State);
	m_pStates.push_back(pSearch_State);
	m_pStates.push_back(pDealWithEnemy_State);

	//TRANSITIONS
	EnemySpotted_Transition* pEnemySpotted_Transition{ new EnemySpotted_Transition{} };
	ItemHouseSpotted_Transition* pItemHouseSpotted_Transition{ new ItemHouseSpotted_Transition{} };
	NoneSpotted_Transition* pNoneSpotted_Transition{ new NoneSpotted_Transition{} };
	m_pTransitions.push_back(pEnemySpotted_Transition);
	m_pTransitions.push_back(pItemHouseSpotted_Transition);
	m_pTransitions.push_back(pNoneSpotted_Transition);

	m_pFSM = new Elite::FiniteStateMachine{ pExplore_State, pBlackboard };
	
	//TRANSITION LOGIC
	//exploring to searching and back
	m_pFSM->AddTransition(pExplore_State, pSearch_State, pItemHouseSpotted_Transition);
	m_pFSM->AddTransition(pSearch_State, pExplore_State, pNoneSpotted_Transition);
	//exploring to deal with enemy and back
	m_pFSM->AddTransition(pExplore_State, pDealWithEnemy_State, pEnemySpotted_Transition);
	m_pFSM->AddTransition(pDealWithEnemy_State, pExplore_State, pNoneSpotted_Transition);
	//search to deal with enemy and back
	m_pFSM->AddTransition(pSearch_State, pDealWithEnemy_State, pEnemySpotted_Transition);
	m_pFSM->AddTransition(pDealWithEnemy_State, pSearch_State, pItemHouseSpotted_Transition);
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//steering behaviors
	delete m_pWander;
	delete m_pSeek;
	delete m_pFace;

	//trees
	//delete m_pBT_Enemy;
	//delete m_pBT_ItemHouse;

	//worldgrid
	delete m_pWorldGrid;

	//FSM
	//root
	delete m_pFSM;

	//states
	for (Elite::FSMState* state : m_pStates)
	{
		delete state;
	}

	//transitions
	for (Elite::FSMTransition* transition : m_pTransitions)
	{
		delete transition;
	}
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = false; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	//GENERAL
	m_pWorldGrid->CalculateAgentCurrentCell();
	m_pFSM->Update(dt);
	ManageHealth(m_pInterface);
	ManageEnergy(m_pInterface);

	//GET DATA
	SteeringPlugin_Output steeringOutput{};
	Elite::Vector2 target{};
	Elite::Vector2 lookAt{};
	SteeringBehavior steering{};
	std::vector<Elite::Vector2> visitedHouses{};
	Elite::Blackboard* pBlackboard{ m_pFSM->GetBlackboard() };

	bool dataAvailable{ pBlackboard->GetData("Target", target)
						&& pBlackboard->GetData("LookAt", lookAt)
						&& pBlackboard->GetData("Steering", steering)
						&& pBlackboard->GetData("VisitedHouses", visitedHouses)};

	if (!dataAvailable)
	{
		std::cout << "Target/Steering not found in blackboard, Plugin, UpdateSteering\n";
		return steeringOutput;
	}


	EvadePurgezone(target, m_pInterface);

	//AI HOUSE MEMORY
	if (visitedHouses.size() > 3) //as items respawn, revisit houses after finding some other houses
	{
		visitedHouses.front() = std::move(visitedHouses.back());
		visitedHouses.pop_back();
	}

	pBlackboard->ChangeData("VisitedHouses", visitedHouses);

	//STEERING
	switch (steering)
	{
	case SteeringBehavior::wander:
		steeringOutput = m_pWander->CalculateSteering(dt, m_pInterface->Agent_GetInfo());
		steeringOutput.RunMode = false;
		steeringOutput.AutoOrient = true;
		break;
	case SteeringBehavior::seek:
		m_pSeek->SetTarget(target);
		steeringOutput = m_pSeek->CalculateSteering(dt, m_pInterface->Agent_GetInfo());
		steeringOutput.RunMode = false;
		steeringOutput.AutoOrient = true;
		break;
	case SteeringBehavior::flee:
		m_pSeek->SetTarget(target);
		steeringOutput = m_pSeek->CalculateSteering(dt, m_pInterface->Agent_GetInfo());
		steeringOutput.RunMode = true;
		steeringOutput.AutoOrient = true;
		break;
	case SteeringBehavior::shoot:
		m_pSeek->SetTarget(target);
		static_cast<Face*>(m_pFace)->SetLookAt(lookAt);
		steeringOutput = m_pSeek->CalculateSteering(dt, m_pInterface->Agent_GetInfo());
		steeringOutput.AngularVelocity = m_pFace->CalculateSteering(dt, m_pInterface->Agent_GetInfo()).AngularVelocity;
		steeringOutput.RunMode = false;
		steeringOutput.AutoOrient = false;
		break;
	}

	m_pInterface->Draw_SolidCircle(target, .7f, { 0,0 }, { 1, 0, 0 });

	return steeringOutput;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	m_pWorldGrid->DrawGrid();
	
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
	m_pInterface->Draw_Direction(m_pInterface->Agent_GetInfo().Position, m_pInterface->Agent_GetInfo().LinearVelocity, 5.f, { 1.f, 0.f, 0.f } );
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}


Elite::Blackboard* Plugin::CreateBlackboard(AgentInfo* pAgentInfo) const
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("Interface", m_pInterface);
	pBlackboard->AddData("Target", Elite::Vector2{});	
	pBlackboard->AddData("Steering", SteeringBehavior::wander);

	pBlackboard->AddData("VisitedHouses", std::vector<Elite::Vector2>{});
	pBlackboard->AddData("SearchingHouseInfo", std::pair<HouseCorner, int>{});
	pBlackboard->AddData("CurrentHouse", HouseInfo{});
	pBlackboard->AddData("HasTargetedHouse", bool{});
	
	pBlackboard->AddData("CurrentItem", EntityInfo{});
	pBlackboard->AddData("HasTargetedItem", bool{});
	
	pBlackboard->AddData("EnemySpotted", bool{false});
	pBlackboard->AddData("LookAt", Elite::Vector2{});

	return pBlackboard;
}