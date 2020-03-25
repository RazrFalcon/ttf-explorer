#pragma once

#include <QtGlobal>

#include "src/parser.h"

enum class IndexToLocFormat { Short, Long };

void parseAvar(Parser &parser);
void parseCff(Parser &parser);
void parseCff2(Parser &parser);
void parseCmap(Parser &parser);
void parseFvar(Parser &parser);
void parseGdef(Parser &parser);
void parseGlyf(const quint16 numberOfGlyphs, const IndexToLocFormat format,
               const gsl::span<const quint8> locaData, Parser &parser);
void parseGvar(Parser &parser);
void parseHead(Parser &parser);
void parseHhea(Parser &parser);
void parseHmtx(const quint16 numberOfMetrics, const quint16 numberOfGlyphs, Parser &parser);
void parseHvar(Parser &parser);
void parseLoca(const quint16 numberOfGlyphs, const IndexToLocFormat format, Parser &parser);
void parseMaxp(Parser &parser);
void parseMvar(Parser &parser);
void parseName(Parser &parser);
void parseOS2(Parser &parser);
void parsePost(const quint32 end, Parser &parser);
void parseSbix(const quint16 numberOfGlyphs, Parser &parser);
void parseStat(Parser &parser);
void parseVhea(Parser &parser);
void parseVmtx(const quint16 numberOfMetrics, const quint16 numberOfGlyphs, Parser &parser);
void parseVvar(Parser &parser);
void parseVorg(Parser &parser);

IndexToLocFormat parseHeadIndexToLocFormat(ShadowParser parser);
quint16 parseMaxpNumberOfGlyphs(ShadowParser parser);
quint16 parseHheaNumberOfMetrics(ShadowParser parser);
quint16 parseVheaNumberOfMetrics(ShadowParser parser);
void parseHvarDeltaSet(Parser &parser);
void parseItemVariationStore(Parser &parser);
