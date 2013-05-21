#include "widget.h"
#include <QtNetwork>
#include <QNetworkAddressEntry>
#define DEBUG


#if defined(WIN32) && defined(_DEBUG)
  #define new DEBUG_NEW
#endif

/*

  ALPHA PHASE BEGINS: CODE ALMSOT MEETS ORIGINAL REQUIREMENTS OR ALL ARE BEING IMPLEMENTED

  Changelog 0.9 - 1.0

  - Moved from QTableView to QTreeWidget. Re-Wrote the code controlling all the lists to be compatible.
  - Gui Changes
  - Did experimentation with 64 bit systems. Found that loopback is not always interface 0. Need to implement search.
  - Added locking to the buttons. Buttons will eventually be removed.
  - Overall Code Streamlining.
  - Changed from a QWidget to a QMainWindow class (a subclass of QWidget) Enabling menu bar. Added about Qt. Need to fix Program About button.

 Changelog 1.0 - 1.1
  - Added socket error processing
  - Fixed the infinite login bug
  - Fixed 64 bit processing
  - Code optimization
  - Began Adding File Transfer Protocol Server to the SERVER mode. No crashes yet! Includes WSOCK32.lib in the makefile, so probably needs
  WSOCK32.dll now.

  Changelog 1.1 - 1.2
  -Completely Added the FTP Server. Acts like any other. Default login is "lanclient" "tseug".
  -Implemented a "search(file, dir)" function to assist in applying cracks. Will detect changes to Program Files and auto-search
   for the file to replace.
  -Began Implementing FTP Client.

  Changelog 1.2 - 1.3
  - Finished FTP Client: it auto reads all directories in Sync and downloads them to the application directory.
  Will try to find a way to change the download directory. Added the client progress bar.
  -Added a status bar to the server to indicate client download state


  Changelog 1.3 - 1.3.1 : Jan 26, 2010

  - Discovered a huge bug in the code that prevented FTP Accross a network, instead of through loopback. Fixed it (set the client to Active
    instead of Passive mode, and forced Binary transfers)
  - Discovered a huge bug in the broadcasting code of the client. Found a Qt Example called "Network Chat Example Qt4.6" that illustrates a fix.
  - No changes to the actuall interface so no change to the Major Build Number

  Changelog 1.3.1 - 1.3.2 : Jan 27, 2010
  -Fixed the bug with UDP Broadcast, it now transmits on all interfaces that support broadcasting(which is usually the only one connected)

  Changelog 1.3.2 - 1.4 : Jan 28, 2010
  -Fixed client disconnect/reconnect issues.
  -Added SystemTrayIcon (noticed that showMessage() doesnt work, so no popup windows yet. Maybe my copy of XP?)

  Changelog 1.4 - 1.5 : Jan 29, 2010
  -Added support in the backend for debugging (although it relies on showMessage(). Maybe move over to logfile?)
  -No, that was just my computer. Mesage Popups are now enabled and working by default.
  -New Icons (Thanks Adrian)
  -Much improved debug mode, now writes to logfile and sends important messages to the screen.
  -Icons show error status, clicking the icon clears the error, and resets the icon.
  -Fully implemented settings, though they shouldn't really be changed.
  -Built the installer and added the icons to the resource system
  -Beta Phase begins

  BETA PHASE BEINGS: CODE MEETS ORIGINAL REQUIREMENTS, AND NEW FEATURES ARE BEING ADDED

  Changelog 1.5 : June 25, 2010
  -Added the background capability to do remote download start/management on the client.

  Changelog 1.5 - 1.5.2 : July 27, 2010
  -Began work on the Server File Download dialog. So far, have got procedural button generation and directory genreation.
  -Moved the server workspace inside a QTabWidget to allow for more server functionality

  Changelog 1.5.2 - 1.5.5 : July 28, 2010
  -Server File Download page modified with more functionality
    -Bugs found:
        -Directory-inside-Directory doesn't get named
        -Wierd non-existant files with blank names
        -Directory-inside-directory doesn't affect status of directory's above it
        
  Changelog 1.5.5 - 1.5.8 : August 4, 2010
  -Bugs from 1.5.5 are all fixed
  -Added all the background needed to send files. Unfortunately, it doesn't work.
    -Check button, and try and get DEBUG mode working again. Re-initialize the debug setting to view packtets arriving


*/

/*

 NOTES:
-Implement Bytes In/ Out by default

*/

typedef DWORD (_stdcall *TGetIfTable) (
  MIB_IFTABLE *pIfTable,  // buffer for interface table
  ULONG *pdwSize,         // size of buffer
  BOOL bOrder             // sort the table by index?
);

typedef DWORD (_stdcall *TGetNumberOfInterfaces) (
  PDWORD pdwNumIf  // pointer to number of interfaces
);

TGetIfTable pGetIfTable;
TGetNumberOfInterfaces pGetNumberOfInterfaces;


ULONG uRetCode;
MIB_IFTABLE * m_pTable;

DWORD m_dwAdapters;

    unsigned long long bytesTotal; //64 bits can suck it
    int id;
    QString ret;
    QStringList files;
    QStringList paths;
    QStringList remPaths;
    QString curDir;
    QString workingDir = "/";
    bool inEventLoop = false;
    bool amIRite = false;
    QString curRoot = "";
    QString currentDir;
    qint64 total = 100;
    qint64 current = 0;
    QList<QHostAddress> broadcastAddresses;
    QList<QHostAddress> ipAddresses;
    QList<QTreeWidgetItem*> fileList;
    QList<QCheckBox*> boxList;

     QString releaseVer = "1.5.8";
     QString releaseType = "Beta";


    //setup main varibles, settings
    int udpBroadcastRate = 1000;
    int tcpCheckRate = 5000;
    int standardUdpPort = 45454;
    int standardTcpPort = 1337;
    int networkUpdateRate = 1000;
    bool debugMode = true;


