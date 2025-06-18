#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cv2
import numpy as np
import os
from pathlib import Path
from datetime import datetime

try:
    from ultralytics import YOLO
    YOLO_AVAILABLE = True
except ImportError:
    YOLO_AVAILABLE = False
    print("Warning: ultralytics not available")

def draw_detection_boxes(frame, detections):
    """在视频帧上绘制检测框"""
    overlay_frame = frame.copy()
    
    for detection in detections:
        x = int(detection['x'])
        y = int(detection['y'])
        w = int(detection['width'])
        h = int(detection['height'])
        confidence = detection['confidence']
        class_name = detection['class']
        is_risk = detection.get('risk', False)
        
        # 选择颜色 (BGR格式)
        if is_risk:
            color = (0, 0, 255)  # 红色 - 高风险
        elif class_name == 'person':
            color = (0, 255, 0)  # 绿色 - 人员
        elif class_name in ['bicycle', 'motorcycle', 'car', 'truck', 'bus']:
            color = (255, 0, 0)  # 蓝色 - 车辆
        else:
            color = (0, 255, 255)  # 黄色 - 其他
        
        # 绘制检测框
        cv2.rectangle(overlay_frame, (x, y), (x + w, y + h), color, 2)
        
        # 绘制标签
        label = f"{class_name} {confidence:.1%}"
        label_size = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.6, 2)[0]
        
        # 标签背景
        cv2.rectangle(overlay_frame, 
                     (x, y - label_size[1] - 10), 
                     (x + label_size[0], y), 
                     color, -1)
        
        # 标签文字
        cv2.putText(overlay_frame, label, (x, y - 5), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        
        # 风险标识
        if is_risk:
            cv2.putText(overlay_frame, "RISK!", (x + w - 60, y + 25), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
    
    return overlay_frame

def assess_risk(class_name, x, y, w, h, confidence, frame_width, frame_height):
    """评估检测目标的风险等级"""
    is_risk = False
    
    # 人员聚集风险
    if class_name == 'person' and confidence > 0.6:
        # 检查是否在画面中央区域（可能的聚集区域）
        center_x = x + w // 2
        center_y = y + h // 2
        if (frame_width * 0.3 < center_x < frame_width * 0.7 and 
            frame_height * 0.3 < center_y < frame_height * 0.7):
            is_risk = True
    
    # 车辆风险
    elif class_name in ['bicycle', 'motorcycle', 'car', 'truck', 'bus'] and confidence > 0.7:
        is_risk = True
    
    return is_risk

def detect_objects_yolo(frame, model):
    """使用YOLO进行目标检测"""
    if not model:
        return []
    
    try:
        results = model(frame, verbose=False)
        detections = []
        
        for result in results:
            boxes = result.boxes
            if boxes is not None:
                for box in boxes:
                    # 获取检测信息
                    x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                    confidence = box.conf[0].cpu().numpy()
                    class_id = int(box.cls[0].cpu().numpy())
                    
                    # 获取类别名称
                    class_name = model.names[class_id]
                    
                    # 只处理感兴趣的类别
                    if class_name not in ['person', 'bicycle', 'motorcycle', 'car', 'truck', 'bus']:
                        continue
                    
                    # 置信度过滤
                    if confidence < 0.5:
                        continue
                    
                    # 计算边界框
                    x, y, w, h = int(x1), int(y1), int(x2-x1), int(y2-y1)
                    
                    # 评估风险
                    is_risk = assess_risk(class_name, x, y, w, h, confidence, 
                                        frame.shape[1], frame.shape[0])
                    
                    detection = {
                        'class': class_name,
                        'confidence': float(confidence),
                        'x': x,
                        'y': y,
                        'width': w,
                        'height': h,
                        'risk': is_risk
                    }
                    
                    detections.append(detection)
        
        return detections
        
    except Exception as e:
        print(f"YOLO检测失败: {e}")
        return []

def test_video_detection():
    """测试视频检测并生成结果图片"""
    print("=== 开始视频AI识别测试 ===")
    
    # 检查视频文件
    video_path = "../resources/video/VID_20250617094821.wmv"
    if not Path(video_path).exists():
        video_path = "../resources/video/VID_20250617094821.mp4"
    
    if not Path(video_path).exists():
        print("❌ 视频文件不存在")
        return
    
    print(f"✅ 找到视频文件: {video_path}")
    
    # 加载YOLO模型
    if not YOLO_AVAILABLE:
        print("❌ YOLO不可用，无法进行检测")
        return
    
    try:
        model = YOLO('yolo11n.pt')
        print("✅ YOLO模型加载成功")
    except Exception as e:
        print(f"❌ YOLO模型加载失败: {e}")
        return
    
    # 打开视频
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print("❌ 无法打开视频文件")
        return
    
    # 获取视频信息
    fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    
    print(f"✅ 视频信息 - 分辨率: {width}x{height}, FPS: {fps}, 总帧数: {total_frames}")
    
    # 创建输出目录
    output_dir = "detection_results"
    os.makedirs(output_dir, exist_ok=True)
    
    # 处理视频帧
    frame_count = 0
    total_detections = 0
    person_count = 0
    vehicle_count = 0
    risk_count = 0
    
    # 每隔30帧处理一次（约1秒间隔）
    frame_interval = 30
    max_frames = 10  # 最多处理10帧以节省时间
    
    print(f"开始处理视频帧（每{frame_interval}帧处理一次，最多{max_frames}帧）...")
    
    while frame_count < max_frames:
        # 跳到目标帧
        cap.set(cv2.CAP_PROP_POS_FRAMES, frame_count * frame_interval)
        
        ret, frame = cap.read()
        if not ret:
            break
        
        current_frame_num = int(cap.get(cv2.CAP_PROP_POS_FRAMES))
        print(f"处理第 {current_frame_num} 帧...")
        
        # 进行目标检测
        detections = detect_objects_yolo(frame, model)
        
        if detections:
            print(f"  检测到 {len(detections)} 个目标:")
            for det in detections:
                print(f"    - {det['class']}: {det['confidence']:.2%} {'(风险)' if det['risk'] else ''}")
                
                # 统计
                total_detections += 1
                if det['class'] == 'person':
                    person_count += 1
                elif det['class'] in ['bicycle', 'motorcycle', 'car', 'truck', 'bus']:
                    vehicle_count += 1
                if det['risk']:
                    risk_count += 1
            
            # 绘制检测框
            result_frame = draw_detection_boxes(frame, detections)
            
            # 保存结果图片
            timestamp = datetime.now().strftime("%H%M%S")
            output_filename = f"{output_dir}/frame_{current_frame_num:06d}_{timestamp}.jpg"
            cv2.imwrite(output_filename, result_frame)
            print(f"  保存结果图片: {output_filename}")
        else:
            print("  未检测到目标")
        
        frame_count += 1
    
    cap.release()
    
    # 输出统计结果
    print("\n=== 检测统计结果 ===")
    print(f"处理帧数: {frame_count}")
    print(f"总检测数: {total_detections}")
    print(f"人员检测: {person_count}")
    print(f"车辆检测: {vehicle_count}")
    print(f"风险警报: {risk_count}")
    print(f"结果图片保存在: {output_dir}/ 目录")
    
    # 如果有检测结果，尝试显示第一张图片
    if total_detections > 0:
        result_files = [f for f in os.listdir(output_dir) if f.endswith('.jpg')]
        if result_files:
            first_result = os.path.join(output_dir, sorted(result_files)[0])
            print(f"\n可以查看检测结果图片: {first_result}")

if __name__ == "__main__":
    test_video_detection() 