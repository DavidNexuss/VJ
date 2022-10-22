#include "resource.hpp"

void IResource::signalUpdate() { acquisitionCount++; }
int IResource::updateCount() { return acquisitionCount; }