Widget::Widget()
{
    // This class is the host of the whole application. This is BAD. Needs to be rectified. Should split this up into either: More classes OR More Files

    firstComp = NULL; //Setup the linked list

    //Spawn a QMainWindow, and set its attributes
    mainWindow = new QMainWindow();
    mainWindow->resize(400, 400);
    mainWindow->setWindowTitle("Lan Party Manager " + releaseType + " " + releaseVer);

    setup = new QSettings(QSettings::IniFormat, QSettings::SystemScope, "Alldritt Coding", "LanParty Manager", this);
    QStringList keys = setup->allKeys();

    if(keys.length() != 8)
    {
        setup->clear();
        setup->setValue("udpBroadcast", 1000);
        setup->setValue("tcpBroadcast", 5000);
        setup->setValue("udpPort", 45454);
        setup->setValue("tcpPort", 1337);
        setup->setValue("debug", false);
        setup->setValue("releaseType", releaseType);
        setup->setValue("releaseVer", releaseVer);
        setup->setValue("networkUpdateRate", 1000);
    }
    else
    {

            udpBroadcastRate = setup->value("udpBroadcast", 1000).toInt();

            tcpCheckRate = setup->value("tcpBroadcast", 5000).toInt();

            standardUdpPort = setup->value("udpPort", 45454).toInt();

            standardTcpPort = setup->value("tcpPort", 1337).toInt();

            debugMode = setup->value("debug", false).toBool();

            //releaseType = setup->value("releaseType", "").toString();

            //releaseVer = setup->value("releaseVer", "").toString();

            networkUpdateRate = setup->value("networkUpdateRate", 1000).toInt();
    }

    settings = new QWidget(mainWindow, Qt::Dialog);
    settings->resize(300, 300);
    settings->setWindowTitle("Settings");
    settings->hide();
    QLabel * la = new QLabel(settings);
    la->move(15, 15);
    la->setText("UDP Broadcast Delay");
    QLabel * lb = new QLabel(settings);
    lb->move(15, 45);
    lb->setText("TCP Broadcast Delay");
    QLabel * lc = new QLabel(settings);
    lc->move(15, 75);
    lc->setText("Default UDP Port (Advanced)");
    QLabel * ld = new QLabel(settings);
    ld->move(15, 105);
    ld->setText("Default TCP Port (Advanced)");
    QLabel * le = new QLabel(settings);
    le->move(15, 165);
    le->setText("Debug Mode");
    QLabel * lf = new QLabel(settings);
    lf->move(15, 135);
    lf->setText("Total Bytes Refresh Rate");

    uPort = new QLineEdit(settings);
    uPort->setGeometry(200, 75, 50, 20);
    uPort->setText(QString().number(standardUdpPort));

    tPort = new QLineEdit(settings);
    tPort->setGeometry(200, 105, 50, 20);
    tPort->setText(QString().number(standardTcpPort));

    uDelay = new QLineEdit(settings);
    uDelay->setGeometry(200, 15, 50, 20);
    uDelay->setText(QString().number(udpBroadcastRate));

    tDelay = new QLineEdit(settings);
    tDelay->setGeometry(200, 45, 50, 20);
    tDelay->setText(QString().number(tcpCheckRate));

    nDelay = new QLineEdit(settings);
    nDelay->setGeometry(200, 135, 50, 20);
    nDelay->setText(QString().number(networkUpdateRate));


    debugOn = new QCheckBox(settings);
    debugOn->move(200, 165);
    debugOn->setChecked(debugMode);

    QPushButton * apply = new QPushButton(settings);
    apply->setGeometry(200, 200, 60, 30);
    apply->setText("Apply");
    QObject::connect(apply, SIGNAL(clicked()), this, SLOT(apply()));


    QWidget * passWindow = new QWidget(mainWindow, Qt::Tool);
    passWindow->resize(200, 80);
    passWindow->hide();

    QLabel * passLabel = new QLabel(passWindow);
    passLabel->move(50, 10);
    passLabel->setText("Enter Password");

    passBox = new QLineEdit(passWindow);
    passBox->setGeometry(40, 25, 130, 20);

    QPushButton * enter = new QPushButton(passWindow);
    enter->setGeometry(40, 50, 50, 25);
    enter->setText("Enter");

    QObject::connect(enter, SIGNAL(clicked()), this, SLOT(passEnter()));
    QObject::connect(enter, SIGNAL(clicked()), passWindow, SLOT(hide()));
    QObject::connect(passBox, SIGNAL(returnPressed()), this, SLOT(passEnter()));
    QObject::connect(passBox, SIGNAL(returnPressed()), passWindow, SLOT(hide()));


    icon = new QIcon(":/images/icon.png");
    sysIcon = new QSystemTrayIcon(*icon, this);
    sysIcon->show();
    sysMenu = new QMenu(mainWindow);
    QAction * exitAction = sysMenu->addAction("Exit");
    sysIcon->setContextMenu(sysMenu);
    connect(exitAction, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    connect(exitAction, SIGNAL(triggered(bool)), this, SLOT(close()));
    bool success = connect(sysIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconClicked(QSystemTrayIcon::ActivationReason)));
    if(success != true)
    {
		sysIcon->showMessage("System Tray Error", "Window Respawn Connection Failed", QSystemTrayIcon::Information, 10000 );
    }


	QMenu* file = mainWindow->menuBar()->addMenu(tr("&File"));
	QMenu* help = mainWindow->menuBar()->addMenu(tr("&Help"));

	QAction* exitAct = new QAction(tr("&Exit"), this);
	exitAct->setStatusTip(tr("Quit"));
	connect(exitAct, SIGNAL(triggered()), qApp, SLOT(quit()));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	QAction* settingAct = new QAction(tr("&Settings"), this);
	settingAct->setStatusTip(tr("Access the Settings page"));
	connect(settingAct, SIGNAL(triggered()), this, SLOT(settingsWindow()));

	QAction* aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	QAction* aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	help->addAction(aboutQtAct);
	help->addAction(aboutAct);
	file->addAction(settingAct);
	file->addAction(exitAct);
	mainWindow->show();

    server = new QPushButton(mainWindow);
    client = new QPushButton(mainWindow);


    log = new QFile("log.txt");
    currentTime = new QDateTime(); //create the log timestamp
    *currentTime = QDateTime::currentDateTime(); //get system time


    if(!log->open(QIODevice::ReadWrite|QIODevice::Append))
    {
        QMessageBox::information(0, "Error", "Can't open logfile. Reason: " + log->errorString());
    }

    if(debugMode)
    {
    writeLog("LanMan " + releaseType + " " + releaseVer + " Logfile Start");
    writeLog("Time Format is: DD/MM/YYYY - HOUR:MIN:SEC AM/PM");
    }

    server->setGeometry(10, 30, 100, 30);
    client->setGeometry(290, 30, 100, 30);
    server->setText("SERVER MODE");
    client->setText("CLIENT MODE");
    client->show();
    server->show();
    connect(client, SIGNAL(released()), this, SLOT(startClient()));
    connect(server, SIGNAL(released()), passWindow, SLOT(show()));
    mainLabel = new QLabel("", mainWindow);
    mainLabel->move(165, 40);
    mainLabel->show();
    udpSocket = new QUdpSocket(this);
}

