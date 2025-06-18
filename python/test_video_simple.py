#!/usr/bin/env python3
import cv2
import numpy as np
from pathlib import Path

def test_video_file():
    """测试视频文件读取"""
    video_path = "../resources/video/VID_20250617094821.wmv"
    
    print(f"测试视频文件: {video_path}")
    
    if not Path(video_path).exists():
        print("❌ 视频文件不存在！")
        return False
    
    cap = cv2.VideoCapture(video_path)
    
    if not cap.isOpened():
        print("❌ 无法打开视频文件！")
        return False
    
    # 获取视频信息
    fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    
    print(f"✅ 视频信息:")
    print(f"   分辨率: {width}x{height}")
    print(f"   帧率: {fps} FPS")
    print(f"   总帧数: {total_frames}")
    print(f"   时长: {total_frames/fps:.2f} 秒")
    
    # 读取前几帧
    frame_count = 0
    for i in range(min(5, total_frames)):
        ret, frame = cap.read()
        if ret:
            frame_count += 1
            print(f"   成功读取第 {i+1} 帧，尺寸: {frame.shape}")
        else:
            print(f"   读取第 {i+1} 帧失败")
            break
    
    cap.release()
    print(f"✅ 成功读取 {frame_count} 帧")
    return True

def test_yolo_detection():
    """测试YOLO检测"""
    try:
        from ultralytics import YOLO
        print("✅ 导入YOLO成功")
        
        # 加载模型
        model = YOLO('yolo11n.pt')
        print("✅ YOLO模型加载成功")
        
        # 创建测试图像（包含一些简单形状）
        test_image = np.zeros((640, 640, 3), dtype=np.uint8)
        test_image.fill(128)  # 灰色背景
        
        # 绘制一些简单图形
        cv2.rectangle(test_image, (100, 100), (200, 300), (255, 255, 255), -1)  # 白色矩形
        cv2.circle(test_image, (400, 200), 50, (0, 255, 0), -1)  # 绿色圆形
        
        # 进行检测
        results = model(test_image, verbose=False)
        print(f"✅ YOLO检测完成，结果数量: {len(results)}")
        
        for result in results:
            boxes = result.boxes
            if boxes is not None and len(boxes) > 0:
                print(f"   检测到 {len(boxes)} 个目标")
                for i, box in enumerate(boxes):
                    class_id = int(box.cls[0])
                    confidence = float(box.conf[0])
                    class_name = model.names[class_id]
                    print(f"   目标 {i+1}: {class_name} (置信度: {confidence:.2f})")
            else:
                print("   未检测到任何目标")
        
        return True
        
    except Exception as e:
        print(f"❌ YOLO测试失败: {e}")
        return False

if __name__ == "__main__":
    print("=== 视频文件测试 ===")
    video_ok = test_video_file()
    
    print("\n=== YOLO检测测试 ===")
    yolo_ok = test_yolo_detection()
    
    if video_ok and yolo_ok:
        print("\n✅ 所有测试通过！系统应该能正常工作")
    else:
        print("\n❌ 部分测试失败，请检查配置") 