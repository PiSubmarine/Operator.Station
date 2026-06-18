#include <spdlog/spdlog.h>

// TODO Rename to PiSubmarine::Operator::Station::Logging. Don't forget to rename the directory too.
namespace PiSubmarine::Operator::Station::Infrastructure
{
    void InstallQtMessageHandler(std::shared_ptr<spdlog::logger> logger);

    void UninstallQtMessageHandler();
}