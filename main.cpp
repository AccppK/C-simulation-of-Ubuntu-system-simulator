#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <direct.h>
#include <windows.h>
#include <io.h>
#include <fstream>
#include <cstring>
#include <conio.h> // 用于 getch()

// 命令函数原型
typedef void (*CommandFunction)(const std::vector<std::string>&);

// 全局变量
std::map<std::string, CommandFunction> commands;
std::string currentUser = "user";
std::string currentHost = "ubuntu";
std::string currentDir = "C:/Users/user";
bool running = true;

// 辅助函数
std::vector<std::string> splitString(const std::string& input) {
    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;
    char quoteChar = '\0';

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if ((c == '\'' || c == '"') && !inQuotes) {
            inQuotes = true;
            quoteChar = c;
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else if (c == quoteChar && inQuotes) {
            inQuotes = false;
            tokens.push_back(token);
            token.clear();
        } else if (std::isspace(c) && !inQuotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string getCurrentTime() {
    std::time_t now = std::time(0);
    std::tm* localTime = std::localtime(&now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", localTime);
    return std::string(buffer);
}

// 命令实现
void cmd_help(const std::vector<std::string>& args) {
    std::cout << "Available commands:" << std::endl;
    std::cout << "  help        - Show this help" << std::endl;
    std::cout << "  exit        - Exit terminal" << std::endl;
    std::cout << "  clear       - Clear screen" << std::endl;
    std::cout << "  ls          - List directory contents" << std::endl;
    std::cout << "  cd          - Change directory" << std::endl;
    std::cout << "  pwd         - Print working directory" << std::endl;
    std::cout << "  mkdir       - Create directory" << std::endl;
    std::cout << "  rmdir       - Remove directory" << std::endl;
    std::cout << "  touch       - Create file" << std::endl;
    std::cout << "  rm          - Remove file" << std::endl;
    std::cout << "  cat         - Show file content" << std::endl;
    std::cout << "  echo        - Display text" << std::endl;
    std::cout << "  date        - Show current date/time" << std::endl;
    std::cout << "  whoami      - Show current user" << std::endl;
    std::cout << "  uname       - Show system info" << std::endl;
    std::cout << "  ps          - Show process status" << std::endl;
    std::cout << "  top         - Show task manager" << std::endl;
    std::cout << "  sudo        - Execute as superuser" << std::endl;
}

void cmd_exit(const std::vector<std::string>& args) {
    running = false;
    std::cout << "Logging out..." << std::endl;
}

void cmd_clear(const std::vector<std::string>& args) {
    std::system("cls");
}

void cmd_ls(const std::vector<std::string>& args) {
    std::string path = args.size() > 1 ? args[1] : ".";
    std::string fullPath = currentDir + "/" + path;

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA((fullPath + "/*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "ls: cannot access '" << path << "'" << std::endl;
        return;
    }

    do {
        std::string filename = findFileData.cFileName;
        if (filename == "." || filename == "..") continue;

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            std::cout << filename << "/ ";
        } else {
            std::cout << filename << " ";
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
    std::cout << std::endl;
}

void cmd_cd(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        currentDir = "C:/Users/" + currentUser;
        return;
    }

    std::string newDir = args[1];
    if (newDir == "~") {
        currentDir = "C:/Users/" + currentUser;
        return;
    }

    if (newDir.find(":") != std::string::npos) {
        // Absolute path
        if (_access(newDir.c_str(), 0) == 0) {
            currentDir = newDir;
        } else {
            std::cerr << "cd: " << newDir << ": No such file or directory" << std::endl;
        }
    } else {
        // Relative path
        std::string fullPath = currentDir + "/" + newDir;
        if (_access(fullPath.c_str(), 0) == 0) {
            currentDir = fullPath;
        } else {
            std::cerr << "cd: " << newDir << ": No such file or directory" << std::endl;
        }
    }
}

void cmd_pwd(const std::vector<std::string>& args) {
    std::cout << currentDir << std::endl;
}

void cmd_mkdir(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: mkdir <dirname>" << std::endl;
        return;
    }

    std::string dirname = args[1];
    std::string fullPath = currentDir + "/" + dirname;

    if (_access(fullPath.c_str(), 0) == 0) {
        std::cerr << "mkdir: cannot create directory '" << dirname << "': File exists" << std::endl;
    } else {
        _mkdir(fullPath.c_str());
    }
}

void cmd_rmdir(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: rmdir <dirname>" << std::endl;
        return;
    }

    std::string dirname = args[1];
    std::string fullPath = currentDir + "/" + dirname;

    if (_access(fullPath.c_str(), 0) != 0) {
        std::cerr << "rmdir: cannot remove '" << dirname << "': No such file or directory" << std::endl;
    } else if (!(GetFileAttributesA(fullPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY)) {
        std::cerr << "rmdir: cannot remove '" << dirname << "': Not a directory" << std::endl;
    } else {
        _rmdir(fullPath.c_str());
    }
}

void cmd_touch(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: touch <filename>" << std::endl;
        return;
    }

    std::string filename = args[1];
    std::string fullPath = currentDir + "/" + filename;

    std::ofstream file(fullPath.c_str());
    if (!file.is_open()) {
        std::cerr << "touch: cannot create file '" << filename << "'" << std::endl;
    }
    file.close();
}

void cmd_rm(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: rm <filename>" << std::endl;
        return;
    }

    std::string filename = args[1];
    std::string fullPath = currentDir + "/" + filename;

    if (_access(fullPath.c_str(), 0) != 0) {
        std::cerr << "rm: cannot remove '" << filename << "': No such file or directory" << std::endl;
    } else if (GetFileAttributesA(fullPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
        std::cerr << "rm: cannot remove '" << filename << "': Is a directory" << std::endl;
    } else {
        std::remove(fullPath.c_str());
    }
}

void cmd_cat(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: cat <filename>" << std::endl;
        return;
    }

    std::string filename = args[1];
    std::string fullPath = currentDir + "/" + filename;

    if (_access(fullPath.c_str(), 0) != 0) {
        std::cerr << "cat: " << filename << ": No such file or directory" << std::endl;
        return;
    }

    std::ifstream file(fullPath.c_str());
    if (!file.is_open()) {
        std::cerr << "cat: cannot open file '" << filename << "'" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }
    file.close();
}

void cmd_echo(const std::vector<std::string>& args) {
    for (size_t i = 1; i < args.size(); ++i) {
        std::cout << args[i] << " ";
    }
    std::cout << std::endl;
}

void cmd_date(const std::vector<std::string>& args) {
    std::cout << getCurrentTime() << std::endl;
}

void cmd_whoami(const std::vector<std::string>& args) {
    std::cout << currentUser << std::endl;
}

void cmd_uname(const std::vector<std::string>& args) {
    std::cout << "Windows " << currentHost << " 10.0.19041" << std::endl;
}

void cmd_ps(const std::vector<std::string>& args) {
    std::cout << "  PID TTY          TIME CMD" << std::endl;
    std::cout << "    1 ?        00:00:01 system" << std::endl;
    std::cout << " 1234 ?        00:00:00 cmd" << std::endl;
    std::cout << " 5678 ?        00:00:00 ps" << std::endl;
}

void cmd_top(const std::vector<std::string>& args) {
    std::cout << "Tasks: 100 total, 1 running, 99 sleeping, 0 stopped, 0 zombie" << std::endl;
    std::cout << "%CPU(s):  5.0 us,  2.5 sy,  0.0 ni, 92.5 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st" << std::endl;
    std::cout << "Memory: 16G total, 8G free, 4G used, 4G buff/cache" << std::endl;
    std::cout << "Swap: 4G total, 3G free, 1G used" << std::endl;
    std::cout << std::endl;
    std::cout << "  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND" << std::endl;
    std::cout << "    1 system    20   0    100M    10M    5M S   0.0  0.1   1:30.00 system" << std::endl;
    std::cout << " 1234 " << currentUser << "     20   0    50M     8M    6M S   0.0  0.1   0:00.50 cmd" << std::endl;
    std::cout << " 5678 " << currentUser << "     20   0    40M     6M    5M R   0.3  0.1   0:00.01 top" << std::endl;
}

void cmd_sudo(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: sudo <command>" << std::endl;
        return;
    }

    std::cout << "[sudo] password for " << currentUser << ": ";
    std::string password;
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b' && !password.empty()) {
            std::cout << "\b \b";
            password.erase(password.end() - 1);
        } else if (std::isprint(ch)) {
            std::cout << '*';
            password += ch;
        }
    }
    std::cout << std::endl;

    if (password != "password") {
        std::cerr << "sudo: authentication failed" << std::endl;
        return;
    }

    std::string command = args[1];
    if (commands.find(command) != commands.end()) {
        std::vector<std::string> newArgs(args.begin() + 1, args.end());
        commands[command](newArgs);
    } else {
        std::cerr << "sudo: " << command << ": command not found" << std::endl;
    }
}

