#pragma once

#include "src/parser.h"

struct CblcIndex
{
    quint16 imageFormat;
    Range range;
};

void parseAvar(Parser &parser);
void parseCbdt(const QVector<CblcIndex> &cblcLocations, Parser &parser);
void parseCblc(Parser &parser);
void parseCff(Parser &parser);
void parseCff2(Parser &parser);
void parseCmap(Parser &parser);
void parseFeat(const NamesHash &names, Parser &parser);
void parseFvar(const NamesHash &names, Parser &parser);
void parseGdef(Parser &parser);
void parseGlyf(const quint16 numberOfGlyphs, const QVector<quint32> &glyphOffsets, Parser &parser);
void parseGvar(Parser &parser);
void parseHead(Parser &parser);
void parseHhea(Parser &parser);
void parseHmtx(const quint16 numberOfMetrics, const quint16 numberOfGlyphs, Parser &parser);
void parseHvar(Parser &parser);
void parseKern(Parser &parser);
void parseLoca(const quint16 numberOfGlyphs, const quint16 indexToLocationFormat, Parser &parser);
void parseMaxp(Parser &parser);
void parseMvar(Parser &parser);
void parseName(Parser &parser);
void parseOS2(Parser &parser);
void parsePost(Parser &parser);
void parseSbix(const quint16 numberOfGlyphs, Parser &parser);
void parseStat(const NamesHash &names, Parser &parser);
void parseSvg(Parser &parser);
void parseVhea(Parser &parser);
void parseVmtx(const quint16 numberOfMetrics, const quint16 numberOfGlyphs, Parser &parser);
void parseVorg(Parser &parser);
void parseVvar(Parser &parser);

QVector<quint32> collectLocaOffsets(const quint16 numberOfGlyphs,
                                    const quint16 indexToLocationFormat,
                                    ShadowParser &parser);
NamesHash collectNameNames(ShadowParser &parser);
void parseItemVariationStore(Parser &parser);
void parseHvarDeltaSet(Parser &parser);
QVector<CblcIndex> parseCblcLocations(ShadowParser &parser);
