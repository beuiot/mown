#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebugstream.h"
#include <QFileDialog>
#include <QDir>
#include <QSettings>

/* TODO:
 *
 * Remember last configuration
 *
 * Random content generator
 * Benchmarks
 *
 * Proper tests
 * Make real impl for ContentFactory
 *
 * uses: yaml-cpp, boost, qt 5.4
 * */

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m_IgnoreUIEvents = false;

	connect(&m_Watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedSlot(QString)));
	connect(&m_Watcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChangedSlot(QString)));

	connect(&m_ExportTimer, SIGNAL(timeout()), this, SLOT(exportTimerTimeout()));

	connect(ui->statusBar, SIGNAL(mouseClick()), this, SLOT(on_showConsoleButton_clicked()));

	m_QOut = new QDebugStream(std::cout, ui->logWindow, false, ui->statusBar);
	m_QOutError = new QDebugStream(std::cerr, ui->logWindow, true, ui->statusBar);
	ui->logWindow->hide();

	LoadSettings();

	std::cout << "Ready" << std::endl;

	if (m_CurrentProjectFolder.length() > 0)
		Load(m_CurrentProjectFolder);
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_QOut;
	delete m_QOutError;
}

void MainWindow::SaveSettings()
{
	QSettings settings("Nestor", "Mown");

	settings.setValue("CurrentProjectFolder", QString(m_CurrentProjectFolder.c_str()));
	settings.setValue("CurrentLocalUrl", QString(m_CurrentLocalUrl.c_str()));
	settings.setValue("ForceAll", ui->forceAll->isChecked());
	settings.setValue("EnableComments", ui->enableComments->isChecked());
	settings.setValue("ServerPath", ui->serverPath->text());
}

void MainWindow::LoadSettings()
{
	m_IgnoreUIEvents = true;

	std::cout << "Loading settings... " << std::endl;
	QSettings settings("Nestor", "Mown");

	m_CurrentProjectFolder = settings.value("CurrentProjectFolder", "").value<QString>().toStdString();
	m_CurrentLocalUrl = settings.value("CurrentLocalUrl", "about:blank").value<QString>().toStdString();

	ui->forceAll->setChecked(settings.value("ForceAll", false).toBool());
	ui->enableComments->setChecked(settings.value("EnableComments", true).toBool());

	QString path = settings.value("ServerPath", "/").value<QString>();
	ui->serverPath->setText(path);

	std::cout << "Settings loaded." << std::endl;
	m_IgnoreUIEvents = false;
}

void MainWindow::DelayedExport()
{
	if (m_ExportTimer.isActive())
	{
		m_ExportTimer.stop();
	}

	m_ExportTimer.start(50);
}

bool MainWindow::CanExport()
{
	if (m_ExportTimer.isActive())
		return false;

	return true;
}

void MainWindow::on_exportButton_clicked()
{
	if (CanExport())
	{
		Export(false);
		QDesktopServices::openUrl(QUrl::fromUserInput(QString::fromStdString(m_CurrentProjectFolder + "/mown-export")));
	}
}

void MainWindow::Load(std::string path)
{
	m_CurrentProjectFolder = path;

	Export(true);

	setWindowTitle("Mown - " + QString(path.c_str()));

	ui->webView->setUrl(QUrl(QString::fromUtf8(m_CurrentLocalUrl.c_str())));
}

void MainWindow::on_loadButton_clicked()
{
	QFileDialog fd;
	fd.setFileMode(QFileDialog::DirectoryOnly);
	int ret = fd.exec();
	QStringList tmp = fd.selectedFiles();

	if (tmp.length() > 0)
	{
		QString fileName = tmp[0];

		if (ret == 1)
		{
			Load(fileName.toStdString());
			SaveSettings();
		}
	}
}

void MainWindow::on_openSourceFileButton_clicked()
{
	std::cout << "Opening " << m_CurrentSourceFile << std::endl;
	QDesktopServices::openUrl(QUrl::fromUserInput(QString::fromStdString(m_CurrentSourceFile)));
}

void MainWindow::on_openSourceFolderButton_clicked()
{
	QDesktopServices::openUrl(QUrl::fromUserInput(QString::fromStdString(m_CurrentProjectFolder)));
}

void MainWindow::UpdateWatcher(const QString & fileName)
{
	m_Watcher.removePaths(m_Watcher.directories());
	m_Watcher.removePaths(m_Watcher.files());

	m_Watcher.addPath(fileName);
	QDir dir(fileName);
	QFileInfoList list = dir.entryInfoList();
	std::cout << "Watching ";
	for (int i = 0; i < list.length(); i++)
	{
		std::cout << list[i].fileName().toStdString() << " ";
		m_Watcher.addPath(list[i].absoluteFilePath());
	}
	std::cout << std::endl;
}

void MainWindow::fileChangedSlot(QString path)
{
	std::cout << "File changed: " << path.toStdString() << std::endl;
	DelayedExport();
}

void MainWindow::directoryChangedSlot(QString path)
{
	if (path.indexOf("mown-preview") >= 0 && path.indexOf("mown-export") >= 0)
	{
		std::cout << "Directory changed: " << path.toStdString() << std::endl;
		DelayedExport();
	}
}

void MainWindow::exportTimerTimeout()
{
	m_ExportTimer.stop();
	std::cout << "Delayed export start" << std::endl;
	Export(true);
}

void MainWindow::Export(bool local)
{
	m_Mown.m_ForceAll = ui->forceAll->isChecked();
	m_Mown.m_EnableComments = ui->enableComments->isChecked() && !local;
	m_Mown.m_LocalPreview = local;
	m_Mown.m_WebsiteRoot = ui->serverPath->text().toStdString();

	if (m_Mown.Export(m_CurrentProjectFolder))
	{

		if (local)
		{
			m_CurrentLocalUrl = m_Mown.GetLocalUrl();
			ui->webView->reload();
		}

		UpdateWatcher(m_CurrentProjectFolder.c_str());
		m_Mown.Dump(false);
	}
}

void MainWindow::on_refreshButton_clicked()
{
	Export(true);
}

void MainWindow::on_forceAll_toggled(bool checked)
{
	if (m_IgnoreUIEvents)
		return;

	Export(true);

	SaveSettings();
}

void MainWindow::on_localPreview_toggled(bool checked)
{
	if (m_IgnoreUIEvents)
		return;

	Export(true);
}

void MainWindow::on_enableComments_toggled(bool checked)
{
	if (m_IgnoreUIEvents)
		return;

	Export(true);

	SaveSettings();
}

void MainWindow::on_indexButton_clicked()
{
	ui->webView->setUrl(QUrl(QString::fromUtf8(m_CurrentLocalUrl.c_str())));
}

void MainWindow::on_showConsoleButton_clicked()
{
	if (ui->logWindow->isHidden())
		ui->logWindow->show();
	else
		ui->logWindow->hide();
}

void MainWindow::on_serverPath_editingFinished()
{
	if (m_IgnoreUIEvents)
		return;

	SaveSettings();
}

void MainWindow::on_webView_urlChanged(const QUrl& url)
{
	std::string sourceFile = m_Mown.GetSourceFilenameForPreviewFile(url.fileName().toStdString());
	std::cout << url.fileName().toStdString() << " has source file " << sourceFile << std::endl;
	m_CurrentSourceFile = sourceFile;

	QFileInfo fileInfo(QString::fromStdString(m_CurrentSourceFile));
	ui->openSourceFileButton->setText(fileInfo.fileName());

	ui->openSourceFileButton->setVisible(!sourceFile.empty());
}
