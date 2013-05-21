#ifndef COMPUTER_H
#define COMPUTER_H

#include <QObject>
#include <QWidget>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QtCore>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QStandardItem>
#include <QProgressBar>


class Computer
{
public:
    Computer * nextComp;
    QString name;
    int id;
    unsigned long long bandwidthTotal;
    unsigned long long in;
    unsigned long long out;
    QTreeWidgetItem * getItem();
    QTreeWidgetItem *  itemMain;
    QProgressBar * bar;
    QProgressBar * getBar();
    QTcpSocket* socket;
    QHostAddress host;
    QHostAddress getHost();
    void setHost(QHostAddress ho);
    void setNext(Computer * next);
    Computer * getNext();
    void setSocket(QTcpSocket* socket);
    QTcpSocket* getSocket();
    void setId(int id);
    int getId();
    void setTotal(unsigned long long total);
    unsigned long long getTotal();
    QString getName();
    void setName(QString nm);
    unsigned long long getIn();
    void setIn(unsigned long long i);
    unsigned long long getOut();
    void setOut(unsigned long long o);
    Computer(QWidget * parent);


};

#endif // COMPUTER_H
