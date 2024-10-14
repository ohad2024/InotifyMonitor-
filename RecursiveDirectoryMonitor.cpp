
#include "RecursiveDirectoryMonitor.hpp"
#include <cstdlib>
#include <dirent.h>
#include <unistd.h>

RecursiveDirectoryMonitor::RecursiveDirectoryMonitor(const std::string &rootDirectoryPath)
    : stopFlag(false), rootDirectory(rootDirectoryPath) {
    inotifyFileDescriptor = inotify_init();
    if (inotifyFileDescriptor < 0) {
        std::cerr << "Failed to initialize inotify" << std::endl;
        exit(1);
    }
    addWatchRecursively(rootDirectoryPath);
}

RecursiveDirectoryMonitor::~RecursiveDirectoryMonitor() {
    cleanupWatches();
    close(inotifyFileDescriptor);
}

void RecursiveDirectoryMonitor::addWatchToDirectory(const std::string &directoryPath) {
    int watchDescriptor = inotify_add_watch(inotifyFileDescriptor, directoryPath.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
    if (watchDescriptor != -1) {
        directoryWatches.push_back({watchDescriptor, directoryPath});
    } else {
        std::cerr << "Failed to add watch for " << directoryPath << std::endl;
    }
}

void RecursiveDirectoryMonitor::addWatchRecursively(const std::string &directoryPath) {
    addWatchToDirectory(directoryPath);
    DIR *directory = opendir(directoryPath.c_str());
    if (!directory) {
        std::cerr << "Failed to open directory: " << directoryPath << std::endl;
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(directory)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            std::string entryName = entry->d_name;
            if (entryName != "." && entryName != "..") {
                std::string subDirectoryPath = directoryPath + "/" + entryName;
                addWatchRecursively(subDirectoryPath);
            }
        }
    }

    closedir(directory);
}

void RecursiveDirectoryMonitor::monitorDirectories() {
    std::cout << "Monitoring directories for changes..." << std::endl;
    while (!stopFlag) {
        int bufferLength = read(inotifyFileDescriptor, eventBuffer, BUFFER_LENGTH);
        if (bufferLength < 0) {
            std::cerr << "Failed to read events" << std::endl;
            break;
        }
        processEventBuffer(bufferLength);
    }
}

void RecursiveDirectoryMonitor::processEventBuffer(int bufferLength) {
    int i = 0;
    while (i < bufferLength) {
        struct inotify_event *event = (struct inotify_event *)&eventBuffer[i];
        if (event->len) {
            std::string directoryPath = getDirectoryPathFromDescriptor(event->wd);
            if (directoryPath == rootDirectory && (event->mask & IN_DELETE) && (event->mask & IN_ISDIR)) {
                rootDirectoryDeleted();
                break;
            }
            handleEvent(event, directoryPath);
        }
        i += EVENT_SIZE + event->len;
    }
}

void RecursiveDirectoryMonitor::handleEvent(struct inotify_event *event, const std::string &directoryPath) {
    std::string filePath = directoryPath + "/" + event->name;

    if (event->mask & IN_CREATE) {
        if (event->mask & IN_ISDIR) {
            std::cout << "Directory " << filePath << " was created." << std::endl;
            addWatchRecursively(filePath);
        } else {
            std::cout << "File " << filePath << " was created." << std::endl;
        }
    } else if (event->mask & IN_DELETE) {
        if (event->mask & IN_ISDIR) {
            std::cout << "Directory " << filePath << " was deleted." << std::endl;
        } else {
            std::cout << "File " << filePath << " was deleted." << std::endl;
        }
    } else if (event->mask & IN_MODIFY) {
        if (event->mask & IN_ISDIR) {
            std::cout << "Directory " << filePath << " was modified." << std::endl;
        } else {
            std::cout << "File " << filePath << " was modified." << std::endl;
        }
    }
}

std::string RecursiveDirectoryMonitor::getDirectoryPathFromDescriptor(int watchDescriptor) {
    for (const auto &directoryWatch : directoryWatches) {
        if (directoryWatch.watchDescriptor == watchDescriptor) {
            return directoryWatch.directoryPath;
        }
    }
    return "";
}

void RecursiveDirectoryMonitor::cleanupWatches() {
    for (const auto &directoryWatch : directoryWatches) {
        inotify_rm_watch(inotifyFileDescriptor, directoryWatch.watchDescriptor);
    }
}

void RecursiveDirectoryMonitor::stopMonitoring() {
    stopFlag = true;
}

void RecursiveDirectoryMonitor::rootDirectoryDeleted() {
    std::cout << "Root directory " << rootDirectory << " was deleted. Stopping monitoring." << std::endl;
    stopMonitoring();
}
