#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h" // the header file of QCustomPlot. Don't forget to add it to your project, if you use an IDE, so it gets compiled.
#include "DSOutils.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void setupPlot(QCustomPlot *customPlot);
    ~MainWindow();

private slots:

    void on_btnExit_clicked();

    void on_btnGet_clicked();

    void updatePlot();

    void on_comboBoxV1div_currentIndexChanged(int index);

    void on_comboSampling_currentIndexChanged(int index);

    void on_dialTrigger_valueChanged(int value);

    void on_dialCursorX1_valueChanged(int value);

    void on_dialCursorX2_valueChanged(int value);

    void onYRangeChanged(const QCPRange &range);

    void on_comboBoxV2div_currentIndexChanged(int index);

    void on_dialCursorV1_valueChanged(int value);

    void on_dialCursorV1_2_valueChanged(int value);

    void on_comboBoxCHSel_currentIndexChanged(int index);

    void on_comboBox_4_currentIndexChanged(int index);

    void on_comboMemDepth_currentIndexChanged(int index);

    void on_actionSave_to_file_triggered();

    void on_checkBoxCH1ON_toggled(bool checked);

    void on_checkBoxCH2ON_toggled(bool checked);

    void on_comboBox_rise_currentIndexChanged(int index);

    void on_actionCalibrate_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
