#pragma once

#include <QDate>
#include <QDebug>
#include <QString>
#include <QtEndian>

#include <algorithm>
#include <charconv>
#include <memory>
#include <optional>

#include "src/utils.h"

#include "treemodel.h"

#define DEFAULT_DEBUG(klass) \
    friend QDebug operator<<(QDebug dbg, const klass &value) \
    { \
        QString str; \
        QDebug(&str).nospace().noquote() << (#klass) << "(" << value.d << ")"; \
        return dbg << str; \
    }

#define FOURCC(x) ( (uint32_t) (((x[3])<<24) | ((x[2])<<16) | ((x[1])<<8) | (x[0])) )

using NamesHash = QHash<quint16, QString>;

// Way faster than QString::number()
// NOTE: no float support on clang
template<typename T>
static inline QString numberToString(const T value)
{
    std::array<char, 64> str;
    if (auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), value); ec == std::errc()) {
        return QString::fromUtf8(str.data(), ptr - str.data());
    }

    return QString();
}

template<typename T>
static inline QString floatToString(const T value)
{
    auto s = QString::number(value);

    // Force trailing zero for floats.
    if (!s.contains('.')) {
        s += ".0";
    }

    return s;
}

struct Int8
{
    static const int Size = 1;
    static const QString Type;

    static Int8 parse(const quint8 *data)
    { return { qint8(data[0]) }; }

    static QString toString(const Int8 &value)
    { return numberToString(value.d); }

    DEFAULT_DEBUG(Int8)

    operator qint8() const { return d; }

    qint8 d;
};

struct UInt8
{
    static const int Size = 1;
    static const QString Type;

    static UInt8 parse(const quint8 *data)
    { return { data[0] }; }

    static QString toString(const UInt8 &value)
    { return numberToString(value.d); }

    DEFAULT_DEBUG(UInt8)

    operator quint8() const { return d; }

    quint8 d;
};

struct UInt16
{
    static const int Size = 2;
    static const QString Type;

    static UInt16 parse(const quint8 *data)
    { return { qFromBigEndian<quint16>(data) }; }

    static QString toString(const UInt16 &value)
    { return numberToString(value.d); }

    DEFAULT_DEBUG(UInt16)

    operator quint16() const { return d; }

    quint16 d;
};

struct Int16
{
    static const int Size = 2;
    static const QString Type;

    static Int16 parse(const quint8 *data)
    { return { qFromBigEndian<qint16>(data) }; }

    static QString toString(const Int16 &value)
    { return numberToString(value.d); }

    DEFAULT_DEBUG(Int16)

    operator qint16() const { return d; }

    qint16 d;
};

struct GlyphId
{
    static const int Size = 2;
    static const QString Type;

    static GlyphId parse(const quint8 *data)
    { return { UInt16::parse(data) }; }

    static QString toString(const GlyphId &value)
    { return numberToString(value.d); }

    DEFAULT_DEBUG(GlyphId)

    operator quint16() const { return d; }

    quint16 d;
};

struct Offset16
{
    static const int Size = 2;
    static const QString Type;

    static Offset16 parse(const quint8 *data)
    { return { UInt16::parse(data) }; }

    static QString toString(const Offset16 &value)
    { return QString::number(value.d); }

    DEFAULT_DEBUG(Offset16)

    operator quint16() const { return d; }

    quint16 d;
};

struct OptionalOffset16
{
    static const int Size = 2;
    static const QString Type;
    static const QString NullValue;

    static OptionalOffset16 parse(const quint8 *data)
    { return { qFromBigEndian<quint16>(data) }; }

    static QString toString(const OptionalOffset16 &value)
    {
        if (value.d == 0) {
            return NullValue;
        } else {
            return numberToString(value.d);
        }
    }

    bool isNull() const { return d == 0; }

    DEFAULT_DEBUG(OptionalOffset16)

    bool operator<(const OptionalOffset16 &other) const
    { return d < other.d; }

    operator quint16() const { return d; }

    quint16 d;
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
    { return numberToString(value.d); }

    DEFAULT_DEBUG(UInt24)

    operator quint32() const { return d; }

    quint32 d;
};

struct UInt32
{
    static const int Size = 4;
    static const QString Type;