// 初始化命令
void initializeCommands() {
    commands["help"] = cmd_help;
    commands["exit"] = cmd_exit;
    commands["clear"] = cmd_clear;
    commands["ls"] = cmd_ls;
    commands["cd"] = cmd_cd;
    commands["pwd"] = cmd_pwd;
    commands["mkdir"] = cmd_mkdir;
    commands["rmdir"] = cmd_rmdir;
    commands["touch"] = cmd_touch;
    commands["rm"] = cmd_rm;
    commands["cat"] = cmd_cat;
    commands["echo"] = cmd_echo;
    commands["date"] = cmd_date;
    commands["whoami"] = cmd_whoami;
    commands["uname"] = cmd_uname;
    commands["ps"] = cmd_ps;
    commands["top"] = cmd_top;
    commands["sudo"] = cmd_sudo;
}

int main() {
    // 设置当前目录
    char cwd[1024];
    if (_getcwd(cwd, sizeof(cwd))) {
        currentDir = cwd;
    }

    // 初始化命令
    initializeCommands();

    // 显示欢迎信息
    std::cout << "Welcome to Ubuntu Terminal Simulator (Dev-C++ 5.11 compatible)" << std::endl;
    std::cout << "Type 'help' to see available commands" << std::endl;

    // 运行终端
    std::string input;
    while (running) {
        // 显示提示符
        std::cout << currentUser << "@" << currentHost << ":" << currentDir << "$ ";

        // 获取输入
        std::getline(std::cin, input);
        if (input.empty()) continue;

        // 解析命令
        std::vector<std::string> args = splitString(input);
        std::string command = args[0];

        // 执行命令
        if (commands.find(command) != commands.end()) {
            commands[command](args);
        } else {
            std::cerr << command << ": command not found" << std::endl;
        }
    }

    return 0;
}    
