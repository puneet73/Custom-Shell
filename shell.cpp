#include <bits/stdc++.h>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

using namespace std;

vector<string> builtin_commands = {"cd", "ls", "mkdir", "touch", "rm", "cp", "mv", "echo", "cat", "grep", "help", "exit", "sudo","wait"};

int (*builtin_functions[])(vector<string>&) = {&lsh_cd, &lsh_ls, &lsh_mkdir, &lsh_touch, &lsh_rm, &lsh_cp, &lsh_mv, &lsh_echo, &lsh_cat, &lsh_grep, &lsh_help, &lsh_exit, &lsh_sudo,&lsh_wait};

int num_builtin_commands() {
    return builtin_commands.size();
}

int lsh_cd(vector<string>& args) {
    if (args.size() == 1) {
        const char* home_dir = getenv("HOME");
        if (home_dir == nullptr) {
            cerr << "cd: No HOME environment variable set" << endl;
        } else {
            if (chdir(home_dir) != 0) {
                perror("cd");
            }
        }
    } else {
        if (chdir(args[1].c_str()) != 0) {
            perror("cd");
        }
    }
    return 1;
}

int lsh_ls(vector<string>& args) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(".")) == nullptr) {
        perror("ls");
        return 1;
    }

    while ((entry = readdir(dir)) != nullptr) {
        cout << entry->d_name << endl;
    }

    closedir(dir);
    return 1;
}

int lsh_mkdir(vector<string>& args) {
    if (args.size() < 2) {
        cerr << "mkdir: missing operand" << endl;
        return 1;
    }

    for (size_t i = 1; i < args.size(); ++i) {
        if (mkdir(args[i].c_str(), 0777) != 0) {   // 7 = {read + write + execute}
            perror("mkdir");
        }
    }
    return 1;
}

int lsh_touch(vector<string>& args) {
    if (args.size() < 2) {
        cerr << "touch: missing operand" << endl;
        return 1;
    }

    for (size_t i = 1; i < args.size(); ++i) {
        if (creat(args[i].c_str(), 0666) == -1) {
            perror("touch");
        }
    }
    return 1;
}

int lsh_rm(vector<string>& args) {
    if (args.size() < 2) {
        cerr << "rm: missing operand" << endl;
        return 1;
    }

    for (size_t i = 1; i < args.size(); ++i) {
        if (remove(args[i].c_str()) != 0) {
            perror("rm");
        }
    }
    return 1;
}

int lsh_cp(vector<string>& args) {
    if (args.size() < 3) {
        cerr << "cp: missing operand" << endl;
        return 1;
    }

    if (args.size() > 3) {
        cerr << "cp: extra operand '" << args[3] << "'" << endl;
        return 1;
    }

    if (rename(args[1].c_str(), args[2].c_str()) != 0) {
        perror("cp");
    }
    return 1;
}

int lsh_mv(vector<string>& args) {
    if (args.size() < 3) {
        cerr << "mv: missing operand" << endl;
        return 1;
    }

    if (args.size() > 3) {
        cerr << "mv: extra operand '" << args[3] << "'" << endl;
        return 1;
    }

    if (rename(args[1].c_str(), args[2].c_str()) != 0) {
        perror("mv");
    }
    return 1;
}

int lsh_echo(vector<string>& args) {
    for (size_t i = 1; i < args.size(); ++i) {
        cout << args[i] << " ";
    }
    cout << endl;
    return 1;
}

int lsh_cat(vector<string>& args) {
    if (args.size() < 2) {
        cerr << "cat: missing operand" << endl;
        return 1;
    }

    for (size_t i = 1; i < args.size(); ++i) {
        ifstream file(args[i]);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                cout << line << endl;
            }
            file.close();
        } else {
            perror("cat");
        }
    }
    return 1;
}

int lsh_grep(vector<string>& args) {
    if (args.size() < 3) {
        cerr << "grep: missing operand" << endl;
        return 1;
    }

    string pattern = args[1];
    for (size_t i = 2; i < args.size(); ++i) {
        ifstream file(args[i]);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                if (line.find(pattern) != string::npos) {
                    cout << line << endl;
                }
            }
            file.close();
        } else {
            perror("grep");
        }
    }
    return 1;
}

int lsh_help(vector<string>& args) {
    cout << "Custom Shell by YourName\n";
    cout << "Type commands and arguments, and hit enter.\n";
    cout << "The following built-in commands are available:\n";

    for (const auto& cmd : builtin_commands) {
        cout << "  " << cmd << endl;
    }

    return 1;
}

int lsh_exit(vector<string>& args) {
    return 0;
}


int lsh_execute(vector<string>& args) {
    if (args.empty()) {
        return 1;
    }

    for (int i = 0; i < num_builtin_commands(); ++i) {
        if (args[0] == builtin_commands[i]) {
            return (*builtin_functions[i])(args);
        }
    }

    return lsh_launch(args);
}

int lsh_wait(vector<string>& args) {
    pid_t wpid;
    int status;

    // Wait for all child processes to terminate
    while ((wpid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Do nothing, just waiting
    }

    return 1;
}


int lsh_sudo(vector<string>& args) {
    // Check if the user has a valid request 
    if (args.size() < 2) {
        cerr << "sudo: no command specified" << endl;
        return 1;
    }

    // with administrative privileges
    vector<string> command_args(args.begin() + 1, args.end());
    command_args.insert(command_args.begin(), "sudo");
    return lsh_execute(command_args);
}


int lsh_launch(vector<string>& args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        char** argv = new char*[args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i] = strdup(args[i].c_str());
        }
        argv[args.size()] = nullptr;

        if (execvp(argv[0], argv) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1

}

void lsh_loop() {
    string line;
    vector<string> args;
    int status;

    do {
        cout << "> ";
        getline(cin, line);
        args.clear();
        size_t pos = 0;
        string token;
        while ((pos = line.find(' ')) != string::npos) {
            token = line.substr(0, pos);
            args.push_back(token);
            line.erase(0, pos + 1);
        }
        args.push_back(line);

        status = lsh_execute(args);
    } while (status);
}

int main() {
    lsh_loop();

    return EXIT_SUCCESS;
}
