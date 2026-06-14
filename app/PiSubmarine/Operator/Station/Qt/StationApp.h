#pragma once

#include <memory>
#include <string>

#include <QQmlApplicationEngine>
#include <QThread>

#include "PiSubmarine/Operator/Station/Qt/VideoTailFactory.h"

namespace spdlog
{
    class logger;
}

class QCommandLineParser;
class QGuiApplication;

namespace PiSubmarine::Operator::Station::Qt
{
    class VideoItem;
    class VideoRuntimeWorker;

    class StationApp final
    {
    public:
        [[nodiscard]] int Run(QGuiApplication& application);

    private:
        bool ConfigureCommandLine(QGuiApplication& application, QCommandLineParser& parser) const;
        bool LoadMainWindow();
        bool StartVideoRuntime(bool useFakeVideo, std::uint16_t videoPort, const std::string& bindAddress);
        void StopVideoRuntime();

        QQmlApplicationEngine m_Engine;
        QThread m_RuntimeThread;
        VideoTailFactory m_TailFactory;
        VideoRuntimeWorker* m_RuntimeWorker = nullptr;
        VideoItem* m_VideoItem = nullptr;
        std::shared_ptr<spdlog::logger> m_Logger;
        bool m_HasReceivedFirstFrame = false;
    };
}
