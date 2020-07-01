#pragma once

#include <unordered_map>
#include <memory>

template<typename _Entity_Type>
class EntityTable
{
private:
    typedef std::unordered_map<u32, std::unique_ptr<_Entity_Type>> _Table;

public:
    //These custom iterators return second element of pair when referenced
    //which is very useful
    class iterator
    {
    private:
        friend class EntityTable<_Entity_Type>;
        typename _Table::iterator _internal_it;

    public:
        iterator() {}
        iterator(typename _Table::iterator _it) : _internal_it(_it)  {}

        _Entity_Type& operator*()  const {return *_internal_it->second.get();}
        _Entity_Type* operator->() const {return _internal_it->second.get();}
        _Entity_Type* operator&()  const {return _internal_it->second.get();}

        bool operator==(const iterator& rhs) const {return _internal_it == rhs._internal_it;}
        bool operator!=(const iterator& rhs) const {return _internal_it != rhs._internal_it;}

        iterator& operator++()   {++_internal_it; return *this;}
        iterator operator++(int) {iterator _copy_it(_internal_it); ++(*this); return _copy_it;}

        iterator& operator--()   {--_internal_it; return *this;}
        iterator operator--(int) {iterator _copy_it(_internal_it); --(*this); return _copy_it;}
    };

    class const_iterator
    {
    private:
        friend class EntityTable<_Entity_Type>;
        typename _Table::const_iterator _internal_it;
    public:
        const_iterator() {}
        const_iterator(const typename _Table::const_iterator& _it) : _internal_it(_it) {}

        const _Entity_Type& operator*()  const {return *_internal_it->second.get();}
        const _Entity_Type* operator->() const {return _internal_it->second.get();}
        const _Entity_Type* operator&()  const {return _internal_it->second.get();}

        bool operator==(const const_iterator& rhs) const {return _internal_it == rhs._internal_it;}
        bool operator!=(const const_iterator& rhs) const {return _internal_it != rhs._internal_it;}

        const_iterator& operator++()   {++_internal_it; return *this;}
        const_iterator operator++(int) {const_iterator _copy_it(_internal_it); ++(*this); return _copy_it;}

        const_iterator& operator--()   {--_internal_it; return *this;}
        const_iterator operator--(int) {const_iterator _copy_it(_internal_it); --(*this); return _copy_it;}
    };

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

    size_t size() const;

    void clear();

private:
    _Table m_table;
};

#include "entity_table.inl"
