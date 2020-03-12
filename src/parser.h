#pragma once

#include <QDate>
#include <QDebug>
#include <QString>
#include <QtEndian>

#include <algorithm>
#include <memory>
#include <optional>

#include "3rdparty/gsl-lite.hpp"

#include "treemodel.h"

#define DEFAULT_DEBUG(klass) \
    friend QDebug operator<<(QDebug dbg, const klass &value) \
    { \
        QString str; \
        QDebug(&str).nospace().noquote() << Type << "(" << value.d << ")"; \
        return dbg << str; \
    }

struct Int8
{
    static const int Size = 1;
    static const QString Type;

    static Int8 parse(const quint8 *data)
    {
        return { qint8(data[0]) };
    }

    static QString toString(const Int8 &value)
    {
        return QString::number(value.d);
    }

    DEFAULT_DEBUG(Int8)

    operator qint8() const { return d; }

    qint8 d;
};

struct UInt8
{
    static const int Size = 1;
    static const QString Type;

    static UInt8 parse(const quint8 *data)
    {
        return { data[0] };
    }

    static QString toString(const UInt8 &value)
    {
        return QString::number(value.d);
    }

    DEFAULT_DEBUG(UInt8)

    operator quint8() const { return d; }

    quint8 d;
};

struct UInt16
{
    static const int Size = 2;
    static const QString Type;

    static UInt16 parse(const quint8 *data)
    {
        return { qFromBigEndian<quint16>(data) };
    }

    static QString toString(const UInt16 &value)
    {
        return QString::number(value.d);
    }

    DEFAULT_DEBUG(UInt16)

    operator quint16() const { return d; }

    quint16 d;
};

struct GlyphId
{
    static const int Size = 2;
    static const QString Type;

    static GlyphId parse(const quint8 *data)
    {
        return { UInt16::parse(data) };
    }

    static QString toString(const GlyphId &value)
    {
        return QString::number(value.d);
    }

    DEFAULT_DEBUG(GlyphId)

    operator quint16() const { return d; }

    quint16 d;
};

struct Offset16
{
    static const int Size = 2;
    static const QString Type;

    static Offset16 parse(const quint8 *data)
    {
        return { UInt16::parse(data) };
    }

    static QString toString(const Offset16 &value)
    {
        return QString::number(value.d);
    }

    DEFAULT_DEBUG(Offset16)

    operator quint16() const { return d; }

    quint16 d;
};

struct Int16
{
    static const int Size = 2;
    static const QString Type;

    static Int16 parse(const quint8 *data)
    {
        return { qFromBigEndian<qint16>(data) };
    }

    static QString toString(const Int16 &value)
    {
        return QString::number(value.d);
    }

    DEFAULT_DEBUG(Int16)

    operator qint16() const { return d; }

    qint16 d;
};

struct UInt24
{
    static const int Size = 3;
    static const QString Type;

    static UInt24 parse(const quint8 *data)
    {
        const auto value = quint32(data[0]) << 16 | quint32(data[1]) << 8 | quint32(data[2]);
        return { value };
    }

    static QString toString(const UInt24 &value)
    {
        return QString::number(value.d);
    }

    DEFAULT_DEBUG(UInt24)

    operator quint32() const { return d; }

    quint32 d;
};

struct UInt32
{
    static const int Size = 4;
    static const QString Type;

    static UInt32 parse(const quint8 *data)
    {
        return { qFromBigEndian<quint32>(data) };
    }

    static QString toString(const UInt32 &value)
    {
        return QString::number(value.d);
    }

    DEFAULT_DEBUG(UInt32)

    operator quint32() const { return d; }

    quint32 d;
};

struct Int32
{
    static const int Size = 4;
    static const QString Type;

    static Int32 parse(const quint8 *data)
    {
        return { qFromBigEndian<qint32>(data) };
    }

    static QString toString(const Int32 &value)
    {
        return QString::number(value.d);
    }

    DEFAULT_DEBUG(Int32)

    operator qint32() const { return d; }

    qint32 d;
};

struct Offset32
{
    static const int Size = 4;
    static const QString Type;

    static Offset32 parse(const quint8 *data)
    {
        return { qFromBigEndian<quint32>(data) };
    }

    static QString toString(const Offset32 &value)
    {
        return QString::number(value.d);
    }

    DEFAULT_DEBUG(Offset32)

    bool operator<(const Offset32 &other) const
    {
        return d < other.d;
    }

    operator quint32() const { return d; }

    quint32 d;
};

struct F2DOT14
{
    static const int Size = 2;
    static const QString Type;

    static F2DOT14 parse(const quint8 *data)
    {
        return { float(Int16::parse(data)) / 16384.0f };
    }

    static QString toString(const F2DOT14 &value)
    {
        return QString::number(static_cast<double>(value.d));
    }

