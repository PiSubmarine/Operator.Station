#include <spdlog/spdlog.h>

namespace PiSubmarine::Operator::Station::Infrastructure
{
    void InstallQtMessageHandler(std::shared_ptr<spdlog::logger> logger);

    void UninstallQtMessageHandler();
}