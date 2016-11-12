#ifndef __LAGWATCHER_H
#define __LAGWATCHER_H

/* 
Adapt server behavior depending on current "lag" or world update time
*/
class LagWatcher
{
public:
		static LagWatcher* instance()
        {
            static LagWatcher instance;
            return &instance;
        }

        void MapUpdateStart(Map const& map);
        void MapUpdateEnd(Map const& map);

        void Update();

private:
};

#define sLagWatcher LagWatcher::instance()

#endif // __LAGWATCHER_H

