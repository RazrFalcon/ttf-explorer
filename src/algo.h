#pragma once

#include <QVector>

namespace algo {

// From http://reedbeta.com/blog/python-like-enumerate-in-cpp17/
template <typename T,
          typename TIter = decltype(std::begin(std::declval<T>())),
          typename = decltype(std::end(std::declval<T>()))>
constexpr auto enumerate(T&& iterable)
{
    struct iterator
    {
        int i;
        TIter iter;

        bool operator !=(const iterator &other) const { return iter != other.iter; }
        void operator ++() { ++i; ++iter; }
        auto operator *() const { return std::tie(i, iter); }
    };

    struct iterable_wrapper
    {
        T iterable;

        auto begin() { return iterator { 0, std::begin(iterable) }; }
        auto end() { return iterator { 0, std::end(iterable) }; }
    };

    return iterable_wrapper { std::forward<T>(iterable) };
}

template<typename T, typename Predicate>
void dedup_vector(QVector<T> &vec, Predicate p)
{
    vec.erase(std::unique(vec.begin(), vec.end(), p), vec.end());
}

template<typename T>
void dedup_vector(QVector<T> &vec)
{
    dedup_vector(vec, [](const auto &a, const auto &b){ return a == b; });
}

template <typename Container, typename Predicate>
std::optional<typename Container::value_type> find_if(const Container &c, Predicate p)
{
    const auto it = std::find_if(std::begin(c), std::end(c), p);
    if (it != std::end(c)) {
        return *it;
    } else {
        return std::nullopt;
    }
}

template <typename Container>
void sort_all(Container &c)
{
    std::sort(std::begin(c), std::end(c));
}

template <typename Container, typename Predicate>
void sort_all(Container &c, Predicate p)
{
    std::sort(std::begin(c), std::end(c), p);
}

}
