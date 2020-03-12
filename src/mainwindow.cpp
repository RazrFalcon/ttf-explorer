#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>

#include "src/algo.h"
#include "src/parser.h"
#include "src/tables/tables.h"

#include "mainwindow.h"

static void parse(const QByteArray &fontData, TreeModel *model)
{
    Parser parser(fontData, model);

    parser.beginGroup("Header");
    const auto magic = parser.read<UInt32>("Magic");
    if (magic != 0x00010000 && magic != 0x4F54544F) {
        throw "not a TrueType font";
    }

    const auto numTables = parser.read<UInt16>("Number of tables");
    parser.read<UInt16>("Search range");
    parser.read<UInt16>("Entry selector");
    parser.read<UInt16>("Range shift");
    parser.endGroup();

    struct Table {
        QString title;
        Tag name;
        quint32 offset;
        quint32 length;

        quint32 end() const { return offset + length; } // TODO: check overflow
    };

    QVector<Table> tables;

    parser.beginGroup("Table Records");
    for (auto i = 0; i < numTables; ++i) {
        const auto tag = parser.peek<Tag>();

        QString table;
             if (tag == "avar") table = "Axis Variations Table";
        else if (tag == "CFF ") table = "Compact Font Format Table";
        else if (tag == "CFF2") table = "Compact Font Format 2 Table";
        else if (tag == "fvar") table = "Font Variations Table";
        else if (tag == "GDEF") table = "Glyph Definition Table";
        else if (tag == "glyf") table = "Glyph Data Table";
        else if (tag == "gvar") table = "Glyph Variations Table";
        else if (tag == "head") table = "Font Header Table";
        else if (tag == "hhea") table = "Horizontal Header Table";
        else if (tag == "hmtx") table = "Horizontal Metrics Table";
        else if (tag == "loca") table = "Index to Location Table";
        else if (tag == "maxp") table = "Maximum Profile Table";
        else if (tag == "MVAR") table = "Metrics Variations Table";
        else if (tag == "name") table = "Naming Table";
        else if (tag == "OS/2") table = "OS/2 and Windows Metrics Table";
        else if (tag == "post") table = "PostScript Table";
        else if (tag == "STAT") table = "Style Attributes Table";
        else if (tag == "VORG") table = "Vertical Origin Table";
        else table = "Unknown Table";

        parser.beginGroup(table, tag.toString());
        parser.read<Tag>("Tag");
        parser.read<UInt32>("Checksum");
        const auto offset = parser.read<UInt32>("Offset");
        const auto length = parser.read<UInt32>("Length");
        parser.endGroup();

        table += " (" + tag.toString().trimmed() + ')';
        tables.append({ table, tag, offset, length });
    }
    parser.endGroup();

    std::sort(tables.begin(), tables.end(), [](const auto &a, const auto &b){
        return a.offset < b.offset;
    });

    quint16 numberOfGlyphs = 0;
    if (const auto maxp = algo::find_if(tables, [](const auto t){ return t.name == "maxp"; })) {
        parser.jumpTo(maxp->offset);
        numberOfGlyphs = parseMaxpNumberOfGlyphs(parser.shadow());
    } else {
        throw "no 'maxp' table";
    }

    auto indexToLocFormat = IndexToLocFormat::Short;
    if (const auto head = algo::find_if(tables, [](const auto t){ return t.name == "head"; })) {
        parser.jumpTo(head->offset);
        indexToLocFormat = parseHeadIndexToLocFormat(parser.shadow());
    } else {
        throw "no 'head' table";
    }

    for (const auto &table : tables) {
        parser.jumpTo(table.offset);

        if (table.title.startsWith("Unknown")) {
            continue;
        }

        parser.beginGroup(table.title);

        if (table.name == "avar") {
            parseAvar(parser);
        } else if (table.name == "CFF ") {
            parseCff(parser);
        } else if (table.name == "CFF2") {
            parseCff2(parser);
        } else if (table.name == "fvar") {
            parseFvar(parser);
        } else if (table.name == "GDEF") {
            parseGdef(parser);
        } else if (table.name == "glyf") {
            if (const auto loca = algo::find_if(tables, [](const auto t){ return t.name == "loca"; })) {
                const auto raw = reinterpret_cast<const quint8*>(fontData.constData());
                gsl::span<const quint8> locaData(raw + loca->offset, raw + loca->end());
                parseGlyf(numberOfGlyphs, indexToLocFormat, locaData, parser);
            } else {
                throw "no 'loca' table";
            }
        } else if (table.name == "gvar") {
            parseGvar(parser);
        } else if (table.name == "head") {
            parseHead(parser);
        } else if (table.name == "hhea") {
            parseHhea(parser);
        } else if (table.name == "hmtx") {
            if (const auto hhea = algo::find_if(tables, [](const auto t){ return t.name == "hhea"; })) {
                const auto raw = reinterpret_cast<const quint8*>(fontData.constData());
                gsl::span<const quint8> hheaData(raw + hhea->offset, raw + hhea->end());
                const auto numberOfMetrics = parseHheaNumberOfMetrics(ShadowParser(hheaData));
                parseHmtx(numberOfMetrics, numberOfGlyphs, parser);
            } else {
                throw "no 'hhea' table";
            }
        } else if (table.name == "loca") {
            parseLoca(numberOfGlyphs, indexToLocFormat, parser);
        } else if (table.name == "maxp") {
            parseMaxp(parser);
        } else if (table.name == "MVAR") {
            parseMvar(parser);
        } else if (table.name == "name") {
            parseName(parser);
        } else if (table.name == "OS/2") {
            parseOS2(parser);
        } else if (table.name == "post") {
            parsePost(table.end(), parser);
        } else if (table.name == "STAT") {
            parseStat(parser);
        } else if (table.name == "VORG") {
            parseVorg(parser);
        }

        parser.endGroup();
    }
}

