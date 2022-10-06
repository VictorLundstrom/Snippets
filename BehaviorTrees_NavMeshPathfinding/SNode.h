#pragma once
#include "SPoint.h"
#include <tgCV3D.h>

#include	<tgMemoryDisable.h>
#include	<vector>
#include	<tgMemoryEnable.h>


struct SNode
{
   SPoint p1; //1st Vertex Pos in the triangle and keeps track of it's parent
   SPoint p2; //2nd Vertex Pos in the triangle and keeps track of it's parent
   SPoint p3; //3rd Vertex Pos in the triangle and keeps track of it's parent

   tgUInt32 id; //Unique ID

   std::vector<SNode*> neighbours; //A list of other SNodes that are neighbours to this node

   tgBool operator==(SNode& rNode) //When comparing nodes, if any Vertex position is shared with the compared Node, it will return true as in it is a neighbour.
   {
      return (  (p1.pos == rNode.p1.pos || p1.pos == rNode.p2.pos || p1.pos == rNode.p3.pos) || (p2.pos == rNode.p1.pos || p2.pos == rNode.p2.pos || p2.pos == rNode.p3.pos ) || (p3.pos == rNode.p1.pos || p3.pos == rNode.p2.pos || p3.pos == rNode.p3.pos));
   }

   SNode(tgCV3D & P1, tgCV3D & P2, tgCV3D & P3, tgUInt32 ID) //Constructor
   {
      p1.pos = P1;
      p2.pos = P2;
      p3.pos = P3;

      id = ID;

      p1.triangle = this;
      p2.triangle = this;
      p3.triangle = this;

   }

   SPoint* winner = NULL;

};	// SNode