
#include "diskstat.hpp"
#include <map>
#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <istream>


std::map<std::string, DiskIoSnapshot> read_disk_io_snapshots(std::istream& diskstats_stream) {
    std::map<std::string, DiskIoSnapshot> snapshots;

    diskstats_stream.clear();
    diskstats_stream.seekg(0, std::ios::beg);

    std::cerr << "[DEBUG] read_disk_io_snapshots: Reading /proc/diskstats..." << std::endl;

    std::string line;
    int line_count = 0;
    while (std::getline(diskstats_stream, line)) {
        line_count++;
        std::istringstream iss(line);
        int major, minor;
        std::string dev_name;
        uint64_t reads_completed, reads_merged, sectors_read, time_reading;
        uint64_t writes_completed, writes_merged, sectors_written, time_writing;

        iss >> major >> minor >> dev_name >> reads_completed >> reads_merged >>
            sectors_read >> time_reading >> writes_completed >> writes_merged >>
            sectors_written >> time_writing;

        // --- DEBUG INFO ---
        std::cerr << "[DEBUG] Line " << line_count << ": "
                  << "dev=" << dev_name << ", "
                  << "major=" << major << ", "
                  << "minor=" << minor;

        // The filter you are testing
        if (major > 0) {
            std::cerr << " -> ADDING" << std::endl;
            snapshots[dev_name] = {
                .bytes_read = sectors_read * 512,
                .bytes_written = sectors_written * 512
            };
        } else {
            std::cerr << " -> SKIPPING (major == 0)" << std::endl;
        }
        // --- END DEBUG ---
    }

    std::cerr << "[DEBUG] read_disk_io_snapshots: Finished. Collected "
              << snapshots.size() << " devices." << std::endl;

    return snapshots;
}
