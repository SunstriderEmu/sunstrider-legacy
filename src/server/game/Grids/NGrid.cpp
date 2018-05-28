#include "NGrid.h"
#include "Random.h"

GridInfo::GridInfo()
		: i_timer(0), vis_Update(0, irand(0, DEFAULT_VISIBILITY_NOTIFY_PERIOD)),
		i_unloadActiveLockCount(0), i_unloadExplicitLock(false), i_unloadReferenceLock(false) 
{ }

GridInfo::GridInfo(time_t expiry, bool unload /*= true*/)
		: i_timer(expiry), vis_Update(0, irand(0, DEFAULT_VISIBILITY_NOTIFY_PERIOD)),
		i_unloadActiveLockCount(0), i_unloadExplicitLock(!unload), i_unloadReferenceLock(false) 
{ }