Widget::~Widget()
{

}

Widget::Mode Widget::getMode()
{
    return Widget::Client;
}

void Widget::writeLog(QString input)
{
   QString date = currentTime->toString("dd/MM/yyyy - h:m:s ap");
   log->write(QByteArray(QString( date + " :: " + input + "\r\n").toAscii()));
}

void Widget::about()
{

    QMessageBox::about( mainWindow, "About LanMan " + releaseType + " " + releaseVer, "LanMan is a Lan Party Manager currently capable of auto file sync, hostname identification, automatic network testing and other things. This is build " + releaseType + " " + releaseVer);
}

void Widget::iconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
    mainWindow->show();
    normal();
    }
}

void Widget::close()
{

    setup->setValue("udpBroadcast", udpBroadcastRate);
    setup->setValue("tcpBroadcast", tcpCheckRate);
    setup->setValue("udpPort", standardUdpPort);
    setup->setValue("tcpPort", standardTcpPort);
    setup->setValue("debug", debugMode);
    setup->setValue("releaseType", releaseType);
    setup->setValue("releaseVer", releaseVer);
    setup->setValue("networkUpdateRate", networkUpdateRate);
    setup->sync();
    log->close();
    sysIcon->hide();
    sysIcon->~QSystemTrayIcon();

}

void Widget::settingsWindow()
{

    uPort->setText(QString().number(standardUdpPort));

    tPort->setText(QString().number(standardTcpPort));

    uDelay->setText(QString().number(udpBroadcastRate));

    tDelay->setText(QString().number(tcpCheckRate));

    debugOn->setChecked(debugMode);

    nDelay->setText(QString().number(networkUpdateRate));

    settings->show();
}

void Widget::apply()
{
    standardUdpPort = uPort->text().toInt();

    standardTcpPort = tPort->text().toInt();

    udpBroadcastRate = uDelay->text().toInt();

    tcpCheckRate = tDelay->text().toInt();

    debugMode = debugOn->isChecked();

    networkUpdateRate = nDelay->text().toInt();

    settings->show();
}

void Widget::error()
{
    sysIcon->setIcon(QIcon(":/images/err.png"));
}

