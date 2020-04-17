#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QScrollBar>
#include <QStyle>

#include <cmath>

#include "hexview.h"

static constexpr int BytesPerLine = 16;

static QVector<QStaticText> initHexTable(const QFont &font)
{
    QVector<QStaticText> vec;
    for (int i = 0; i < 256; ++i) {
        const auto text = QString::number(i, 16).toUpper().rightJustified(2, QLatin1Char('0'));
        QStaticText staticText(text);
        staticText.prepare(QTransform(), font);
        vec << staticText;
    }

    return vec;
}

static QPixmap renderRegion(const QSize &size, const QRect &rect, const QColor &color)
{
    const auto scaleRatio = static_cast<int>(qApp->primaryScreen()->devicePixelRatio());

    const QRect scaledRect(rect.x(), rect.y(), rect.width() * scaleRatio, rect.height() * scaleRatio);

    QPixmap pix(size * scaleRatio);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setOpacity(0.5);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawRoundedRect(scaledRect, 4, 4);
    p.end();

    pix.setDevicePixelRatio(scaleRatio);

    return pix;
}

HexView::HexView(QWidget *parent)
    : QAbstractScrollArea(parent)
    , m_hexTable(initHexTable(QFont(QLatin1String("monospace"))))
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](){
        viewport()->update();
    });

    viewport()->setFont(QFont(QLatin1String("monospace")));

    const auto fm = viewport()->fontMetrics();
    m_charMetrics.width = fm.horizontalAdvance(QLatin1Char('0'));
    m_charMetrics.padding = m_charMetrics.width / 2;
    m_charMetrics.height = fm.height();
    m_charMetrics.ascent = fm.ascent();
    m_charMetrics.descent = fm.descent();
    m_charMetrics.hexWidth = m_charMetrics.width * 2;
    m_charMetrics.blockWidth = m_charMetrics.width * 3;

    {
        const QSize size(m_charMetrics.blockWidth, m_charMetrics.height + 1);
        const auto brush = palette().color(QPalette::Highlight);

        // Larger rect will be clipped and that's what we want.
        m_pixmaps.rangeSingle = renderRegion(size, QRect(0, 0, size.width() - 3, size.height()), brush);
        m_pixmaps.rangeStart = renderRegion(size, QRect(0, 0, size.width() + 6, size.height()), brush);
        m_pixmaps.rangeMiddle = renderRegion(size, QRect(-6, 0, size.width() + 12, size.height()), brush);
        m_pixmaps.rangeEnd = renderRegion(size, QRect(-6, 0, size.width(), size.height()), brush);
    }

    prepareMinWidth();
}

void HexView::setData(const QByteArray &data, const QVector<Range> &ranges)
{
    Q_ASSERT(!ranges.isEmpty());

    clear();
    m_totalLines = static_cast<quint32>(std::ceil(static_cast<double>(data.size()) / BytesPerLine));
    int rangeIdx = 0;

    for (quint32 i = 0; i < quint32(data.size()); ++i) {
        auto posType = RangePosType::None;

        const auto range = ranges.at(rangeIdx);
        if (range.contains(i)) {
            if (range.isSingle()) {
                posType = RangePosType::Single;

                // Jump to the next range.
                // Ranges must be sorted.
                if (rangeIdx + 1 < ranges.size()) {
                    rangeIdx += 1;
                }
            } else if (range.isStart(i)) {
                posType = RangePosType::Start;
            } else if (range.isEnd(i)) {
                posType = RangePosType::End;

                // Jump to the next range.
                // Ranges must be sorted.
                if (rangeIdx + 1 < ranges.size()) {
                    rangeIdx += 1;
                }
            } else {
                posType = RangePosType::Middle;
            }
        }

        m_data.append(HexViewByte {
            static_cast<uchar>(data.at(int(i))),
            posType,
        });
    }

    verticalScrollBar()->setMaximum(qMax(0, int(m_totalLines - maxLinesPerView())));
}

void HexView::clear()
{
    m_data.clear();
    m_totalLines = 0;
    m_selection = std::nullopt;
}

void HexView::selectRegion(const Range &region)
{
    m_selection = region;
    viewport()->update();
}

void HexView::clearSelection()
{
    m_selection = std::nullopt;
    viewport()->update();
}

void HexView::scrollTo(const int offset)
{
    const auto row = static_cast<quint32>(std::floor(static_cast<double>(offset) / BytesPerLine));

    if (row >= scrollPosition() && row <= scrollPosition() + maxLinesPerView()) {
        // Do not reset scroll when selected data is already in a view.
        return;
    }

    verticalScrollBar()->setValue(int(row));
}

void HexView::paintEvent(QPaintEvent *)
{
    if (m_data.isEmpty()) {
        return;
    }

    QPainter p(viewport());
    drawView(p);
}

