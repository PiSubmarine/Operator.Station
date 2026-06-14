#include <QGuiApplication>
#include "PiSubmarine/Operator/Station/Qt/StationApp.h"

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    PiSubmarine::Operator::Station::Qt::StationApp stationApp;
    return stationApp.Run(application);
}
