#include "PiSubmarine/Operator/Station/Infrastructure/QtLog.h"

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <QString>

namespace
{
    std::shared_ptr<spdlog::logger> QtLogger;
    QtMessageHandler PreviousQtHandler = nullptr;

    spdlog::level::level_enum toSpdlogLevel(QtMsgType type)
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

    void qtMessageHandler(
        QtMsgType type,
        const QMessageLogContext& context,
        const QString& message)
    {
        auto logger = QtLogger ? QtLogger : spdlog::default_logger();

        if (logger)
        {
            const auto level = toSpdlogLevel(type);

            // Qt category, file, function and line may be null / 0,
            // especially in release builds.
            const char* category = context.category ? context.category : "qt";
            const char* file = context.file ? context.file : "";
            const char* function = context.function ? context.function : "";
            int line = context.line;

            logger->log(
                spdlog::source_loc{file, line, function},
                level,
                "[{}] {}",
                category,
                message.toStdString());
        }

        // Optional: also call the original Qt handler.
        // Usually disable this to avoid duplicate console output.
        /*
        if (PreviousQtHandler)
            PreviousQtHandler(type, context, message);
        */
    }
}

namespace PiSubmarine::Operator::Station::Infrastructure
{
    void InstallQtMessageHandler(std::shared_ptr<spdlog::logger> logger)
    {
        QtLogger = std::move(logger);
        PreviousQtHandler = qInstallMessageHandler(qtMessageHandler);
    }

    void UninstallQtMessageHandler()
    {
        qInstallMessageHandler(PreviousQtHandler);
        PreviousQtHandler = nullptr;
        QtLogger.reset();
    }
}