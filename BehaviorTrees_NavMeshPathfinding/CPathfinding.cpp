#include	<tgSystem.h>
#include "CPathfinding.h"

#include	"Managers/CWorldManager.h"
#include <tgCDebugManager.h>
#include <tgLog.h>
#include <tgCLine3D.h>
#include <tgCSphere.h>

#include <tgMemoryDisable.h>
#include <algorithm>
#include <tgMemoryEnable.h>

CPathfinding::CPathfinding(tgCV3D *pEnemyPos) :
	m_pEnemyPos(pEnemyPos),
	m_pStartNode(NULL),
	m_pGoalNode(NULL),
	m_pCurrNode(NULL),
	m_pPointInList(NULL),
	m_pCurrPoint(NULL),
	m_Result(EResult::PATH_NOT_FOUND),
	m_pNavMeshWorld(NULL)
{

	// Setup CreateFlags
	const tgUInt32	CreateFlagsWorld = tgCMesh::CREATEFLAG_VERTEXARRAY | tgCMesh::CREATEFLAG_INDEXARRAY;

	// Load NavMeshWorld
	m_pNavMeshWorld = CWorldManager::GetInstance().LoadWorld("worlds/NavMeshEditedV2.tfw", "NavMesh", CreateFlagsWorld);

	tgCMesh& rMesh = m_pNavMeshWorld->GetSector(0)->pMeshArray[0];

	const tgUInt32 NumTriangles = rMesh.GetNumTotalTriangles();
	tgCMesh::SVertex *pVertexArr = rMesh.GetVertexArray();
	tgUInt32 *pIndexArray = rMesh.GetIndexArray();
	
	for (tgUInt32 i = 0; i < NumTriangles * 3; i+=3) //Create all nodes by assigning a Triangle from the Navmesh for each node
	{
		tgCV3D &p1 = pVertexArr[pIndexArray[i + 0]].Position;
		tgCV3D &p2 = pVertexArr[pIndexArray[i + 1]].Position;
		tgCV3D &p3 = pVertexArr[pIndexArray[i + 2]].Position;

		SNode* pNode = new SNode(p1, p2, p3, i);
		m_NodeList.push_back(pNode);
	}

	for (SNode *pNode : m_NodeList) //Find and Add neighbours to each node
	{
		for (tgUInt32 nodeIndex = 0; nodeIndex < m_NodeList.size(); nodeIndex++)
		{
			if (*pNode == *m_NodeList[nodeIndex] && pNode->id != m_NodeList[nodeIndex]->id) //If node has a shared vertex with other node AND it's not the same node.
			{
				pNode->neighbours.push_back(m_NodeList[nodeIndex]);
			}
		}
	}

	//Specified Goals to choose from
	const tgCV3D Goal1 = tgCV3D(14.0f, 0.03f, 13.0f);
	const tgCV3D Goal2 = tgCV3D(39.0f, 3.18f, 13.64f);
	const tgCV3D Goal3 = tgCV3D(22.39f, 0.06f, -17.10f);
	m_PatrolPoints.push_back(Goal1);
	m_PatrolPoints.push_back(Goal2);
	m_PatrolPoints.push_back(Goal3);
}

CPathfinding::~CPathfinding(void)
{
	for (SNode *pNode : m_NodeList)
	{
		delete pNode;
	}
	m_NodeList.clear();

	CWorldManager::GetInstance().DestroyWorld(m_pNavMeshWorld);
}

void CPathfinding::Render()
{
	RenderNavMesh();
	RenderPath();
}

