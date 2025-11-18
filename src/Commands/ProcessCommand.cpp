#include "../../include/Commands/ProcessCommand.h"
#include "../../include/Modules/ProcessManager.h"
#include <sstream>
#include <fstream>

std::string ProcessCommand::execute(const std::string& args) {

    std::stringstream ss(args);
    std::string action;
    ss >> action;

    // ---- LIST ----
    if (action == "list") {
        auto list = ProcessModule::list();

        std::ofstream f("process_list.txt");
        for (auto& p : list) {
            f << p.pid << " - " << p.name << "\n";
        }
        f.close();

        return "[Process] list saved to process_list.txt";
    }

    // ---- KILL <PID> ----
    if (action == "kill") {
        unsigned long pid;
        ss >> pid;

        if (!ss) return "[Process] Invalid PID";

        bool ok = ProcessModule::kill(pid);
        return ok ? "[Process] killed" : "[Process] kill failed";
    }

    // ---- START <path> ----
    if (action == "start") {
        std::string path;
        std::getline(ss, path);

        // Remove leading space
        if (!path.empty() && path[0] == ' ')
            path = path.substr(1);

        if (path.empty())
            return "[Process] Missing file path";

        bool ok = ProcessModule::start(path);
        return ok ? "[Process] started" : "[Process] start failed";
    }

    return "[Process] Unknown process command: " + args;
}
