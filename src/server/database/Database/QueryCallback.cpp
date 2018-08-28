/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "QueryCallback.h"
#include "Errors.h"

template<typename T, typename... Args>
inline void Construct(T& t, Args&&... args)
{
    new (&t) T(std::forward<Args>(args)...);
}

template<typename T>
inline void Destroy(T& t)
{
    t.~T();
}

template<typename T>
inline void ConstructActiveMember(T* obj)
{
    switch (obj->_type)
    {
    case QueryCallback::STRING_QUERY:   Construct(obj->_string); break;
    case QueryCallback::PREPARED_QUERY: Construct(obj->_prepared); break;
    case QueryCallback::TRANSACTION:    Construct(obj->_transaction); break;
    }
}

template<typename T>
inline void DestroyActiveMember(T* obj)
{
    switch (obj->_type)
    {
    case QueryCallback::STRING_QUERY:   Destroy(obj->_string); break;
    case QueryCallback::PREPARED_QUERY: Destroy(obj->_prepared); break;
    case QueryCallback::TRANSACTION:    Destroy(obj->_transaction); break;
    }
}

template<typename T>
inline void MoveFrom(T* to, T&& from)
{
    ASSERT(to->_type == from._type);

    switch (to->_type)
    {
    case QueryCallback::STRING_QUERY:   to->_string = std::move(from._string); break;
    case QueryCallback::PREPARED_QUERY: to->_prepared = std::move(from._prepared); break;
    case QueryCallback::TRANSACTION:    to->_transaction = std::move(from._transaction); break;
    }
}

struct QueryCallback::QueryCallbackData
{
public:
    friend class QueryCallback;

    QueryCallbackData(std::function<void(QueryCallback&, QueryResult)>&& callback) : _string(std::move(callback)), _type(STRING_QUERY) { }
    QueryCallbackData(std::function<void(QueryCallback&, PreparedQueryResult)>&& callback) : _prepared(std::move(callback)), _type(PREPARED_QUERY) { }
    QueryCallbackData(std::function<void(QueryCallback&)>&& callback) : _transaction(std::move(callback)), _type(TRANSACTION) { }
    QueryCallbackData(QueryCallbackData&& right)
    {
        _type = right._type;
        ConstructActiveMember(this);
        MoveFrom(this, std::move(right));
    }
    QueryCallbackData& operator=(QueryCallbackData&& right)
    {
        if (this != &right)
        {
            if (_type != right._type)
            {
                DestroyActiveMember(this);
                _type = right._type;
                ConstructActiveMember(this);
            }
            MoveFrom(this, std::move(right));
        }
        return *this;
    }
    ~QueryCallbackData() { DestroyActiveMember(this); }

private:
    QueryCallbackData(QueryCallbackData const&) = delete;
    QueryCallbackData& operator=(QueryCallbackData const&) = delete;

    template<typename T> friend void ConstructActiveMember(T* obj);
    template<typename T> friend void DestroyActiveMember(T* obj);
    template<typename T> friend void MoveFrom(T* to, T&& from);

    union
    {
        std::function<void(QueryCallback&, QueryResult)> _string;
        std::function<void(QueryCallback&, PreparedQueryResult)> _prepared;
        std::function<void(QueryCallback&)> _transaction;
    };
    QueryCallback::QueryType _type;
};

// Not using initialization lists to work around segmentation faults when compiling with clang without precompiled headers
QueryCallback::QueryCallback(QueryResultFuture&& result)
    : _type(STRING_QUERY)
{
    Construct(_string, std::move(result));
}

QueryCallback::QueryCallback(PreparedQueryResultFuture&& result)
    : _type(PREPARED_QUERY)
{
    Construct(_prepared, std::move(result));
}

QueryCallback::QueryCallback(TransactionCompleteFuture&& result)
    : _type(TRANSACTION)
{
    Construct(_transaction, std::move(result));
}

QueryCallback::QueryCallback(QueryCallback&& right)
{
    _type = right._type;
    ConstructActiveMember(this);
    MoveFrom(this, std::move(right));
    _callbacks = std::move(right._callbacks);
}