    DEFAULT_DEBUG(F2DOT14)

    operator float() const { return d; }
    operator double() const { return double(d); }

    bool operator==(const float other) const
    {
        return qFuzzyCompare(d, other);
    }

    bool operator!=(const float other) const
    {
        return !qFuzzyCompare(d, other);
    }

    float d;
};

struct Fixed
{
    static const int Size = 4;
    static const QString Type;

    static Fixed parse(const quint8 *data)
    {
        return { float(Int32::parse(data)) / 65536.0f };
    }

    static QString toString(const Fixed &value)
    {
        return QString::number(static_cast<double>(value.d));
    }

    DEFAULT_DEBUG(Fixed)

    operator float() const { return d; }

    bool operator==(const float other) const
    {
        return qFuzzyCompare(d, other);
    }

    bool operator!=(const float other) const
    {
        return !qFuzzyCompare(d, other);
    }

    float d;
};

struct Tag
{
    static const int Size = 4;
    static const QString Type;

    static Tag parse(const quint8 *data)
    {
        return { data[0], data[1], data[2], data[3] };
    }

    static QString toString(const Tag &value)
    {
        return QString::fromUtf8(reinterpret_cast<const char*>(value.d.data()), 4);
    }

    QString toString() const
    {
        return Tag::toString(*this);
    }

    friend QDebug operator<<(QDebug dbg, const Tag &value)
    {
        QString str;
        QDebug(&str).nospace().noquote() << "Tag(" << toString(value) << ")";
        return dbg << str;
    }

    bool operator==(const char *tag) const
    {
        return strlen(tag) == 4 && qstrncmp(reinterpret_cast<const char*>(d.data()), tag, 4) == 0;
    }

    std::array<quint8, 4> d;
};

struct LongDateTime
{
    static const int Size = 8;
    static const QString Type;

    static LongDateTime parse(const quint8 *data)
    {
        LongDateTime time;
        time.d = qFromBigEndian<qint64>(data);
        return time;
    }

    static QString toString(const LongDateTime &value)
    {
        return value.toString();
    }

    QString toString() const
    {
        // Date represented in number of seconds since 12:00 midnight, January 1, 1904.
        QDateTime date(QDate(1904, 1, 1), QTime(0, 0), Qt::UTC);
        date = date.addSecs(d);
        return date.toString();
    }

    DEFAULT_DEBUG(LongDateTime)

    qint64 d;
};

class ShadowParser
{
public:
    explicit ShadowParser(const quint8 *start, const quint8 *end)
        : m_start(start)
        , m_data(start)
        , m_end(end)
    {
    }

    explicit ShadowParser(const gsl::span<const quint8> &span)
        : m_start(span.data())
        , m_data(span.data())
        , m_end(span.data() + span.size())
    {
    }

    quint32 offset() const
    {
        return quint32(m_data - m_start);
    }

    quint32 left() const
    {
        return quint32(m_end - m_data);
    }

    void jumpTo(const quint32 offset)
    {
        if (m_start + offset >= m_end) {
            throw std::out_of_range("");
        }

        m_data = m_start + offset;
    }

    bool atEnd() const
    {
        return m_data >= m_end;
    }

    bool atEnd(const quint32 size) const
    {
        return m_data + size > m_end;
    }

    template<typename T>
    T read()
    {
        if (atEnd(T::Size)) {
            throw std::out_of_range("");
        }

        const auto value = T::parse(m_data);
        m_data += T::Size;
        return value;
    }

    gsl::span<const quint8> readBytes(const quint32 size)
    {
        if (atEnd(size)) {
            throw std::out_of_range("");
        }

        const auto value = gsl::make_span(m_data, size);
        m_data += size;

        return value;
    }

private:
    Q_DISABLE_COPY(ShadowParser)

private:
    const quint8 *m_start;
    const quint8 *m_data;
    const quint8 *m_end;
};

class Parser
{
public:
    explicit Parser(const QByteArray &data, TreeModel *model)
        : m_start(reinterpret_cast<const quint8*>(data.constData()))
        , m_data(m_start)
        , m_end(m_start + data.size())
        , m_model(model)
        , m_parent(model->rootItem())
    {
    }

    quint32 offset() const
    {
        return quint32(m_data - m_start);
    }

    void jumpTo(const quint32 offset)
    {
        if (m_start + offset >= m_end) {
            throw std::out_of_range("");
        }

        m_data = m_start + offset;
    }

    void advance(const quint32 size)
    {
        if (m_data + size >= m_end) {
            throw std::out_of_range("");
        }

        m_data += size;
    }

    bool atEnd() const
    {
        return m_data >= m_end;
    }

    bool atEnd(const quint32 size) const
    {
        return m_data + size > m_end;
    }