void Widget::normal()
{
    sysIcon->setIcon(QIcon(":/images/icon.png"));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////SERVER FUNCTIONS//////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Widget::passEnter()
{
    checkPassword(passBox->text().toAscii());
}

void Widget::checkPassword(QByteArray pass)
{
#ifndef DEBUG
    QCryptographicHash * hash = new QCryptographicHash(QCryptographicHash::Md5);
    QFile * password = new QFile("password.hash", this);
    if(password->open(QIODevice::ReadOnly))
    {
        QByteArray hashSt = password->readAll();
        hash->addData(pass);
        QByteArray hashTs = hash->result().toHex();
        QString test = QString(hashTs);
        if(hashSt == hashTs)
        {
#endif
            startServer();
#ifndef DEBUG
        }
        else
        {
            QMessageBox::information(mainWindow, "Password", "That password does not hash to the one stored in password.hash");
        }
    }
    else
    {
        QMessageBox::information(mainWindow, "Password", "password.hash Not found");
    }
#endif
}


void Widget::startServer()
{

#ifdef WIN32
    WSADATA WSAData;
    if( WSAStartup( MAKEWORD(1, 0), &WSAData) != 0 ) {
            //printf("-WSAStartup failure: WSAGetLastError=%d\r\n", WSAGetLastError() );
        QMessageBox::information(0, "Error", "Winsock error: " + WSAGetLastError());
           // return 0;
    }
#endif


QString dir = qApp->applicationDirPath();

if(QDir().exists("/Sync") == false)
{
    QDir().mkdir(dir + "/Sync");
}

/*

Begin FTP Server Initialization

*/
ftpServer = new CFtpServer();
ftpServer->SetDataPortRange(100, 900);

CFtpServer::CUserEntry *FtpUser = ftpServer->AddUser("lanclient", "tseug", QString(dir + "/Sync").toAscii());
ftpServer->AddUser("anonymous", NULL, "C:\\dir");

if(FtpUser)
{
                FtpUser->SetMaxNumberOfClient( 0 );
                FtpUser->SetPrivileges( CFtpServer::READFILE | CFtpServer::WRITEFILE |
                                        CFtpServer::LIST | CFtpServer::DELETEFILE | CFtpServer::CREATEDIR |
                                        CFtpServer::DELETEDIR );
}
else
{
    writeLog("FTP Server Could Not Create User");
     sysIcon->showMessage("FTP Error", "No user could be created on the FTP Server. Login Not Possible", QSystemTrayIcon::Critical, 10000 );
     error();
}
if(ftpServer->StartListening(INADDR_ANY, 21))
{
   if(ftpServer->StartAccepting() == false)
    {
        writeLog("FTP Server Could Not Begin Accepting on Port 21. Code corruption or incompatible dll");
        sysIcon->showMessage("FTP Error", "FTP Server Couldn't start accepting connections.", QSystemTrayIcon::Critical, 10000 );
        error();
    }
}
else
{
    writeLog("FTP Server Could Not Start Listening On Port 21. Check WINSOCK. This application does not support concurrent instances.");
    sysIcon->showMessage("FTP Error", "FTP Server Couldn't Listen on port 21. It may be in use already.", QSystemTrayIcon::Critical, 10000 );
    error();
}

/*
End FTP Server Initialization
All Files in the applications working directory are now availible through FTP with user "lanclient" with pass "tseug"
*/



client->setEnabled(false);
udpSocket->bind(QHostAddress::Any, standardUdpPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
connect(udpSocket, SIGNAL(readyRead()), this, SLOT(signalReceived()));

//resize the mainWindow because we need a bigger interface for server mode
mainWindow->resize(600, 500);

//create the tab-widget that we parent the sub-windows too
computerManager = new QTabWidget(mainWindow);
computerManager->setGeometry(5, 70, 590, 430);


primaryStatus = new QWidget(computerManager);
ftpWindow = new QWidget(computerManager);
serverDownload = new QPushButton(ftpWindow);
serverDownload->setGeometry(20, 355, 545, 45);

QObject::connect(serverDownload, SIGNAL(clicked()), this, SLOT(serverDownloadStart()));

computersConnected = 0;

ftpFiles = new QTreeWidget(ftpWindow);
ftpFiles->setGeometry(0, 0, 580, 350);

fileModel = new QTreeWidgetItem(0);

computerNames.append("Files");

ftpFiles->setHeaderLabels(computerNames);

computerManager->addTab(primaryStatus, "Network Status");
computerManager->addTab(ftpWindow, "File Control");

mapper = new QSignalMapper(this);
computerTable = new QTreeWidget(primaryStatus);
computerTable->setGeometry(0, 0, 580, 400);
QTreeWidgetItem * compModel = new QTreeWidgetItem(0);
compModel->setText(0, "Name");
compModel->setText(1,"Bytes Total");
compModel->setText(2, "Active");
computerTable->setHeaderItem(compModel);
computerTable->setColumnCount(3);
computerTable->setColumnWidth(0, 150);
computerTable->setColumnWidth(1, 120);
computerTable->setColumnWidth(2, 305);
computerTable->show();
timer = new QTimer(this);
connect(timer, SIGNAL(timeout()), this, SLOT(update()));

fileWatcher = new QFileSystemWatcher(this);

fileWatcher->addPath(dir + "/Sync");

connect(fileWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(sendUpdate()));

timer->start(tcpCheckRate);

computerManager->show();
updateFileTable();
}

void Widget::generateBoxLayout(QTreeWidgetItem * item)
{
    qDebug("Generating...");
    for(int columns = 1; columns < ftpFiles->columnCount(); columns++)
    {
        for(int i = 0; i < item->childCount(); i++)
        {
            item->child(i)->setCheckState(columns, Qt::Unchecked);

            if(item->child(i)->childCount() != 0)
                generateBoxLayout(item->child(i));
        }
    }
}

void Widget::boxUpdated(QTreeWidgetItem* item, int column)
{
    int state = 2;
    qDebug("Box Clicked");
    for(int columns = 1; columns < ftpFiles->columnCount(); columns++)
    {
        int checkedCount = 0;
        int ucheckedCount = 0;

        if(item->childCount() == 0 && item->parent() != NULL)
        {
            for(int i = 0; i < item->parent()->childCount(); i++)
            {
                int val = item->parent()->child(i)->checkState(columns);

                switch(val)
                {
                    case 0:
                        ucheckedCount++;
                    break;

                    case 1:
                        ucheckedCount++;
                    break;

                    case 2:
                        checkedCount++;
                    break;
                }
            }

            if(checkedCount == item->parent()->childCount())
            {
                item->parent()->setCheckState(columns, Qt::Checked);
            }
            else if(ucheckedCount == item->parent()->childCount())
            {
                item->parent()->setCheckState(columns, Qt::Unchecked);
            }
            else
            {
                item->parent()->setCheckState(columns, Qt::PartiallyChecked);
            }
        }
        else
        {
            for(int i = 0; i < item->childCount(); i++)
            {
                item->child(i)->setCheckState(columns, item->checkState(columns));
            }
        }
    }
}

void Widget::updateFileTable()
{
    totalDownloads = 0;
    QString dir = qApp->applicationDirPath();
    QTreeWidgetItem * root = new QTreeWidgetItem(0);
    QTreeWidgetItem * tempItem = root;
    QTreeWidgetItem * tmp;
    QDir sncdr(dir + "/Sync");

    QFileInfoList syncDir = sncdr.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries);


    while(!syncDir.empty())
    {
              if(syncDir.at(0).isDir())
              {
                  tmp = scanDirectory(syncDir.at(0).filePath());
                  tmp->setText(0, syncDir.at(0).fileName());
                  ftpFiles->addTopLevelItem(tmp);
                  totalDownloads++;
              }
              else
              {
                  tmp = new QTreeWidgetItem(0);
                  tmp->setText(0, syncDir.at(0).fileName());
                  ftpFiles->addTopLevelItem(tmp);
                  totalDownloads++;
              }
          syncDir.removeAt(0);
    }
}

QTreeWidgetItem * Widget::scanDirectory(QString path)
{
    QTreeWidgetItem * root;
    QDir myDir(path);
    QTreeWidgetItem * tmp;
    root = new QTreeWidgetItem(0);
    root->setText(0, myDir.dirName());
    QString dir = qApp->applicationDirPath();
    QDirIterator * scanDir= new QDirIterator(path, QDir::AllEntries|QDir::NoDotAndDotDot);
    QString nxt;


    while(scanDir->hasNext())
    {
        nxt = scanDir->next();

        qDebug(scanDir->filePath().toAscii());

        if(scanDir->fileInfo().isDir())
        {
            root->addChild(scanDirectory(scanDir->filePath()));
            totalDownloads++;
        }
        else
        {
            tmp = new QTreeWidgetItem(0);
            tmp->setText(0, scanDir->fileName());
            root->addChild(tmp);
            totalDownloads++;
        }
    }


    /*if(!nxt.isNull())
    {
        qDebug(scanDir->filePath().toAscii());
        if(scanDir->fileInfo().isDir())
        {
            root->addChild(scanDirectory(scanDir->filePath()));
        }
        else
        {
            tmp = new QTreeWidgetItem(0);
            tmp->setText(0, scanDir->fileName());
            root->addChild(tmp);
        }
    }*/

   return root;
}

void Widget::serverDownloadStart()
{
   QTreeWidgetItem * item = ftpFiles->invisibleRootItem();
   QTreeWidgetItem * header = ftpFiles->headerItem();
   QStringList lst;
   Computer * comp;

   for(int i = 1; i < ftpFiles->columnCount(); i++)
   {
      lst = rebuildPath(item, "", i);
      comp = findByName(header->text(i));
      foreach(QString str, lst)
      {
          comp->getSocket()->write(QByteArray("ADFL::" + str.toAscii()));
      }
      comp->getSocket()->write(QByteArray("DNLD::"));
    }
}

QStringList Widget::rebuildPath(QTreeWidgetItem * start, QString path, int column)
{
    QStringList fullPaths;
    path.append(start->text(0) + "/");
    for(int i = 0; i < start->childCount(); i++)
    {
        if(start->child(i)->childCount() == 0) //A FILE
        {
            if(start->child(i)->checkState(column) == Qt::Checked)
                fullPaths.append(path + start->child(i)->text(0));
        }
        else
            fullPaths.append(rebuildPath(start->child(i), path, column));
    }

   return fullPaths;
}

Computer * Widget::findByName(QString name)
{
    Computer * comp = firstComp;
    while(comp != NULL)
    {
        if(comp->getName() == name)
            break;
        else
            comp = comp->getNext();
    }

    return comp;
}

void Widget::sendUpdate()
{
    Computer * comp = firstComp;

    while(comp != NULL)
    {
    comp->getSocket()->write(QByteArray("UPDT"));
    comp = comp->getNext();
    }

    while(syncDir->hasNext())
    {
    friendlyDownloads.append(syncDir->fileName());
    syncDir->next();
    }
    syncDir->path();
}

void Widget::signalReceived()
{
    QString edit;
    QHostAddress *sender = new QHostAddress();
    QHostAddress *check = new QHostAddress();
    quint16 *port = new quint16();
    while (udpSocket->hasPendingDatagrams()) {
         QByteArray datagram;
         datagram.resize(udpSocket->pendingDatagramSize());
         udpSocket->readDatagram(datagram.data(),datagram.size(),sender,port);
         if(check != sender)
         {

            establishTcp(*sender, standardTcpPort);
            if(debugMode)
            {
                writeLog("User Found at: " + sender->toString());
            }

         }
         check = sender;
     }
}

void Widget::establishTcp(QHostAddress host, qint16 port)
{
    Computer * comp;
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(socketClosed()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(sockConn()));
    comp = newComp();
    comp->setSocket(tcpSocket);
    comp->setId(id);
    comp->setHost(host);

    tcpSocket->connectToHost(host, port);
    id++;
}


Computer * Widget::newComp()
{
    Computer * comp = new Computer(primaryStatus);
    Computer * nextComp;
    if(firstComp == NULL)
    {
        firstComp = comp;
    }
    else
    {
       nextComp = firstComp;
        while(nextComp->getNext() != NULL)
       {
           nextComp = nextComp->getNext();
       }
       nextComp->setNext(comp);
    }
 return comp;
}

void Widget::dataReceived()
{

    if(debugMode)
    {
        writeLog("Data Received");
    }

Computer * comp = firstComp;
Computer * comp2 = firstComp;
QObject * sender = QObject::sender();

if((QObject*)comp->getSocket() == sender)
    {
        computerRead(comp);
    }
else
{
while(comp2 != NULL)
{
    comp->getSocket();
    if((QObject*)comp->getSocket() == sender)
    {
        computerRead(comp);
    }
    comp2 = comp->getNext();
    comp = comp->getNext();
}

}
}

void Widget::computerRead(Computer * comp)
{
    if(debugMode)
    {
        writeLog("Reading Data");
    }
   QByteArray datab = comp->getSocket()->read(comp->getSocket()->bytesAvailable());
   QString data = QString(datab);
   writeLog(data);
   QString datac = data;
   QString datad = data;
   datac.truncate(6);

   QStringList commands = datad.split(QRegExp("....::", Qt::CaseInsensitive), QString::SkipEmptyParts);

for(int i = 0; i <= commands.length(); i++)
   {
       if(datac == "INIT::")
       {
           comp->getSocket()->write(QByteArray("REDY"));
           comp->getSocket()->write(QByteArray("RPRT"));
       }
       else if(datac == "BAND::")
       {
           comp->setTotal(commands.at(i).toULongLong());
       }
       else if(datac == "BDIN::")
       {
           comp->setIn(commands.at(i).toULongLong());
       }
       else if(datac == "BOUT::")
       {
           comp->setOut(commands.at(i).toULongLong());
       }
       else if(datac == "HOST::")
       {
           comp->setName(commands.at(i));
           if(computerNames.indexOf(comp->getName()) == -1)
           {
               computerNames.append(comp->getName());
               ftpFiles->setHeaderLabels(computerNames);
               ftpFiles->show();
               generateBoxLayout(ftpFiles->invisibleRootItem());
               QObject::connect(ftpFiles, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(boxUpdated(QTreeWidgetItem*, int)));
           }
       }
       else if(datac == "PING::")
       {
           comp->getSocket()->write(QByteArray("RPRT"));
       }
       else if(datac == "ERRR::")
       {

       }
       else if(datac == "BTOT::")
       {
           comp->getBar()->setMaximum(commands.at(i).toLongLong());
       }
       else if(datac == "BCUR::")
       {
           comp->getBar()->setValue(commands.at(i).toLongLong());
       }

if(commands.length() > 1 && i != commands.length())
       {
        datac = data;
        datac.remove(0,6 + commands.at(i).length());
        data = datac;
        datac.truncate(6);
       }
}
updateTable(comp);
}

void Widget::updateTable(Computer * comp)
{
    if(debugMode)
    {
        writeLog("Refreshing Table");
    }
comp->getItem();
}

void Widget::socketClosed()
{
    QTreeWidgetItem * item;
    int row = 0;
    Computer * comp = firstComp;
    Computer * prevComp = firstComp;
    QObject * sender = QObject::sender();
    computersConnected--;

    if((QObject*)comp->getSocket() == sender)
    {
        row = comp->getId();
        item = comp->getItem();
        computerNames.removeAt(computerNames.indexOf(comp->getName()));
        ftpFiles->setHeaderLabels(computerNames);
        ftpFiles->setColumnCount(computersConnected + 1);
        firstComp = comp->getNext();
        delete comp;
    }
    else
    {
        while(comp != NULL)
        {
            comp->getSocket();
            if((QObject*)comp->getSocket() == sender)
            {
                row = comp->getId();
                item = comp->getItem();
                computerNames.removeAt(computerNames.indexOf(comp->getName()));
                ftpFiles->setHeaderLabels(computerNames);
                ftpFiles->setColumnCount(computersConnected + 1);
                prevComp->setNext(comp->getNext());
                delete comp;
                comp = prevComp->getNext();
            }
            else
            {
                prevComp = comp;
                comp = comp->getNext();
            }
        }

    }
    row = computerTable->indexOfTopLevelItem(item);
    computerTable->takeTopLevelItem(row);
}

void Widget::update()
{
    Computer * comp = firstComp;

    while(comp != NULL)
    {
    comp->getSocket()->write(QByteArray("RPRT"));
    comp = comp->getNext();
    }


}

void Widget::socketError(QAbstractSocket::SocketError)
{
    Computer * comp = firstComp;
    QObject * sender = QObject::sender();
    QString errors;

    error();

    if((QObject*)comp->getSocket() == sender)
    {
        errors = comp->getSocket()->errorString();
    }
    else
    {
    while(comp != NULL)
    {
    comp->getSocket();
    if((QObject*)comp->getSocket() == sender)
    {
        errors = comp->getSocket()->errorString();
    }
    comp = comp->getNext();
    }

    }

    writeLog(errors);
}

void Widget::sockConn()
{
    Computer * comp = firstComp;
    QObject * sender = QObject::sender();
    computersConnected++;

    if((QObject*)comp->getSocket() == sender)
    {
        computerTable->addTopLevelItem(comp->getItem());
        computerTable->setItemWidget(comp->getItem(), 2, (QWidget*)comp->getBar());
    }
    else
    {
    while(comp != NULL)
    {
    comp->getSocket();
    if((QObject*)comp->getSocket() == sender)
    {
        computerTable->addTopLevelItem(comp->getItem());
        computerTable->setItemWidget(comp->getItem(), 2, (QWidget*)comp->getBar());
    }
    comp = comp->getNext();
    }

    }
}


/*
Network Command List:
           Client->Server:
                INIT : Initial command, first command sent after socket initialization. Used to test the allocation of memory to the socket
                BAND : Start of a bandwidth report. Will be followed by unsigned long bandwidth total.
                BDIN : Start of a bytes received report. Will only be sent if requested by server.
                BOUT : Start of a bytes sent report. Will only be sent if requested by server.
                HOST : Name of the computer.
                PING : Sent if computer doesn't receive a request for 5 minutes. If there is no response, client returns to search mode.
                ERRR : An error occured. The error string or an error code follows.
                BTOT : Report on the current file being downloaded. Total Bytes in the file. (0 is no file / dont know)
                BCUR : Report on the current file being downloaded. Current bytes aquired.

           Server->Client:
                REDY : Part of the initialization. Sent when server has completed the registration of the computer.
                RPRT : Orders the computer to send a status report.
                SHDN : Orders computer to shutdown.
                RSRT : Orders computer to restart.
                ROUT : Request for bytes sent.
                RQIN : Request for bytes received.
                RHST : Request for Hostname.
                PING : Sent if computer doesn't respond to commands. If there is no response to the ping, delete the compuer from the linked list.
                RTOT : See BTOT
                RCUR : See BCUR
                DNLD : Orders the client to download the file specified (full path)
                ADFL : Adds a file to the download queue
                STOP : Stops all current operations
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// CLIENT FUNCTIONS START HERE/////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Widget::startClient()
{
    QString dir = qApp->applicationDirPath();

    if(QDir().exists("/Downloads") == false)
    {
        QDir().mkdir(dir + "/Downloads");
    }

    server->setEnabled(false);
    progress = new QProgressBar(mainWindow);
    progress->setGeometry(15, 350, 400, 25);
    progress->show();

    TCHAR name[1024];
    DWORD name_length = 1024;

    computerTable = new QTreeWidget(mainWindow);
    computerTable->setGeometry(10, 70, 380, 200);
    QTreeWidgetItem * compModel = new QTreeWidgetItem(0);
    compModel->setText(0, "FileName");
    compModel->setText(1,"Bytes Total");
    compModel->setText(2, "Download");
    computerTable->setHeaderItem(compModel);
    computerTable->setColumnCount(3);
    computerTable->setColumnWidth(0, 130);
    computerTable->setColumnWidth(1, 80);
    computerTable->setColumnWidth(2, 168);
    computerTable->show();
    dl = new QPushButton(mainWindow);
    dl->setGeometry(10, 285, 70, 40);
    dl->setText("Download");
    dl->show();
    connect(dl, SIGNAL(clicked()), this, SLOT(getSelected()));


    QPushButton * update = new QPushButton(mainWindow);
    update->setGeometry(200, 285, 70, 40);
    update->setText("Update List");
    connect(update, SIGNAL(clicked()), this, SLOT(updateList()));
    update->show();

    QPushButton * abort = new QPushButton(mainWindow);
    abort->setGeometry(270, 285, 70, 40);
    abort->setText("Abort");
    abort->show();

    GetComputerName(name, &name_length);
    ret = QString::fromUtf16((ushort*)name, name_length);

    ftpClient = new QFtp(this);
    connect(ftpClient, SIGNAL(listInfo(QUrlInfo)), this, SLOT(list(QUrlInfo)));
    connect(ftpClient, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(updateProgress(qint64,qint64)));
    connect(ftpClient, SIGNAL(done(bool)), this, SLOT(ftpDone(bool)));
    connect(ftpClient, SIGNAL(commandFinished(int, bool)), this, SLOT(ftpCommandDone(int, bool)));
    connect(abort, SIGNAL(clicked()), ftpClient, SLOT(abort()));

    QTimer *networkRefresh = new QTimer(this);
    connect(networkRefresh, SIGNAL(timeout()), this, SLOT(refreshNet()));
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(broadcastUdp()));
    tcpServer= new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any, standardTcpPort);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(serverCalled()));
    timer->start(udpBroadcastRate);

    updateAddresses();


    pGetIfTable=(TGetIfTable)GetProcAddress(LoadLibrary(TEXT("Iphlpapi.dll")),"GetIfTable");
    pGetNumberOfInterfaces=(TGetNumberOfInterfaces)GetProcAddress(LoadLibrary(TEXT("Iphlpapi.dll")),"GetNumberOfInterfaces");

    if (!pGetIfTable || !pGetNumberOfInterfaces)
    {
        QMessageBox::critical(0, "Error", "Could not open iphlpapi.dll");
        error();
    }
    else
    {

        m_pTable = NULL;

        m_dwAdapters=0;

        uRetCode=pGetIfTable(m_pTable,&m_dwAdapters,TRUE);

            if (uRetCode == 122 /* The data area passed to a system call is too small.*/)
            {
                    // now we know how much memory allocate
                    m_pTable=new MIB_IFTABLE[m_dwAdapters];
                    pGetIfTable(m_pTable,&m_dwAdapters,TRUE);
            }
        refreshNet();
        mainLabel->setText(QString().setNum(bytesTotal));
     }

    networkRefresh->start(networkUpdateRate);
}


