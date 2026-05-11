#include <stdio.h>
#include <dlfcn.h>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <stddef.h>

std::unordered_map<std::string, std::string>& get_dictionary() {
    static std::unordered_map<std::string, std::string> dict;
    return dict;
}

std::string get_module_dir() {
    Dl_info info;
    if (dladdr((void*)get_dictionary, &info) && info.dli_fname) {
        std::string path = info.dli_fname;
        size_t pos = path.find_last_of('/');
        if (pos != std::string::npos) {
            return path.substr(0, pos);
        }
    }
    return ".";
}

void load_dictionary() {
    std::string dict_path = get_module_dir() + "/language.txt";
    std::ifstream file(dict_path);
    if (!file.is_open()) {
        fprintf(stderr, "[IDA Hook] Failed to open %s\n", dict_path.c_str());
        return;
    }

    auto find_quote = [](const std::string& s, size_t pos) {
        while (true) {
            pos = s.find('"', pos);
            if (pos == std::string::npos) return pos;
            size_t bs = 0;
            for (int i = pos - 1; i >= 0 && s[i] == '\\'; --i) bs++;
            if (bs % 2 == 0) return pos;
            pos++;
        }
    };

    auto unescape = [](std::string s) {
        std::string res;
        for (size_t i = 0; i < s.length(); ++i) {
            if (s[i] == '\\' && i + 1 < s.length()) {
                if (s[i+1] == '"') { res += '"'; i++; }
                else if (s[i+1] == 'n') { res += '\n'; i++; }
                else if (s[i+1] == 't') { res += '\t'; i++; }
                else if (s[i+1] == '\\') { res += '\\'; i++; }
                else { res += s[i]; }
            } else {
                res += s[i];
            }
        }
        return res;
    };

    std::string line;
    int count = 0;
    auto& dict = get_dictionary();
    while (std::getline(file, line)) {
        size_t first_quote = line.find("L\"");
        if (first_quote == std::string::npos) continue;
        size_t first_end = find_quote(line, first_quote + 2);
        if (first_end == std::string::npos) continue;

        size_t second_quote = line.find("L\"", first_end + 1);
        if (second_quote == std::string::npos) continue;
        size_t second_end = find_quote(line, second_quote + 2);
        if (second_end == std::string::npos) continue;

        std::string eng = line.substr(first_quote + 2, first_end - first_quote - 2);
        std::string chi = line.substr(second_quote + 2, second_end - second_quote - 2);

        dict[unescape(eng)] = unescape(chi);
        count++;
    }
    fprintf(stderr, "[IDA Hook] Loaded %d translations from %s\n", count, dict_path.c_str());
}

__attribute__((constructor))
void init_hook() {
    load_dictionary();
}

typedef void (*fromutf8_func_t)(void*, ptrdiff_t, const char*);
static fromutf8_func_t orig_fromutf8 = nullptr;

extern "C" void _ZN2QT7QString8fromUtf8ENS_14QByteArrayViewE(void* ret, ptrdiff_t size, const char* data) {
    if (!orig_fromutf8) {
        orig_fromutf8 = (fromutf8_func_t)dlsym(RTLD_NEXT, "_ZN2QT7QString8fromUtf8ENS_14QByteArrayViewE");
    }

    if (data && size > 0) {
        std::string s(data, size);
        auto& dict = get_dictionary();
        auto it = dict.find(s);
        if (it != dict.end()) {
            if (orig_fromutf8) {
                orig_fromutf8(ret, it->second.size(), it->second.data());
                return;
            }
        }
    }

    if (orig_fromutf8) {
        orig_fromutf8(ret, size, data);
    }
}

typedef void (*translate_func_t)(void*, const char*, const char*, const char*, int);
static translate_func_t orig_translate = nullptr;

extern "C" void _ZN2QT16QCoreApplication9translateEPKcS2_S2_i(void* ret, const char* context, const char* sourceText, const char* disambiguation, int n) {
    if (!orig_translate) {
        orig_translate = (translate_func_t)dlsym(RTLD_NEXT, "_ZN2QT16QCoreApplication9translateEPKcS2_S2_i");
    }

    if (sourceText) {
        auto& dict = get_dictionary();
        auto it = dict.find(sourceText);
        if (it != dict.end()) {
            if (orig_translate) {
                orig_translate(ret, context, it->second.c_str(), disambiguation, n);
                return;
            }
        }
    }

    if (orig_translate) {
        orig_translate(ret, context, sourceText, disambiguation, n);
    }
}
