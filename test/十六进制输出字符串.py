import struct

def string_to_uint8_array(s):
    # 将字符串转换为字节数组
    bytes_array = bytearray(s, 'utf-8')
    # 将字节数组转换为uint8_t数组
    uint8_array = [x for x in bytes_array]
    return uint8_array

def print_hex(arr):
    # 打印16进制格式的数组
    for i in range(len(arr)):
        print(f"0x{arr[i]:02x}", end=", ")
    print()

# 测试字符串
s = "3-1-2-3\x00\x00\x00"
# 将字符串转换为uint8_t数组
arr = string_to_uint8_array(s)
# 打印16进制格式的数组
print_hex(arr)