CPathfinding::EResult CPathfinding::FindPath()
{
	//Setup StartPoint and add to Closed list
	m_Start.GCost = 0;
	m_Start.HCost = (m_Goal.pos - m_Start.pos).Length();
	m_Start.FCost = m_Start.GCost + m_Start.HCost;

	m_ClosedList.push_back(&m_Start);

	//Set StartPoint as parent to all vertices of the triangle that StartPoint is contained in.
	//Add the triangle's vertices to Open list
	m_pStartNode->p1.parent = &m_Start;
	m_OpenList.push_back(&m_pStartNode->p1);

	m_pStartNode->p2.parent = &m_Start;
	m_OpenList.push_back(&m_pStartNode->p2);

	m_pStartNode->p3.parent = &m_Start;
	m_OpenList.push_back(&m_pStartNode->p3);

	//Select Vertex with lowest F Cost and set it as CurrPoint
	m_pCurrPoint = LowestInitFCost(&m_pStartNode->p1, &m_pStartNode->p2, &m_pStartNode->p3);

	//Set current Node(Triangle) to Startnode
	m_pCurrNode = m_pStartNode;

	//Add CurrPoint to the closed list and remove it from Open List
	m_ClosedList.push_back(m_pCurrPoint);
	RemoveFromOpenList(m_OpenList, m_pCurrPoint);
	
	///////////////////////////////////////////////////////////////////////////

	while (!m_ClosedList.empty()) //While our currPoint is not in the Goal Triangle
	{
		//For each Neighboring Triangle of our CurrNode
		for (SNode * pNeighbour : m_pCurrNode->neighbours)
		{
			if (pNeighbour->p1.pos == m_pCurrPoint->pos || pNeighbour->p2.pos == m_pCurrPoint->pos || pNeighbour->p3.pos == m_pCurrPoint->pos) //If this neighbouring triangle shares the "currPoint"
			{
				if (!InClosedList(m_ClosedList, &pNeighbour->p1))
				{
					if (!InOpenList(m_OpenList, &pNeighbour->p1)) //If Neighbours 1st vertex is NOT in the open list
						AddToOpenList(m_OpenList, &pNeighbour->p1, m_pCurrPoint); //Add 1st vertex to open list and set parent of that Vertex to our currPoint.
					else
						AlreadyInOpenList(m_pCurrPoint); //Re-calculate G Cost using currPoint. If this would yield a cheaper path, update G Cost and set parent to currPoint.
				}

				////////////////////////////////////////////////

				if (!InClosedList(m_ClosedList, &pNeighbour->p2))
				{
					if (!InOpenList(m_OpenList, &pNeighbour->p2)) //^
						AddToOpenList(m_OpenList, &pNeighbour->p2, m_pCurrPoint); //^
					else
						AlreadyInOpenList(m_pCurrPoint); //^
				}

				////////////////////////////////////////////////

				if (!InClosedList(m_ClosedList, &pNeighbour->p3))
				{
					if (!InOpenList(m_OpenList, &pNeighbour->p3)) //^
						AddToOpenList(m_OpenList, &pNeighbour->p3, m_pCurrPoint); //^
					else
						AlreadyInOpenList(m_pCurrPoint); //^
				}

			}
		}

		//////////////////////////////////////////////////////////

		m_pCurrPoint = LowestFCostOpenList(m_OpenList); //Set our current point to our Lowest F Cost point in the open list
		m_pCurrNode = m_pCurrPoint->triangle; //Move to other Triangle, and set that triangle as our currentNode

		m_ClosedList.push_back(m_pCurrPoint); //Add current point to closed list
		RemoveFromOpenList(m_OpenList, m_pCurrPoint); //remove current point from open list


		if (m_pCurrPoint->pos == m_pGoalNode->p1.pos || m_pCurrPoint->pos == m_pGoalNode->p2.pos || m_pCurrPoint->pos == m_pGoalNode->p3.pos) //If our current point is inside the goal Triangle
		{
			m_Goal.parent = m_pCurrPoint;
			break;
		}

	}
	///////////////////////////////////////////////

	SPoint* pCurrent = &m_Goal; //Traverse from Goal to Start using their parent and create the Path
	while (pCurrent != &m_Start)
	{
		m_Path.push_back(pCurrent);
		pCurrent = pCurrent->parent;

		if (pCurrent == NULL)
			return EResult::PATH_NOT_FOUND;
	}

	const tgBool FoundPath = (!m_Path.empty()) && (m_Path.front() == &m_Goal);

	Reverse(); //Reverse the path so we can use it for the AI Agent

	SimplifyPath();//Smooth out path skipping unnecessary Points

	tgLogMessage("AstarSolver %i \n", m_ClosedList.size());

	if (FoundPath)
		return EResult::PATH_FOUND;
	else
		return EResult::PATH_NOT_FOUND;
	
}

