#pragma once
#include <tgCV3D.h>

struct SNode;

struct SPoint
{
	tgCV3D pos = tgCV3D (tgCV3D::Zero);
	tgFloat GCost = TG_FLOAT_MAX;
	tgFloat HCost = TG_FLOAT_MAX;
	tgFloat FCost = TG_FLOAT_MAX;
	SPoint* parent = NULL;
	SNode* triangle = NULL;

	SPoint* next = NULL;
	tgUInt32 id;
};