#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <cmath>
#include "json/json.h"

#define DIRECTORY_SIZE 3000
#define BLOCK_SIZE 10

using namespace std;

class fs {
private:
    Json::Value dir;
    Json::Value *curr;
    string diskDir;
    string diskContent;
    int inode = 0;
    
    vector<string> split(string str)
    {
        vector<string> res;
        int pre = 0;
        for (int p = str.find("/"); p > -1; p = str.find("/", pre)) {
            res.push_back(str.substr(pre, p-pre));
            pre = p+1;
        }
        res.push_back(str.substr(pre));
        return res;
    }
    
    vector<string> split2(string str)
    {
        vector<string> res;
        int pre = 0;
        for (int p = str.find(","); p > -1; p = str.find(",", pre)) {
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
    
    void listFileData(string name)
    {
        bool flag = false;
        string data;
        
        for (auto item : (*curr)["files"])
        {
            if( item["name"] == name)
            {
                flag = true;
                //whether "type" is "file"
                if( item["type"] != "file")
                {
                    cout<<"This is not a file."<<endl;
                    return;
                }
                //get "size"
                int dataSize = item["size"].asInt();//file data size
                //get block numbers
                vector<string> blockNumStr = split2(item["block_num"].asString());
                vector<int> blockNumInt(blockNumStr.size());
                for (int i=0; i<blockNumStr.size();i++)
                {
                    blockNumInt[i]=stoi(blockNumStr[i].c_str());
                }
                //read data
                int pData = 0;
                for(int bn:blockNumInt)
                {
                    if(dataSize-pData<=BLOCK_SIZE)
                    {
                        data += diskContent.substr(DIRECTORY_SIZE+BLOCK_SIZE*bn,dataSize-pData);
                    }
                    else
                    {
                        data +=diskContent.substr(DIRECTORY_SIZE+BLOCK_SIZE*bn,BLOCK_SIZE);
                        pData += BLOCK_SIZE;
                    }
                }
            }
        }
        //if no such file name
        if(flag==false)
            cout << "There is no such file named: "+name+" ." << endl;
        //list data
        else if(flag==true)
        {
            cout << data << endl;
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
    
    void makeFile(string name = "New file")
    {
        Json::Value item;
        time_t now = time(0);
        char* dt = ctime(&now);
        int s = name.length();
        item["name"] = name;
        item["type"] = "file";
        item["author"] = "fs";
        item["time"] = dt;
        item["block_num"] = inode;
        item["size"] = s;
        inode++;
        if (inode == 255)
        {
            cout<<"The disk is full." <<endl;
            return;
        }
        (*curr)["files"].append(item);
    }
    
    void addData(string name,string data)
    {
        bool flag = false;
        int index = 0;
        for (auto item : (*curr)["files"])
        {
            if( item["name"] == name)
            {
                flag = true;
                //whether "type" is "file"
                if( item["type"] != "file")
                {
                    cout<<"Can't put data into this type of data."<<endl;
                    return;
                }
                //modify "size"
                int s = data.length();
                int temp0 = item["size"].asInt();//temp0: original size
                int temp = temp0 + s;//temp: updated size
                (*curr)["files"][index]["size"] = temp;
                //modify "block_num"
                int b_num0 = ceil(temp0/double(BLOCK_SIZE));//b_num0: original block numbers
                int b_num1 = ceil(temp/double(BLOCK_SIZE));//b_num1: updated block numbers
                int newBlockNum = b_num1 - b_num0;
                vector<string> blockNumStr0 = split2((*curr)["files"][index]["block_num"].asString());
                vector<int> blockNumInt0(blockNumStr0.size());
                for (int i=0; i<blockNumStr0.size();i++)
                {
                    blockNumInt0[i]=stoi(blockNumStr0[i].c_str());
                }
                for (int i=0;i<newBlockNum;i++)
                 //if( (*curr)["files"][index]["size"] >= 10)
                 {
                     if (inode == 255)
                     {
                         cout<<"The disk is full." <<endl;
                         return;
                     }
                     string temp1 = (*curr)["files"][index]["block_num"].asString();
                     char temp2[3];
                     sprintf(temp2, "%d", inode);
                     inode++;
                     temp1 = temp1+","+temp2;
                     //cout << temp1 << endl;
                     (*curr)["files"][index]["block_num"]= temp1;
                }
                //add data
                //get block_num in vector<int> format
                vector<string> blockNumStr = split2((*curr)["files"][index]["block_num"].asString());
                vector<int> blockNumInt(blockNumStr.size());
                for (int i=0; i<blockNumStr.size();i++)
                {
                    blockNumInt[i]=stoi(blockNumStr[i].c_str());
                }
                //add new data
                int b0 = DIRECTORY_SIZE+BLOCK_SIZE*blockNumInt0[blockNumInt0.size()-1];
                int offset0 = temp0-(temp0/BLOCK_SIZE)*BLOCK_SIZE;
                int pData=0;
                if(data.length()<=BLOCK_SIZE-offset0)
                {
                    diskContent.replace(b0+offset0,data.length(),data);
                }
                else
                {
                    diskContent.replace(b0+offset0,BLOCK_SIZE-offset0,data,0,BLOCK_SIZE-offset0);
                    pData=BLOCK_SIZE-offset0;
                }
                for (int i=blockNumInt0.size(); i< blockNumInt.size();i++)
                {
                    int bi = DIRECTORY_SIZE+BLOCK_SIZE*blockNumInt[i];
                    if(data.length()-pData<=BLOCK_SIZE)
                    {
                        diskContent.replace(bi,data.length()-pData,data,pData,data.length()-pData);
                    }
                    else
                    {
                        diskContent.replace(bi,BLOCK_SIZE,data,pData,BLOCK_SIZE);
                        pData+=BLOCK_SIZE;
                    }
                }
            }
            index++;
        }
        //if no such file name
        if(flag==false)
            cout << "There is no such file named: "+name+". Adding data failed!" << endl;
        
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
        ofs.open("/Users/Spencer/Documents/test.txt");
        ofs << diskContent;
        ofs.close();
    }
    
};

int main(int argc, const char * argv[])
{
    fs fs("/Users/Spencer/Documents/disk2.txt");
    fs.list();
    fs.makeFile("test");
    fs.makeFile("test2");
    fs.addData("test2","hello world, I love you!");
    fs.addData("Non existing file test","none");
    fs.makeFile("test3");
    fs.makeFile("test4");
    fs.addData("test4","Test Again");
    fs.addData("test2"," hello world again!!!!!!!!!!!!!!!!!!");
    fs.listFileData("test2");
    cout << "after making new file test:" << endl;
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
