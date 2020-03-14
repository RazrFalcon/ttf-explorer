#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

using namespace NameCommon;

struct Range
{
    quint32 start;
    quint32 end;

    quint32 size() const { return end - start; }
};

struct Utf16BE
{
    static const QString Type;

    static Utf16BE parse(const gsl::span<const quint8> data)
    {
        ShadowParser parser(data);

        QVector<quint16> bytes;
        while (!parser.atEnd()) {
            bytes << parser.read<UInt16>();
        }

        const auto name = QString::fromUtf16(bytes.constData(), bytes.size());

        return { name };
    }

    DEFAULT_DEBUG(Utf16BE)

    operator QString() const { return d; }

    QString d;
};

const QString Utf16BE::Type = "UTF-16 BE";


void parseName(Parser &parser)
{
    const auto tableStart = parser.offset();

    const auto format = parser.read<UInt16>("Format");
    const auto count = parser.read<UInt16>("Count");
    const auto stringOffset = parser.read<Offset16>("Offset to string storage");

    QVector<Range> nameOffsets;
    parser.beginGroup("Name records", QString::number(count));
    for (int i = 0; i < count; ++i) {
        parser.beginGroup(QString());

        const auto platformID = parser.read<PlatformID>("Platform ID");

        {
            const auto id = parser.peek<UInt16>();
            parser.readValue(2, "Encoding ID", encodingName(platformID, id));
        }

        {
            const auto id = parser.peek<UInt16>();
            parser.readValue(2, "Language ID", languageName(platformID, id));
        }

        const auto nameId = parser.read<UInt16>("Name ID");
        const auto len = parser.read<UInt16>("String length");
        const auto offset = parser.read<Offset16>("String offset");
        parser.endGroup(QString("Record %1").arg(nameId));

        // Parse only Unicode names.
        if (platformID == PlatformID::Unicode || (platformID == PlatformID::Windows && WINDOWS_UNICODE_BMP_ENCODING_ID)) {
            nameOffsets.append({ static_cast<quint32>(offset), static_cast<quint32>(offset) + len });
        }
    }
    parser.endGroup();

    if (format == 1) {
        const auto count = parser.read<UInt16>("Number of language-tag records");
        parser.beginGroup("Language-tag records", QString::number(count));
        for (int i = 0; i < count; ++i) {
            parser.beginGroup(QString("Record %1").arg(i));
            parser.read<UInt16>("String length");
            parser.read<Offset16>("String offset");
            parser.endGroup();
        }
        parser.endGroup();
    }

    // Sort offsets.
    algo::sort_all_by_key(nameOffsets, &Range::start);

    // Dedup offsets. There can be multiple records with the same offset.
    algo::dedup_vector_by_key(nameOffsets, &Range::start);

    for (const auto [i, range]: algo::enumerate(nameOffsets)) {
        parser.jumpTo(tableStart + stringOffset + range->start);
        parser.readVariable<Utf16BE>(range->size(), QString("Record %1").arg(i)); // TODO: use name ID
    }
}
