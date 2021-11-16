#pragma once

#include <QItemSelectionModel>
#include <QLabel>
#include <QTreeView>
#include <QMainWindow>
#include <QFile>

#include "parser.h"
#include "hexview.h"
#include "treemodel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void loadFile(const QString &filePath);

private:
    void onStart();
    void onOpenFile();
    void onTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    HexView * const m_hexView;
    QTreeView * const m_treeView;
    QLabel * const m_lblStatus;
    QScopedPointer<TreeModel> m_model;
    QString m_currentPath;
    QFile m_file;
};
