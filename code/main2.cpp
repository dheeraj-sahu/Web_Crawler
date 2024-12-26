#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
void push(std::vector<std::vector<int>>* v, int range_start, int range_end, int line_number) {
    std::vector<int> temp = {range_start, range_end, line_number};
    v->push_back(temp);
}

void lpsInit(int lps[], const std::string& key) {
    int len = 0;
    lps[0] = 0;
    int i = 1;

    while (i < key.size()) {
        if (key[i] == key[len]) {
            len++;
            lps[i] = len;
            i++;
        } else if (len > 0) {
            len = lps[len - 1];
        } else {
            lps[i] = 0;
            i++;
        }
    }
}

void kmp(const std::string& input, const std::string& key, std::vector<std::vector<int>>* result, int line_number) {
    int lps[key.size()];
    lpsInit(lps, key);
    int i = 0;
    int j = 0;

    while (i < input.size()) {
        if (input[i] == key[j]) {
            i++;
            j++;
            if (j == key.size()) {
                push(result, i - key.size(), i, line_number);
                j = lps[j - 1];
            }
        } else {
            if (j > 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }
}

void parseIndexFile(const std::string& indexFilename, std::map<int, std::pair<int, std::string>>& lineToFileMap) {
    std::ifstream indexFile(indexFilename);
    if (!indexFile.is_open()) {
        std::cerr << "Error: Unable to open index file " << indexFilename << std::endl;
        return;
    }

    std::string line, filename;
    int startLine = 0, endLine = 0;
    while (std::getline(indexFile, line)) {
        if (line.find("HTML File:") != std::string::npos) {
            size_t pos = line.find("HTML File: ");
            filename = line.substr(pos + 11);
        } else if (line.find("Data Start Line:") != std::string::npos) {
            size_t pos = line.find("Data Start Line: ");
            startLine = std::stoi(line.substr(pos + 17));
        } else if (line.find("Data End Line:") != std::string::npos) {
            size_t pos = line.find("Data End Line: ");
            endLine = std::stoi(line.substr(pos + 15));
            lineToFileMap[startLine] = {endLine, filename};
        }
    }

    indexFile.close();
}

std::string getFileFromLine(const std::map<int, std::pair<int, std::string>>& lineToFileMap, int lineNumber) {
    for (const auto& entry : lineToFileMap) {
        if (lineNumber >= entry.first && lineNumber <= entry.second.first) {
            return entry.second.second;
        }
    }
    return "Unknown File";
}

void fileHandler(const std::string& filename, const std::string& key, std::vector<std::vector<int>>& result) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    std::string lineContent;
    int line = 1; // Line numbers are typically 1-based

    while (std::getline(file, lineContent)) {
        kmp(lineContent, key, &result, line);
        line++;
    }

    file.close();
}