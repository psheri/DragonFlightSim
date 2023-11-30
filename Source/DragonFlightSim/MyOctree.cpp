


#include "MyOctree.h"

void MyOctree::SetWorldBounds(FVector min, FVector max)
{
	this->WorldBounds = FBox(min, max);
}

MyOctree::MyOctree()
{
}

MyOctree::~MyOctree()
{
}
