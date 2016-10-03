#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QTimer>
#include <string>
#include "qdebugstream.h"
#include "mown.h"

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
	Mown m_Mown;
	std::string m_CurrentProjectFolder;
	std::string m_CurrentLocalUrl;
	std::string m_CurrentSourceFile;
	void Load(std::string path);
	void Export(bool local);
	void DelayedExport();
	bool CanExport();

	void LoadSettings();
	void SaveSettings();

	void UpdateWatcher(const QString &fileName);

	QFileSystemWatcher m_Watcher;
	QTimer m_ExportTimer;
	QTimer m_NewFileCheckTimer;

	QDebugStream * m_QOut;
	QDebugStream * m_QOutError;

	bool m_IgnoreUIEvents;

	private slots:
	void on_exportButton_clicked();
	void on_loadButton_clicked();
	void on_openSourceFileButton_clicked();
	void on_openSourceFolderButton_clicked();
	void on_createArticleButton_clicked();
	void fileChangedSlot(QString path);
	void directoryChangedSlot(QString path);
	void exportTimerTimeout();
	void newFileTimerTimeout();
	void on_refreshButton_clicked();
	void on_forceAll_toggled(bool checked);
	void on_localPreview_toggled(bool checked);
	void on_enableComments_toggled(bool checked);
	void on_indexButton_clicked();
	void on_showConsoleButton_clicked();
	void on_serverPath_editingFinished();
	void on_webView_urlChanged(const QUrl& url);
};

#endif // MAINWINDOW_H
