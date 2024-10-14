
#include "RecursiveDirectoryMonitor.hpp"

int main() {
    RecursiveDirectoryMonitor monitor("/home/ohad/test_dir");
    monitor.monitorDirectories();
    return 0;
}
