#pragma once

#include <QItemSelectionModel>
#include <QLabel>
#include <QTreeView>
#include <QMainWindow>

#include "verdigris/wobjectdefs.h"

#include "hexview.h"
#include "treemodel.h"

class MainWindow : public QMainWindow
{
    W_OBJECT(MainWindow)

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void loadFile(const QString &path);

private:
    void onStart();
    void onOpenFile();
    void onTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onHexViewByteClicked(const uint index);

private:
    HexView * const m_hexView;
    QTreeView * const m_treeView;
    QLabel * const m_lblStatus;
    QScopedPointer<TreeModel> m_model;
};
