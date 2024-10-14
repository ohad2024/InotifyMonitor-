
# Recursive Directory Monitor

This program is a recursive directory monitoring tool built using C++ and `inotify`. It monitors a specified directory and all its subdirectories for file changes (creation, deletion, and modification), printing notifications in real-time when these changes occur.

## Struct: DirectoryWatch
The `DirectoryWatch` struct is designed to store information about each directory that is being monitored. It includes:
- **`int watchDescriptor`**: A unique identifier returned by `inotify_add_watch`, used to reference each specific directory watch.
- **`std::string directoryPath`**: The path to the directory being monitored. This helps in mapping each watch descriptor back to the directory it represents.

## Class: RecursiveDirectoryMonitor
The `RecursiveDirectoryMonitor` class is responsible for recursively adding watches to directories and their subdirectories, processing events, and managing the monitoring lifecycle.

### Private Member Variables
1. **`int inotifyFileDescriptor`**: Holds the file descriptor for the `inotify` instance, initialized by `inotify_init`. It is used for all `inotify` operations.
2. **`std::vector<DirectoryWatch> directoryWatches`**: A vector storing `DirectoryWatch` structs, each containing a watch descriptor and its corresponding directory path. This allows the program to track all directories being monitored.
3. **`char eventBuffer[BUFFER_LENGTH]`**: A buffer used to store raw data from `inotify` events. The buffer size is defined by `BUFFER_LENGTH` to handle multiple events at once.
4. **`bool stopFlag`**: A flag used to control the monitoring loop. When set to `true`, the monitoring stops.
5. **`std::string rootDirectory`**: Stores the path of the root directory being monitored, allowing the program to check if this specific directory is deleted.

## Function Call Flow

### 1. **Creating the Monitor Object**
```cpp
RecursiveDirectoryMonitor monitor("/home/ohad/test_dir");
```
- This creates a new object of the `RecursiveDirectoryMonitor` class for the directory `/home/ohad/test_dir`.
- **What happens**: The constructor `RecursiveDirectoryMonitor::RecursiveDirectoryMonitor` is called, which initializes the `inotify` instance, sets the root directory path, and adds watches recursively for the specified root directory and its subdirectories.

### 2. **Constructor: `RecursiveDirectoryMonitor::RecursiveDirectoryMonitor(const std::string &rootDirectoryPath)`**
1. **First action**: 
   - Initializes `inotify` using `inotify_init` and stores the file descriptor in `inotifyFileDescriptor`.
2. **Stores root directory**:
   - Saves the root directory path into the `rootDirectory` variable for future reference.
3. **Calls**: `addWatchRecursively(rootDirectoryPath)` on the root directory, e.g., `/home/ohad/test_dir`.

### 3. **Function: `void addWatchRecursively(const std::string &directoryPath)`**
1. **Calls**: `addWatchToDirectory(directoryPath)`
   - Adds a watch on the specific directory to monitor changes in it.
2. **Opens the directory**: 
   - Opens the directory at `directoryPath` and starts iterating through its contents.
3. **Iterates over subdirectories**: 
   - For each subdirectory, it recursively calls `addWatchRecursively`, ensuring all levels are covered.

### 4. **Function: `void addWatchToDirectory(const std::string &directoryPath)`**
1. **Adds the watch**:
   - Calls `inotify_add_watch` with the `directoryPath`, returning a watch descriptor (`watchDescriptor`).
2. **Stores the information**:
   - Adds the watch descriptor and directory path to the `directoryWatches` vector for tracking.

### 5. **Monitoring Process Begins**
1. **Monitoring Starts**:
   - The setup is complete, and now every directory and subdirectory is being watched for changes.

```cpp
monitor.monitorDirectories();
```
- This begins the monitoring process for directories and files.
- **What happens**: The function `monitorDirectories` is called, which runs a continuous loop that reads events from `inotify`.

### 6. **Function: `void monitorDirectories()`**
1. **Monitoring Loop**:
   - Starts a loop that continues as long as `stopFlag` is `false`. 
   - It uses `read` to fetch events from `inotify`, storing them in `eventBuffer`.
2. **Calls**: `processEventBuffer(bufferLength)` 
   - Sends the buffer length to process the events stored in `eventBuffer`.

### 7. **Function: `void processEventBuffer(int bufferLength)`**
1. **Iterates through events**:
   - Processes each event in `eventBuffer`.
2. **Checks for root directory deletion**:
   - For each event, it checks if the event corresponds to the deletion of the `rootDirectory`. If so, it calls `rootDirectoryDeleted` and stops further processing.
3. **Calls**: `handleEvent(event, directoryPath)`
   - Handles each event by passing the event pointer and directory path.

### 8. **Function: `void handleEvent(struct inotify_event *event, const std::string &directoryPath)`**
1. **Checks event type**:
   - Identifies whether the event is creation, deletion, or modification and prints an appropriate message. 
   - If a directory is created, it recursively calls `addWatchRecursively` to monitor the new directory and its subdirectories.

### 9. **Function: `void rootDirectoryDeleted()`**
1. **Handles root directory deletion**:
   - Prints a message indicating that the root directory has been deleted.
   - Calls `stopMonitoring()` to stop the monitoring process.

### 10. **End of the Program**
When `stopMonitoring` is called, the value of `stopFlag` becomes `true`, causing the loop in `monitorDirectories` to exit. 
When the `monitor` object goes out of scope (or at the end of the program):
  
- **Destructor `~RecursiveDirectoryMonitor()`**:
  - Calls `cleanupWatches` to remove all watches from `inotify`.
  - Closes the `inotifyFileDescriptor`, releasing all resources.

This setup allows for continuous and recursive monitoring of changes in directories and subdirectories in real-time. If the root directory is deleted, the program will print a message and gracefully terminate monitoring.
