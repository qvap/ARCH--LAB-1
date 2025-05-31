#include <wx/wx.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <iphlpapi.h>
#else
    #include <ifaddrs.h>
    #include <net/if_dl.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif

using namespace std;

// Преобразуем IP строку в uint32_t
uint32_t ipToUInt(const string& ipStr) {
    uint32_t ip = 0;
    int a, b, c, d;
    if (sscanf(ipStr.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        if (a >= 0 && a <= 255 && b >= 0 && b <= 255 && 
            c >= 0 && c <= 255 && d >= 0 && d <= 255) {
            ip = (static_cast<uint32_t>(a) << 24) | 
                 (static_cast<uint32_t>(b) << 16) | 
                 (static_cast<uint32_t>(c) << 8) | 
                 static_cast<uint32_t>(d);
        }
    }
    return ip;
}

// Обратно из uint32_t в строку
string uintToIP(uint32_t ip) {
    ostringstream oss;
    oss << ((ip >> 24) & 0xFF) << "."
        << ((ip >> 16) & 0xFF) << "."
        << ((ip >> 8) & 0xFF) << "."
        << (ip & 0xFF);
    return oss.str();
}

// Маска сети по IP-диапазону
uint32_t calculateNetmask(uint32_t ip1, uint32_t ip2) {
    uint32_t diff = ip1 ^ ip2;
    uint32_t mask = 0xFFFFFFFF;
    while (diff > 0) {
        mask <<= 1;
        diff >>= 1;
    }
    return mask;
}

// MAC-адрес на Windows/macOS
string getMacAddress() {
#ifdef _WIN32
    string mac = "Not found";
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    
    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        return mac;
    }
    
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            return mac;
        }
    }
    
    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            // Убираем условие проверки IP и ищем любой физический адаптер
            if (pAdapter->AddressLength == 6 && 
                (pAdapter->Type == MIB_IF_TYPE_ETHERNET || 
                 pAdapter->Type == IF_TYPE_IEEE80211)) {  // Добавляем Wi-Fi
                // Проверяем, что MAC не нулевой
                bool hasValidMac = false;
                for (UINT i = 0; i < pAdapter->AddressLength; i++) {
                    if (pAdapter->Address[i] != 0) {
                        hasValidMac = true;
                        break;
                    }
                }
                
                if (hasValidMac) {
                    ostringstream mac_stream;
                    for (UINT i = 0; i < pAdapter->AddressLength; i++) {
                        mac_stream << hex << setfill('0') << setw(2)
                                   << static_cast<int>(pAdapter->Address[i]);
                        if (i != pAdapter->AddressLength - 1) 
                            mac_stream << ":";
                    }
                    mac = mac_stream.str();
                    // Преобразуем в верхний регистр для читаемости
                    transform(mac.begin(), mac.end(), mac.begin(), ::toupper);
                    break;
                }
            }
            pAdapter = pAdapter->Next;
        }
    }
    
    if (pAdapterInfo) {
        free(pAdapterInfo);
    }
    
    return mac;
#else
    struct ifaddrs *ifap, *ifa;
    string mac = "Not found";

    if (getifaddrs(&ifap) != 0) {
        return mac;
    }

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
            struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;
            if (sdl->sdl_alen == 6) {
                unsigned char* macPtr = (unsigned char*)LLADDR(sdl);
                
                // Проверяем, что MAC не нулевой
                bool hasValidMac = false;
                for (int i = 0; i < 6; i++) {
                    if (macPtr[i] != 0) {
                        hasValidMac = true;
                        break;
                    }
                }
                
                if (hasValidMac) {
                    ostringstream mac_stream;
                    for (int i = 0; i < 6; i++) {
                        mac_stream << hex << setfill('0') << setw(2)
                                   << static_cast<int>(macPtr[i]);
                        if (i != 5) mac_stream << ":";
                    }
                    mac = mac_stream.str();
                    // Преобразуем в верхний регистр
                    transform(mac.begin(), mac.end(), mac.begin(), ::toupper);
                    break;
                }
            }
        }
    }

    freeifaddrs(ifap);
    return mac;
#endif
}

// Класс главного окна
class MainFrame : public wxFrame {
public:
    MainFrame() : wxFrame(nullptr, wxID_ANY, "IP-Range Analyzer", 
                         wxDefaultPosition, wxSize(500, 400)) {
        CreateControls();
        BindEvents();
        
#ifdef _WIN32
        // Инициализация Winsock для Windows
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }
    
