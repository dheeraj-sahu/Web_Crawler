#include <bits/stdc++.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <libxml/uri.h>
#include <openssl/sha.h>
#include <libxml/uri.h>
#include "main2.cpp"
#include "main3.cpp"
enum ftype
{
    HTML,
    CSS,
    JS
};

int getLastEndLine(const std::string &indexFilename)
{
    std::ifstream indexFile(indexFilename);
    if (!indexFile.is_open())
        return 0;

    std::string line;
    std::regex endLineRegex("Data End Line: (\\d+)");
    int lastEndLine = 0;

    while (std::getline(indexFile, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, endLineRegex))
        {
            lastEndLine = std::stoi(match[1]); // Update the last end line number
        }
    }

    indexFile.close();
    return lastEndLine;
}

void processHTMLToFiles(const std::string &htmlFilename)
{
    std::ifstream htmlFile(htmlFilename);
    if (!htmlFile.is_open())
    {
        std::cerr << "Error: Unable to open the HTML file: " << htmlFilename << "\n";
        return;
    }

    std::ofstream dataFile("data.txt", std::ios::app);  
    std::ofstream indexFile("index.txt", std::ios::app);

    if (!dataFile.is_open() || !indexFile.is_open())
    {
        std::cerr << "Error: Unable to open output files.\n";
        return;
    }

    int lastEndLine = getLastEndLine("index.txt");
    int startLine = lastEndLine + 1; // New start line

    std::string line;
    std::regex textRegex(">([^<]+)<"); // Regex to extract text between HTML tags
    int lineCount = startLine - 1;     // Initialize to last end line

    while (std::getline(htmlFile, line))
    {
        line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), ""); // Trim whitespace

        // Extract and write text to data.txt
        auto textBegin = std::sregex_iterator(line.begin(), line.end(), textRegex);
        auto textEnd = std::sregex_iterator();
        for (auto i = textBegin; i != textEnd; ++i)
        {
            std::smatch match = *i;
            std::string extractedText = match[1].str();

            // Convert the text to lowercase
            std::transform(extractedText.begin(), extractedText.end(), extractedText.begin(), [](unsigned char c)
                           { return std::tolower(c); });

            dataFile << extractedText << "\n"; // Write the extracted lowercase text to data.txt
            lineCount++;                       // Increment line count for each line written
        }
    }

    int endLine = lineCount; // Mark the ending line after writing new data

    // Write metadata to index.txt
    indexFile << "HTML File: " << htmlFilename << "\n";
    indexFile << "Data Start Line: " << startLine << "\n";
    indexFile << "Data End Line: " << endLine << "\n";
    indexFile << "----------------------------------\n";

    htmlFile.close();
    dataFile.close();
    indexFile.close();

    std::cout << "Processing completed: data.txt and index.txt updated.\n";
}

struct URL
{
    char data[1000];
};

void printer(std::vector<std::string> a)
{
    for (int i = 0; i < a.size(); i++)
    {
        std::cout << a[i] << " ";
    }
}

std::string makeAbsoluteURL(const std::string &baseURL, const std::string &relativeURL)
{
    xmlChar *absolute = xmlBuildURI(reinterpret_cast<const xmlChar *>(relativeURL.c_str()),
                                    reinterpret_cast<const xmlChar *>(baseURL.c_str()));
    std::string absoluteURL;
    if (absolute)
    {
        absoluteURL = reinterpret_cast<const char *>(absolute);
        xmlFree(absolute);
    }
    return absoluteURL;
}
std::string sanitize(std::string a)
{
    std::regex restrictedChars(R"([\\/:*?"<>|])");
    return std::regex_replace(a, restrictedChars, "_");
}
std::string hashURL(const std::string &url)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, url.c_str(), url.size());
    SHA256_Final(hash, &sha256);

    std::ostringstream hexStream;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return hexStream.str();
}
size_t file_handler(char *buffer, size_t size, size_t nmemb, void *userdata)
{
    std::ofstream *file = static_cast<std::ofstream *>(userdata);
    file->write(buffer, size * nmemb);
    return size * nmemb;
}
std::string generateFilename(const char *url)
{
    std::string filename = hashURL(url);
    return "storage/" + filename;
}
void traverse(xmlNode *node, std::vector<std::string> &hrefs, std::vector<std::string> &css,
              std::vector<std::string> &js, const std::string &baseUri)
{
    while (node)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            std::string nodeName = reinterpret_cast<const char *>(node->name);

            if (nodeName == "a")
            {
                for (xmlAttr *attr = node->properties; attr; attr = attr->next)
                {
                    if (std::string(reinterpret_cast<const char *>(attr->name)) == "href")
                    {
                        xmlChar *value = xmlNodeListGetString(node->doc, attr->children, 1);
                        if (value)
                        {
                            hrefs.emplace_back(makeAbsoluteURL(baseUri, reinterpret_cast<const char *>(value)));
                            xmlFree(value);
                        }
                    }
                }
            }
            else if (nodeName == "link")
            {
                std::string rel;
                for (xmlAttr *attr = node->properties; attr; attr = attr->next)
                {
                    if (std::string(reinterpret_cast<const char *>(attr->name)) == "rel")
                    {
                        rel = reinterpret_cast<const char *>(attr->children ? (const char *)attr->children->content : "");
                    }
                    if (std::string(reinterpret_cast<const char *>(attr->name)) == "href")
                    {
                        xmlChar *value = xmlNodeListGetString(node->doc, attr->children, 1);
                        if (value && rel == "stylesheet")
                        {
                            css.emplace_back(makeAbsoluteURL(baseUri, reinterpret_cast<const char *>(value)));
                            xmlFree(value);
                        }
                    }
                }
            }
            else if (nodeName == "script")
            {
                for (xmlAttr *attr = node->properties; attr; attr = attr->next)
                {
                    if (std::string(reinterpret_cast<const char *>(attr->name)) == "src")
                    {
                        xmlChar *value = xmlNodeListGetString(node->doc, attr->children, 1);
                        if (value)
                        {
                            js.emplace_back(makeAbsoluteURL(baseUri, reinterpret_cast<const char *>(value)));
                            xmlFree(value);
                        }
                    }
                }
            }
        }

        traverse(node->children, hrefs, css, js, baseUri);
        node = node->next;
    }
}

