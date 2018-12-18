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
	ui(new Ui::MainWindow),
	m_CompactMode(false),
	m_CompactModeSpacer(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Ignored))
{
	ui->setupUi(this);

	m_IgnoreUIEvents = false;

	connect(&m_Watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedSlot(QString)));
	connect(&m_Watcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChangedSlot(QString)));

	connect(&m_ExportTimer, SIGNAL(timeout()), this, SLOT(exportTimerTimeout()));
	connect(&m_NewFileCheckTimer, SIGNAL(timeout()), this, SLOT(newFileTimerTimeout()));

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

void MainWindow::resizeEvent(QResizeEvent * event)
{
	long sideToolbarNeededWidth = ui->leftToolbarLayout->geometry().width() + ui->rightToolbarLayout->geometry().width();
	long centerToolbarNeededWidth = ui->centerToolbarLayout->geometry().width();

	long widthAvailable = size().width();

	bool shouldEnableCompactMode = widthAvailable < (sideToolbarNeededWidth + centerToolbarNeededWidth + 16);


	std::cout << "Side toolbar size: " << sideToolbarNeededWidth << ", center toolbar size: " << centerToolbarNeededWidth << ", window size: " << widthAvailable << ", should be compact: " << shouldEnableCompactMode << std::endl;
	if (sideToolbarNeededWidth == 0 || centerToolbarNeededWidth == 0)
		return;

	SetCompactMode(shouldEnableCompactMode);
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

void MainWindow::on_createArticleButton_clicked()
{
	std::string articleName = ui->articleNameInput->text().toStdString();
	std::cout << "Creating article " << articleName << std::endl;

	std::ofstream f(m_CurrentProjectFolder + "/" + articleName + ".yaml");

	DelayedExport();
}

void MainWindow::SetCompactMode(bool compactMode)
{
	if (m_CompactMode != compactMode)
	{
		m_CompactMode = compactMode;

		if (m_CompactMode)
		{
			ui->toolbarHorizontalLayout->removeItem(ui->stretchedCenterToolbarLayout);
			ui->toolbarVerticalLayout->addItem(ui->stretchedCenterToolbarLayout);

			ui->widget->setMinimumHeight(75);
			ui->widget->setMaximumHeight(75);

			ui->toolbarHorizontalLayout->insertSpacerItem(1, m_CompactModeSpacer);
		}
		else
		{
			ui->toolbarVerticalLayout->removeItem(ui->stretchedCenterToolbarLayout);
			ui->toolbarHorizontalLayout->insertItem(1, ui->stretchedCenterToolbarLayout);

			ui->widget->setMinimumHeight(50);
			ui->widget->setMaximumHeight(50);

			ui->toolbarHorizontalLayout->removeItem(m_CompactModeSpacer);
		}

		ui->toolbarHorizontalLayout->update();

		update();
		repaint();
	}
}

void MainWindow::UpdateWatcher(const QString & fileName)
{
	m_Watcher.removePaths(m_Watcher.directories());
	m_Watcher.removePaths(m_Watcher.files());

	QDir dir(fileName);
	QFileInfoList list = dir.entryInfoList();
	std::cout << "Watching ";
	for (int i = 0; i < list.length(); i++)
	{
		std::cout << list[i].fileName().toStdString() << " ";
		m_Watcher.addPath(list[i].absoluteFilePath());
	}
	m_Watcher.addPath(dir.absolutePath());
	std::cout << std::endl;
}

void MainWindow::fileChangedSlot(QString path)
{
	std::cout << "File changed: " << path.toStdString() << std::endl;
	DelayedExport();
}

void MainWindow::directoryChangedSlot(QString path)
{
	if (!(path.toStdString().find("mown-preview") >= 0 || path.toStdString().find("mown-export") >= 0))
	{
		std::cout << "Directory has been changed: " << path.toStdString() << std::endl;
		if (m_NewFileCheckTimer.isActive())
		{
			m_NewFileCheckTimer.stop();
		}

		m_NewFileCheckTimer.start(50);
	}
}

void MainWindow::exportTimerTimeout()
{
	m_ExportTimer.stop();
	std::cout << "Delayed export start" << std::endl;
	Export(true);
}

void MainWindow::newFileTimerTimeout()
{
	m_NewFileCheckTimer.stop();
	if (m_Mown.PathHasNewContent(m_CurrentProjectFolder))
	{
		std::cout << "New file(s) detected" << std::endl;
		DelayedExport();
	}
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
	try {
		auto article = m_Mown.GetArticleForPreviewFile(url.fileName().toStdString());
		const auto& sourceFile = article.GetSourceFilePath();
		std::cout << url.fileName().toStdString() << " has source file " << sourceFile << std::endl;
		m_CurrentSourceFile = sourceFile;

		QFileInfo fileInfo(QString::fromStdString(m_CurrentSourceFile));
		ui->pageFileAndLanguage->setText(fileInfo.fileName() + "(" + QString::fromStdString(article.GetCurrentLanguage()) + ")");
		ui->pageTitle->setText(QString::fromStdString(article.GetFileName()) + ": " + QString::fromStdString(article.GetTitle()));

		ui->openSourceFileButton->setVisible(!sourceFile.empty());
	}
	catch (std::invalid_argument e) {
		ui->pageFileAndLanguage->setText("");
		ui->pageTitle->setText("No current article");
		ui->openSourceFileButton->setVisible(false);
	}
	ui->webView->setUrl(url);
}
