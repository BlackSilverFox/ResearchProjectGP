#pragma once
#include "Exam_HelperStructs.h"
#include "stdafx.h"

enum
{
	INVALID = -1,
};

enum class HouseCorner
{
	NONE = -1,
	leftBottom = 0,
	rightBottom = 1,
	rightTop = 2,
	leftTop = 3,
};

enum class SteeringBehavior
{
	wander = 0,
	seek = 1,
	flee = 2,
	shoot = 3,
};

inline bool IsInsideBounds(const Elite::Vector2& point, const Elite::Vector2& boundsCenter, const Elite::Vector2& boundsDimensions)
{
	return point.x > (boundsCenter.x - boundsDimensions.x / 2)
		   && point.x < (boundsCenter.x + boundsDimensions.x / 2)
		   && point.y > (boundsCenter.y - boundsDimensions.y / 2)
		   && point.y < (boundsCenter.y + boundsDimensions.y / 2);
}

inline bool AgentIsCloseToPoint(const Elite::Vector2& agentPos, const Elite::Vector2& point)
{
	float margin{ 8.5f };

	return Elite::DistanceSquared(agentPos, point) < margin;
}

static void EvadePurgezone(Elite::Vector2& target, IExamInterface* pInterface)
{
	EntityInfo ei{};
	for (int i{};; ++i)
	{
		if (pInterface->Fov_GetEntityByIndex(i, ei))
		{
			if (ei.Type == eEntityType::PURGEZONE) break;
			else continue;
		}
		return;
	}

	Elite::Vector2 fleeVector{pInterface->Agent_GetInfo().Position - ei.Location};

	target = pInterface->Agent_GetInfo().Position + fleeVector;
}

struct Rectf
{
	Elite::Vector2 leftBottom{};
	Elite::Vector2 rightBottom{};
	Elite::Vector2 rightTop{};
	Elite::Vector2 leftTop{};

	Elite::Vector2 GetCornerByInt(int i) const
	{
		switch (i)
		{
		case 0:
			return leftBottom;
			break;
		case 1:
			return rightBottom;
			break;
		case 2:
			return rightTop;
			break;
		case 3:
			return leftTop;
			break;
		default:
			return leftBottom;
		}
	}
};

static Rectf MakeRect( const Elite::Vector2& center, const Elite::Vector2& dimensions)
{
	Rectf rect{};
	rect.leftBottom = center - dimensions / 2.f;
	rect.rightTop = center + dimensions / 2.f;
	rect.leftTop = { center.x - dimensions.x / 2.f, center.y + dimensions.y / 2.f };
	rect.rightBottom = { center.x + dimensions.x / 2.f, center.y - dimensions.y / 2.f };

	return rect;
}