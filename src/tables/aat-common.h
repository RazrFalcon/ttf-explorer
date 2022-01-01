#pragma once

#include <QVector>

class Parser;

QVector<quint32> parseAatLookup(const quint16 numberOfGlyphs, Parser &parser);
