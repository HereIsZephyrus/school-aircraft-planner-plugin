#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
测试视频处理系统
用于验证Python端能否正确处理视频文件并返回检测结果
"""

import cv2
import json
import time
import sys
import os
from pathlib import Path

# 添加当前目录到Python路径
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

try:
    from yolo_detection import DroneVideoDetector
    YOLO_MODULE_AVAILABLE = True
except ImportError as e:
    print(f"无法导入YOLO检测模块: {e}")
    YOLO_MODULE_AVAILABLE = False

def test_video_file_exists():
    """测试视频文件是否存在"""
    print("=== 测试视频文件 ===")
    video_path = Path("../resources/video/VID_20250617094821.wmv")
    
    if video_path.exists():
        print(f"✓ 视频文件存在: {video_path.absolute()}")
        
        # 测试能否打开视频文件
        cap = cv2.VideoCapture(str(video_path))
        if cap.isOpened():
            fps = cap.get(cv2.CAP_PROP_FPS)
            frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
            width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
            height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
            
            print(f"✓ 视频文件可以打开")
            print(f"  - 分辨率: {width}x{height}")
            print(f"  - 帧率: {fps} FPS")
            print(f"  - 总帧数: {frame_count}")
            print(f"  - 时长: {frame_count/fps:.2f} 秒")
            
            # 读取第一帧进行测试
            ret, frame = cap.read()
            if ret:
                print(f"✓ 成功读取第一帧，尺寸: {frame.shape}")
                return True, str(video_path)
            else:
                print("✗ 无法读取视频帧")
                return False, None
        else:
            print("✗ 无法打开视频文件")
            return False, None
    else:
        print(f"✗ 视频文件不存在: {video_path.absolute()}")
        return False, None

def test_yolo_detection():
    """测试YOLO检测模块"""
    print("\n=== 测试YOLO检测模块 ===")
    
    if not YOLO_MODULE_AVAILABLE:
        print("✗ YOLO检测模块不可用")
        return False
    
    try:
        detector = DroneVideoDetector()
        print("✓ YOLO检测器创建成功")
        
        # 创建测试帧
        test_frame = cv2.imread("test_frame.jpg") if Path("test_frame.jpg").exists() else None
        if test_frame is None:
            # 创建一个模拟帧
            import numpy as np
            test_frame = np.zeros((480, 640, 3), dtype=np.uint8)
            test_frame[:] = (50, 50, 50)  # 灰色背景
        
        # 执行检测
        detections = detector.detect_objects_yolo(test_frame)
        print(f"✓ 检测完成，发现 {len(detections)} 个目标")
        
        for i, detection in enumerate(detections[:3]):  # 只显示前3个
            print(f"  检测 {i+1}: {detection['class']} (置信度: {detection['confidence']:.2f})")
        
        return True
        
    except Exception as e:
        print(f"✗ YOLO检测测试失败: {e}")
        return False

def test_video_processing():
    """测试视频处理"""
    print("\n=== 测试视频处理 ===")
    
    video_exists, video_path = test_video_file_exists()
    if not video_exists:
        return False
    
    try:
        cap = cv2.VideoCapture(video_path)
        processed_frames = 0
        max_frames = 10  # 只处理前10帧
        
        if YOLO_MODULE_AVAILABLE:
            detector = DroneVideoDetector()
            
        while processed_frames < max_frames:
            ret, frame = cap.read()
            if not ret:
                break
            
            # 模拟检测过程
            if YOLO_MODULE_AVAILABLE:
                detections = detector.detect_objects_yolo(frame)
                print(f"帧 {processed_frames + 1}: 检测到 {len(detections)} 个目标")
            else:
                print(f"帧 {processed_frames + 1}: 已处理 (模拟模式)")
            
            processed_frames += 1
            
            # 控制处理速度
            time.sleep(0.1)
        
        cap.release()
        print(f"✓ 成功处理 {processed_frames} 帧")
        return True
        
    except Exception as e:
        print(f"✗ 视频处理测试失败: {e}")
        return False

def test_tcp_connection():
    """测试TCP连接"""
    print("\n=== 测试TCP连接 ===")
    
    import socket
    
    try:
        # 尝试连接到Qt应用程序
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(2)  # 2秒超时
        
        result = sock.connect_ex(('localhost', 8888))
        if result == 0:
            print("✓ 成功连接到Qt应用程序 (localhost:8888)")
            
            # 发送测试数据
            test_data = {
                'timestamp': time.strftime('%Y-%m-%d %H:%M:%S'),
                'frame_id': 1,
                'detections': [
                    {
                        'class': 'person',
                        'confidence': 0.85,
                        'x': 100,
                        'y': 200,
                        'width': 50,
                        'height': 100,
                        'risk': False,
                        'risk_level': '低'
                    }
                ],
                'stats': {
                    'total_frames': 1,
                    'total_detections': 1
                }
            }
            
            json_data = json.dumps(test_data) + '\n'
            sock.send(json_data.encode('utf-8'))
            print("✓ 测试数据发送成功")
            
            sock.close()
            return True
        else:
            print("✗ 无法连接到Qt应用程序 (localhost:8888)")
            print("  请确保Qt应用程序正在运行并且TCP服务器已启动")
            return False
            
    except Exception as e:
        print(f"✗ TCP连接测试失败: {e}")
        return False
    finally:
        try:
            sock.close()
        except:
            pass

def main():
    """主函数"""
    print("开始测试视频处理系统...")
    print("=" * 50)
    
    # 运行各项测试
    tests = [
        ("视频文件", test_video_file_exists),
        ("YOLO检测", test_yolo_detection),
        ("视频处理", test_video_processing),
        ("TCP连接", test_tcp_connection)
    ]
    
    results = {}
    for test_name, test_func in tests:
        try:
            results[test_name] = test_func()
        except Exception as e:
            print(f"✗ {test_name}测试出现异常: {e}")
            results[test_name] = False
    
    # 显示测试结果摘要
    print("\n" + "=" * 50)
    print("测试结果摘要:")
    for test_name, passed in results.items():
        status = "✓ 通过" if passed else "✗ 失败"
        print(f"  {test_name}: {status}")
    
    passed_count = sum(results.values())
    total_count = len(results)
    
    print(f"\n总计: {passed_count}/{total_count} 项测试通过")
    
    if passed_count == total_count:
        print("🎉 所有测试通过！系统应该可以正常工作。")
    else:
        print("⚠️  部分测试失败，请检查上述错误信息。")
    
    return passed_count == total_count

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1) 