void Widget::refreshNet()
{
    unsigned long long temp1 = 0;
    unsigned long long temp2 = 0;
    temp1 = bytesIn();
temp2 = bytesOut();
bytesTotal = temp1 + temp2;
mainLabel->setText(QString().setNum(bytesTotal));
}

void Widget::serverCalled()
{
    tcpSocket = tcpServer->nextPendingConnection();
    timer->stop();
    tcpServer->close();
    if(debugMode)
    {
        writeLog("Server Found!");
    }
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(clientReceived()));

    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    tcpSocket->write(QByteArray("INIT::"));
    ftpClient->setTransferMode(QFtp::Active);
    ftpClient->connectToHost(tcpSocket->peerAddress().toString(), 21);
    ftpClient->login("lanclient", "tseug");
    ftpClient->list();
}

void Widget::broadcastUdp()
{
QByteArray datagram = "LanMan Broadcast Message";
if(debugMode)
{
    writeLog("Broadcasting UDP...");
}


bool validBroadcastAddresses = true;

foreach (QHostAddress address, broadcastAddresses)
{
    if(debugMode)
    {
        writeLog(address.toString());
    }

    if (udpSocket->writeDatagram(datagram, address, standardUdpPort) == -1)
        validBroadcastAddresses = false;
}

if (!validBroadcastAddresses)
    updateAddresses();

udpSocket->writeDatagram(datagram, QHostAddress::Broadcast, standardUdpPort);
if(debugMode)
    {
        writeLog(QHostAddress(QHostAddress::Broadcast).toString());
    }
}

