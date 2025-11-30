#include <iostream>
#include <string>
#include <memory> // For std::unique_ptr
#include <algorithm> // For string manipulation if needed

// Include Factory and Interface
#include "../include/CommandFactory.h"
#include "../include/modules/Application.h"

// Include platform-specific implementation
#ifdef _WIN32
#include "../include/modules/Application_Windows.h" // Check if your file is named Application_Window.h or Application_Windows.h
#endif

// --- Helper: Factory function to create Application based on OS ---
std::unique_ptr<Application> createApplication() {
#ifdef _WIN32
    // Create Windows-specific Application object
    return std::make_unique<Application_Windows>(); 
#else
    // Linux/Mac are not supported yet
    std::cerr << "Error: This platform is not supported yet.\n";
    return nullptr;
#endif
}

int main() {
    // 1. Initialize the application
    auto app = createApplication();
    if (!app) {
        return 1; // Exit if initialization fails
    }

    std::string inputLine;

    // Print usage instructions
    std::cout << "=== APPLICATION MANAGER CLI ===\n";
    std::cout << "Available commands:\n";
    std::cout << " 1. list              : List all running applications in to file text\n";
    std::cout << " 2. start [path]      : Start an application (e.g., start notepad.exe)\n";
    std::cout << " 3. stop [path]       : Stop an application by PID (e.g., stop 1234)\n";
    std::cout << " 4. screenshot [path] : Take a screenshot (e.g., screenshot myimage.bmp)\n";
    std::cout << " 5. exit              : Exit the program\n";
    std::cout << "===============================\n";

    // 2. Main loop
    while (true) {
        std::cout << "\n> "; // Command prompt
        
        // Read the entire line to capture arguments with spaces
        if (!std::getline(std::cin, inputLine)) {
            break; // Exit on stream error or Ctrl+C
        }

        // Handle empty lines or exit command
        if (inputLine.empty()) continue;
        if (inputLine == "exit") break;

        // 3. Parse Command Name and Arguments
        // Example: "start C:/Program Files/App.exe"
        // -> cmdName = "start"
        // -> args    = "C:/Program Files/App.exe"
        
        std::string cmdName;
        std::string args;
        
        size_t spacePos = inputLine.find(' ');
        if (spacePos != std::string::npos) {
            cmdName = inputLine.substr(0, spacePos);
            args = inputLine.substr(spacePos + 1);
        } else {
            cmdName = inputLine; // No arguments (e.g., "list")
            args = "";
        }

        // 4. Use Factory to create Command object
        // Pass raw pointer (app.get()) to the factory
        ICommand* command = CommandFactory::create(cmdName, app.get());

        if (command != nullptr) {
            // 5. Execute command and print result
            std::string result = command->execute(args);
            std::cout << result << std::endl;

            // 6. Memory cleanup (IMPORTANT)
            // Since Factory uses 'new', we must 'delete' manually
            delete command;
        } else {
            std::cout << "Error: Unknown command '" << cmdName << "'. Type 'exit' to quit.\n";
        }
    }

    std::cout << "Program exited.\n";
    return 0;
}

/* BUILD FILE .exe
g++ -std=c++17 -I./include src/main.cpp src/core/CommandFactory.cpp src/modules/Application_Windows.cpp src/modules/Sreenshot.cpp src/Commands/ListApplicationCommand.cpp src/Commands/StartApplicationCommand.cpp src/Commands/StopApplicationCommand.cpp src/Commands/ExportApplicationCommand.cpp src/Commands/ScreenshotCommand.cpp
 -o AppManager.exe -lgdi32 -luser32
*/