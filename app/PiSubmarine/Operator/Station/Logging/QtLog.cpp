#include "PiSubmarine/Operator/Station/Logging/QtLog.h"

#include <memory>

#include <QString>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

namespace
{
    std::shared_ptr<spdlog::logger> QtLogger;
    QtMessageHandler PreviousQtHandler = nullptr;

    spdlog::level::level_enum ToSpdlogLevel(const QtMsgType type)
    {
        switch (type)
        {
        case QtDebugMsg:
            return spdlog::level::debug;
        case QtInfoMsg:
            return spdlog::level::info;
        case QtWarningMsg:
            return spdlog::level::warn;
        case QtCriticalMsg:
            return spdlog::level::err;
        case QtFatalMsg:
            return spdlog::level::critical;
        }

        return spdlog::level::info;
    }

    void QtMessageHandlerFunction(
        const QtMsgType type,
        const QMessageLogContext& context,
        const QString& message)
    {
        auto logger = QtLogger ? QtLogger : spdlog::default_logger();

        if (logger)
        {
            const auto level = ToSpdlogLevel(type);
            const char* category = context.category ? context.category : "qt";
            const char* file = context.file ? context.file : "";
            const char* function = context.function ? context.function : "";

            logger->log(
                spdlog::source_loc{file, static_cast<int>(context.line), function},
                level,
                "[{}] {}",
                category,
                message.toStdString());
        }
    }
}

namespace PiSubmarine::Operator::Station::Logging
{
    void InstallQtMessageHandler(std::shared_ptr<spdlog::logger> logger)
    {
        QtLogger = std::move(logger);
        PreviousQtHandler = qInstallMessageHandler(QtMessageHandlerFunction);
    }

    void UninstallQtMessageHandler()
    {
        qInstallMessageHandler(PreviousQtHandler);
        PreviousQtHandler = nullptr;
        QtLogger.reset();
    }
}
