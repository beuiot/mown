#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QTimer>
#include <string>
#include "qdebugstream.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    std::string m_CurrentProjectFolder;
    std::string m_CurrentLocalUrl;
    void Load(std::string path);
    void Export(bool local);
    void DelayedExport();
    bool CanExport();

    void LoadSettings();
    void SaveSettings();

    void UpdateWatcher(const QString &fileName);

    QFileSystemWatcher m_Watcher;
    QTimer m_ExportTimer;

    QDebugStream * m_QOut;
    QDebugStream * m_QOutError;

    bool m_IgnoreUIEvents;

private slots:
    void on_exportButton_clicked();
    void on_loadButton_clicked();
    void fileChangedSlot(QString path);
    void directoryChangedSlot(QString path);
    void exportTimerTimeout();
    void on_refreshButton_clicked();
    void on_forceAll_toggled(bool checked);
    void on_localPreview_toggled(bool checked);
    void on_enableComments_toggled(bool checked);
    void on_indexButton_clicked();
    void on_showConsoleButton_clicked();
    void on_serverPath_editingFinished();
};

#endif // MAINWINDOW_H