void CPathfinding::RenderNavMesh()
{
	// Render red debug triangles for Navmesh
	const tgCV3D z_fighting(0.0f, 0.1f, 0.0f);

	for (SNode * pNode : m_NodeList)
	{
		const tgCV3D a = pNode->p1.pos + z_fighting;
		const tgCV3D b = pNode->p2.pos + z_fighting;
		const tgCV3D c = pNode->p3.pos + z_fighting;

		const tgCTriangle3D debugTriangle(a, b, c);
		tgCDebugManager::GetInstance().AddTriangle3D(debugTriangle, tgCColor::Red);
	}
}

void CPathfinding::RenderPath()
{
	const tgCSphere startSphere = tgCSphere(m_Start.pos, 0.3f);
	tgCDebugManager::GetInstance().AddLineSphere(startSphere, tgCColor::Green);

	for (SPoint * pPoint : m_Path)
	{
		const tgCSphere pathSphere = tgCSphere(pPoint->pos, 0.3f);

		if (pPoint->pos == m_Goal.pos)
		{
			tgCDebugManager::GetInstance().AddLineSphere(pathSphere, tgCColor::Black);
			continue;
		}

		tgCDebugManager::GetInstance().AddLineSphere(pathSphere, tgCColor::Blue);
	}
}

void CPathfinding::SimplifyPath()
{
	//Set the "next" variable for all Points
	for (tgUInt32 i = 0; i < m_Path.size() - 1; i++) // Minus 1 because our "Goal->next" should be NULL
	{
		m_Path[i]->next = m_Path[i + 1];
		m_Path[i]->id = i;
	}

	SPoint* pCheckPoint = m_Path.front();
	SPoint* pCurrentPoint = pCheckPoint->next;

	while (pCurrentPoint->next != NULL)
	{
		if (Walkable(pCheckPoint, pCurrentPoint->next))
		{
			//Make a straight path between those points
			SPoint* pTemp = pCurrentPoint;
			pCurrentPoint = pCurrentPoint->next;

			m_Path.erase(m_Path.begin() + pTemp->id); //Remove pTemp from path
			
			for (tgUInt32 i = pTemp->id; i < m_Path.size(); i++) //Update the ID-indices for the points in the path because we removed one element
			{
				m_Path[i]->id--;
			}
		}
		else 
		{
			pCheckPoint = pCurrentPoint;
			pCurrentPoint = pCurrentPoint->next;
		}

	}
}

bool CPathfinding::Walkable(SPoint* p1, SPoint* p2) //p1 == checkpoint, p2 == currentpoint->next
{
	std::vector<tgCV3D> InnerPoints;
	const float NumInnerPoints = 4;

	const float Percentage = 0.25f;
	const tgCV3D InnerPointOffset = (p2->pos - p1->pos) * Percentage;
	tgCV3D CurrPoint = p1->pos + InnerPointOffset;

	for (tgUInt32 i = 0; i < NumInnerPoints; i++)
	{
		InnerPoints.push_back(CurrPoint);
		CurrPoint += InnerPointOffset;
	}

	tgBool GoNextPoint = false;
	tgUInt32 NumValidPoints = 0;

	for (tgCV3D point : InnerPoints)
	{
		for (tgUInt32 j = 0; j < m_Path.size() -1; j++)
		{
			if (GoNextPoint)
			{
				GoNextPoint = false;
				break;
			}

			for (tgUInt32 k = 0; k < m_Path[j]->triangle->neighbours.size(); k++)
			{
				if (PointInsideTriangle(point, m_Path[j]->triangle->neighbours[k]->p1.pos, m_Path[j]->triangle->neighbours[k]->p2.pos, m_Path[j]->triangle->neighbours[k]->p3.pos)) //if the point is inside any of the path's neighbouring Triangles
				{
					GoNextPoint = true;
					NumValidPoints++;
					break;
				}
			}
		}
	}

	if (NumValidPoints == InnerPoints.size())
		return true;
	else
		return false;
}