void parse(const std::string &filename, std::vector<std::string> &hrefs, std::vector<std::string> &css, std::vector<std::string> &js, const URL &startURL)
{
    std::cout << "This is Filename" << filename;
    htmlDocPtr doc = htmlReadFile(filename.c_str(), nullptr, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc)
    {
        std::cerr << "Error: Could not parse the HTML file: " << filename << std::endl;
        return;
    }

    xmlNode *root_element = xmlDocGetRootElement(doc);
    traverse(root_element, hrefs, css, js, startURL.data);
    std::cout << "links";
    printer(hrefs);
    std::cout << "css";
    printer(css);
    std::cout << "js";
    printer(js);
    std::cout << std::endl;
    xmlFreeDoc(doc);
    xmlCleanupParser();
}

std::string processLineWithAbsoluteURLs(const std::string &line, const std::string &baseUri)
{
    std::string modifiedLine = line;
    std::regex urlRegex(R"((href|src)\s*=\s*["']([^"']+)["'])");
    std::smatch match;

    std::string::const_iterator searchStart(line.cbegin());
    while (std::regex_search(searchStart, line.cend(), match, urlRegex))
    {
        std::string attribute = match[1];
        std::string relativeURL = match[2];
        std::string absoluteURL = makeAbsoluteURL(baseUri, relativeURL);

        std::string fullMatch = match[0];
        std::string modifiedMatch = attribute + "=\"" + absoluteURL + "\"";

        modifiedLine.replace(modifiedLine.find(fullMatch), fullMatch.length(), modifiedMatch);

        searchStart = match.suffix().first;
    }
    return modifiedLine;
}

void getFile(const URL &target, const std::string &sessionFolder, ftype type)
{
    std::filesystem::create_directory(sessionFolder);
    std::string filename;
    switch (type)
    {
    case HTML:
        filename = sessionFolder + "/" + sanitize(target.data) + ".html";
        break;
    case CSS:
        filename = sessionFolder + "/" + sanitize(target.data) + ".css";
        break;
    case JS:
        filename = sessionFolder + "/" + sanitize(target.data) + ".js";
        break;
    default:
        return;
    }
    if (std::filesystem::exists(filename))
    {
        std::cout << "File already exists: " << filename << ". Skipping download.\n";
        return;
    }
    std::ofstream tempFile(filename, std::ios::binary);
    if (!tempFile)
    {
        std::cerr << "Error: Unable to open file " << filename << " for writing.\n";
        return;
    }
    CURL *handle = curl_easy_init();
    if (!handle)
    {
        std::cerr << "Error: Unable to initialize CURL.\n";
        return;
    }
    curl_easy_setopt(handle, CURLOPT_URL, target.data);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, file_handler);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &tempFile);
    CURLcode res = curl_easy_perform(handle);
    if (res != CURLE_OK)
    {
        std::cerr << "Error: CURL request failed.\n";
    }
    curl_easy_cleanup(handle);
    tempFile.close();
    std::ifstream inputFile(filename);
    if (!inputFile)
    {
        std::cerr << "Error: Unable to open file " << filename << " for reading.\n";
        return;
    }
    std::string modifiedContent;
    std::string baseUri = target.data; 
    std::string line;
    while (std::getline(inputFile, line))
    {
        line = processLineWithAbsoluteURLs(line, baseUri);
        modifiedContent += line + '\n';
    }
    inputFile.close();
    std::ofstream outputFile(filename, std::ios::trunc);
    if (!outputFile)
    {
        std::cerr << "Error: Unable to open file " << filename << " for writing modified content.\n";
        return;
    }
    outputFile << modifiedContent;
    outputFile.close();

    std::cout << "Page saved to: " << filename << "\n";
    if (type == HTML)
    {
        processHTMLToFiles(filename);
    }
}