    static UInt32 parse(const quint8 *data)
    { return { qFromBigEndian<quint32>(data) }; }

    static QString toString(const UInt32 &value)
    { return numberToString(value.d); }

    DEFAULT_DEBUG(UInt32)

    operator quint32() const { return d; }

    quint32 d;
};

struct Int32
{
    static const int Size = 4;
    static const QString Type;

    static Int32 parse(const quint8 *data)
    { return { qFromBigEndian<qint32>(data) }; }

    static QString toString(const Int32 &value)
    { return numberToString(value.d); }

    DEFAULT_DEBUG(Int32)

    operator qint32() const { return d; }

    qint32 d;
};

struct Offset32
{
    static const int Size = 4;
    static const QString Type;

    static Offset32 parse(const quint8 *data)
    { return { qFromBigEndian<quint32>(data) }; }

    static QString toString(const Offset32 &value)
    { return numberToString(value.d); }

    DEFAULT_DEBUG(Offset32)

    bool operator<(const Offset32 &other) const
    { return d < other.d; }

    operator quint32() const { return d; }

    quint32 d;
};

struct OptionalOffset32
{
    static const int Size = 4;
    static const QString Type;
    static const QString NullValue;

    static OptionalOffset32 parse(const quint8 *data)
    { return { qFromBigEndian<quint32>(data) }; }

    static QString toString(const OptionalOffset32 &value)
    {
        if (value.d == 0) {
            return NullValue;
        } else {
            return numberToString(value.d);
        }
    }

    bool isNull() const { return d == 0; }

    DEFAULT_DEBUG(OptionalOffset32)

    bool operator<(const OptionalOffset32 &other) const
    { return d < other.d; }

    operator quint32() const { return d; }

    quint32 d;
};

struct F2DOT14
{
    static const int Size = 2;
    static const QString Type;

    static F2DOT14 parse(const quint8 *data)
    { return { float(Int16::parse(data)) / 16384.0f }; }

    static QString toString(const F2DOT14 &value)
    { return floatToString(value.d); }

    DEFAULT_DEBUG(F2DOT14)

    operator float() const { return d; }
    operator double() const { return double(d); }

    bool operator==(const float other) const
    { return qFuzzyCompare(d, other); }

    bool operator!=(const float other) const
    { return !qFuzzyCompare(d, other); }

    float d;
};

struct F16DOT16
{
    static const int Size = 4;
    static const QString Type;

    static F16DOT16 parse(const quint8 *data)
    { return { float(Int32::parse(data)) / 65536.0f }; }

    static QString toString(const F16DOT16 &value)
    { return floatToString(value.d); }

    DEFAULT_DEBUG(F16DOT16)

    operator float() const { return d; }

    bool operator==(const float other) const
    { return qFuzzyCompare(d, other); }

    bool operator!=(const float other) const
    { return !qFuzzyCompare(d, other); }

    float d;
};

struct Tag
{
    static const int Size = 4;
    static const QString Type;

    static Tag parse(const quint8 *data)
    { return { qFromLittleEndian<quint32>(data) }; } // Yes, little.

    static QString toString(const Tag &value)
    { return QString::fromUtf8(reinterpret_cast<const char*>(&value.d), 4); }

    QString toString() const
    { return Tag::toString(*this); }

    friend QDebug operator<<(QDebug dbg, const Tag &value)
    {
        QString str;
        QDebug(&str).nospace().noquote() << "Tag(" << toString(value) << ")";
        return dbg << str;
    }

    bool operator==(const char *tag) const
    { return strlen(tag) == 4 && (*reinterpret_cast<const quint32*>(tag)) == d; }

    bool operator!=(const char *tag) const
    { return !(*this == tag); }

    quint32 d;
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
    { return value.toString(); }

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
    static const std::array<quint16, 256> MacRomanEncoding;

    explicit ShadowParser(const quint8 *start, const quint8 *end)
        : m_start(start)
        , m_data(start)
        , m_end(end)
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
        if (offset >= m_end - m_start) {
            throw QString("read out of bounds");
        }

