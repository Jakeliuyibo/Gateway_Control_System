#include <bits/stdc++.h>
#include "logger.h"
#include "configparser.h"

using namespace std;
using namespace utility;


int main()
{
    /* 解析配置文件 */
    IniConfigParser parser;
    bool parserFlag = false;
    string parserInfo = "";
    parserFlag = parser.load("../config/defconfig.ini", parserInfo);

    /* 解析logger配置 */
    string logger_folderpath, logger_name; 
    parser.getValue<string>("LOGGER", "LOGGER_FOLDERPATH"   , logger_folderpath, parserInfo);
    parser.getValue<string>("LOGGER", "LOGGER_NAME"         , logger_name, parserInfo);

    cout << logger_folderpath << endl;
    cout << logger_name << endl;
    cout << "parserFlag = " << parserFlag << endl;
    cout << "parserInfo = " << parserInfo << endl;



    return 0;
}