unsigned long long Widget::bytesIn()
{
    unsigned long long bytes = 0;
    unsigned long long transit = 0;
                pGetIfTable(m_pTable,&m_dwAdapters,TRUE);



                for (UINT i=0;i<m_pTable->dwNumEntries;i++)
                        {
                                MIB_IFROW Row=m_pTable->table[i];
                                if(Row.dwType == 6||Row.dwType == 71)
                                {
                                transit = Row.dwInOctets;
                                bytes = bytes + transit;
                                }
                         }
return bytes;
}

unsigned long long Widget::bytesOut()
{
    unsigned long long bytes = 0;
    unsigned long long transit = 0;
                pGetIfTable(m_pTable,&m_dwAdapters,TRUE);

                MIB_IFROW Row=m_pTable->table[1];


                for (UINT i=0;i<m_pTable->dwNumEntries;i++)
                        {
                                MIB_IFROW Row=m_pTable->table[i];
                                if(Row.dwType == 6||Row.dwType == 71)
                                {
                                transit = Row.dwOutOctets;
                                bytes = bytes + transit;
                            }
                         }
    return bytes;
}

void Widget::clientReceived()
{


    QByteArray datab = tcpSocket->read(tcpSocket->bytesAvailable());
    QString data = QString(datab);

    if(debugMode)
       writeLog(data);

    QString datac = data;
    QString datad = data;
    datac.truncate(6);

    QStringList commands = datad.split(QRegExp("....::", Qt::CaseInsensitive), QString::SkipEmptyParts);
    foreach(QString str, commands)
        writeLog(str);

 for(int i = 0; i <= commands.length(); i++)
 {
       writeLog(datac);
       if(datac == "REDY")
       {
           // This message indicates that the computer was added sucessfully.
       }
       else if(datac == "RPRT")
       {
           tcpSocket->write(QByteArray("BAND::" + QString().setNum(bytesIn() + bytesOut()).toAscii()));
           tcpSocket->write(QByteArray("HOST::" + ret.toAscii()));
           tcpSocket->write(QByteArray("BTOT::" + QString().setNum(total).toAscii()));
           tcpSocket->write(QByteArray("BCUR::" + QString().setNum(current).toAscii()));
       }
       else if(datac == "SHDN")
       {
           // Do nothing(FOR NOW MUAH HA HA)
       }
       else if(datac == "RSRT")
       {
           // Do Nothing
       }
       else if(datac == "ROUT")
       {
           tcpSocket->write(QByteArray("BOUT::" + QString().setNum(bytesOut()).toAscii()));
       }
       else if(datac == "RQIN")
       {
           tcpSocket->write(QByteArray("BDIN::" + QString().setNum(bytesIn()).toAscii()));
       }
       else if(datac == "RHST")
       {
           tcpSocket->write(QByteArray("HOST::" + ret.toAscii()));
       }
       else if(datac == "PING")
       {
           tcpSocket->write(QByteArray("PING::"));
       }
       else if(datac == "UPDT")
       {
           updateList();
       }
       else if(datac == "ADFL::")
       {
           writeLog("ADFL DETECTED");
           remPaths.append(commands.at(i));
       }
       else if(datac == "DNLD::")
       {
           writeLog("DOWNLOAD DETECTED");
           QStringList temp = paths;
           paths = remPaths;
           getFirst();
           //paths = temp;

       }
       else if(datac == "STOP")
       {
           ftpClient->abort();
       }

       if(commands.length() > 1 && i != commands.length())
          {
           datac = data;
           datac.remove(0,6 + commands.at(i).length());
           data = datac;
           datac.truncate(6);
          }
   }

}

