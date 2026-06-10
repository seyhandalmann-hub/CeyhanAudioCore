#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

// =================================================================
// PROJECT: CeyhanAudioCore (CAC) - Continuous Daemon Version
// FOUNDER: Eymen Ceyhan (2026)
// =================================================================

struct ID3v1Tag {
    std::string title;
    std::string artist;
    std::string album;
    std::string year;
    bool found = false;
};

class CeyhanAudioCore {
public:
    std::string version = "1.5.0-Production";

    void showBanner() {
        std::cout << "\033[1;32m";
        std::cout << "=========================================" << std::endl;
        std::cout << "   CEYHAN AUDIO CORE (CAC) v" << version << std::endl;
        std::cout << "   Continuous Audio Scanner Online        " << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << "\033[0m";
    }

    ID3v1Tag parseID3v1(const std::vector<char>& audioData) {
        ID3v1Tag tag;
        if (audioData.size() < 128) return tag;

        size_t offset = audioData.size() - 128;
        if (audioData[offset] != 'T' || audioData[offset+1] != 'A' || audioData[offset+2] != 'G')
            return tag;

        auto readField = [&](size_t start, size_t len) -> std::string {
            std::string s(audioData.begin() + start, audioData.begin() + start + len);
            s.erase(std::find(s.begin(), s.end(), '\0'), s.end());
            while (!s.empty() && s.back() == ' ') s.pop_back();
            return s;
        };

        tag.found  = true;
        tag.title  = readField(offset + 3,  30);
        tag.artist = readField(offset + 33, 30);
        tag.album  = readField(offset + 63, 30);
        tag.year   = readField(offset + 93, 4);
        return tag;
    }

    std::string generateAudioFingerprint(const std::vector<char>& audioData) {
        unsigned int hash = 5381;
        for (char byte : audioData) {
            hash = ((hash << 5) + hash) + byte;
        }
        std::stringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << hash;
        return ss.str();
    }

    bool loadAudioFile(const std::string& filePath) {
        std::string cleanPath = filePath;
        if (!cleanPath.empty() && (cleanPath.front() == '"' || cleanPath.front() == '\'')) {
            cleanPath.erase(0, 1);
            cleanPath.pop_back();
        }

        std::ifstream file(cleanPath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "\033[1;31m[Error] Audio file could not be opened: " << cleanPath << "\033[0m" << std::endl;
            return false;
        }

        std::vector<char> audioData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        std::cout << "\033[1;32m[Success] Audio successfully loaded!\033[0m" << std::endl;
        std::cout << "[Info] Total Audio Size: " << audioData.size() << " bytes." << std::endl;

        ID3v1Tag tag = parseID3v1(audioData);
        if (tag.found) {
            std::cout << "\033[1;34m--- ID3v1 METADATA ---\033[0m" << std::endl;
            std::cout << "[Title]  " << (tag.title.empty()  ? "N/A" : tag.title)  << std::endl;
            std::cout << "[Artist] " << (tag.artist.empty() ? "N/A" : tag.artist) << std::endl;
            std::cout << "[Album]  " << (tag.album.empty()  ? "N/A" : tag.album)  << std::endl;
            std::cout << "[Year]   " << (tag.year.empty()   ? "N/A" : tag.year)   << std::endl;
        } else {
            std::cout << "[Metadata] No ID3v1 tag found in this file." << std::endl;
        }

        std::cout << "\033[1;36m--- COPYRIGHT FINGERPRINTING ---\033[0m" << std::endl;
        std::string uniqueCode = generateAudioFingerprint(audioData);

        std::cout << "[CAC-ID] Unique Song Code: \033[1;33mCAC-" << uniqueCode << "\033[0m" << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
        return true;
    }
};

int main() {
    CeyhanAudioCore cac;
    cac.showBanner();

    std::string audioPath;

    // SONSUZ DÖNGÜ: Kullanıcı 'q' yazana kadar sürekli şarkı bekler
    while (true) {
        std::cout << "\n\033[1;35m[Type 'q' to quit] or DRAG and DROP an audio file: \033[0m";
        std::getline(std::cin, audioPath);

        // Eğer kullanıcı sadece 'q' veya 'Q' yazdıysa döngüden çık ve kapat
        if (audioPath == "q" || audioPath == "Q" || audioPath == "'q'" || audioPath == "\"q\"") {
            std::cout << "\n[System] Terminating CeyhanAudioCore safely. Core offline.\n";
            break;
        }

        // Eğer boş bırakıp enter'a bastıysa döngüyü başa sar
        if (audioPath.empty()) {
            continue;
        }

        // Şarkıyı yükle ve kodunu üret
        cac.loadAudioFile(audioPath);
    }

    return 0;
}