#include "src/parser.h"
#include "tables.h"

void parseLoca(const quint16 numberOfGlyphs, const IndexToLocFormat format, Parser &parser)
{
    for (int i = 0; i <= numberOfGlyphs; ++i) {
        if (format == IndexToLocFormat::Short) {
            parser.read<Offset16>(QString("Offset %1").arg(i));
        } else {
            parser.read<Offset32>(QString("Offset %1").arg(i));
        }
    }
}
