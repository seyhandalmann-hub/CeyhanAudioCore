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

struct AudioTag {
    std::string title;
    std::string artist;
    std::string album;
    std::string year;
    std::string version;
    bool found = false;
};

class CeyhanAudioCore {
public:
    std::string version = "1.6.0-Production";

    void showBanner() {
        std::cout << "\033[1;32m";
        std::cout << "=========================================" << std::endl;
        std::cout << "   CEYHAN AUDIO CORE (CAC) v" << version << std::endl;
        std::cout << "   Continuous Audio Scanner Online        " << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << "\033[0m";
    }

    std::string cleanString(const std::vector<char>& data, size_t start, size_t len) {
        std::string s(data.begin() + start, data.begin() + start + len);
        s.erase(std::find(s.begin(), s.end(), '\0'), s.end());
        while (!s.empty() && (s.back() == ' ' || s.back() == '\r')) s.pop_back();
        return s;
    }

    AudioTag parseID3v2(const std::vector<char>& data) {
        AudioTag tag;
        if (data.size() < 10) return tag;
        if (data[0] != 'I' || data[1] != 'D' || data[2] != '3') return tag;

        uint8_t major = data[3];
        // syncsafe integer for total tag size
        uint32_t tagSize = ((uint8_t)data[6] << 21) | ((uint8_t)data[7] << 14) |
                           ((uint8_t)data[8] << 7)  |  (uint8_t)data[9];

        tag.version = "ID3v2." + std::to_string(major);
        size_t pos = 10;
        size_t end = std::min((size_t)(10 + tagSize), data.size());

        while (pos + 10 < end) {
            std::string frameID(data.begin() + pos, data.begin() + pos + 4);
            if (frameID[0] == '\0') break;

            uint32_t frameSize;
            if (major >= 4) {
                frameSize = ((uint8_t)data[pos+4] << 21) | ((uint8_t)data[pos+5] << 14) |
                            ((uint8_t)data[pos+6] << 7)  |  (uint8_t)data[pos+7];
            } else {
                frameSize = ((uint8_t)data[pos+4] << 24) | ((uint8_t)data[pos+5] << 16) |
                            ((uint8_t)data[pos+6] << 8)  |  (uint8_t)data[pos+7];
            }

            pos += 10;
            if (pos + frameSize > end || frameSize == 0) break;

            uint8_t encoding = (uint8_t)data[pos];
            std::string value;

            if (encoding == 0x01 || encoding == 0x02) {
                // UTF-16: convert to UTF-8
                size_t start = pos + 1;
                // skip BOM if present
                if (start + 1 < pos + frameSize &&
                    (uint8_t)data[start] == 0xFF && (uint8_t)data[start+1] == 0xFE)
                    start += 2;
                for (size_t i = start; i + 1 < pos + frameSize; i += 2) {
                    uint16_t cp = (uint8_t)data[i] | ((uint8_t)data[i+1] << 8);
                    if (cp == 0) break;
                    if (cp < 0x80) {
                        value += (char)cp;
                    } else if (cp < 0x800) {
                        value += (char)(0xC0 | (cp >> 6));
                        value += (char)(0x80 | (cp & 0x3F));
                    } else {
                        value += (char)(0xE0 | (cp >> 12));
                        value += (char)(0x80 | ((cp >> 6) & 0x3F));
                        value += (char)(0x80 | (cp & 0x3F));
                    }
                }
            } else {
                // Latin-1 or UTF-8
                value = std::string(data.begin() + pos + 1, data.begin() + pos + frameSize);
                value.erase(std::find(value.begin(), value.end(), '\0'), value.end());
            }
            while (!value.empty() && (value.back() == ' ' || value.back() == '\r')) value.pop_back();

            if      (frameID == "TIT2") tag.title  = value;
            else if (frameID == "TPE1") tag.artist = value;
            else if (frameID == "TALB") tag.album  = value;
            else if (frameID == "TDRC" || frameID == "TYER") tag.year = value;

            pos += frameSize;
        }

        tag.found = !tag.title.empty() || !tag.artist.empty();
        return tag;
    }

    AudioTag parseID3v1(const std::vector<char>& data) {
        AudioTag tag;
        if (data.size() < 128) return tag;

        size_t offset = data.size() - 128;
        if (data[offset] != 'T' || data[offset+1] != 'A' || data[offset+2] != 'G')
            return tag;

        tag.found   = true;
        tag.version = "ID3v1";
        tag.title   = cleanString(data, offset + 3,  30);
        tag.artist  = cleanString(data, offset + 33, 30);
        tag.album   = cleanString(data, offset + 63, 30);
        tag.year    = cleanString(data, offset + 93, 4);
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

        AudioTag tag = parseID3v2(audioData);
        if (!tag.found) tag = parseID3v1(audioData);

        if (tag.found) {
            std::cout << "\033[1;34m--- METADATA (" << tag.version << ") ---\033[0m" << std::endl;
            std::cout << "[Title]  " << (tag.title.empty()  ? "N/A" : tag.title)  << std::endl;
            std::cout << "[Artist] " << (tag.artist.empty() ? "N/A" : tag.artist) << std::endl;
            std::cout << "[Album]  " << (tag.album.empty()  ? "N/A" : tag.album)  << std::endl;
            std::cout << "[Year]   " << (tag.year.empty()   ? "N/A" : tag.year)   << std::endl;
        } else {
            std::cout << "[Metadata] No ID3 tag found in this file." << std::endl;
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