#pragma once

namespace ai
{
    using namespace std;

    class Qualified
    {
    public:
        Qualified() {};

    public:
        void Qualify(std::string qualifier) { this->qualifier = qualifier; }

    protected:
        std::string qualifier;
    };

    template <class T> class NamedObjectFactory
    {
    protected:
        typedef T* (*ActionCreator) (PlayerbotAI* ai);
        map<string, ActionCreator> creators;

    public:
        T* create(std::string name, PlayerbotAI* ai)
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

            T *object = (*creator)(ai);
            Qualified *q = dynamic_cast<Qualified *>(object);
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

        T* create(std::string name, PlayerbotAI* ai)
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
            for (auto i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    delete i->second;
            }

            created.clear();
        }

        void Update()
        {
            for (typename map<string, T*>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    i->second->Update();
            }
        }

        void Reset()
        {
            for (typename map<string, T*>::iterator i = created.begin(); i != created.end(); i++)
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
        map<string, T*> created;
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

        T* GetObject(std::string name, PlayerbotAI* ai)
        {
            for (auto i = contexts.begin(); i != contexts.end(); i++)
            {
                T* object = (*i)->create(name, ai);
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
            for (auto i = factories.begin(); i != factories.end(); i++)
                delete *i;
        }

        void Add(NamedObjectFactory<T>* context)
        {
            factories.push_front(context);
        }

        T* GetObject(std::string name, PlayerbotAI* ai)
        {
            for (typename list<NamedObjectFactory<T>*>::iterator i = factories.begin(); i != factories.end(); i++)
            {
                T* object = (*i)->create(name, ai);
                if (object) return object;
            }
            return NULL;
        }

    private:
        list<NamedObjectFactory<T>*> factories;
    };
};
