#pragma once

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <map>
#include <memory>

class JsonParser
{
public:
    void loadAll(const std::string& dir);

    void loadDocument(const std::string& filename, const std::string& id);
    void loadString(const std::string& str, const std::string& id);

    rapidjson::Document *getDocument(const std::string& id);
    const rapidjson::Document *getDocument(const std::string &id) const;

    bool isLoaded(const std::string& id) const;

private:
    std::map<std::string, std::unique_ptr<rapidjson::Document>> m_documents;
};
