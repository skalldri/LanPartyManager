#ifndef WIDGET_H
#define WIDGET_H

#include <QObject>
#include <QApplication>
#include <QtGui>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QtCore>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QTableView>
#include <QStandardItemModel>
#include "computer.h"
#include "./include/windows.h"
#include "./include/winbase.h"
#include "./include/windowsx.h"
#include "./include/iphlpapi.h"
#include "./include/Iprtrmib.h"
#include "./include/stdlib.h"
#include "CFtpServer.h"
#include <QFtp>
#include <QNetworkInterface>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QTabWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <QDirIterator>
#include <QSignalMapper>


class Widget : public QObject
{
    Q_OBJECT

private:

    QTabWidget * computerManager;
    QWidget * primaryStatus;
    QWidget * ftpWindow;
    QTreeWidget * ftpFiles;
    QTreeWidgetItem * fileModel;
    QStringList computerNames;
    QStringList friendlyDownloads;
    QDirIterator * syncDir;
    int computersConnected;
    int totalDownloads;

    void updateFileTable();
    QTreeWidgetItem * scanDirectory(QString path);
    void generateBoxLayout(QTreeWidgetItem * item);
    QSignalMapper * boxMapper;
    QPushButton * serverDownload;

    Computer * findByName(QString);
    QStringList rebuildPath(QTreeWidgetItem *, QString, int);


public:
    enum Mode{Server, Client};

    QFileSystemWatcher* fileWatcher;

    QLineEdit * uPort;
    QLineEdit * tPort;
    QLineEdit * uDelay;
    QLineEdit * tDelay;
    QLineEdit * nDelay;
    QCheckBox * debugOn;

    QPushButton * dl;

    QLineEdit * passBox;
    CFtpServer * ftpServer;
    QDateTime * currentTime;
    QFile * file;
    QFtp * ftpClient;
    QFile * log;
    QIcon * icon;
    QSystemTrayIcon * sysIcon;
    QMenu * sysMenu;
    void writeLog(QString);
    QLabel * mainLabel;
    QTreeWidget * computerTable;
    QProgressBar * progress;
    QPushButton * client;
    QPushButton * server;
    QWidget * settings;
    QSettings * setup;
    Computer * firstComp;
    QUdpSocket * udpSocket;
    QTcpSocket * tcpSocket;
    QTcpServer * tcpServer;
    QSignalMapper * mapper;
    Widget();
    QMainWindow * mainWindow;
    QTimer * timer;
    Mode getMode();
    unsigned long long bytesIn();
    unsigned long long bytesOut();
    void establishTcp(QHostAddress host, qint16 port);
    QString search(QString file, QString dir);
    Computer * newComp();
    void updateTable(Computer * comp);
    void updateAddresses();
    void checkPassword(QByteArray pass);
    ~Widget();

public slots:
    void broadcastUdp();
    void signalReceived();
    void startClient();
    void startServer();
    void refreshNet();
    void serverCalled();
    void dataReceived();
    void computerRead(Computer * comp);
    void clientReceived();
    void socketClosed();
    void update();
    void socketError(QAbstractSocket::SocketError);
    void sockConn();
    void list(QUrlInfo);
    void updateProgress(qint64, qint64);
    void ftpDone(bool);
    void about();
    void iconClicked(QSystemTrayIcon::ActivationReason);
    void close();
    void disconnected();
    void settingsWindow();
    void apply();
    void error();
    void normal();
    void passEnter();
    void getFirst();
    void getSelected();
    void ftpCommandDone(int, bool errorb);
    void updateList();
    void sendUpdate();
    void boxUpdated(QTreeWidgetItem*, int);
    void serverDownloadStart();





};

#endif // WIDGET_H
