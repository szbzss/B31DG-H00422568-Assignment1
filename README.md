# B31DG-H00422568-Assignment1
## Project Structure

This repository is organized to contain source code for both Arduino and ESP-IDF development environments, as required by the assignment deliverables.

```text
.
├── .vscode/                                # Global VS Code settings
├── B31DG-H00422568-ASSIGNMENT1/           # Core ESP-IDF Project Directory
│   ├── .devcontainer/                     # Containerized development settings
│   ├── .vscode/                           # Project-specific IDE configurations
│   ├── main/                              # ESP-IDF source files
│   │   └── hello_world_main.c             # C implementation of the signal generator
│   ├── CMakeLists.txt                     # Build system script
│   ├── sdkconfig                          # ESP-IDF project configuration
│   └── .gitignore                         # Local exclusion for build artifacts
├── Arduino_code.ino                       # Arduino IDE source code implementation
├── README.md                              # This documentation file
