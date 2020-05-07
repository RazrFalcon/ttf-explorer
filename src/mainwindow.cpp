#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>

#include "mainwindow.h"

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
        auto fileMenu = menuBar->addMenu(QLatin1String("File"));
        auto openAction = fileMenu->addAction(QLatin1String("Open"));
        connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
        setMenuBar(menuBar);
    }

    m_treeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_treeView->setVerticalScrollMode(QTreeView::ScrollPerPixel);
    m_treeView->header()->setSectionsMovable(false);
    m_treeView->header()->setSectionsClickable(false);
    m_treeView->header()->setSortIndicatorShown(false);
    m_treeView->setItemDelegateForColumn(Column::Title, new TitleItemDelegate(this));

    connect(m_hexView, &HexView::byteClicked, this, &MainWindow::onHexViewByteClicked);

    resize(1200, 600);
    setWindowTitle(QLatin1String("TTF Explorer"));

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
    const auto path = QFileDialog::getOpenFileName(this, QLatin1String("Open Font"), QDir::homePath(),
                                                   QLatin1String("Font Files (*.ttf *.otf)"));
    if (!path.isEmpty()) {
        loadFile(path);
    }
}

void MainWindow::loadFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, QLatin1String("Warning"), QLatin1String("Failed to open a font file."));
        return;
    }
    const auto fontData = file.readAll();

    try {
        auto [tree, warnings] = TTFCore::Tree::parse(fontData);

        if (!warnings.isEmpty()) {
            QMessageBox::warning(this, QLatin1String("Warning"), warnings);
        }

        m_hexView->clear();
        m_model.reset(new TreeModel(std::move(tree)));

        m_hexView->setData(fontData, m_model->collectRanges());

        m_treeView->setModel(m_model.get());
        connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &MainWindow::onTreeSelectionChanged);

        m_treeView->header()->resizeSection(
            Column::Value, m_treeView->fontMetrics().horizontalAdvance(QLatin1String("00000000000000000000")));

        m_treeView->header()->resizeSection(
            Column::Type, m_treeView->fontMetrics().horizontalAdvance(QLatin1String("__LongDateTime__")));

        m_treeView->header()->setSectionResizeMode(Column::Type, QHeaderView::Fixed);
        m_treeView->header()->setStretchLastSection(false);
        m_treeView->resizeColumnToContents(Column::Title);

        setWindowTitle(QLatin1String("TTF Explorer: ") + path);
    } catch (const QString &msg) {
        QMessageBox::critical(this, QLatin1String("Error"), msg);
    }
}

void MainWindow::onTreeSelectionChanged(const QItemSelection &selected,
                                        const QItemSelection & /*deselected*/)
{
    const auto indexes = selected.indexes();
    if (indexes.isEmpty()) {
        return;
    }

    const auto index = indexes.first();
    const auto itemId = index.internalId();
    const auto range = m_model->itemRange(itemId);

    QStringList itemPath;
    {
        auto parentId = m_model->parentItem(itemId);
        while (parentId != m_model->rootId()) {
            const auto title = m_model->itemTitle(parentId.value());
            if (!title.isEmpty()) {
                itemPath.prepend(title);
            }

            parentId = m_model->parentItem(parentId.value());
        }
    }

    if (itemPath.isEmpty()) {
        m_lblStatus->setText(QString::fromLatin1("%1..%2 %3B")
            .arg(range.start).arg(range.end).arg(range.size()));
    } else {
        m_lblStatus->setText(QString::fromLatin1("%1..%2 %3B : %4")
            .arg(range.start).arg(range.end).arg(range.size())
            .arg(itemPath.join(QLatin1String(" / "))));
    }

    m_hexView->clearSelection();
    m_hexView->selectRegion(range);
    m_hexView->scrollTo(int(range.start));
}

void MainWindow::onHexViewByteClicked(const uint index)
{
    if (const auto itemId = m_model->itemByByte(index)) {
        const auto itemIndex = m_model->index(itemId.value());

        m_treeView->clearSelection();
        m_treeView->selectionModel()->select(
            itemIndex,
            QItemSelectionModel::Select | QItemSelectionModel::Rows
        );

        m_treeView->expand(itemIndex);
        auto parentId = m_model->parentItem(itemId.value());
        while (parentId != m_model->rootId()) {
            m_treeView->expand(m_model->index(parentId.value()));
            parentId = m_model->parentItem(parentId.value());
        }

        m_treeView->scrollTo(itemIndex);
    }
}