void CPathfinding::FindStartAndGoalTriangles()
{
	//Find Start Node(Triangle)
	m_Start.pos = *m_pEnemyPos;
	for (SNode *pNode : m_NodeList)
	{
		if (PointInsideTriangle(m_Start.pos, pNode->p1.pos, pNode->p2.pos, pNode->p3.pos))
		{
			m_pStartNode = pNode;
			break;
		}
	}

	//Find Goal Node(Triangle)
	for (SNode *pNode : m_NodeList)
	{
		if (PointInsideTriangle(m_Goal.pos, pNode->p1.pos, pNode->p2.pos, pNode->p3.pos))
		{
			m_pGoalNode = pNode;
			break;
		}
	}
}

void CPathfinding::DeterminePath(const tgCV3D& rGoalPos)
{
	ResetVariables(); //Reset variables from last call

	m_Goal.pos = rGoalPos;

	FindStartAndGoalTriangles();

	//Case 1. Both StartPos and GoalPos are in the same triangle
			// This means that we can use a straight line as our path and for sure this is the shortest way. No further intensive calculations necessary.
	if (m_pStartNode->id == m_pGoalNode->id)
	{
		m_Path.push_back(&m_Goal);
		m_Result = EResult::PATH_FOUND;

		return;
	}

	//Case 2. One or both points are outside all triangles
			//Out of bounds. A point is outside the Navmesh.
	if (m_pStartNode == NULL || m_pGoalNode == NULL)
	{
		if (m_pStartNode == NULL)
			tgLogWarning("StartNode Out of bounds");
		if (m_pGoalNode == NULL)
			tgLogWarning("GoalNode Out of bounds");

		m_Result = EResult::PATH_NOT_FOUND;

		return;
	}

	//Case 3. Both points are located on different triangles
	m_Result = FindPath();

	////////////////////////////////////////////////////////

	switch (m_Result)
	{
	case EResult::PATH_FOUND:
		printf("Path was found!");
		break;
	case EResult::PATH_NOT_FOUND:
		printf("Path was NOT found!");
		break;
	}
}

void CPathfinding::ResetVariables()
{
	m_Result = EResult::PATH_NOT_FOUND;
	m_OpenList.clear();
	m_ClosedList.clear();
	m_Path.clear();
}

bool CPathfinding::PointInsideTriangle(tgCV3D p, tgCV3D a, tgCV3D b, tgCV3D c)
{
	if (SameSide(p, a, b, c) && SameSide(p, b, a, c) && SameSide(p, c, a, b))
		return true;
	else
		return false;
}

bool CPathfinding::SameSide(tgCV3D p1, tgCV3D p2, tgCV3D a, tgCV3D b)
{
	tgCV3D cp1;
	cp1.CrossProduct((b - a), (p1 - a));

	tgCV3D cp2;
	cp2.CrossProduct((b - a), (p2 - a));

	if (cp1.DotProduct(cp2) >= 0)
		return true;
	else
		return false;
}

SPoint* CPathfinding::LowestInitFCost(SPoint* pVertex1, SPoint* pVertex2, SPoint* pVertex3)
{
	//Calculate G,H and F cost for the 3 vertices in the start triangle
	SetInitPointCosts(&m_Start, &m_Goal, pVertex1);
	SetInitPointCosts(&m_Start, &m_Goal, pVertex2);
	SetInitPointCosts(&m_Start, &m_Goal, pVertex3);

	//Calculate and return the LOWEST F cost vertex out of the 3 vertices in the start triangle
	if (pVertex1->FCost <= pVertex2->FCost && pVertex1->FCost <= pVertex3->FCost)
		return pVertex1;
	else if (pVertex2->FCost <= pVertex1->FCost && pVertex2->FCost <= pVertex3->FCost)
		return pVertex2;
	else if (pVertex3->FCost <= pVertex1->FCost && pVertex3->FCost <= pVertex2->FCost)
		return pVertex3;
	
	tgLogError("Error at LowestInitFCost()");
	return false;

}

SPoint* CPathfinding::LowestFCost(SPoint* pVertex1, SPoint* pVertex2, SPoint* pVertex3)
{
	SetPointCosts(pVertex1->parent, &m_Goal, pVertex1);
	SetPointCosts(pVertex2->parent, &m_Goal, pVertex2);
	SetPointCosts(pVertex3->parent, &m_Goal, pVertex3);

	if (pVertex1->FCost <= pVertex2->FCost && pVertex1->FCost <= pVertex3->FCost)
		return pVertex1;
	else if (pVertex2->FCost <= pVertex1->FCost && pVertex2->FCost <= pVertex3->FCost)
		return pVertex2;
	else if (pVertex3->FCost <= pVertex1->FCost && pVertex3->FCost <= pVertex2->FCost)
		return pVertex3;
	
	tgLogError("Error at LowestFCost()");
	return false;

}

