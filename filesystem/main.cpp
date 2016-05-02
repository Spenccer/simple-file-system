#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include "json/json.h"

#define DIRECTORY_SIZE 5000

using namespace std;

class fs {
private:
    Json::Value dir;
    Json::Value *curr;
    string diskDir;
    string diskContent;
    
    vector<string> split(string str)
    {
        vector<string> res;
        size_t pre = 0;
        for (size_t p = str.find("/"); p > -1; p = str.find("/", pre)) {
            res.push_back(str.substr(pre, p-pre));
            pre = p+1;
        }
        res.push_back(str.substr(pre));
        return res;
    }
public:
    fs(string d)
    {
        diskDir = d;
        
        // read disk file
        std::ifstream in(diskDir);
        std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        diskContent = s;
        
        Json::Reader reader;
        if (!reader.parse(s.substr(0, DIRECTORY_SIZE), dir, false))
        {
            std::cout << "Failed!\n";
        }
        curr = &dir;
    }
    
    void changeDir(string dirStr)
    {
        if (dirStr[0] == '/') {
            // Absolute dir
            curr = &dir;
        }
        vector<string> dirs = split(dirStr);
        for (string d : dirs) {
            int index = 0;
            for (auto item : (*curr)["files"]) {
                if (item["name"].asString() == d) {
                    curr = &(*curr)["files"][index];
                    break;
                }
                index++;
            }
        }
    }
    
    void list()
    {
        if ((*curr)["files"].size() == 0) {
            cout << "Current directory is empty" << endl;
            return;
        }
        for (auto item : (*curr)["files"]) {
            cout << item["name"].asString() << "(" << item["type"].asString() << ")" << endl;
        }
    }
    
    void makeDir(string name)
    {
        Json::Value item;
        item["name"] = name;
        item["type"] = "dir";
        Json::Value files;
        files.append(Json::Value::null);
        files.clear();
        item["files"] = files;
        (*curr)["files"].append(item);
    }
    
    void makeFile(string name)
    {
        Json::Value item;
        item["name"] = name;
        item["type"] = "file";
        (*curr)["files"].append(item);
    }
    
    void deleteDir(string dirStr)
    {
        if (dirStr[0] == '/') {
            // Absolute dir
            curr = &dir;
        }
        vector<string> dirs = split(dirStr);
        for (string d : dirs) {
            int index = 0;
            for (auto item : (*curr)["files"]) {
                if (item["name"].asString() == d) {
                    (*curr)["files"].removeIndex(index, &(*curr)["files"][index]);
                    break;
                }
                index++;
            }
        }
    }
    
    void deleteFile(string dirStr)
    {
        deleteDir(dirStr);
    }
    
    void saveDir()
    {
        std::string out = dir.toStyledString();
        Json::FastWriter writer;
        std::string strWrite = writer.write(dir);
        
        // delete line break symbol
        strWrite = strWrite.substr(0, strWrite.length() - 1);
        diskContent.replace(0, strWrite.length(), strWrite);
        
        std::ofstream ofs;
        ofs.open("/Users/wangsijie/desktop/test.txt");
        ofs << diskContent;
        ofs.close();
    }
    
};

int main(int argc, const char * argv[])
{
    fs fs("/Users/wangsijie/desktop/disk.txt");
    fs.list();
    fs.makeDir("test");
    cout << "after making new dir test:" << endl;
    fs.list();
    fs.changeDir("home");
    cout << "after changing dir to home:" << endl;
    fs.list();
    fs.changeDir("peter");
    cout << "after changing dir to peter:" << endl;
    fs.list();
    
    fs.changeDir("/home");
    cout << "after changing dir to /home:" << endl;
    fs.list();
    fs.deleteDir("peter");
    cout << "after deleting dir to peter:" << endl;
    fs.list();
    
    fs.saveDir();
    
    return 0;
}
