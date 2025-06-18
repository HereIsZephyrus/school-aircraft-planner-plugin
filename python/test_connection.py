#!/usr/bin/env python3
import socket
import time

def test_connection():
    """测试与Qt应用程序的连接"""
    host = 'localhost'
    port = 8888
    
    print(f"正在测试连接到 {host}:{port}...")
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)  # 5秒超时
        result = sock.connect_ex((host, port))
        
        if result == 0:
            print("✅ 连接成功！Qt应用程序的TCP服务器正在运行")
            sock.close()
            return True
        else:
            print("❌ 连接失败！Qt应用程序可能未启动或TCP服务器未开启")
            print(f"错误代码: {result}")
            return False
            
    except Exception as e:
        print(f"❌ 连接异常: {e}")
        return False
    finally:
        try:
            sock.close()
        except:
            pass

if __name__ == "__main__":
    # 测试连接
    success = test_connection()
    
    if not success:
        print("\n📝 解决步骤:")
        print("1. 确保Qt应用程序正在运行")
        print("2. 在Qt应用程序中点击'开始飞行模拟'")
        print("3. 查看Qt调试输出中是否有'TCP server started on port 8888'")
        
    time.sleep(2) 