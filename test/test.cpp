#include <iostream>
#include <uv.h>
#include "systime.h"
#include "logger.h"
#include "configparser.h"

using namespace std;
using namespace utility;

int main()
{
    uv_loop_t *loop = (uv_loop_t *) malloc(sizeof(uv_loop_t));
    uv_loop_init(loop);


    printf("Hello Word!\n");
    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);
    return 0;

    // /* 初始化日志模块 */
    // Logger::instance()->init("../logs/C.log", Logger::STREAM_BOTH, Logger::MODE_SYNC, 
    //                                           Logger::LEVEL_DEBUG, Logger::LEVEL_WARNING, Logger::LEVEL_DEBUG);
    // critical("program start ...");

    // /* 解析配置模块 */
    // IniConfigParser parser;
    // bool parserFlag = true;
    // parserFlag = parser.load("../config/defconfig.ini");

    // /* 解析logger配置 */
    // // string logger_folderpath, logger_level; 
    // // parserFlag &= parser.getValue<string>("LOGGER", "LOGGER_FOLDERPATH", logger_folderpath);
    // // parserFlag &= parser.getValue<string>("LOGGER", "LOGGER_LEVEL"     , logger_level     );
    // if(!parserFlag)
    // {
    //     return 0;
    // }

    // critical("{}.{}", getSystime(), getSystimeByFilenameFormat());


    // /* 注销日志模块 */
    // critical("program end ...");
    // Logger::instance()->deinit();

    // return 0;
}