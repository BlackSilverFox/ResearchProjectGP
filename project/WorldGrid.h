#pragma once
#include <vector>

#include "IExamInterface.h"
#include "Exam_HelperStructs.h"
#include "stdafx.h"

enum class CellInfo
{
	empty = 0,
	houseCenter = 1,
	food = 2,
	houseCenterAndFood = 3,
};

struct Cell
{
	Elite::Vector2 center;
	Elite::Vector2 dimensions;
	CellInfo cellInfo;
	bool visited;
};


class WorldGrid
{
public:
	WorldGrid(IExamInterface* pInterface, int nrOfCols);
	~WorldGrid() = default;

	WorldGrid(const WorldGrid& other) = delete;
	WorldGrid& operator=(const WorldGrid& other) = delete;
	WorldGrid(WorldGrid&& other) = delete;
	WorldGrid& operator=(WorldGrid&& other) = delete;

	void DrawGrid() const;
	inline bool AreAllCellsMapped() const;

	Elite::Vector2 GetNextTarget_Mapping(int distance = 1);

	void CalculateAgentCurrentCell();
	void ResetTargetCell();

private:
	WorldInfo m_WorldInfo;
	IExamInterface* m_pInterface;
	int m_Columns;

	int m_CurrentPathIndex;

	std::vector<Cell> m_Cells;

	int m_VisitedCellNr;
	int m_CurrentCellIndex;
	int m_TargetCellIndex;

	bool IsInsideTargetCell() const;
	void OnCellExit();
};