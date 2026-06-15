#include <QGuiApplication>
#include "PiSubmarine/Operator/Station/Qt/App.h"

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    PiSubmarine::Operator::Station::Qt::App stationApp;
    return stationApp.Run(application);
}