    ~MainFrame() {
#ifdef _WIN32
        WSACleanup();
#endif
    }

private:
    // Элементы интерфейса
    wxTextCtrl* m_ip1Text;
    wxTextCtrl* m_ip2Text;
    wxButton* m_analyzeButton;
    wxTextCtrl* m_resultText;
    
    void CreateControls() {
        // Создаем главную панель
        wxPanel* panel = new wxPanel(this);
        
        // Создаем элементы
        wxStaticText* label1 = new wxStaticText(panel, wxID_ANY, "Start IP:");
        m_ip1Text = new wxTextCtrl(panel, wxID_ANY, "192.168.1.1", 
                                   wxDefaultPosition, wxSize(150, -1));
        
        wxStaticText* label2 = new wxStaticText(panel, wxID_ANY, "End IP:");
        m_ip2Text = new wxTextCtrl(panel, wxID_ANY, "192.168.1.100", 
                                   wxDefaultPosition, wxSize(150, -1));
        
        m_analyzeButton = new wxButton(panel, wxID_ANY, "Analyze");
        
        wxStaticText* label3 = new wxStaticText(panel, wxID_ANY, "Results:");
        m_resultText = new wxTextCtrl(panel, wxID_ANY, "", 
                                      wxDefaultPosition, wxSize(-1, 200),
                                      wxTE_MULTILINE | wxTE_READONLY);
        
        // Настраиваем макет
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        
        // Горизонтальный сайзер для IP полей
        wxBoxSizer* ipSizer = new wxBoxSizer(wxHORIZONTAL);
        ipSizer->Add(label1, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        ipSizer->Add(m_ip1Text, 0, wxRIGHT, 20);
        ipSizer->Add(label2, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        ipSizer->Add(m_ip2Text, 0, wxRIGHT, 10);
        
        mainSizer->Add(ipSizer, 0, wxALL, 10);
        mainSizer->Add(m_analyzeButton, 0, wxALL | wxCENTER, 10);
        mainSizer->Add(label3, 0, wxLEFT | wxRIGHT | wxTOP, 10);
        mainSizer->Add(m_resultText, 1, wxEXPAND | wxALL, 10);
        
        panel->SetSizer(mainSizer);
        
        // Центрируем окно
        Center();
    }
    
    void BindEvents() {
        Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnAnalyze, this, m_analyzeButton->GetId());
    }
      void OnAnalyze(wxCommandEvent&) {
        wxString ip1Str = m_ip1Text->GetValue().Trim().Trim(false);
        wxString ip2Str = m_ip2Text->GetValue().Trim().Trim(false);
        
        // Проверяем, что поля не пустые
        if (ip1Str.IsEmpty() || ip2Str.IsEmpty()) {
            wxMessageBox("Please enter two IPs!", "Error", 
                        wxOK | wxICON_ERROR);
            return;
        }
        
        // Конвертируем в std::string
        string ip1StdStr = string(ip1Str.mb_str());
        string ip2StdStr = string(ip2Str.mb_str());
        
        uint32_t ip1 = ipToUInt(ip1StdStr);
        uint32_t ip2 = ipToUInt(ip2StdStr);
        
        // Проверяем валидность IP-адресов
        if (ip1 == 0 || ip2 == 0) {
            wxMessageBox("Wrong IP format! Use xxx.xxx.xxx.xxx", 
                        "Error", wxOK | wxICON_ERROR);
            return;
        }
        
        if (ip1 > ip2) swap(ip1, ip2);
        
        uint32_t netmask = calculateNetmask(ip1, ip2);
        uint32_t network = ip1 & netmask;
        uint32_t broadcast = network | ~netmask;
        
        // Формируем результат
        wxString result;
        result += "Analyzer result:\n\n";
        result += wxString::Format("Start IP-address: %s\n", uintToIP(ip1));
        result += wxString::Format("End IP-address:  %s\n", uintToIP(ip2));
        result += wxString::Format("Network:                %s\n", uintToIP(network));
        result += wxString::Format("Broadcast-address:           %s\n", uintToIP(broadcast));
        result += wxString::Format("Mask:                %s\n", uintToIP(netmask));
        result += wxString::Format("MAC-address:      %s\n", getMacAddress());
        
        m_resultText->SetValue(result);
    }
};

// Класс приложения
class IPAnalyzerApp : public wxApp {
public:
    bool OnInit() override {
        MainFrame* frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};

// Точка входа
wxIMPLEMENT_APP(IPAnalyzerApp);
