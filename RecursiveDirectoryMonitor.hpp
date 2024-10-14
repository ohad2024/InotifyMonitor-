
#ifndef RECURSIVE_DIRECTORY_MONITOR_HPP
#define RECURSIVE_DIRECTORY_MONITOR_HPP

#include <iostream>
#include <vector>
#include <string>
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_LENGTH (1024 * (EVENT_SIZE + 16))

struct DirectoryWatch {
    int watchDescriptor;
    std::string directoryPath;
};

class RecursiveDirectoryMonitor {
    int inotifyFileDescriptor;
    std::vector<DirectoryWatch> directoryWatches;
    char eventBuffer[BUFFER_LENGTH];
    bool stopFlag;
    std::string rootDirectory;

    void addWatchToDirectory(const std::string &directoryPath);
    void addWatchRecursively(const std::string &directoryPath);
    void processEventBuffer(int bufferLength);
    void handleEvent(struct inotify_event *event, const std::string &directoryPath);
    std::string getDirectoryPathFromDescriptor(int watchDescriptor);
    void cleanupWatches();
    void rootDirectoryDeleted();

public:
    RecursiveDirectoryMonitor(const std::string &rootDirectoryPath);
    ~RecursiveDirectoryMonitor();
    void monitorDirectories();
    void stopMonitoring();
};

#endif // RECURSIVE_DIRECTORY_MONITOR_HPP
