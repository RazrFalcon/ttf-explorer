#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>

#include "truetype.h"

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_hexView(new HexView)
    , m_treeView(new QTreeView)
    , m_lblStatus(new QLabel)
{
    setCentralWidget(new QWidget());

    m_lblStatus->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto lay = new QGridLayout(centralWidget());
    lay->setContentsMargins(2, 2, 2, 2);
    lay->addWidget(m_hexView, 0, 0);
    lay->addWidget(m_treeView, 0, 1);
    lay->addWidget(m_lblStatus, 1, 0, 1, 2);

#ifdef Q_OS_MAC
    lay->setVerticalSpacing(1);
    lay->setHorizontalSpacing(4);
    lay->setContentsMargins(0, 0, 0, 2);
    m_hexView->setFrameShape(QFrame::NoFrame);
    m_treeView->setFrameShape(QFrame::NoFrame);
#endif

    {
        auto menuBar = new QMenuBar(this);
        auto fileMenu = menuBar->addMenu("File");
        auto openAction = fileMenu->addAction("Open");
        connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
        setMenuBar(menuBar);
    }

    m_treeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_treeView->setVerticalScrollMode(QTreeView::ScrollPerPixel);
    m_treeView->header()->setSectionsMovable(false);
    m_treeView->header()->setSectionsClickable(false);
    m_treeView->header()->setSortIndicatorShown(false);

    resize(1200, 600);
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
                                                   "TrueType Fonts (*.ttf *.otf *.ttc *.otc)");
    if (!path.isEmpty()) {
        loadFile(path);
    }
}

void MainWindow::loadFile(const QString &filePath)
{
    m_hexView->clear();
    m_model.reset(new TreeModel());
    m_file.close();
    m_currentPath.clear();

    m_file.setFileName(filePath);
    if (!m_file.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open a file.");
        return;
    }

    if (m_file.size() > UINT_MAX) {
        QMessageBox::critical(this, "Error", "The selected file is too big.");
        return;
    }

    const uchar* data = m_file.map(0, m_file.size());

    m_currentPath = filePath;

    try {
        QElapsedTimer timer;
        timer.start();

        Parser parser(data, m_file.size(), m_model.get()->rootItem());
        const auto warnings = TrueType::parse(parser);

        auto ranges = parser.ranges();
        m_hexView->setData(data, m_file.size(), std::move(ranges));

        const auto elapsedMs = (double)timer.nsecsElapsed() / 1000000.0;
        qDebug().noquote() << QString::number(elapsedMs, 'f', 1) + "ms";

        if (!warnings.isEmpty()) {
            QMessageBox::warning(this, "Warning", warnings.join('\n'));
        }
    } catch (const QString &msg) {
        QMessageBox::warning(this, "Error", msg);

        auto ranges = Ranges {
            { 0, (quint32)m_file.size(), },
            { 0, (quint32)m_file.size(), },
        };
        m_hexView->setData(data, m_file.size(), std::move(ranges));
    } catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error.");

        auto ranges = Ranges {
            { 0, (quint32)m_file.size(), },
            { 0, (quint32)m_file.size(), },
        };
        m_hexView->setData(data, m_file.size(), std::move(ranges));
    }

    m_treeView->setModel(m_model.get());
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onTreeSelectionChanged);

    m_treeView->header()->resizeSection(
        Column::Title, 300);

    m_treeView->header()->resizeSection(
        Column::Value, m_treeView->fontMetrics().horizontalAdvance("00000000000000000000"));

    m_treeView->header()->resizeSection(
        Column::Type, m_treeView->fontMetrics().horizontalAdvance("__LongDateTime__"));

    m_treeView->header()->setSectionResizeMode(Column::Type, QHeaderView::Fixed);
    m_treeView->header()->setStretchLastSection(false);
    m_treeView->resizeColumnToContents(Column::Title);

    setWindowTitle("TTF Explorer: " + filePath);
}

void MainWindow::onTreeSelectionChanged(const QItemSelection &selected,
                                        const QItemSelection & /*deselected*/)
{
    const auto indexes = selected.indexes();
    if (indexes.isEmpty()) {
        return;
    }

    const auto index = indexes.first();
    if (!index.isValid()) {
        m_hexView->clearSelection();
        return;
    }

    const auto item = m_model->itemByIndex(index);
    const auto range = item->range;

    auto msg = QString(" %1..%2 - %3")
        .arg(range.start).arg(range.end).arg(Utils::prettySize(range.size()));

    QStringList itemPath;
    {
        auto parentItem = item->parent();
        while (parentItem != m_model->rootItem()) {
            const auto title = parentItem->title;
            if (!title.isEmpty()) {
                itemPath.prepend(title);
            }

            parentItem = parentItem->parent();
        }
    }

    if (!itemPath.isEmpty()) {
        msg += " - " +itemPath.join(" / ");
    }

    m_lblStatus->setText(msg);

    m_hexView->selectRegion(range);
    m_hexView->scrollTo(int(range.start));
}
