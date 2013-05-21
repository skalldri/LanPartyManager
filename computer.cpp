#include "computer.h"

#if defined(WIN32) && defined(_DEBUG)
  #define new DEBUG_NEW
#endif

Computer::Computer(QWidget* parent)
{
    nextComp = NULL;
    itemMain = new QTreeWidgetItem(0);
    bar = new QProgressBar(parent);
    //bar->resize(
    socket = NULL;
    id = 0;

}

Computer * Computer::getNext()
{
    return nextComp;
}

void Computer::setNext(Computer* next)
{
    nextComp = next;
}

QTcpSocket* Computer::getSocket()
{
    return socket;
}

void Computer::setSocket(QTcpSocket* socket2)
{
socket = socket2;
}

int Computer::getId()
{
    return id;
}
void Computer::setId(int i)
{
    id = i;
}
QHostAddress Computer::getHost()
{
    return host;
}

void Computer::setHost(QHostAddress ho)
{
    host = ho;
}

unsigned long long Computer::getTotal()
{
    return bandwidthTotal;
}

void Computer::setTotal(unsigned long long total)
{
    bandwidthTotal = total;
}

QString Computer::getName()
{
    return name;
}

void Computer::setName(QString nm)
{
    name = nm;
}

unsigned long long Computer::getIn()
{
    return in;
}

void Computer::setIn(unsigned long long i)
{
    in = i;
}

unsigned long long Computer::getOut()
{
    return out;
}

void Computer::setOut(unsigned long long o)
{
    out = o;
}

QTreeWidgetItem * Computer::getItem()
{
    itemMain->setText(0, name);
    itemMain->setText(1, QString().number(bandwidthTotal));
    return itemMain;
}

QProgressBar * Computer::getBar()
{
    return bar;
}
