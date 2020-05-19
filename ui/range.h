#pragma once

#include <QtGlobal>

struct Range
{
    bool contains(const quint32 index) const
    { return index >= start && index < end; }

    bool isStart(const quint32 index) const
    { return index == start; }

    bool isMiddle(const quint32 index) const
    { return index > start && index < end - 1; }

    bool isEnd(const quint32 index) const
    { return index == end - 1; }

    bool isSingle() const
    { return size() == 1; }

    bool overlaps(const Range other) const
    {
        return (other.start >= start && other.start < end) ||
               (other.end >= start && other.end < end);
    }

    quint32 size() const
    { return end - start; }

    quint32 start = 0;
    quint32 end = 0;
};
