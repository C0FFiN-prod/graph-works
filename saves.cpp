#include <QFileDialog>
#include <QSaveFile>
#include "mainwindow.h"

template<typename T>
char numberToChar(const T &value)
{
    return QString::number(value).toStdString();
}

template<typename T>
QString matrixToString(const T &matrix, const QString &sep = ";", const QString &newLine = "\n")
{
    int h = matrix.size();
    if (!h)
        return {};
    int w = matrix[0].size();
    int i, j;
    QString result("");
    for (i = 0; i != h; i++) {
        for (j = 0; j != w; j++) {
            if (j)
                result += sep;
            result += QString::number(matrix[i][j]);
        }
        result += newLine;
    }
    return result;
}

template QString matrixToString(const Matrix2D &matrix, const QString &sep, const QString &newLine);
template QString matrixToString(const Matrix2I &matrix, const QString &sep, const QString &newLine);

void MainWindow::saveGraphToCSV(bool saveToNew = false)
{
    QString filePath((saveToNew || currentFile.isEmpty())
                         ? QFileDialog::getSaveFileName(this, "Save as", "", "CSV Files (*.csv)")
                         : currentFile);
    if ((!saveToNew && !graph.isUnsaved()) || filePath.isEmpty())
        return;
    QSaveFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "Nodes count\n";
    out << QString::number(graph.getAmount()) + '\n';
    out << "Adjacent\n";
    out << matrixToString(graph.getMatrixAdjacent());
    out << "Bandwidth\n";
    out << matrixToString(graph.getMatrixBandwidth());
    out << "Flow\n";
    out << matrixToString(graph.getMatrixFlow());
    if (file.commit()) {
        currentFile = filePath;
        graph.changesSaved();
        updateFileStatus();
    } else {
        QMessageBox::warning(this, title, "Error during saving:\n" + file.errorString());
    }
}
bool MainWindow::checkForSave()
{
    if (!graph.isUnsaved())
        return true;
    int dialogResponse = QMessageBox::warning(this,
                                              title,
                                              "All unsaved changes will be lost.\nContinue?",
                                              QMessageBox::StandardButton::Yes
                                                  | QMessageBox::StandardButton::Save
                                                  | QMessageBox::StandardButton::No);
    switch (dialogResponse) {
    case QMessageBox::StandardButton::Yes:
        break;
    case QMessageBox::StandardButton::Save:
        saveGraphToCSV();
        break;
    case QMessageBox::StandardButton::No:
    default:
        return false;
    }
    return true;
}

void MainWindow::clearGraph()
{
    if (!checkForSave())
        return;
    graph.clear();
}
void MainWindow::readGraphFromCSV()
{
    if (!checkForSave())
        return;
    graph.clear();
    QString filePath(QFileDialog::getOpenFileName(this, "Save as", "", "CSV Files (*.csv)"));
    if (filePath == currentFile || filePath.isEmpty())
        return;
    QFile file(filePath);
    file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly | QIODevice::Text);
    QTextStream fstr(&file);
    QStringList data(fstr.readAll().split('\n'));
    int cursor, amount, dataLength = data.size();
    QMap<QString, int> headers{{"Nodes count", -1},
                               {"Adjacent", -1},
                               {"Bandwidth", -1},
                               {"Flow", -1}};
    for (int headersSetCnt = 0, i = 0; i < dataLength && headersSetCnt < 4; i++) {
        if (headers.contains(data[i])) {
            if (headers[data[i]] == -1)
                headersSetCnt++;
            headers[data[i]] = i;
        }
    }
    auto wrongFormat = [this]() { QMessageBox::warning(this, title, "Wrong file format."); };
    if (headers["Nodes count"] == -1 || headers["Adjacent"] == -1) {
        wrongFormat();
        return;
    }
    cursor = headers["Nodes count"];
    if (cursor + 1 >= dataLength || !reValidInt.match(data[cursor + 1]).hasMatch()) {
        wrongFormat();
        return;
    }
    amount = data[cursor + 1].toInt();
    QStringList dataLine(amount);
    auto fillMatrixFromData =
        [&cursor, &amount, &dataLine, &wrongFormat, &dataLength, &data](Matrix2D &m) {
            if ((cursor == -1) || (cursor + amount + 1 > dataLength)) {
                wrongFormat();
                return;
            }
            bool ok;
            for (int shift = cursor + 1, i = 0; i < amount; i++) {
                dataLine = data[shift + i].split(';');
                if (dataLine.count() != amount) {
                    wrongFormat();
                    return;
                }
                for (int j = amount; j--;) {
                    m[i][j] = dataLine[j].toDouble(&ok);
                    if (!ok) {
                        wrongFormat();
                        return;
                    }
                }
            }
        };
    Matrix2D adjacent(amount, QList<double>(amount, 0));
    cursor = headers["Adjacent"];
    fillMatrixFromData(adjacent);
    Matrix2D bandwidth(amount, QList<double>(amount, 0));
    cursor = headers["Bandwidth"];
    if (cursor != -1)
        fillMatrixFromData(bandwidth);
    Matrix2D flow(amount, QList<double>(amount, 0));
    cursor = headers["Flow"];
    if (cursor != -1)
        fillMatrixFromData(flow);
    graph.setMatrixAdjacent(adjacent);
    if (headers["Bandwidth"] != -1)
        graph.setMatrixBandwidth(bandwidth);
    if (headers["Flow"] != -1)
        graph.setMatrixFlow(flow);
    currentFile = filePath;
    graph.changesSaved();
    updateTables();
    updateFileStatus();
}

void MainWindow::newGraph()
{
    if (!checkForSave())
        return;
    currentFile.clear();
    graph.clear();
    graph.changesSaved();
    for (auto &m : graphMatrixViews) {
        auto model = static_cast<QStandardItemModel *>(m->model());
        model->setColumnCount(0);
        model->setRowCount(0);
    }
    for (auto &m : graphListViews) {
        auto model = static_cast<QStandardItemModel *>(m->model());
        model->setRowCount(0);
        addRowToList(model);
    }
    updateFileStatus();
}
