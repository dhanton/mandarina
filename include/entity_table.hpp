#pragma once

#include <unordered_map>
#include <memory>

template<typename _Entity_Type>
class EntityTable
{
private:
    typedef std::unordered_map<u32, std::unique_ptr<_Entity_Type>> _Table;
    typedef std::pair<u32, std::unique_ptr<_Entity_Type>> _Pair;

public:
    //These custom iterators return second element of pair when referenced
    //which is very useful
    class iterator : public _Table::iterator
    {
    private:
        _Entity_Type* second;
    public:
        iterator() : _Table::iterator(), second(nullptr) {}
        iterator(typename _Table::iterator it) : _Table::iterator(it), second(it->second.get())  {}
        _Entity_Type& operator*()  {return *second;}
        _Entity_Type* operator->() {return second;}

        //I'm not sure this one should be used
        _Entity_Type* operator&()  {return second;}
    };
    class const_iterator : public _Table::const_iterator
    {
    private:
        _Entity_Type* second;
    public:
        const_iterator() : _Table::const_iterator(), second(nullptr) {}
        const_iterator(const typename _Table::const_iterator& it) : _Table::const_iterator(it), second(it->second.get())  {}
        const _Entity_Type& operator*()  const {return *second;}
        const _Entity_Type* operator->() const {return second;}
        const _Entity_Type* operator&()  const {return second;}
    };

    // typedef typename _Table::iterator iterator;
    // typedef typename _Table::const_iterator const_iterator;

public:
    _Entity_Type* atUniqueId(u32 uniqueId);
    const _Entity_Type* atUniqueId(u32 uniqueId) const;

    _Entity_Type* addEntity(_Entity_Type* entity);
    
    iterator removeEntity(u32 uniqueId);
    iterator removeEntity(iterator it);
    //should we add removeEntity that return const_iterator ??

    iterator begin();
    const_iterator begin() const;

    iterator end();
    const_iterator end() const;

private:
    _Table m_table;
};

#include "entity_table.inl"
