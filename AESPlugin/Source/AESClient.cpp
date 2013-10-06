#include "AESClient.h"
#include "JSON.h"

using namespace std;

AESClient::AESClient(const string& url)
{
    m_CURL.ParseUrl(url, &m_Server, &m_Path);
}

AESClient::~AESClient()
{
}

bool AESClient::Ping()
{
    string response = "";
    if (!m_CURL.Get(m_Server, NULL, response))
    {
        m_UserName = "";
        m_Password = "";
        return false;
    }
    
    return !response.empty();
}

bool AESClient::Login(const string& userName, const string& password)
{
	map<string,string> headers;
	headers.insert(make_pair("Content-Type", "application/x-www-form-urlencoded"));
    
	string loginUrl = m_Server + "/login";
	string response = "";
	string postData = "username=" + userName;
    postData += "&password=" + password;
    
    if (!m_CURL.Post(loginUrl, postData, &headers, response))
    {
        m_UserName = "";
        m_Password = "";
        return false;
    }
    
    map<string,string>::const_iterator i = headers.find("Location");
    if (i == headers.end() || i->second.empty() || i->second.find("index.html") == string::npos)
    {
        m_UserName = "";
        m_Password = "";
        return false;
    }
    
    m_UserName = userName;
    m_Password = password;
    return true;
}

bool AESClient::Exists(const std::string& revision, const std::string& path, AESEntry* entry)
{
	string response = "";
    string url = m_Server + m_Path + "/" + revision + path + "?info";

    if (!m_CURL.GetJSON(url, response))
    {
        return false;
    }
    
    JSONValue* json = JSON::Parse(response.c_str());
    if (json == NULL || !json->IsObject())
    {
        return false;
    }
    
    JSONObject obj = json->AsObject();
    bool res = (obj.find(L"data") != obj.end());
    if (res && entry != NULL)
    {
        JSONObject data = obj.find(L"data")->second->AsObject();
        wstring temp = ((data.find(L"ref")->second)->AsString());
        entry->SetReference(string(temp.begin(), temp.end()));
        
        temp = ((obj.find(L"hash")->second)->AsString());
        entry->SetHash(string(temp.begin(), temp.end()));
            
        entry->SetDir((obj.find(L"directory")->second)->AsBool());
        entry->SetSize((int)(obj.find(L"size")->second)->AsNumber());
    }
    
    delete json;
    return res;
}