void HexView::drawView(QPainter &p) const
{
    const QRect r = p.viewport();

    // draw lines
    auto lineIdx = scrollPosition();
    const int maxH = r.height() + m_charMetrics.height + 4;
    for (int y = m_charMetrics.height; y < maxH; y += m_charMetrics.height + 4) {
        drawLine(p, lineIdx, y);
        lineIdx += 1;

        if (lineIdx >= m_totalLines) {
            break;
        }
    }
}

void HexView::drawLine(QPainter &p, const quint32 lineIdx, const int y) const
{
    const quint32 startIdx = lineIdx * BytesPerLine;
    const quint32 endIdx = quint32(qMin(int(startIdx) + BytesPerLine, m_data.size()));

    int x = m_charMetrics.padding;
    for (quint32 i = startIdx; i < endIdx; ++i) {
        const auto viewByte = m_data.at(int(i));

        if (m_selection && m_selection->contains(i)) {
            const auto range = m_selection.value();
            const auto imgX = x - m_charMetrics.padding / 2;
            const auto imgY = y - m_charMetrics.height + m_charMetrics.descent;
            if (range.isSingle()) {
                p.drawPixmap(imgX, imgY, m_pixmaps.rangeSingle);
            } else if (range.isStart(i)) {
                p.drawPixmap(imgX, imgY, m_pixmaps.rangeStart);
            } else if (range.isMiddle(i)) {
                p.drawPixmap(imgX, imgY, m_pixmaps.rangeMiddle);
            } else if (range.isEnd(i)) {
                p.drawPixmap(imgX, imgY, m_pixmaps.rangeEnd);
            }
        } else {
            const auto c = palette().color(QPalette::Active, QPalette::Highlight).darker(150);
            const auto uy = y + 3;
            const auto h = 2;

            switch (viewByte.posType) {
                case RangePosType::None : break;
                case RangePosType::Single : {
                    p.fillRect(x, uy, m_charMetrics.hexWidth, h, c);
                    break;
                }
                case RangePosType::Start : {
                    const auto w = i + 1 == endIdx ? m_charMetrics.hexWidth : m_charMetrics.blockWidth;
                    p.fillRect(x, uy, w, h, c);
                    break;
                }
                case RangePosType::Middle : {
                    const auto w = i + 1 == endIdx ? m_charMetrics.hexWidth : m_charMetrics.blockWidth;
                    p.fillRect(x, uy, w, h, c);
                    break;
                }
                case RangePosType::End : {
                    p.fillRect(x, uy, m_charMetrics.hexWidth, h, c);
                    break;
                }
            }
        }

        if (viewByte.posType == RangePosType::None) {
            p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
        } else {
            p.setPen(palette().color(QPalette::Active, QPalette::Text));
        }

        p.drawStaticText(x, y - m_charMetrics.ascent, m_hexTable.at(viewByte.byte));
        p.setPen(palette().color(QPalette::Active, QPalette::Text));

        x += m_charMetrics.blockWidth;
    }
}

int HexView::cursorAt(const QPoint &point) const
{
    const int y = point.y() - 2;
    const int row = qBound(0, verticalScrollBar()->value() + y / (m_charMetrics.height + 4), int(m_totalLines));
    const int x = point.x();
    const int col = qBound(0, x / m_charMetrics.blockWidth, 15);
    return row * BytesPerLine + col;
}

int HexView::cursorFromMousePos(const QPoint &mousePos) const
{
    return cursorAt(mousePos);
}

quint32 HexView::maxLinesPerView() const
{
    return static_cast<quint32>(std::ceil(static_cast<double>(viewport()->height()) / (m_charMetrics.height + 4) - 1));
}

quint32 HexView::lastVisibleRow() const
{
    return scrollPosition() + maxLinesPerView();
}

quint32 HexView::scrollPosition() const
{
    if (verticalScrollBar()->value() > 0) {
        return quint32(verticalScrollBar()->value());
    } else {
        return 0;
    }
}

void HexView::resizeEvent(QResizeEvent *)
{
    if (m_data.isEmpty()) {
        return;
    }

    const auto max = qMax(0u, m_totalLines - maxLinesPerView());
    verticalScrollBar()->setMaximum(int(max));
}

void HexView::mousePressEvent(QMouseEvent *e)
{
    if (m_data.isEmpty()) {
        return;
    }

    if (e->button() != Qt::LeftButton) {
        return;
    }

    const auto pos = cursorFromMousePos(e->pos());
    if (pos >= 0) {
        emit byteClicked(uint(pos));
    }
}

void HexView::prepareMinWidth()
{
    setFixedWidth(
          m_charMetrics.blockWidth * BytesPerLine
        + 5 // TODO: find exact value
        + style()->pixelMetric(QStyle::PM_ScrollBarExtent)
    );
}
