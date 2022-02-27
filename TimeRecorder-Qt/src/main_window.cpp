#include "main_window.h"
#include <QHeaderView>
#include <QIcon>
#include <QStandardItemModel>
#include <iostream>
#include <vector>
#include <windows.h>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    this->setGeometry(1000, 300, 550, 500);
    this->setWindowTitle("TimeRecorder");
    this->setWindowIcon(QIcon("../image/icon.jpg"));

    m_tr_core = new QProcess(this);
    m_tr_core->start("TimeRecorderCore.exe");
    if (!m_tr_core->waitForStarted())
        qDebug() << "init failed\n";

    // m_timer
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::write_ls);
    m_timer->start(1000);

    connect(m_tr_core, &QProcess::readyReadStandardOutput, this,
            &MainWindow::read_all);

    // m_tree_model
    m_tree_model = new QStandardItemModel(m_tree);

    // m_tree
    m_tree = new QTreeView(this);
    m_tree->setModel(m_tree_model);
    this->setCentralWidget(m_tree);
    // m_tree->setColumnWidth(1, 400); // 设置第一列宽度
}

MainWindow::~MainWindow() {}

void MainWindow::UpdateTreeData(const tr::api::_apps& apps) {
    m_tree_model->setHorizontalHeaderLabels(
        QStringList() << QStringLiteral("名称") << QStringLiteral("最近使用")
                      << QStringLiteral("今日使用")
                      << QStringLiteral("总计使用"));

    auto rowCount = apps.size();
    auto colCount = 4;

    m_tree_model->setRowCount(rowCount);
    m_tree_model->setColumnCount(colCount);

    for (int row = 0; row < rowCount; row++) {
        QStandardItem* col_0 = new QStandardItem();
        col_0->setText(apps[row][0].name +
                       QString(" (%1)").arg(apps[row].size()));
        for (auto& i : apps[row]) {
            QList<QStandardItem*> items;
            items.append(new QStandardItem(i.name));
            items.append(new QStandardItem(i.recently));
            items.append(new QStandardItem(i.today));
            items.append(new QStandardItem(i.total));
            col_0->appendRow(items);
        }
        m_tree_model->setItem(row, 0, col_0);
        m_tree_model->setItem(row, 1, new QStandardItem(apps[row][0].recently));
        m_tree_model->setItem(row, 2, new QStandardItem(apps[row][0].today));
        m_tree_model->setItem(row, 3, new QStandardItem(apps[row][0].total));
    }
}

void MainWindow::write_ls() { m_tr_core->write("ls\n"); }

void MainWindow::read_all() {
    QByteArray result = m_tr_core->readAll();
    try {
        nlohmann::json j = nlohmann::json::parse(result.toStdString());
        auto apps = j.get<tr::api::_apps>();
        UpdateTreeData(apps);
    } catch (...) {
        ; // parse failed
    }
}