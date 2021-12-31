#pragma once

#include <QAbstractScrollArea>
#include <QStaticText>

#include <optional>

#include "range.h"

class HexView : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit HexView(QWidget *parent = nullptr);

    void setData(const uchar *data, const quint32 dataSize, Ranges &&ranges);
    void clear();

    void selectRegion(const Range &region);
    void clearSelection();

    void scrollTo(const int offset);

signals:
    void byteClicked(uint);

private:
    void prepareMinWidth();

    void drawView(QPainter &p) const;

    int cursorAt(const QPoint &point) const;
    int cursorFromMousePos(const QPoint &mousePos) const;
    quint32 maxLinesPerView() const;
    quint32 lastVisibleRow() const;
    quint32 scrollPosition() const;

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *e);

private:
    const QVector<QStaticText> m_hexTable;
    const uchar * m_data = nullptr;
    quint64 m_dataSize = 0;
    Ranges m_ranges;
    quint64 m_totalLines = 0;
    std::optional<Range> m_selection;

    struct {
        int hexWidth = 0;
        int blockWidth = 0;
        int width = 0;
        int height = 0;
        int ascent = 0;
        int descent = 0;
        int padding = 0;
    } m_charMetrics;

    struct Pixmaps {
        QPixmap rangeStart;
        QPixmap rangeMiddle;
        QPixmap rangeEnd;
        QPixmap rangeSingle;
    } m_pixmaps;
};
