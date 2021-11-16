#include "utils.h"

QString Utils::monospacedFont()
{
#ifdef Q_OS_MACX
    return "Monaco";
#else
    return "monospace";
#endif
}

QString Utils::prettySize(const quint32 size)
{
    QString number = QString::number(size);
    const auto len = number.size();
    if (len > 3) {
        number.insert(number.size() - 3, ' ');
    }
    if (len > 6) {
        number.insert(number.size() - 7, ' ');
    }

    return number + " B";
}
