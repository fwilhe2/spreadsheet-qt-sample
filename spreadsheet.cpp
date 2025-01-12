// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "spreadsheet.h"
#include "spreadsheetdelegate.h"
#include "spreadsheetitem.h"

#include <QtWidgets>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDebug>

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

    QVector<QVector<QString>> sampleData = {
    { "", "2020", "2021", "2022" },
    { "Acc 1", "2.22", "3.22", "4.22" },
    { "Acc 2", "5.22", "6.22", "9.22" },
    { "Acc 3", "12.22", "13.22", "14.22" },
    { "Sum", "", "", "" },
    { "Diff", "", "", "" }
};

    loadSpreadsheetFromXml("/home/florian/code/fwilhe2/kalkulationsbogen-examples/accountsSpreadsheet.fods");
    // saveSpreadsheetToXml("spreadsheet.fods", sampleData);
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

void loadSpreadsheetFromXml(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for reading";
        return;
    }

    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "table" && xml.namespaceUri() == "urn:oasis:names:tc:opendocument:xmlns:table:1.0") {
                QString tableName = xml.attributes().value("name").toString();
                qDebug() << "Table Name:" << tableName;
                continue;
            }
            if (xml.name() == "table-row" && xml.namespaceUri() == "urn:oasis:names:tc:opendocument:xmlns:table:1.0") {
                int rowIndex = 0; // You can manage row index manually if needed
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "table-row")) {
                    token = xml.readNext();
                    if (token == QXmlStreamReader::StartElement && xml.name() == "table-cell" && xml.namespaceUri() == "urn:oasis:names:tc:opendocument:xmlns:table:1.0") {
                        QString valueType = xml.attributes().value("value-type").toString();
                        QString cellValue;
                        if (valueType == "string") {
                            if (xml.readNextStartElement() && xml.name() == "p" && xml.namespaceUri() == "urn:oasis:names:tc:opendocument:xmlns:text:1.0") {
                                cellValue = xml.attributes().value("value").toString();
                            }
                        } else if (valueType == "currency") {
                            cellValue = xml.attributes().value("value").toString();
                        }
                        // Load data into your spreadsheet structure
                        qDebug() << "Row:" << rowIndex << "Cell Value:" << cellValue << "Value Type:" << valueType;
                    }
                }
                rowIndex++;
            }
        }
    }

    if (xml.hasError()) {
        qDebug() << "XML error: " << xml.errorString();
    }

    file.close();
}

void saveSpreadsheetToXml(const QString &filePath, const QVector<QVector<QString>> &spreadsheetData) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for writing";
        return;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("office:document");

    // Writing all the namespaces
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:office:1.0", "office");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:table:1.0", "table");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:text:1.0", "text");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:style:1.0", "style");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0", "fo");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0", "svg");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:chart:1.0", "chart");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0", "dr3d");
    xml.writeNamespace("http://www.w3.org/1998/Math/MathML", "math");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:form:1.0", "form");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:script:1.0", "script");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:config:1.0", "config");
    xml.writeNamespace("http://www.w3.org/1999/xlink", "xlink");
    xml.writeNamespace("http://purl.org/dc/elements/1.1/", "dc");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:meta:1.0", "meta");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0", "number");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:of:1.2", "of");
    xml.writeNamespace("http://www.w3.org/2002/xforms", "xforms");
    xml.writeNamespace("http://www.w3.org/2001/XMLSchema", "xsd");
    xml.writeNamespace("http://www.w3.org/2001/XMLSchema-instance", "xsi");
    xml.writeNamespace("http://www.w3.org/2003/g/data-view#", "grddl");
    xml.writeNamespace("http://www.w3.org/1999/xhtml", "xhtml");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:presentation:1.0", "presentation");
    xml.writeNamespace("http://www.w3.org/TR/css3-text/", "css3t");
    xml.writeNamespace("urn:openoffice:names:experimental:ooxml-odf-interop:xmlns:form:1.0", "formx");
    xml.writeNamespace("http://openoffice.org/2004/calc", "oooc");
    xml.writeNamespace("http://openoffice.org/2004/writer", "ooow");
    xml.writeNamespace("http://openoffice.org/2005/report", "rpt");
    xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0", "draw");
    xml.writeNamespace("http://openoffice.org/2004/office", "ooo");
    xml.writeNamespace("urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0", "calcext");
    xml.writeNamespace("http://openoffice.org/2009/table", "tableooo");
    xml.writeNamespace("http://openoffice.org/2010/draw", "drawooo");
    xml.writeNamespace("urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0", "loext");
    xml.writeNamespace("http://www.w3.org/2001/xml-events", "dom");
    xml.writeNamespace("urn:openoffice:names:experimental:ooo-ms-interop:xmlns:field:1.0", "field");

    // Adding office-specific attributes
    xml.writeAttribute("office:version", "1.3");
    xml.writeAttribute("office:mimetype", "application/vnd.oasis.opendocument.spreadsheet");

    xml.writeStartElement("office:body");
    xml.writeStartElement("office:spreadsheet");
    xml.writeStartElement("table:table");
    xml.writeAttribute("table:name", "Sheet1");

    for (const auto &row : spreadsheetData) {
        xml.writeStartElement("table:table-row");

        for (const auto &cell : row) {
            xml.writeStartElement("table:table-cell");
            xml.writeAttribute("office:value-type", "string");
            xml.writeStartElement("text:p");
            xml.writeCharacters(cell);
            xml.writeEndElement(); // text:p
            xml.writeEndElement(); // table:table-cell
        }

        xml.writeEndElement(); // table:table-row
    }

    xml.writeEndElement(); // table:table
    xml.writeEndElement(); // office:spreadsheet
    xml.writeEndElement(); // office:body
    xml.writeEndElement(); // office:document
    xml.writeEndDocument();

    file.close();
}