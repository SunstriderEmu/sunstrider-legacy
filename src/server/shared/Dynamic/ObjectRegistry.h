
#ifndef TRINITY_OBJECTREGISTRY_H
#define TRINITY_OBJECTREGISTRY_H

#include "Define.h"

#include <string>
#include <map>
#include <vector>
#include <memory>

/** ObjectRegistry holds all registry item of the same type
 */
template<class T, class Key = std::string>
class ObjectRegistry final
{
    public:
        typedef std::map<Key, std::unique_ptr<T>> RegistryMapType;

        static ObjectRegistry<T, Key>* instance()
        {
            static ObjectRegistry<T, Key> instance;
            return &instance;
        }

        /// Returns a registry item
        T const* GetRegistryItem(Key const& key) const
        {
            auto itr = _registeredObjects.find(key);
            if (itr == _registeredObjects.end())
                return nullptr;
            return itr->second.get();
        }

        /// Inserts a registry item
        bool InsertItem(T* obj, Key const& key, bool force = false)
        {
            auto itr = _registeredObjects.find(key);
            if (itr != _registeredObjects.end())
            {
                if (!force)
                    return false;
                _registeredObjects.erase(itr);
            }

            _registeredObjects.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(obj));
            return true;
        }

        /// Returns true if registry contains an item
        bool HasItem(Key const& key) const
        {
            return (_registeredObjects.count(key) > 0);
        }

        /// Return the map of registered items
        RegistryMapType const& GetRegisteredItems() const
        {
            return _registeredObjects;
        }

private:
    RegistryMapType _registeredObjects;

    // non instanceable, only static
    ObjectRegistry() { }
    ~ObjectRegistry() { }
    ObjectRegistry(ObjectRegistry const&) = delete;
    ObjectRegistry& operator=(ObjectRegistry const&) = delete;
};

#endif
