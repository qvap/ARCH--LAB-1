#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

// Преобразуем IP строку в uint32_t
uint32_t ipToUInt(const string& ipStr) {
    struct in_addr ip_addr{};
    inet_pton(AF_INET, ipStr.c_str(), &ip_addr);
    return ntohl(ip_addr.s_addr);
}

// Обратно из uint32_t в строку
string uintToIP(uint32_t ip) {
    struct in_addr ip_addr{};
    ip_addr.s_addr = htonl(ip);
    char buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip_addr, buffer, INET_ADDRSTRLEN);
    return string(buffer);
}

// Маска сети по IP-диапазону (по XOR двух IP и определению кол-ва битов)
uint32_t calculateNetmask(uint32_t ip1, uint32_t ip2) {
    uint32_t diff = ip1 ^ ip2;
    uint32_t mask = 0xFFFFFFFF;
    while (diff > 0) {
        mask <<= 1;
        diff >>= 1;
    }
    return mask;
}

// MAC-адрес на macOS
string getMacAddress() {
    struct ifaddrs *ifap, *ifa;
    string mac = "не найден";

    if (getifaddrs(&ifap) != 0) {
        perror("getifaddrs");
        return mac;
    }

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
            struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;
            if (sdl->sdl_alen == 6) {  // Проверка длины MAC-адреса
                unsigned char* macPtr = (unsigned char*)LLADDR(sdl);
                ostringstream mac_stream;
                for (int i = 0; i < 6; i++) {
                    mac_stream << hex << setfill('0') << setw(2)
                               << static_cast<int>(macPtr[i]);
                    if (i != 5) mac_stream << ":";
                }
                mac = mac_stream.str();
                break;
            }
        }
    }

    freeifaddrs(ifap);
    return mac;
}

int main() {
    cout << "Введите диапазон IP-адресов (например: 192.168.1.1 - 192.168.1.100): ";
    string input;
    getline(cin, input);

    size_t dashPos = input.find('-');
    if (dashPos == string::npos) {
        cerr << "Неверный формат! Используйте 'IP1 - IP2'" << endl;
        return 1;
    }

    string ip1Str = input.substr(0, dashPos);
    string ip2Str = input.substr(dashPos + 1);
    ip1Str.erase(remove(ip1Str.begin(), ip1Str.end(), ' '), ip1Str.end());
    ip2Str.erase(remove(ip2Str.begin(), ip2Str.end(), ' '), ip2Str.end());

    uint32_t ip1 = ipToUInt(ip1Str);
    uint32_t ip2 = ipToUInt(ip2Str);
    if (ip1 > ip2) swap(ip1, ip2);

    uint32_t netmask = calculateNetmask(ip1, ip2);
    uint32_t network = ip1 & netmask;
    uint32_t broadcast = network | ~netmask;

    cout << "\n Результаты анализа диапазона:\n";
    cout << "IP-адрес начала диапазона: " << uintToIP(ip1) << endl;
    cout << "IP-адрес конца диапазона : " << uintToIP(ip2) << endl;
    cout << "Адрес сети               : " << uintToIP(network) << endl;
    cout << "Broadcast-адрес          : " << uintToIP(broadcast) << endl;
    cout << "Маска сети              : " << uintToIP(netmask) << endl;
    cout << "MAC-адрес устройства     : " << getMacAddress() << endl;

    return 0;
}