    template<typename T>
    T read(const QString &title)
    {
        if (atEnd(T::Size)) {
            throw std::out_of_range("");
        }

        const auto start = offset();
        const auto value = T::parse(m_data);
        m_data += T::Size;

        TreeItemData data;
        data.title = title;
        data.type = T::Type;
        data.value = T::toString(value);
        data.start = start;
        data.end = offset();
        m_model->appendChild(data, m_parent);

        return value;
    }

    template<>
    std::optional<Offset16> read(const QString &title)
    {
        if (atEnd(Offset16::Size)) {
            throw std::out_of_range("");
        }

        const auto start = offset();
        const auto value = Offset16::parse(m_data);
        m_data += Offset16::Size;

        TreeItemData data;
        data.title = title;
        data.type = Offset16::Type;
        data.value = value == 0 ? "NULL" : Offset16::toString(value);
        data.start = start;
        data.end = offset();
        m_model->appendChild(data, m_parent);

        return value == 0 ? std::nullopt : std::make_optional(value);
    }

    template<>
    std::optional<Offset32> read(const QString &title)
    {
        if (atEnd(Offset32::Size)) {
            throw std::out_of_range("");
        }

        const auto start = offset();
        const auto value = Offset32::parse(m_data);
        m_data += Offset32::Size;

        TreeItemData data;
        data.title = title;
        data.type = Offset32::Type;
        data.value = value == 0 ? "NULL" : Offset32::toString(value);
        data.start = start;
        data.end = offset();
        m_model->appendChild(data, m_parent);

        return value == 0 ? std::nullopt : std::make_optional(value);
    }

    template<typename T>
    T readVariable(const quint32 size, const QString &title)
    {
        const auto start = offset();
        const auto bytes = gsl::make_span(m_data, size);
        m_data += size;

        const auto value = T::parse(bytes);

        TreeItemData data;
        data.title = title;
        data.type = T::Type;
        data.value = value;
        data.start = start;
        data.end = offset();
        m_model->appendChild(data, m_parent);

        return value;
    }

    gsl::span<const quint8> readBytes(const quint32 size, const QString &title)
    {
        if (atEnd(size)) {
            throw std::out_of_range("");
        }

        const auto start = offset();
        const auto value = gsl::make_span(m_data, size);
        m_data += size;

        TreeItemData data;
        data.title = title;
        data.type = "Bytes";
        data.start = start;
        data.end = offset();
        m_model->appendChild(data, m_parent);

        return value;
    }

    QString readString(const quint32 size, const QString &title)
    {
        if (atEnd(size)) {
            throw std::out_of_range("");
        }

        const auto start = offset();
        const auto value = QString::fromUtf8(reinterpret_cast<const char *>(m_data), int(size));
        m_data += size;

        TreeItemData data;
        data.title = title;
        data.type = "String";
        data.value = value;
        data.start = start;
        data.end = offset();
        m_model->appendChild(data, m_parent);

        return value;
    }

    void readValue(const quint32 size, const QString &title, const QString &value)
    {
        if (atEnd(size)) {
            throw std::out_of_range("");
        }

        const auto start = offset();
        m_data += size;

        TreeItemData data;
        data.title = title;
        data.type = "Bytes";
        data.value = value;
        data.start = start;
        data.end = offset();
        m_model->appendChild(data, m_parent);
    }

    // TODO: offset by 0 by default
    template<typename T>
    T peek(const int offset = 1)
    {
        if (atEnd(quint32(T::Size * offset))) {
            throw std::out_of_range("");
        }

        return T::parse(m_data + T::Size * (offset - 1));
    }

    void beginGroup(const QString &title, const QString &value = QString())
    {
        TreeItemData data;
        data.title = title;
        data.value = value;
        data.type = QString();
        data.start = offset();
        data.end = offset(); // Will be set later in `endGroup()`.

        m_parent = m_model->appendChild(data, m_parent);
    }

    void endGroup(const QString &title = QString(), const QString &value = QString(), const quint32 end = 0)
    {
        if (m_parent->parent() && m_parent->hasChildren()) {
            // Update group title after actual parsing.
            if (!title.isEmpty()) {
                m_parent->data().title = title;
            }

            // Update group value after actual parsing.
            if (!value.isEmpty()) {
                m_parent->data().value = value;
            }

            if (end != 0) {
                m_parent->data().end = end;
            } else {
                m_parent->data().end = m_parent->childrenList().last()->data().end;
            }
        }

        m_parent = m_parent->parent();
    }

    ShadowParser shadow()
    {
        return ShadowParser(m_data, m_end);
    }

private:
    Q_DISABLE_COPY(Parser)

private:
    const quint8 *m_start;
    const quint8 *m_data;
    const quint8 *m_end;
    TreeModel *m_model;
    TreeItem *m_parent;
};