SPoint* CPathfinding::LowestFCostOpenList(std::vector<SPoint*>& rOpenList)
{
	SPoint* pWinner = rOpenList.front(); //Set winner as our first element in openList for our starting value

	for (SPoint *pPoint : rOpenList)
	{
		if (pPoint->FCost < pWinner->FCost)
			pWinner = pPoint;
	}

	if (pWinner == NULL) //Error check
	{ 
		tgLogError("winner was NULL in LowestFCostOpenList()");
		return false;
	}
	
	return pWinner;

}

void CPathfinding::SetInitPointCosts(const SPoint* pStartPoint, const SPoint* pGoalPoint, SPoint* pVertexPoint)
{
	pVertexPoint->GCost = (pVertexPoint->pos - pStartPoint->pos).Length();
	pVertexPoint->HCost = (pVertexPoint->pos - pGoalPoint->pos).Length();
	pVertexPoint->FCost = pVertexPoint->GCost + pVertexPoint->HCost;
}

void CPathfinding::SetPointCosts(const SPoint* pParentPoint, const SPoint* pGoalPoint, SPoint* pVertexPoint)
{
	pVertexPoint->GCost = pParentPoint->GCost + (pVertexPoint->pos - pParentPoint->pos).Length();
	pVertexPoint->HCost = (pVertexPoint->pos - pGoalPoint->pos).Length();
	pVertexPoint->FCost = pVertexPoint->GCost + pVertexPoint->HCost;
}

void CPathfinding::AlreadyInOpenList(SPoint* pCurrPoint)
{
	const float CurrentGCost = m_pPointInList->GCost;
	const float NewGCost = pCurrPoint->GCost + (pCurrPoint->pos - m_pPointInList->pos).Length();
	if (NewGCost < CurrentGCost)
	{
		m_pPointInList->GCost = NewGCost;
		m_pPointInList->parent = pCurrPoint;
	}
}

void CPathfinding::AddToOpenList(std::vector<SPoint*> &rOpenList, SPoint* pNeighbourPoint, SPoint* pCurrPoint)
{
	rOpenList.push_back(pNeighbourPoint);
	pNeighbourPoint->parent = pCurrPoint;
	SetPointCosts(pNeighbourPoint->parent, &m_Goal, pNeighbourPoint);
}

void CPathfinding::Reverse()
{
	std::reverse(m_Path.begin(), m_Path.end());
}

tgCV3D CPathfinding::RandomizeGoal()
{
	//Randomize Goal
	const tgUInt32 RandomizedGoalIndex = tgMathRand() % (tgUInt32)m_PatrolPoints.size();
	//Return Randomized GoalPos
	if (m_Goal.pos == m_PatrolPoints[RandomizedGoalIndex]) //If our Randomized goal is same as our previous goal, try to randomize again
		return RandomizeGoal();
	else
		return m_PatrolPoints[RandomizedGoalIndex];
}

void CPathfinding::RemoveFromOpenList(std::vector<SPoint*>& rOpenList, SPoint* pPoint)
{
	for (tgUInt32 i = 0; i < rOpenList.size(); i++)
	{
		if (rOpenList[i]->pos == pPoint->pos) //If this point exists in the open list
			rOpenList.erase(rOpenList.begin() + i);
	}
}

bool CPathfinding::InOpenList(std::vector<SPoint*>& rOpenList, SPoint* pInPoint)
{
	tgBool Result = false;

	for (SPoint *pPoint : rOpenList)
	{
		if (pPoint->pos == pInPoint->pos)
		{
			m_pPointInList = pPoint;
			Result = true;
			break;
		}
	}
	return Result;
}

bool CPathfinding::InClosedList(std::vector<SPoint*> &rClosedList, SPoint* pInPoint)
{
	tgBool Result = false;

	for (SPoint *pPoint : rClosedList)
	{
		if (pPoint->pos == pInPoint->pos)
		{
			Result = true;
			break;
		}
	}
	return Result;
}