static void collectRangesImpl(TreeItem *parent, QVector<Range> &ranges)
{
    for (int i = 0; i < parent->childrenCount(); ++i) {
        auto child = parent->child(i);

        if (child->hasChildren()) {
            collectRangesImpl(child, ranges);
        } else {
            ranges.append({ child->data().start, child->data().end });
        }
    }
}

static QVector<Range> collectRanges(TreeItem *root)
{
    QVector<Range> ranges;
    collectRangesImpl(root, ranges);
    // Make sure to sort them.
    algo::sort_all(ranges, [](const auto &a, const auto &b) { return a.start < b.start; });
    return ranges;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_hexView(new HexView())
    , m_treeView(new QTreeView())
    , m_lblStatus(new QLabel())
{
    setCentralWidget(new QWidget());

    m_lblStatus->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto lay = new QGridLayout(centralWidget());
    lay->setContentsMargins(2, 2, 2, 2);
    lay->addWidget(m_hexView, 0, 0);
    lay->addWidget(m_treeView, 0, 1);
    lay->addWidget(m_lblStatus, 1, 0, 1, 2);

    {
        auto menuBar = new QMenuBar(this);
        auto fileMenu = menuBar->addMenu("File");
        auto openAction = fileMenu->addAction("Open");
        connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
        setMenuBar(menuBar);
    }

    m_treeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(m_hexView, &HexView::byteClicked, this, &MainWindow::onHexViewByteClicked);

    setMinimumWidth(1000);
    setMinimumHeight(500);
    adjustSize();
    setWindowTitle("TTF Explorer");

    QTimer::singleShot(1, this, &MainWindow::onStart);
}

void MainWindow::onStart()
{
    if (qApp->arguments().size() == 2) {
        const auto path = qApp->arguments().at(1);
        if (QFile::exists(path)) {
            loadFile(path);
        }
    }
}

void MainWindow::onOpenFile()
{
    const auto path = QFileDialog::getOpenFileName(this, "Open Font", QDir::homePath(),
                                                   "Font Files (*.ttf *.otf)");
    if (!path.isEmpty()) {
        loadFile(path);
    }
}

void MainWindow::loadFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, "Warning", "Failed to open a font file.");
        return;
    }
    const auto fontData = file.readAll();

    m_hexView->clear();
    m_model.reset(new TreeModel());

    try {
        parse(fontData, m_model.get());
    } catch (const char *e) {
        QMessageBox::warning(this, "Warning", QString("Failed to parse a font cause:\n%1").arg(e));
        return;
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "Warning", QString("Failed to parse a font cause:\n%1").arg(e.what()));
        return;
    }

    m_hexView->setData(fontData, collectRanges(m_model->rootItem()));

    m_treeView->setModel(m_model.get());
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onTreeSelectionChanged);

    m_treeView->header()->resizeSection(
        Column::Type, m_treeView->fontMetrics().horizontalAdvance("__LongDateTime__"));

    m_treeView->header()->setSectionResizeMode(Column::Type, QHeaderView::Fixed);
    m_treeView->header()->setStretchLastSection(false);
    m_treeView->resizeColumnToContents(Column::Title);
    m_treeView->resizeColumnToContents(Column::Value);

    setWindowTitle("TTF Explorer: " + path);
}

void MainWindow::onTreeSelectionChanged(const QItemSelection &selected,
                                        const QItemSelection & /*deselected*/)
{
    const auto indexes = selected.indexes();
    if (indexes.isEmpty()) {
        return;
    }

    const auto index = indexes.first();

    auto item = static_cast<TreeItem*>(index.internalPointer());

    Range range;
    range.start = item->data().start;
    range.end = item->data().end;

    QStringList itemPath;
    {
        auto parent = item->parent();
        while (parent) {
            if (!parent->data().title.isEmpty()) {
                itemPath.prepend(parent->data().title);
            }

            parent = parent->parent();
        }
    }

    if (itemPath.isEmpty()) {
        m_lblStatus->setText(QString("%1..%2 %3B")
            .arg(range.start).arg(range.end).arg(range.size()));
    } else {
        m_lblStatus->setText(QString("%1..%2 %3B : %4")
            .arg(range.start).arg(range.end).arg(range.size())
            .arg(itemPath.join(" / ")));
    }

    m_hexView->clearSelection();
    m_hexView->selectRegion(range);
    m_hexView->scrollTo(int(item->data().start));
}

void MainWindow::onHexViewByteClicked(const uint index)
{
    if (const auto item = m_model->itemByByte(index)) {
        m_treeView->clearSelection();
        m_treeView->selectionModel()->select(
            m_model->index(item),
            QItemSelectionModel::Select | QItemSelectionModel::Rows
        );

        m_treeView->expand(m_model->index(item));
        auto parent = item->parent();
        while (parent != nullptr) {
            m_treeView->expand(m_model->index(parent));
            parent = parent->parent();
        }

        m_treeView->scrollTo(m_model->index(item));
    }
}
