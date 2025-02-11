#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h> // Windows Sockets API
#include <ws2tcpip.h> 
#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib for Winsock functions

bool canDirectConnect(const std::string& domain, const std::string& port = "80") {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }

    struct addrinfo* result_list = NULL;
    struct addrinfo hints = {};
    hints.ai_family = AF_INET; // IPv4 addresses only
    hints.ai_socktype = SOCK_STREAM;

    result = getaddrinfo(domain.c_str(), port.c_str(), &hints, &result_list);
    if (result != 0) {
        std::cerr << "getaddrinfo failed: " << gai_strerror(result) << std::endl;
        WSACleanup();
        return false;
    }

    SOCKET connectSocket = INVALID_SOCKET;
    for (struct addrinfo* ptr = result_list; ptr != NULL; ptr = ptr->ai_next) {
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) {
            std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
            continue;
        }

        result = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result_list);

    if (connectSocket == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    closesocket(connectSocket);
    WSACleanup();
    return true;
}

void resolveDomainToIP(const std::string& domain, std::vector<std::string>& output) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return;
    }

    struct addrinfo* result_list = NULL;
    struct addrinfo hints = {};
    hints.ai_family = AF_INET; // IPv4 addresses only
    hints.ai_socktype = SOCK_STREAM;

    result = getaddrinfo(domain.c_str(), NULL, &hints, &result_list);
    if (result != 0) {
        std::cerr << "getaddrinfo failed: " << gai_strerror(result) << std::endl;
        WSACleanup();
        return;
    }

    for (struct addrinfo* ptr = result_list; ptr != NULL; ptr = ptr->ai_next) {
        struct sockaddr_in* ipAddr = reinterpret_cast<struct sockaddr_in*>(ptr->ai_addr);
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(ptr->ai_family, &(ipAddr->sin_addr), ipStr, INET_ADDRSTRLEN);
        std::string directconnect = "";
        // 尝试直接连接
        if (canDirectConnect(domain)) {
            directconnect = "#网址" + domain + "可以直接连接。" + "\n";
        }
        else {
            directconnect = "#网址" + domain + "无法直接连接。" + "\n";
        }
        output.push_back(std::string(ipStr) + " " + domain + "\t" + directconnect);
        
    }

    freeaddrinfo(result_list);
    WSACleanup();
}

std::string extractDomain(const std::string& input) {
    std::string protocol = "https://";
    size_t start = 0;

    if (input.rfind(protocol, 0) == 0) { // 如果字符串以"https://"开头
        start = protocol.length();
    }
    else {
        protocol = "http://";
        if (input.rfind(protocol, 0) == 0) { // 如果字符串以"http://"开头
            start = protocol.length();
        }
    }

    // 找到第一个斜杠或问号之前的部分
    size_t end = input.find_first_of("/?", start);
    if (end == std::string::npos) {
        end = input.length();
    }

    return input.substr(start, end - start);
}

int main() {
    std::vector<std::string> allOutputs;
    while (true) {
        std::cout << "请输入网址: ";
        std::string input;
        std::getline(std::cin, input); // 读取用户输入

        if (input == "exit") { // 增加退出条件
            break;
        }

        std::string domain = extractDomain(input); // 处理输入以获取域名
        resolveDomainToIP(domain, allOutputs); // 使用处理后的域名作为参数调用函数

        // 输出累计结果
        for (const auto& out : allOutputs) {
            std::cout << out << std::endl;
    
        }

        
    }
    return 0;
}