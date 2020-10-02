#pragma once

/**
 * Based on:
 *      LooseQuadtree written by Zozo (https://github.com/gerazo/loose_quadtree, MIT License)
 *
 * Modified by:
 *      Dhanton (dhantonrevolution@gmail.com)
 *      To include circles and rotating rects to the tree.
 * 
 *      Done for Mandarina, a multiplayer game by Dhanton.
 */

#include <array>
#include <cassert>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <limits>
#include <map>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <vector>

#include <SFML/Graphics/Rect.hpp>

#include "helper.hpp"
#include "bounding_body.hpp"

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
class Quadtree {
public:
    using Number = NumberT;
    using Object = ObjectT;
    using BoundingBoxExtractor = BoundingBoxExtractorT;

private:
    class Impl;

public:
    class Query {
    public:
        ~Query();
        Query() = delete;
        Query(const Query&) = delete;
        Query& operator=(const Query&) = delete;
        Query(Query&&);
        Query& operator=(Query&&);

        bool EndOfQuery() const;
        Object* GetCurrent() const;
        void Next();

    private:
        friend class Quadtree<Number, Object, BoundingBoxExtractor>::Impl;
        class Impl;
        Query(Impl* pimpl);
        Impl* pimpl_;
    };

    Quadtree() {}
    ~Quadtree() {}
    Quadtree(const Quadtree&) = delete;
    Quadtree& operator=(const Quadtree&) = delete;

    bool Insert(Object* object); ///< true if it was inserted (else updated)
    bool Update(Object* object); ///< true if it was updated (else inserted)
    bool Remove(Object* object); ///< true if it was removed
    bool Contains(Object* object) const; ///< true if object is in tree
    Query QueryIntersectsRegion(const BoundingBody<Number>& region);
    Query QueryInsideRegion(const BoundingBody<Number>& region);
    Query QueryContainsRegion(const BoundingBody<Number>& region);
    const BoundingBody<Number>& GetLooseBoundingBox() const;
    ///< double its size to get a bounding box including everything contained for sure
    int GetSize() const;
    bool IsEmpty() const;
    void Clear();
    void ForceCleanup(); ///< does a full data structure and memory cleanup
    ///< cleanup is semi-automatic during queries so you needn't call this normally

private:
    Impl impl_;
};

#include "quadtree.inl"
