#include <iostream>
#include <string>
#include <vector>

using namespace std;







int main(void)
{
    try
    {
        vector<uint8_t> payload = {0x33, 0x2d, 0x31, 0x2d, 0x32, 0x2d, 0x33, 0x00, 0x00, 0x00};
        std::size_t file_id;
        std::vector<std::size_t> retrans_tunk_id;
        _parse_retrans_from_payload(payload, file_id, retrans_tunk_id);
        cout << "解析成功" << endl;
        cout << "file_id = " << file_id << endl;
        for(auto r:retrans_tunk_id)
        {
            cout << "r = " << r << endl;
        }
    }
    catch(const std::exception& e)
    {
        cout << "解析失败 , MSG = " << e.what() << endl;
    }

    return 0;
}