        m_data = m_start + offset;
    }

    void advance(const quint32 size)
    {
        if (m_data + size >= m_end) {
            throw QString("read out of bounds");
        }

        m_data += size;
    }

    void advanceTo(const quint32 offset)
    {
        if (offset < this->offset()) {
            throw "an attempt to advance backward";
        } else if (offset == this->offset()) {
            // nothing
        } else {
            advance(offset - this->offset());
        }
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
    void skip()
    {
        if (atEnd(T::Size)) {
            throw QString("read out of bounds");
        }

        m_data += T::Size;
    }

    template<typename T>
    T read()
    {
        if (atEnd(T::Size)) {
            throw QString("read out of bounds");
        }

        const auto value = T::parse(m_data);
        m_data += T::Size;
        return value;
    }

    QString readUtf16String(const quint32 length)
    {
        if (atEnd(length)) {
            throw QString("read out of bounds");
        }

        if (length == 0) {
            return QString();
        }

        QVarLengthArray<char16_t> bytes;
        for (quint32 i = 0; i < length / 2; i++) {
            bytes << read<UInt16>();
        }

        return QString::fromUtf16(bytes.constData(), bytes.size());
    }

    QString readMacRomanString(const quint32 length)
    {
        if (atEnd(length)) {
            throw QString("read out of bounds");
        }

        if (length == 0) {
            return QString();
        }

        QVarLengthArray<char16_t> bytes;
        for (quint32 i = 0; i < length; i++) {
            bytes << MacRomanEncoding[read<UInt8>()];
        }

        return QString::fromUtf16(bytes.constData(), bytes.size());
    }

    QByteArray readBytes(const quint32 size)
    {
        if (atEnd(size)) {
            throw QString("read out of bounds");
        }

        if (size == 0) {
            return QByteArray();
        }

        // The data would not be copied.
        const auto value = QByteArray::fromRawData((const char *)m_data, size);
        m_data += size;
        return value;
    }

    ShadowParser shadow() const
    {
        return ShadowParser(m_data, m_end);
    }

private:
    const quint8 *m_start;
    const quint8 *m_data;
    const quint8 *m_end;
};

class Parser
{
public:
    static const QString BytesType;
    static const QString ArrayType;
    static const QString BitflagsType;
    static const QString PascalStringType;
    static const QString Utf8StringType;
    static const QString Utf16StringType;
    static const QString MacRomanStringType;
    static const QString CFFNumberType;

    static const QString PaddingTitle;
    static const QString UnsupportedTitle;
    static const QString NameTitle;

    explicit Parser(const quint8 *data, const quint32 len, TreeItem *root)
        : m_start(data)
        , m_data(data)
        , m_end(data + len)
        , m_parent(root)
        , m_ranges(Ranges())
    {
    }

    Ranges&& ranges() {
        return std::move(m_ranges);
    }

    quint32 offset() const
    {
        return m_data - m_start;
    }

    void advance(const quint32 size)
    {
        if (size == 0) {
            return;
        }

        if (m_data + size > m_end) {
            throw QString("read out of bounds");
        }

        auto item = new TreeItem(m_parent);
        item->title = UnsupportedTitle;
        item->value = QString();
        item->type = QString();
        item->range = Range(offset(), offset() + size);
        m_parent->addChild(item);

        m_ranges.offsets.push_back(offset());
        m_ranges.unsupported.push_back(offset());
        m_data += size;
    }

    void advanceTo(const quint32 offset)
    {
        if (offset < this->offset()) {
            throw QString("an attempt to advance backward");
        } else if (offset == this->offset()) {
            // nothing
        } else {
            advance(offset - this->offset());
        }
    }

    void padTo(const quint32 offset)
    {
        if (offset < this->offset()) {
            throw QString("an attempt to advance backward");
        } else if (offset == this->offset()) {
            // nothing
        } else {
            readPadding(offset - this->offset());
        }
    }

