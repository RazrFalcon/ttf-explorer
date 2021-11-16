#pragma once

#include <QDebug>

struct Range
{
    Range() {}
    Range(const quint32 start, const quint32 end)
    {
        this->start = start;
        this->end = end;
    }

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

inline QDebug operator<<(QDebug dbg, const Range &range)
{
    return dbg.noquote() << QString("Range(%1..%2)").arg(range.start).arg(range.end);
}


struct Ranges
{
    std::vector<quint32> offsets;
    std::vector<quint32> unsupported;
};
