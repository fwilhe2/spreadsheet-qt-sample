// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "spreadsheet.h"
#include "spreadsheetdelegate.h"
#include "spreadsheetitem.h"

#include <QtWidgets>

SpreadSheet::SpreadSheet(int rows, int cols, QWidget *parent)
    : QMainWindow(parent),
      toolBar(new QToolBar(this)),
      cellLabel(new QLabel(toolBar)),
      table(new QTableWidget(rows, cols, this)),
      formulaInput(new QLineEdit(this))
{
    addToolBar(toolBar);

    cellLabel->setMinimumSize(80, 0);

    toolBar->addWidget(cellLabel);
    toolBar->addWidget(formulaInput);

    table->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    for (int c = 0; c < cols; ++c) {
        QString character(QChar('A' + c));
        table->setHorizontalHeaderItem(c, new QTableWidgetItem(character));
    }

    table->setItemPrototype(table->item(rows - 1, cols - 1));
    table->setItemDelegate(new SpreadSheetDelegate());

    createActions();
    setupMenuBar();
    setupContents();
    setupContextMenu();
    setCentralWidget(table);

    statusBar();
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheet::updateStatus);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheet::updateLineEdit);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheet::updateStatus);
    connect(formulaInput, &QLineEdit::returnPressed,
            this, &SpreadSheet::returnPressed);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheet::updateLineEdit);

    setWindowTitle(tr("Spreadsheet"));
}

void SpreadSheet::createActions()
{
    aboutSpreadSheet = new QAction(tr("About Spreadsheet"), this);
    connect(aboutSpreadSheet, &QAction::triggered, this, &SpreadSheet::showAbout);

    exitAction = new QAction(tr("E&xit"), this);
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    firstSeparator = new QAction(this);
    firstSeparator->setSeparator(true);

    secondSeparator = new QAction(this);
    secondSeparator->setSeparator(true);
}

//! [implicit tr context]
void SpreadSheet::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
//! [implicit tr context]
    fileMenu->addAction(exitAction);

    menuBar()->addSeparator();

    QMenu *aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutSpreadSheet);
}

void SpreadSheet::updateStatus(QTableWidgetItem *item)
{
    if (item && item == table->currentItem()) {
        statusBar()->showMessage(item->data(Qt::StatusTipRole).toString(), 1000);
        cellLabel->setText(tr("Cell: (%1)").arg(encode_pos(table->row(item), table->column(item))));
    }
}

void SpreadSheet::updateLineEdit(QTableWidgetItem *item)
{
    if (item != table->currentItem())
        return;
    if (item)
        formulaInput->setText(item->data(Qt::EditRole).toString());
    else
        formulaInput->clear();
}

void SpreadSheet::returnPressed()
{
    QString text = formulaInput->text();
    int row = table->currentRow();
    int col = table->currentColumn();
    QTableWidgetItem *item = table->item(row, col);
    if (!item)
        table->setItem(row, col, new SpreadSheetItem(text));
    else
        item->setData(Qt::EditRole, text);
    table->viewport()->update();
}

void SpreadSheet::clear()
{
    const QList<QTableWidgetItem *> selectedItems = table->selectedItems();
    for (QTableWidgetItem *i : selectedItems)
        i->setText(QString());
}

void SpreadSheet::setupContextMenu()
{
    addAction(clearAction);
    setContextMenuPolicy(Qt::ActionsContextMenu);
}

void SpreadSheet::setupContents()
{

}

const char *htmlText =
"<HTML>"
"</HTML>";

void SpreadSheet::showAbout()
{
    QMessageBox::about(this, "About Spreadsheet", htmlText);
}

void decode_pos(const QString &pos, int *row, int *col)
{
    if (pos.isEmpty()) {
        *col = -1;
        *row = -1;
    } else {
        *col = pos.at(0).toLatin1() - 'A';
        *row = pos.right(pos.size() - 1).toInt() - 1;
    }
}

QString encode_pos(int row, int col)
{
    return QString(char16_t(col + 'A')) + QString::number(row + 1);
}

