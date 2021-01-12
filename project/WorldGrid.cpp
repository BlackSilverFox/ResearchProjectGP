#include "stdafx.h"
#include "WorldGrid.h"
#include "Utils.h"

WorldGrid::WorldGrid(IExamInterface* pInterface, int nrOfCols)
	:m_WorldInfo{ pInterface->World_GetInfo() }
	, m_pInterface{ pInterface }
	, m_Columns{ nrOfCols }
	, m_Cells{}
	, m_CurrentPathIndex{0}
	, m_CurrentCellIndex{(m_Columns * m_Columns) / 2}
	, m_TargetCellIndex{}
	, m_VisitedCellNr{ 0 }
{
	m_TargetCellIndex = m_CurrentCellIndex;

	int nrOfCells = Elite::Square(m_Columns);
	m_Cells.reserve(nrOfCells);


	m_WorldInfo.Dimensions.x -= 10.f;
	m_WorldInfo.Dimensions.y -= 10.f;
	float cellWidth{ m_WorldInfo.Dimensions.x / m_Columns };
	float halfCellWidth{ cellWidth / 2.f };

	for (int i{}; i < nrOfCells; ++i)
	{
		Cell cell{};
		cell.center.x = (m_WorldInfo.Center.x + m_WorldInfo.Dimensions.x / 2.f ) - (halfCellWidth + (i % m_Columns) * cellWidth);
		cell.center.y = (m_WorldInfo.Center.y + m_WorldInfo.Dimensions.y / 2.f ) - (halfCellWidth + (i / m_Columns) * cellWidth);
		cell.dimensions = { cellWidth, cellWidth };
		cell.cellInfo = CellInfo::empty;

		m_Cells.push_back(cell);
	}
}

void WorldGrid::DrawGrid() const
{
	for (const Cell& cell : m_Cells)
	{
		Elite::Vector2 cellPoints[4]{};
		cellPoints[0] = {cell.center.x - cell.dimensions.x/2.f, cell.center.y - cell.dimensions.y/2.f};
		cellPoints[1] = {cell.center.x + cell.dimensions.x/2.f, cell.center.y - cell.dimensions.y/2.f};
		cellPoints[2] = {cell.center.x + cell.dimensions.x/2.f, cell.center.y + cell.dimensions.y/2.f};
		cellPoints[3] = {cell.center.x - cell.dimensions.x/2.f, cell.center.y + cell.dimensions.y/2.f};
		
		m_pInterface->Draw_Polygon(&cellPoints[0], 4, { 1.f, 0.f, 0.f });

		if (cell.visited)
		{
			m_pInterface->Draw_SolidCircle(cell.center, 1.f, { 0,0 }, { 0.f, 0.f, 1.f });
		}
		else
		{
			m_pInterface->Draw_SolidCircle(cell.center, 1.f, { 0,0 }, { 1.f, 1.f, 1.f });
		}
	}

	m_pInterface->Draw_Circle(m_Cells[m_TargetCellIndex].center, 3.f, { 1.f, 1.f, 0.f });
}

bool WorldGrid::AreAllCellsMapped() const
{
	return m_VisitedCellNr == Elite::Square(m_Columns);
}

void WorldGrid::CalculateAgentCurrentCell()
{
	if (IsInsideBounds(m_pInterface->Agent_GetInfo().Position, m_Cells[m_CurrentCellIndex].center, m_Cells[m_CurrentCellIndex].dimensions)) return;
	
	for (int i{}; i < Elite::Square(m_Columns); ++i)
	{
		if (IsInsideBounds(m_pInterface->Agent_GetInfo().Position, m_Cells[i].center, m_Cells[i].dimensions))
		{
			OnCellExit();
			m_CurrentCellIndex = i;
			return;
		}
	}
}

void WorldGrid::OnCellExit()
{
	m_Cells[m_CurrentCellIndex].visited = true;
	m_VisitedCellNr++;
}

