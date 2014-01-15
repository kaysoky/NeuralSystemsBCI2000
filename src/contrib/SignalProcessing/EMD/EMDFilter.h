#ifndef EMDFILTER_H
#define EMDFILTER_H

#include "WindowingFilter.h"
#include "EMDThread.h"
#include "FilterCombination.h"

struct EMDFilter : public ThreadedFilter<EMDThread> { };

#endif // EMDFILTER_H