    quint32 left() const
    {
        return m_end - m_data;
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
    T peek(const int offset = 0) const
    {
        if (atEnd(offset)) {
            throw QString("read out of bounds");
        }

        return T::parse(m_data + offset);
    }

    template<typename T>
    T read(const quint32 index)
    {
        return read<T>(cachedIndex(index));
    }

    template<typename T>
    T read(const char *title)
    {
        return read<T>(cachedString(title));
    }

    template<typename T>
    T read(const QString &title)
    {
        if (atEnd(T::Size)) {
            throw QString("read out of bounds");
        }

        const auto start = offset();
        const auto value = T::parse(m_data);
        m_data += T::Size;

        m_ranges.offsets.push_back(start);

        auto item = new TreeItem(m_parent);
        item->title = title;
        item->value = T::toString(value);
        item->type = T::Type;
        item->range = Range(start, start + T::Size);
        m_parent->addChild(item);

        return value;
    }

    QByteArray readBytes(const QString &title, const quint32 size)
    {
        if (atEnd(size)) {
            throw QString("read out of bounds");
        }

        if (size == 0) {
            return QByteArray();
        }

        const auto start = offset();
        // The data would not be copied.
        const auto value = QByteArray::fromRawData((const char *)m_data, size);
        m_data += size;

        m_ranges.offsets.push_back(start);

        auto item = new TreeItem(m_parent);
        item->title = title;
        item->value = QString();
        item->type = BytesType;
        item->range = Range(start, offset());
        m_parent->addChild(item);

        return value;
    }

    void readPadding(const quint32 size)
    {
        readBytes(PaddingTitle, size);
    }

    void readUnsupported(const quint32 size)
    {
        advance(size);
    }

    QString readPascalString(const QString &title)
    {
        const auto start = offset();

        beginGroup();

        QString value;
        const auto length = read<UInt8>("Length");
        if (length > 0) {
            value = QString::fromUtf8(reinterpret_cast<const char *>(m_data), int(length));
            m_data += length;
        }

        m_ranges.offsets.push_back(start);

        auto item = new TreeItem(m_parent);
        item->title = NameTitle;
        item->value = value;
        item->type = BytesType;
        item->range = Range(start, start + length);
        m_parent->addChild(item);

        endGroup(title, value, PascalStringType);

        return value;
    }

    QString readUtf8String(const quint32 index, const quint32 length)
    {
        return readUtf8String(cachedIndex(index), length);
    }

    QString readUtf8String(const char *title, const quint32 length)
    {
        return readUtf8String(cachedString(title), length);
    }

    QString readUtf8String(const QString &title, const quint32 length)
    {
        if (atEnd(length)) {
            throw QString("read out of bounds");
        }

        if (length == 0) {
            return QString();
        }

        const auto value = QString::fromUtf8((const char *)m_data, length);
        readValue(title, value, Utf8StringType, length);
        return value;
    }

    QString readUtf16String(const QString &title, const quint32 length)
    {
        if (atEnd(length)) {
            throw QString("read out of bounds");
        }

        if (length == 0) {
            return QString();
        }

        ShadowParser s(m_data, m_data + length);

        QVarLengthArray<char16_t> bytes;
        while (!s.atEnd()) {
            bytes << s.read<UInt16>();
        }

        const auto value = QString::fromUtf16(bytes.constData(), bytes.size());
        readValue(title, value, Utf16StringType, length);
        return value;
    }

    QString readMacRomanString(const QString &title, const quint32 length)
    {
        if (atEnd(length)) {
            throw QString("read out of bounds");
        }

        if (length == 0) {
            return QString();
        }

        ShadowParser s(m_data, m_data + length);

        QVarLengthArray<char16_t> bytes;
        while (!s.atEnd()) {
            bytes << ShadowParser::MacRomanEncoding[s.read<UInt8>()];
        }

        const auto value = QString::fromUtf16(bytes.constData(), bytes.size());
        readValue(title, value, MacRomanStringType, length);
        return value;
    }

    QString readNameId(const QString &title, const NamesHash &names)
    {
        const auto id = peek<UInt16>();
        if (names.contains(id)) {
            readValue<UInt16>(title, QString("%1 (%2)").arg(names.value(id)).arg(id));
            return names.value(id);
        } else {
            read<UInt16>(title);
            return QString();
        }
    }

    template<typename T>
    void readValue(const QString &title, const QString &value)
    {
        if (atEnd(T::Size)) {
            throw QString("read out of bounds");
        }

        const auto start = offset();
        m_ranges.offsets.push_back(start);

        auto item = new TreeItem(m_parent);
        item->title = title;
        item->value = value;
        item->type = T::Type;
        item->range = Range(start, start + T::Size);
        m_parent->addChild(item);

        m_data += T::Size;
    }

    void readValue(const char *title, const QString &value, const QString &type, const quint32 length)
    {
        return readValue(cachedString(title), value, type, length);
    }

    void readValue(const QString &title, const QString &value, const QString &type, const quint32 length)
    {
        if (atEnd(length)) {
            throw QString("read out of bounds");
        }

        const auto start = offset();
        m_ranges.offsets.push_back(start);

        auto item = new TreeItem(m_parent);
        item->title = title;
        item->value = value;
        item->type = type;
        item->range = Range(start, start + length);
        m_parent->addChild(item);

        m_data += length;
    }

    void beginGroup(const quint32 index)
    {
        beginGroup(cachedIndex(index));
    }

    void beginGroup(const QString &title = QString(), const QString &value = QString())
    {
        auto item = new TreeItem(m_parent);
        item->title = title;
        item->value = value;
        item->range = Range(offset(), offset());
        m_parent->addChild(item);
        m_parent = item;
    }

    void endGroup(const QString &title = QString(),
                  const QString &value = QString(),
                  const QString &type = QString())
    {
        if (m_parent->parent() && m_parent->hasChildren()) {
            // Update group title after actual parsing.
            if (!title.isEmpty()) {
                m_parent->title = title;
            }

            // Update group value after actual parsing.
            if (!value.isEmpty()) {
                m_parent->value = value;
            }

            // Update group type after actual parsing.
            if (!type.isEmpty()) {
                m_parent->type = type;
            }
        }

        m_parent->range.end = offset();
        m_parent->size = Utils::prettySize(m_parent->range.size());

        Q_ASSERT(m_parent->parent() != nullptr);
        m_parent = m_parent->parent();
    }

    void beginArray(const QString &title, quint32 itemsCount)
    {
        QString value;
        if (itemsCount == 1) {
            value = "1 item";
        } else {
            value = QString("%1 items").arg(itemsCount);
        }

        auto item = new TreeItem(m_parent);
        item->title = title;
        item->value = value;
        item->type = ArrayType;
        item->range = Range(offset(), offset());
        item->reserveChildren(itemsCount);
        m_parent->addChild(item);
        m_parent = item;
    }

    void endArray()
    {
        endGroup();
    }

    template <typename F>
    void readArray(const QString &title, quint32 itemsCount, F f) {
        if (itemsCount == 0) {
            return;
        }

        beginArray(title, itemsCount);
        for (quint32 i = 0; i < itemsCount; ++i) {
            f(i);
        }
        endArray();
    }

    template <typename T>
    void readBasicArray(const QString &title, quint32 itemsCount) {
        if (itemsCount == 0) {
            return;
        }

        beginArray(title, itemsCount);
        for (quint32 i = 0; i < itemsCount; ++i) {
            read<T>(i);
        }
        endArray();
    }

    ShadowParser shadow() const
    {
        return ShadowParser(m_data, m_end);
    }

    void finish()
    {
        readUnsupported(left());
        m_ranges.offsets.push_back(m_end - m_start);
    }

private:
    QString cachedString(const char *str)
    {
        // TTF Explorer will allocate a lot of strings. A lot. So we better cache them.
        // This optimization is used mainly to reduce memory usage,
        // because we would have a lot of duplicated strings. But it also improves performance.
        if (m_stringCache.contains(str)) {
            return m_stringCache.value(str);
        } else {
            const auto s = QString(str);
            m_stringCache.insert(str, s);
            return s;
        }
    }

    QString cachedIndex(const quint32 index)
    {
        while (index + 1 > m_indexCache.size()) {
            m_indexCache.reserve(index);
            m_indexCache.append(numberToString(m_indexCache.size()));
        }

        return m_indexCache[index];
    }

private:
    Q_DISABLE_COPY(Parser)

private:
    const quint8 *m_start;
    const quint8 *m_data;
    const quint8 *m_end;
    TreeItem *m_parent;
    Ranges m_ranges;

    // Cache per App instance, not per type instance.
    static QHash<const char*, QString> m_stringCache;
    static QVector<QString> m_indexCache;
};
