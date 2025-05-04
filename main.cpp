#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

struct PCI_DEVICE_data {
    std::string pci_address;
    std::string vendor_id;
    std::string device_id;
    std::string class_code;
    std::string subsystem_vendor_id;
    std::string vendor_name;
    std::string device_name;
};

std::string file_reader(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::string contents;
    std::getline(file, contents);
    return contents;
}

std::unordered_map<std::string, std::string> load_pci_ids(const std::string& path = "/usr/share/misc/pci.ids") {
    std::unordered_map<std::string, std::string> pci_db;
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << path << "\n";
        return pci_db;
    }

    std::string line;
    std::string current_vendor;

    while (std::getline(file, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Строка с Vendor (например, "8086  Intel Corporation")
        if (line.size() >= 6 && line[4] == ' ') {
            current_vendor = line.substr(0, 4);  // "8086"
            pci_db[current_vendor] = line.substr(6);  // "Intel Corporation"
        }
        // Строка с Device (например, "\t1234  Some Device")
        else if (line.size() >= 8 && line[0] == '\t' && line[1] != '\t') {
            std::string device_id = line.substr(1, 4);  // "1234"
            std::string key = current_vendor + ":" + device_id;  // "8086:1234"
            pci_db[key] = line.substr(7);  // "Some Device"
        }
    }

    return pci_db;
}

std::vector<PCI_DEVICE_data> scan_pci_data() {
    auto pci_db = load_pci_ids();
    std::vector<PCI_DEVICE_data> devices;
    const std::string pci_path = "/sys/bus/pci/devices/";

    for (const auto& entry : std::filesystem::directory_iterator(pci_path)) {
        PCI_DEVICE_data dev;
        dev.pci_address = entry.path().filename().string();

        dev.vendor_id = file_reader(entry.path().string() + "/vendor");
        dev.device_id = file_reader(entry.path().string() + "/device");
        dev.class_code = file_reader(entry.path().string()+ "/class");

        auto trim_hex = [](std::string& s) {
            if (s.size() > 2 && s.substr(0, 2) == "0x") {
                s = s.substr(2);
            }
            s.erase(s.find_last_not_of(" \n\r\t") + 1);
        };
        trim_hex(dev.vendor_id);
        trim_hex(dev.device_id);
        trim_hex(dev.class_code);

        // Получаем названия из pci_db
        if (pci_db.count(dev.vendor_id)) {
            dev.vendor_name = pci_db[dev.vendor_id];
        }

        std::string device_key = dev.vendor_id + ":" + dev.device_id;
        if (pci_db.count(device_key)) {
            dev.device_name = pci_db[device_key];
        }

        devices.push_back(dev);
    }
    return devices;
}

// void print_pci_devices(const std::vector<PCI_DEVICE_data>& devices) {
//     for (const auto& dev : devices) {
//         std::cout << dev.pci_address << " : ";
//         std::cout << "Vendor=" << dev.vendor_id << ", ";
//         std::cout << "Device=" << dev.device_id << ", ";
//         std::cout << "Class=" << dev.class_code << "\n";
//     }
// }                    simple output

void print_pci_devices(const std::vector<PCI_DEVICE_data>& devices) {
    for (const auto& dev : devices) {
        std::cout << dev.pci_address << " ";

        if (!dev.vendor_name.empty()) {
            std::cout << dev.vendor_name << " ";
        } else {
            std::cout << "[Vendor: " << dev.vendor_id << "] ";
        }

        if (!dev.device_name.empty()) {
            std::cout << dev.device_name;
        } else {
            std::cout << "[Device: " << dev.device_id << "]";
        }

        std::cout << " (Class: " << dev.class_code << ")\n";
    }
}

int main() {

    auto devices = scan_pci_data();
    print_pci_devices(devices);
    return 0;
}