Elite::Vector2 WorldGrid::GetNextTarget_Mapping(int distance)
{
	if (AreAllCellsMapped()) return { 0.f, 0.f }; // Safeguard against endless searching/crash on not finding any possibility
	if (!IsInsideTargetCell()) return m_Cells[m_TargetCellIndex].center; // do not recalc a target cell every frame

	std::vector<int> cellIndices{};
	
	//search in straight lines for unvisited cells
	int up{ m_CurrentCellIndex - distance * m_Columns };
	int down{ m_CurrentCellIndex + distance * m_Columns };
	int left{ m_CurrentCellIndex + distance };
	int right{ m_CurrentCellIndex - distance };
	
	//safeguard against out-of-bounds and jumping rows with left/right
	if (up < 0) up = INVALID;
	if (down > Elite::Square(m_Columns)-1) down = INVALID;
	if (left / m_Columns != m_CurrentCellIndex / m_Columns || left > Elite::Square(m_Columns) - 1) left = INVALID;
	if (right / m_Columns != m_CurrentCellIndex / m_Columns || right < 0) right = INVALID;

	//check for invalid and visited before adding index to possible indices
	if (!(up == INVALID || m_Cells[up].visited )) cellIndices.push_back(up);
	if (!(down == INVALID || m_Cells[down].visited)) cellIndices.push_back(down);
	if (!(left == INVALID || m_Cells[left].visited)) cellIndices.push_back(left);
	if (!(right == INVALID || m_Cells[right].visited)) cellIndices.push_back(right);

	//always try to go vertical/horizontal first before looking for diagonal ways
	if (cellIndices.size() > 0)
	{
		int randomIndex{ Elite::randomInt(cellIndices.size()) };
		m_TargetCellIndex = cellIndices[randomIndex];
		
		return m_Cells[m_TargetCellIndex].center;
	}

	//look diagonally
	for (int i{ distance }; i > 0; --i)
	{
		int topRight{ up + i };
		int topLeft{ up - i };
		int leftTop{ left - (i - 1) };
		int leftBottom{ left + (i - 1) };
		int bottomRight{ down + i };
		int bottomLeft{ down - i };
		int rightTop{ right - (i - 1) };
		int rightBottom{ right + (i - 1) };

		//TODO: change checks to search further then the second invalid checks
		if (up != INVALID)
		{
			if (right != INVALID && !m_Cells[topRight].visited) cellIndices.push_back(topRight);
			if (left != INVALID && !m_Cells[topLeft].visited) cellIndices.push_back(topLeft);
		}
		if (left != INVALID)
		{
			if (up != INVALID && leftTop != 0 && !m_Cells[leftTop].visited) cellIndices.push_back(leftTop);
			if (down != INVALID && leftBottom != 0 && !m_Cells[leftBottom].visited) cellIndices.push_back(leftBottom);
		}
		if (down != INVALID)
		{
			if (left != INVALID && !m_Cells[bottomLeft].visited) cellIndices.push_back(bottomLeft);
			if (right != INVALID && !m_Cells[bottomRight].visited) cellIndices.push_back(bottomRight);
		}
		if (right != INVALID)
		{
			if (up != INVALID && rightTop != 0 && !m_Cells[rightTop].visited) cellIndices.push_back(rightTop);
			if (down != INVALID && rightBottom != 0 && !m_Cells[rightBottom].visited) cellIndices.push_back(rightBottom);
		}
	}

	if (cellIndices.size() > 0)
	{
		int randomIndex{ Elite::randomInt(cellIndices.size()) };
		m_TargetCellIndex = cellIndices[randomIndex];

		return m_Cells[m_TargetCellIndex].center;
	}

	return GetNextTarget_Mapping(distance + 1);
}

bool WorldGrid::IsInsideTargetCell() const
{
	return m_CurrentCellIndex == m_TargetCellIndex;
}

void WorldGrid::ResetTargetCell()
{
	m_TargetCellIndex = m_CurrentCellIndex;
}