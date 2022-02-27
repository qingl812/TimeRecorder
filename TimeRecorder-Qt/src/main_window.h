#ifndef MAIN_WINDOW_H__
#define MAIN_WINDOW_H__

#include "api.h"
#include <QMainWindow>
#include <QProcess>
#include <QStandardItemModel>
#include <QTableView>
#include <QTableWidget>
#include <QTimer>
#include <QtreeView>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    QTableView* table_all;
    QTreeView* m_tree;
    QStandardItemModel* m_tree_model;
    QProcess* m_tr_core;
    QTimer* m_timer;

    void UpdateTreeData(const tr::api::_apps& apps);
private slots:
    void write_ls();
    void read_all();
};
#endif // MAIN_WINDOW_H__
