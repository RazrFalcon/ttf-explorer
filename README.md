# ttf-explorer

A simple tool to explore a TrueType/OpenType/AAT font content as a tree.
With a final goal to define each byte in a font.

![](.github/screenshot.png)

## Build & Run

The application is made with Qt, so you have to install it first.

You can built it via Qt Creator or using terminal: `qmake && make`.

You will also need a C++ compiler with C++17 support.

## Downloads

You can find prebuilt versions in
[Releases](https://github.com/RazrFalcon/ttf-explorer/releases/latest).

## Supported tables

avar,
bdat,
bloc,
CBDT,
CBLC,
CFF,
CFF2,
cmap,
EBDT,
EBLC,
feat,
fvar,
GDEF (mostly),
glyf,
gvar,
head,
hhea,
hmtx,
HVAR,
kern,
loca,
maxp,
MVAR,
name,
OS/2,
post,
sbix,
STAT (partially),
SVG,
vhea,
vmtx,
VORG,
VVAR

## License

MIT

Also, most of the value names were taken from the Microsoft's
[OpenType™](https://docs.microsoft.com/en-us/typography/opentype/spec/) specification.
