#pragma once

#include <memory>

namespace ai
{
    using namespace std;

    class Qualified
    {
    public:
        Qualified() {};

    public:
        void Qualify(std::string _qualifier) { qualifier = _qualifier; }

    protected:
        std::string qualifier;
    };

    template <class T> class NamedObjectFactory
    {
    protected:
        typedef std::shared_ptr<T> (*ActionCreator) (PlayerbotAI* ai);
        map<string, ActionCreator> creators;

    public:
        std::shared_ptr<T> create(std::string name, PlayerbotAI* ai)
        {
            size_t found = name.find("::");
            std::string qualifier;
            if (found != std::string::npos)
            {
                qualifier = name.substr(found + 2);
                name = name.substr(0, found);
            }

            if (creators.find(name) == creators.end())
                return nullptr;

            ActionCreator creator = creators[name];
            if (!creator)
                return nullptr;

            std::shared_ptr<T> object = (*creator)(ai);
            std::shared_ptr<Qualified> q = std::dynamic_pointer_cast<Qualified>(object);
            if (q)
                q->Qualify(qualifier);

            return object;
        }

        set<std::string> supports()
        {
            set<std::string> keys;
            for (auto it = creators.begin(); it != creators.end(); it++)
                keys.insert(it->first);
            return keys;
        }
    };


    template <class T> class NamedObjectContext : public NamedObjectFactory<T>
    {
    public:
        NamedObjectContext(bool shared = false, bool supportsSiblings = false) :
            NamedObjectFactory<T>(), shared(shared), supportsSiblings(supportsSiblings) {}

        std::shared_ptr<T> create(std::string name, PlayerbotAI* ai)
        {
            if (created.find(name) == created.end())
                return created[name] = NamedObjectFactory<T>::create(name, ai);

            return created[name];
        }

        virtual ~NamedObjectContext()
        {
            Clear();
        }

        void Clear()
        {
            created.clear();
        }

        void Update()
        {
            for (typename map<string, std::shared_ptr<T>>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    i->second->Update();
            }
        }

        void Reset()
        {
            for (typename map<string, std::shared_ptr<T>>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    i->second->Reset();
            }
        }

        bool IsShared() { return shared; }
        bool IsSupportsSiblings() { return supportsSiblings; }

        set<std::string> GetCreated()
        {
            set<std::string> keys;
            for (auto it = created.begin(); it != created.end(); it++)
                keys.insert(it->first);
            return keys;
        }

    protected:
        map<string, std::shared_ptr<T>> created;
        bool shared;
        bool supportsSiblings;
    };

    template <class T> class NamedObjectContextList
    {
    public:
        virtual ~NamedObjectContextList()
        {
            for (auto i = contexts.begin(); i != contexts.end(); i++)
            {
                NamedObjectContext<T>* context = *i;
                if (!context->IsShared())
                    delete context;
            }
        }

        void Add(NamedObjectContext<T>* context)
        {
            contexts.push_back(context);
        }

        std::shared_ptr<T> GetObject(std::string name, PlayerbotAI* ai)
        {
            for (auto i = contexts.begin(); i != contexts.end(); i++)
            {
                std::shared_ptr<T> object = (*i)->create(name, ai);
                if (object) return object;
            }
            return nullptr;
        }

        void Update()
        {
            for (typename list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                if (!(*i)->IsShared())
                    (*i)->Update();
            }
        }

        void Reset()
        {
            for (typename list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                (*i)->Reset();
            }
        }

        set<std::string> GetSiblings(std::string name)
        {
            for (auto i = contexts.begin(); i != contexts.end(); i++)
            {
                if (!(*i)->IsSupportsSiblings())
                    continue;

                set<std::string> supported = (*i)->supports();
                auto found = supported.find(name);
                if (found == supported.end())
                    continue;

                supported.erase(found);
                return supported;
            }

            return set<std::string>();
        }

        set<std::string> supports()
        {
            set<std::string> result;

            for (auto i = contexts.begin(); i != contexts.end(); i++)
            {
                set<std::string> supported = (*i)->supports();

                for (const auto & j : supported)
                    result.insert(j);
            }
            return result;
        }

        set<std::string> GetCreated()
        {
            set<std::string> result;

            for (auto i = contexts.begin(); i != contexts.end(); i++)
            {
                set<std::string> createdKeys = (*i)->GetCreated();

                for (const auto & createdKey : createdKeys)
                    result.insert(createdKey);
            }
            return result;
        }

    private:
        list<NamedObjectContext<T>*> contexts;
    };

    template <class T> class NamedObjectFactoryList
    {
    public:
        virtual ~NamedObjectFactoryList()
        {
        }

        void Add(std::unique_ptr<NamedObjectFactory<T>>&& context)
        {
            factories.push_front(std::move(context));
        }

        std::shared_ptr<T> GetObject(std::string name, PlayerbotAI* ai)
        {
            for (typename list<std::unique_ptr<NamedObjectFactory<T>>>::iterator i = factories.begin(); i != factories.end(); i++)
            {
                std::shared_ptr<T> object = (*i)->create(name, ai);
                if (object) 
                    return object;
            }
            return nullptr;
        }

    private:
        list<std::unique_ptr<NamedObjectFactory<T>>> factories;
    };
};