void runHTML(URL *input)
{
    std::string filename = generateFilename(input->data);
    std::string command = "firefox file:///home/deathhauler/projects/web_crawler/" + filename;
    std::system(command.c_str());
}

void crawl(URL startURL, int depth, std::set<std::string> &visited, const std::string &sessionFolder)
{
   std::queue<std::pair<std::string, int>> queue;
    queue.push({startURL.data, 0});
    while (!queue.empty())
    {
        auto [currentURL, currentDepth] = queue.front();
        queue.pop();
        if (visited.find(currentURL) != visited.end() || currentDepth > depth)
        {
            continue;
        }
        visited.insert(currentURL);
        std::cout << "Crawling: " << currentURL << " (Depth: " << currentDepth << ")\n";
        std::string filename = sessionFolder + "/" + sanitize(currentURL) + ".html";
        URL target;
        strncpy(target.data, currentURL.c_str(), sizeof(target.data) - 1);
        target.data[sizeof(target.data) - 1] = '\0';
        getFile(target, sessionFolder, HTML);
        std::vector<std::string> hrefs;
        std::vector<std::string> css;
        std::vector<std::string> js;
        parse(filename, hrefs, css, js, target);
        for (int i = 0; i < css.size(); i++)
        {
            strncpy(target.data, css[i].c_str(), sizeof(target.data) - 1);
            target.data[sizeof(target.data) - 1] = '\0';
            getFile(target, sessionFolder, CSS);
        }
        for (int i = 0; i < js.size(); i++)
        {
            strncpy(target.data, js[i].c_str(), sizeof(target.data) - 1);
            target.data[sizeof(target.data) - 1] = '\0';
            getFile(target, sessionFolder, JS);
        }
        for (const auto &link : hrefs){
            std::string absoluteLink = makeAbsoluteURL(currentURL, link);
            if (!absoluteLink.empty() && visited.find(absoluteLink) == visited.end())
            {
                queue.push({absoluteLink, currentDepth + 1});
            }
        }
    }
}

void startCrawl()
{
    // strcpy(target.data, "https://news.ycombinator.com/best");
    URL target;
    std::cout << "Enter URL to start crawling: ";
    std::cin >> target.data;
    int depth;
    std::cout << "Enter crawl depth: ";
    std::cin >> depth;
    std::set<std::string> visited;
    std::string safeName = sanitize(target.data);
    std::string sessionFolder = "storage/" + safeName;
    std::filesystem::create_directory(sessionFolder);
    curl_global_init(CURL_GLOBAL_ALL);
    crawl(target, depth, visited, sessionFolder);
    curl_global_cleanup();

    std::cout << "All pages saved in folder: " << sessionFolder << "\n";
}
void search()
{
    std::vector<std::vector<int>> output;
    std::string dataFilename = "/mnt/c/Users/user/Desktop/Work/Web_Crawler/Web_Crawler/data.txt";
    std::string indexFilename = "/mnt/c/Users/user/Desktop/Work/Web_Crawler/Web_Crawler/index.txt";
    std::map<int, std::pair<int, std::string>> lineToFileMap;
    parseIndexFile(indexFilename, lineToFileMap);
    std::string key;
    std::cout << "Enter the key to search: ";
    std::cin.ignore();
    std::getline(std::cin, key);
    std::vector<std::string> aiOutput = extractMessages(apiHandler(key));
    for (auto i : aiOutput)
    {
        fileHandler(dataFilename, key, output);
    }

    std::unordered_map<std::string, int> uniqueMatches;
    if (!output.empty())
    {
        for (const auto &match : output)
        {
            std::string filename = getFileFromLine(lineToFileMap, match[2]);
            uniqueMatches[filename]++;
        }
    }
    else
    {
        std::cout << "Pattern not found in file!" << std::endl;
    }
    for (auto i : uniqueMatches)
    {
        std::cout << i.first << "  " << i.second << std::endl;
    }
}
int main()
{
    URL target;
    int choice = 1;
    while (choice == 1 || choice == 2)
    {
        std::cout << "What Operation do you want to perform(1: archive, 2:search): ";
        std::cin >> choice;
        switch (choice)
        {
        case 1:
            startCrawl();
            break;
        case 2:
            search();
            break;
        default:
            break;
        }
    }
}