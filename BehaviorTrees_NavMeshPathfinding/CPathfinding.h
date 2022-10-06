#pragma once
#include "SNode.h"
#include "SPoint.h"

class CPathfinding
{
public:
	CPathfinding(tgCV3D* pEnemyPos);
	~CPathfinding(void);

	enum class EResult
	{
		PATH_FOUND,
		PATH_NOT_FOUND
	};

	//////////////////////////////////////////////////////////////////////////

	void	Render();

	void	DeterminePath(const tgCV3D &rGoalPos);
	tgCV3D RandomizeGoal();

	void ResetVariables();

	//GET AND SET//
	tgCWorld*	GetNavMeshWorld(void) const { return m_pNavMeshWorld; }
	std::vector<SPoint*> GetPath(void) const { return m_Path; }
	EResult GetResult(void) const { return m_Result; }
	void	SetResult(const EResult result) { m_Result = result; }
	SPoint* GetStart(void) { return &m_Start; }
	SPoint* GetGoal(void) { return &m_Goal; }

private:

	void SimplifyPath();
		bool Walkable(SPoint* p1, SPoint* p2);

	void FindStartAndGoalTriangles();
	EResult FindPath();

	void RenderNavMesh();
	void RenderPath();

	bool PointInsideTriangle(tgCV3D p, tgCV3D a, tgCV3D b, tgCV3D c); //p == AI Agent Position or Goal Position.  a,b,c == Triangle's vertices
	bool SameSide(tgCV3D p1, tgCV3D p2, tgCV3D a, tgCV3D b); //p1 == AI Agent Position or Goal Position.

	SPoint* LowestInitFCost(SPoint* pVertex1, SPoint* pVertex2, SPoint* pVertex3);
		void SetInitPointCosts(const SPoint* pStartPoint, const SPoint* pGoalPoint, SPoint* pVertexPoint);

	SPoint* LowestFCost(SPoint* pVertex1, SPoint* pVertex2, SPoint* pVertex3);
		void SetPointCosts(const SPoint* pParentPoint, const SPoint* pGoalPoint, SPoint* pVertexPoint);

	SPoint* LowestFCostOpenList(std::vector<SPoint*>& rOpenList);

	void AlreadyInOpenList(SPoint* pCurrPoint);
	void AddToOpenList(std::vector<SPoint*> &rOpenList, SPoint* pNeighbourPoint, SPoint* pCurrPoint);

	void Reverse();

	bool InOpenList(std::vector<SPoint*> &rOpenList, SPoint* pInPoint);
	bool InClosedList(std::vector<SPoint*> &rClosedList, SPoint* pInPoint);
	void RemoveFromOpenList(std::vector<SPoint*>& rOpenList, SPoint* pPoint);
	
	std::vector<SPoint*> m_OpenList;
	std::vector<SPoint*> m_ClosedList;
	SPoint m_Goal;
	SPoint m_Start;

	tgCV3D* m_pEnemyPos;
	SNode* m_pStartNode;
	SNode* m_pGoalNode;
	SNode* m_pCurrNode;

	SPoint* m_pPointInList;
	SPoint* m_pCurrPoint;

	EResult m_Result;
	tgCWorld* m_pNavMeshWorld;
	std::vector<SNode*> m_NodeList;
	std::vector<SPoint*> m_Path;
	std::vector<tgCV3D> m_PatrolPoints;
};

