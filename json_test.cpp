#include <jsoncpp/json/json.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <string>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

int main()
{
    // std::string rawJson = R"({"Age": 20, "Name": "colin"})";
    std::string rawJson;
    JSONCPP_STRING err;
    Json::Value root;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        std::cout << "has curl" << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, "localhost");
        curl_easy_setopt(curl, CURLOPT_PORT, 5678);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rawJson);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    const auto rawJsonLength = static_cast<int>(rawJson.length());
    // std::cout << rawJson << std::endl;

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(rawJson.c_str(), rawJson.c_str() + rawJsonLength, &root, &err))
    {
        std::cout << "error" << std::endl;
        return EXIT_FAILURE;
    }
    // std::cout << root << std::endl;

    // std::cout << root["CONTENTS"]["rnbo"]["CONTENTS"]["inst"] << std::endl;

    std::cout << "\n\n-------------------\n"
              << std::endl;

    const Json::Value params = root["CONTENTS"]["rnbo"]["CONTENTS"]["inst"]["CONTENTS"]["0"]["CONTENTS"]["params"]["CONTENTS"];
    std::vector<std::string> paramNames = params.getMemberNames();
    for (int i = 0; i < paramNames.size(); i++)
    {
        // Member name and value
        std::cout << paramNames[i] << std::endl;
    }
    return EXIT_SUCCESS;

    // CONTENTS rnbo CONTENTS inst CONTENTS 0 CONTENTS params CONTENTS
}
