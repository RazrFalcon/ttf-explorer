#pragma once

#include <QApplication>
#include <QFileOpenEvent>

// Required to handle double-click on file events on macOS.

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv)
        : QApplication(argc, argv)
    {
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            auto openEvent = static_cast<QFileOpenEvent*>(event);
            emit openFile(openEvent->file());
        }

        return QApplication::event(event);
    }

signals:
    void openFile(QString);
};