QueryCallback& QueryCallback::operator=(QueryCallback&& right)
{
    if (this != &right)
    {
        if (_type != right._type)
        {
            DestroyActiveMember(this);
            _type = right._type;
            ConstructActiveMember(this);
        }
        MoveFrom(this, std::move(right));
        _callbacks = std::move(right._callbacks);
    }
    return *this;
}

QueryCallback::~QueryCallback()
{
    DestroyActiveMember(this);
}

QueryCallback&& QueryCallback::WithCallback(std::function<void()>&& callback)
{
    return WithChainingCallback([callback](QueryCallback& /*this*/) { callback(); });
}

QueryCallback&& QueryCallback::WithCallback(std::function<void(QueryResult)>&& callback)
{
    return WithChainingCallback([callback](QueryCallback& /*this*/, QueryResult result) { callback(std::move(result)); });
}

QueryCallback&& QueryCallback::WithPreparedCallback(std::function<void(PreparedQueryResult)>&& callback)
{
    return WithChainingPreparedCallback([callback](QueryCallback& /*this*/, PreparedQueryResult result) { callback(std::move(result)); });
}

QueryCallback&& QueryCallback::WithChainingCallback(std::function<void(QueryCallback&)>&& callback)
{
    ASSERT(!_callbacks.empty() || _type == TRANSACTION, "Attempted to set callback function for transaction on a string or prepared async query");
    _callbacks.emplace(std::move(callback));
    return std::move(*this);
}

QueryCallback&& QueryCallback::WithChainingCallback(std::function<void(QueryCallback&, QueryResult)>&& callback)
{
    ASSERT(!_callbacks.empty() || _type == STRING_QUERY, "Attempted to set callback function for string query on a transaction or prepared async query");
    _callbacks.emplace(std::move(callback));
    return std::move(*this);
}

QueryCallback&& QueryCallback::WithChainingPreparedCallback(std::function<void(QueryCallback&, PreparedQueryResult)>&& callback)
{
    ASSERT(!_callbacks.empty() || _type == PREPARED_QUERY, "Attempted to set callback function for prepared query on a transaction or string async query");
    _callbacks.emplace(std::move(callback));
    return std::move(*this);
}

void QueryCallback::SetNextQuery(QueryCallback&& next)
{
    MoveFrom(this, std::move(next));
}

QueryCallback::Status QueryCallback::InvokeIfReady()
{
    QueryCallbackData& callback = _callbacks.front();
    auto checkStateAndReturnCompletion = [this]()
    {
        _callbacks.pop();
        bool hasNext = false;
        switch (_type)
        {
        case STRING_QUERY:   hasNext = _string.valid(); break;
        case PREPARED_QUERY: hasNext = _prepared.valid(); break;
        case TRANSACTION:    hasNext = _transaction.valid(); break;
        }
        if (_callbacks.empty())
        {
            ASSERT(!hasNext);
            return Completed;
        }

        // abort chain
        if (!hasNext)
            return Completed;

        ASSERT(_type == _callbacks.front()._type);
        return NextStep;
    };

    switch (_type)
    {
    case STRING_QUERY:   
        if (_string.valid() && _string.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            QueryResultFuture f(std::move(_string));
            std::function<void(QueryCallback&, QueryResult)> cb(std::move(callback._string));
            cb(*this, f.get());
            return checkStateAndReturnCompletion();
        }
        break;
    case PREPARED_QUERY: 
        if (_prepared.valid() && _prepared.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            PreparedQueryResultFuture f(std::move(_prepared));
            std::function<void(QueryCallback&, PreparedQueryResult)> cb(std::move(callback._prepared));
            cb(*this, f.get());
            return checkStateAndReturnCompletion();
        }
        break;
    case TRANSACTION: 
        if (_transaction.valid() && _transaction.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            TransactionCompleteFuture f(std::move(_transaction));
            std::function<void(QueryCallback&)> cb(std::move(callback._transaction));
            cb(*this);
            return checkStateAndReturnCompletion();
        }
        break;
    }

    return NotReady;
}
