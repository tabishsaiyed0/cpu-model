// dm-cli: CPU-only Dungeon Master client for llama.cpp
// Talks to llama-server (CPU mode: -ngl 0) via HTTP POST /completion
// No external deps on Windows (uses WinINet). Build: cmake -B build && cmake --build build
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Minimal JSON string escaper for prompts
std::string jsonEscape(const std::string& s) {
    std::string o;
    for (char c : s) {
        switch (c) {
            case '"': o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default: o += c;
        }
    }
    return o;
}

// Very small extractor: finds "key": "value" or "key": number
std::string extractStr(const std::string& json, const std::string& key) {
    std::string pat = "\"" + key + "\"";
    auto p = json.find(pat);
    if (p == std::string::npos) return "";
    p = json.find('"', json.find(':', p) + 1);
    if (p == std::string::npos) return "";
    auto q = p + 1;
    std::string out;
    while (q < json.size() && json[q] != '"') {
        if (json[q] == '\\' && q + 1 < json.size()) { out += json[q+1]; q += 2; }
        else { out += json[q++]; }
    }
    return out;
}
int extractInt(const std::string& json, const std::string& key, int def = 0) {
    std::string pat = "\"" + key + "\"";
    auto p = json.find(pat);
    if (p == std::string::npos) return def;
    p = json.find(':', p) + 1;
    try { return std::stoi(json.substr(p)); } catch (...) { return def; }
}
// llama-server /completion returns {"content":"..."} — unwrap it
std::string unwrapContent(const std::string& resp) {
    std::string s = extractStr(resp, "content");
    if (!s.empty()) return s;
    return resp; // fallback: raw
}

std::string httpPost(const std::string& host, int port, const std::string& path, const std::string& body) {
    HINTERNET hNet = InternetOpenA("dm-cli/0.1", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hNet) return "";
    HINTERNET hConn = InternetConnectA(hNet, host.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConn) { InternetCloseHandle(hNet); return ""; }
    HINTERNET hReq = HttpOpenRequestA(hConn, "POST", path.c_str(), NULL, NULL, NULL,
        INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE, 0);
    std::string out;
    if (hReq) {
        const char* hdrs = "Content-Type: application/json";
        if (HttpSendRequestA(hReq, hdrs, (DWORD)strlen(hdrs), (LPVOID)body.c_str(), (DWORD)body.size())) {
            char buf[4096]; DWORD read = 0;
            while (InternetReadFile(hReq, buf, sizeof(buf), &read) && read > 0)
                out.append(buf, read);
        }
        InternetCloseHandle(hReq);
    }
    InternetCloseHandle(hConn);
    InternetCloseHandle(hNet);
    return out;
}

int main(int argc, char** argv) {
    std::string host = "127.0.0.1";
    int port = 8080;
    if (argc > 1) host = argv[1];
    if (argc > 2) port = std::stoi(argv[2]);

    std::string system = readFile("prompts/system.txt");
    if (system.empty()) system = "You are a Dungeon Master. Reply with JSON: narration, location, hp_change, loot.";

    int hp = 30;
    std::string location = "Tavern";
    std::string history;

    std::cout << "=== DM-CLI (CPU-only, llama.cpp) ===\n";
    std::cout << "Talking to http://" << host << ":" << port << " | HP=" << hp << "\n";
    std::cout << "Type 'quit' to exit.\n\n";

    std::string player = "I walk into the tavern and look around.";
    while (true) {
        std::string prompt = system +
            "\n\nState: HP=" + std::to_string(hp) + ", location=" + location +
            ". History: " + history.substr(0, 2000) +
            "\nPlayer: " + player +
            "\nDM JSON:";

        std::string req = "{\"prompt\":\"" + jsonEscape(prompt) +
            "\",\"n_predict\":256,\"temperature\":0.8,\"cache_prompt\":true,\"stop\":[\"Player:\"]}";
        std::string resp = httpPost(host, port, "/completion", req);
        if (resp.empty()) {
            std::cerr << "[error] No response from llama-server. Is it running? See scripts/run_server.ps1\n";
            return 1;
        }
        std::string content = unwrapContent(resp);
        std::string narration = extractStr(content, "narration");
        if (narration.empty()) narration = content; // model ignored grammar, show raw
        std::string newLoc = extractStr(content, "location");
        int hpDelta = extractInt(content, "hp_change", 0);
        std::string loot = extractStr(content, "loot");

        hp += hpDelta;
        if (hp < 1) hp = 1;
        if (!newLoc.empty()) location = newLoc;
        history += " P:" + player + " D:" + narration;

        std::cout << "\n--- [" << location << " | HP " << hp;
        if (hpDelta != 0) std::cout << " (" << (hpDelta > 0 ? "+" : "") << hpDelta << ")";
        std::cout << "] ---\n" << narration << "\n";
        if (!loot.empty() && loot != "none") std::cout << "* Loot: " << loot << "\n";
        if (hp <= 1) std::cout << "* You are barely standing!\n";

        std::cout << "\n> ";
        std::getline(std::cin, player);
        if (player == "quit" || player == "q" || !std::cin) break;
        if (player.empty()) player = "I look around and continue.";
    }
    std::cout << "Adventure saved. Farewell!\n";
    return 0;
}
