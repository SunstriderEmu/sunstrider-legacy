
#ifndef TRINITY_SINGLETON_H
#define TRINITY_SINGLETON_H

/**
 * @brief class Singleton
 */

#include "CreationPolicy.h"
#include "ThreadingModel.h"
#include "ObjectLifeTime.h"

namespace Trinity
{
    template
        <
        typename T,
        class ThreadingModel = Trinity::SingleThreaded<T>,
        class CreatePolicy = Trinity::OperatorNew<T>,
        class LifeTimePolicy = Trinity::ObjectLifeTime<T>
        >
        class Singleton
    {
        public:
            static T& Instance();

        protected:
            Singleton() = default;;

        private:

            // Prohibited actions...this does not prevent hijacking.
            Singleton(const Singleton &);
            Singleton& operator=(const Singleton &);

            // Singleton Helpers
            static void DestroySingleton();

            // data structure
            typedef typename ThreadingModel::Lock Guard;
            static T *si_instance;
            static bool si_destroyed;
    };
}
#endif