QString Widget::search(QString fileName, QString dir)
{
  QDirIterator directoryWalker(dir, QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while(directoryWalker.hasNext() && directoryWalker.fileName() != fileName)
    {
         directoryWalker.next();
    }
    return  directoryWalker.filePath();
}

void Widget::list(QUrlInfo info)
{
    QStringList newList;
    if(info.isDir())
    {
        files.append(info.name());
        newList = files;
        qDebug(info.name().toAscii());
    }
    if(info.isFile())
    {
        qDebug(info.name().toAscii());
        paths.append(curDir + "/" + info.name());

        QTreeWidgetItem * item = new QTreeWidgetItem(0);
        item->setText(0, paths.at(paths.length() - 1));
        item->setText(1, QString().number(info.size()));
        QCheckBox * checkBox = new QCheckBox(mainWindow);
        boxList.append(checkBox);
        computerTable->addTopLevelItem(item);
        computerTable->setItemWidget(item, 2, checkBox);

        fileList.append(item);
    }
}

void Widget::updateProgress(qint64 done, qint64 totale)
{
    current = done;
    total = totale;
    progress->setMaximum(totale);
    progress->setValue(done);
}

void Widget::ftpDone(bool errorb)
{
    if(errorb)
    {
       QMessageBox::information(0, "FTP", "FTP Error: " + ftpClient->errorString());
       error();
    }


    if(!files.isEmpty())
    {
    ftpClient->cd("/");
    QStringList test = files;
    inEventLoop = true;
    amIRite = false;
    ftpClient->list(files.at(0));
    curDir = files.at(0);
    files.removeAt(0);
    }
    else
    {
        inEventLoop = false;
    }
    if(debugMode)
    {
       writeLog("FTP Reported Done Message");
    }

}


void Widget::updateAddresses()
{
    broadcastAddresses.clear();
    ipAddresses.clear();

    foreach(QNetworkInterface herp, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry derp, herp.addressEntries())
        {
            QHostAddress broadcastAddress = derp.broadcast();
            if (broadcastAddress != QHostAddress::Null && derp.ip() != QHostAddress::LocalHost)
            {
                broadcastAddresses << broadcastAddress;
                ipAddresses << derp.ip();
            }
        }
    }
}

