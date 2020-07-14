#include <QApplication>
#include <QPainter>

#include "treemodel.h"

// A custom delegate that allows rendering title + number pairs effectively.
void TitleItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();

    QString str = opt.text;
    opt.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    opt.rect.translate(5, 0);

    const auto id = index.internalId();
    const auto model = static_cast<const TreeModel*>(index.model());

    if (widget) {
        // Prevent flickering.
        painter->setPen(widget->palette().color(QPalette::Active, QPalette::Text));
    }

    const auto title = model->itemTitle(id);
    const auto idx = model->itemIndex(id);

    // Fill the cache if needed.
    while (idx >= m_indexCache.size()) {
        m_indexCache.append(QString::number(m_indexCache.size()));
    }

    const auto indexWidth = idx
            ? painter->fontMetrics().horizontalAdvance(m_indexCache.at(idx.value()))
            : 0;

    const auto itemWidth =
          opt.rect.width()
        - painter->fontMetrics().horizontalAdvance(QLatin1String("   ")) // spacing
        - indexWidth;

    const auto elidedTitle = painter->fontMetrics()
        .elidedText(title, Qt::ElideRight, itemWidth);

    QRect textRect;
    painter->drawText(opt.rect, Qt::AlignLeft | Qt::AlignVCenter, elidedTitle, &textRect);

    if (title != elidedTitle) {
        // When title was elided there is no point in rendering the number.
        return;
    }

    // Render number when present.
    if (idx.has_value()) {
        // Advance to title end.
        opt.rect.translate(textRect.width(), 0);
        // Space.
        opt.rect.translate(painter->fontMetrics().horizontalAdvance(QChar(0x20)), 0);

        painter->drawText(opt.rect, Qt::AlignLeft | Qt::AlignVCenter, m_indexCache.at(idx.value()));
    }
}

TreeModel::TreeModel(TTFCore::Tree &&tree)
    : QAbstractItemModel(nullptr)
    , m_tree(std::move(tree))
{
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case Column::Title : return m_tree.itemTitle(index.internalId());
            case Column::Value : return m_tree.itemValue(index.internalId());
            case Column::Type : return m_tree.itemValueType(index.internalId());
            default: break;
        }
    } else if (role == Qt::ToolTipRole) {
        if (index.column() == Column::Value) {
            return m_tree.itemValue(index.internalId());
        }
    }

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Column::Title : return QLatin1String("Title");
            case Column::Value : return QLatin1String("Value");
            case Column::Type  : return QLatin1String("Type");
            default : break;
        }
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TreeItemId parentId = m_rootId;
    if (parent.isValid()) {
        parentId = parent.internalId();
    }

    if (const auto childId = m_tree.childAt(parentId, row)) {
        return createIndex(row, column, childId.value());
    } else {
        return QModelIndex();
    }
}

QModelIndex TreeModel::index(const TreeItemId id) const
{
    return createIndex(m_tree.childIndex(id), 0, id);
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    const auto parentIdOpt = parentItem(index.internalId());
    if (!parentIdOpt) {
        return QModelIndex();
    }
    const auto parentId = parentIdOpt.value();

    if (parentId == m_rootId) {
        return QModelIndex();
    }

    return createIndex(0, 0, parentId);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_tree.childrenCount(m_rootId);
    } else {
        return m_tree.childrenCount(parent.internalId());
    }
}

int TreeModel::columnCount(const QModelIndex &) const
{
    return Column::LastColumn;
}

QVector<Range> TreeModel::collectRanges() const
{
    return m_tree.collectRanges();
}

std::optional<TreeItemId> TreeModel::parentItem(const TreeItemId id) const
{
    return m_tree.parentItem(id);
}

QString TreeModel::itemTitle(const TreeItemId id) const
{
    return m_tree.itemTitle(id);
}

std::optional<quint32> TreeModel::itemIndex(const TreeItemId id) const
{
    return m_tree.itemIndex(id);
}

Range TreeModel::itemRange(const TreeItemId id) const
{
    return m_tree.itemRange(id);
}

std::optional<TreeItemId> TreeModel::itemByByte(const uint index) const
{
    return m_tree.itemAtByte(index);
}
