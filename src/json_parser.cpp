#include "json_parser.hpp"

#include <iostream>
#include <experimental/filesystem>

namespace filesys = std::experimental::filesystem;

void JsonParser::loadAll(const std::string &dir)
{
    for (auto it = filesys::recursive_directory_iterator(dir); it != filesys::recursive_directory_iterator(); ++it) {
        if (!filesys::is_directory(it->path())) {
            const auto& file = it->path();

            if (file.extension() == ".json") {
                loadDocument(file.string(), file.stem().string());
            }
        }
    }
}

void JsonParser::loadDocument(const std::string &filename, const std::string& id)
{
	FILE* file;
	
#ifdef _WIN32
	fopen_s(&file, filename.c_str(), "rb");
#else
    file = fopen(filename.c_str(), "rb");
#endif

    if (file == nullptr) {
        std::cerr << "Error while reading json file: " << filename << std::endl;
        return;
    }

    char buffer[65536];
    rapidjson::FileReadStream jsonFile(file, buffer, sizeof(buffer));

    std::unique_ptr<rapidjson::Document> document{new rapidjson::Document{}};

    document->ParseStream(jsonFile);

    auto inserted = m_documents.emplace(id, std::move(document));

    if (!inserted.second) {
        std::cerr << "Error - Json document " << filename << " not loaded properly" << std::endl;
    }
}

void JsonParser::loadString(const std::string &str, const std::string &id)
{
    std::unique_ptr<rapidjson::Document> document{new rapidjson::Document{}};

    document->Parse(str.c_str());

    auto inserted = m_documents.emplace(id, std::move(document));

    if (!inserted.second) {
        std::cerr << "Error - Json string not loaded properly" << std::endl;
    }
}

rapidjson::Document* JsonParser::getDocument(const std::string& id)
{
    auto found = m_documents.find(id);

    if (found == m_documents.end()) {
        std::cerr << "Error while getting document - JSON document not loaded" << std::endl;
        return nullptr;
    }

    return found->second.get();
}

const rapidjson::Document* JsonParser::getDocument(const std::string& id) const
{
    auto found = m_documents.find(id);

    if (found == m_documents.end()) {
        std::cerr << "Error while getting document - JSON document not loaded" << std::endl;
        return nullptr;
    }

    return found->second.get();
}