void Widget::disconnected()
{
    startClient();
    normal();
}

void Widget::getFirst()
{
    QStringList Test = paths;
    if(!paths.empty())
    {
        QStringList dirs = paths.at(0).split("/");
        if(dirs.at(0) ==  currentDir)
        {
            file = new QFile("./Downloads/" + dirs.at(1), this);
            file->open(QIODevice::ReadWrite);
            ftpClient->get(dirs.at(1), file, QFtp::Binary);
        }
        else
        {
            if(workingDir != "/")
                {
                ftpClient->cd("/");
                }
            if(dirs.at(0) != "")
            {
                ftpClient->cd(dirs.at(0));
            }
            workingDir = dirs.at(0);
            file = new QFile("./Downloads/" + dirs.at(1), this);
            file->open(QIODevice::ReadWrite);
            ftpClient->get(dirs.at(1), file, QFtp::Binary);
        }
        paths.removeAt(0);
    }
    else
    {
        updateList();
    }
}



void Widget::getSelected()
{

    QStringList testA = paths;
    QList<QCheckBox*> testB = boxList;
    for(int i = 1; i <= boxList.length(); i++)
    {
        if(!boxList.at(i - 1)->isChecked())
        {
            paths.removeAt(i - 1);
            boxList.removeAt(i - 1);
            i--;
        }
    }
    getFirst();
}

void Widget::ftpCommandDone(int, bool errorb)
{
    if(errorb)
    {
       QMessageBox::information(0, "FTP", "FTP Error: " + ftpClient->errorString());
       error();
   }

    if (ftpClient->currentCommand() == QFtp::Get)
    {
        if (errorb)
        {
            file->close();
            file->remove();
        }
        else
        {
            file->close();
            delete file;
            getFirst();
        }

    if (ftpClient->currentCommand() == QFtp::List)
    {
    }
}
}

void Widget::updateList()
{
    computerTable->clear();
    boxList.clear();
    paths.clear();
    ftpClient->list("/");
    curDir = "";
    workingDir = "/";
}
