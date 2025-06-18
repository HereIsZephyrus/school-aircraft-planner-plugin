#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
无人机视频AI识别系统
使用YOLO11进行目标检测，识别人流、井盖、电瓶车、溺水点等
"""

import cv2
import numpy as np
import json
import socket
import time
import threading
import argparse
import logging
import base64
from datetime import datetime
from pathlib import Path
import random

try:
    from ultralytics import YOLO
    YOLO_AVAILABLE = True
except ImportError:
    YOLO_AVAILABLE = False
    print("Warning: ultralytics not available, using simulation mode")

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('yolo_detection.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

class DroneVideoDetector:
    """无人机视频检测器"""
    
    def __init__(self, model_path=None, host='localhost', port=8888):
        self.host = host
        self.port = port
        self.socket = None
        self.running = False
        
        # 检测类别映射
        self.class_mapping = {
            'person': 'person',
            'bicycle': 'bicycle', 
            'motorcycle': 'motorcycle',
            'car': 'car',
            'truck': 'truck',
            'bus': 'bus'
        }
        
        # 风险评估规则
        self.risk_rules = {
            'person': {
                'max_safe_count': 5,
                'risk_areas': [(200, 200, 400, 400)],  # (x, y, w, h) 危险区域
                'confidence_threshold': 0.5
            },
            'bicycle': {
                'max_safe_count': 2,
                'confidence_threshold': 0.6
            },
            'motorcycle': {
                'max_safe_count': 1,
                'confidence_threshold': 0.6
            }
        }
        
        # 初始化YOLO模型
        self.model = None
        if YOLO_AVAILABLE:
            try:
                if model_path and Path(model_path).exists():
                    self.model = YOLO(model_path)
                else:
                    # 使用预训练模型
                    self.model = YOLO('yolo11n.pt')
                logger.info("YOLO模型加载成功")
            except Exception as e:
                logger.error(f"YOLO模型加载失败: {e}")
                self.model = None
        
        # 检测统计
        self.detection_stats = {
            'total_frames': 0,
            'total_detections': 0,
            'person_count': 0,
            'vehicle_count': 0,
            'risk_alerts': 0,
            'start_time': datetime.now()
        }
        
        # 模拟数据生成器（当YOLO不可用时）
        self.simulation_frame_count = 0
    
    def draw_detection_boxes(self, frame, detections):
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
            elif class_name in ['bicycle', 'motorcycle']:
                color = (255, 0, 0)  # 蓝色 - 车辆
            else:
                color = (0, 255, 255)  # 黄色 - 其他
            
            # 绘制检测框
            cv2.rectangle(overlay_frame, (x, y), (x + w, y + h), color, 2)
            
            # 绘制标签
            label = f"{class_name} {confidence:.1%}"
            label_size = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)[0]
            
            # 标签背景
            cv2.rectangle(overlay_frame, 
                         (x, y - label_size[1] - 10), 
                         (x + label_size[0], y), 
                         color, -1)
            
            # 标签文字
            cv2.putText(overlay_frame, label, (x, y - 5), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
            
            # 风险标识
            if is_risk:
                cv2.putText(overlay_frame, "RISK!", (x + w - 50, y + 20), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)
        
        return overlay_frame
    
    def frame_to_base64(self, frame):
        """将视频帧转换为base64字符串"""
        try:
            # 将帧编码为JPEG
            _, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 80])
            # 转换为base64
            frame_base64 = base64.b64encode(buffer).decode('utf-8')
            return frame_base64
        except Exception as e:
            logger.error(f"帧转换base64失败: {e}")
            return None
        
    def connect_to_qt(self):
        """连接到Qt应用程序（作为TCP客户端）"""
        max_retries = 10
        retry_delay = 2
        
        for attempt in range(max_retries):
            try:
                self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.socket.connect((self.host, self.port))
                logger.info(f"成功连接到Qt应用程序 {self.host}:{self.port} (尝试 {attempt + 1}/{max_retries})")
                return True
            except Exception as e:
                if attempt < max_retries - 1:
                    logger.info(f"连接失败，{retry_delay}秒后重试... (尝试 {attempt + 1}/{max_retries}): {e}")
                    time.sleep(retry_delay)
                else:
                    logger.error(f"所有连接尝试都失败了: {e}")
                    return False
        return False
    
    def send_detection_results(self, detections, frame=None):
        """发送检测结果到Qt应用程序"""
        if not self.socket:
            return False
            
        try:
            # 创建可序列化的统计数据副本
            serializable_stats = self.detection_stats.copy()
            if 'start_time' in serializable_stats:
                serializable_stats['start_time'] = serializable_stats['start_time'].isoformat()
            
            data = {
                'timestamp': datetime.now().isoformat(),
                'frame_id': self.detection_stats['total_frames'],
                'detections': detections,
                'stats': serializable_stats
            }
            
            # 如果有帧数据，添加带检测框的图像
            if frame is not None and len(detections) > 0:
                overlay_frame = self.draw_detection_boxes(frame, detections)
                frame_base64 = self.frame_to_base64(overlay_frame)
                if frame_base64:
                    data['frame_image'] = frame_base64
                    logger.debug(f"添加检测框图像，大小: {len(frame_base64)} 字符")
            
            json_data = json.dumps(data) + '\n'
            self.socket.send(json_data.encode('utf-8'))
            return True
        except Exception as e:
            logger.error(f"发送检测结果失败: {e}")
            return False
    
    def detect_objects_yolo(self, frame):
        """使用YOLO进行目标检测"""
        if not self.model:
            return self.generate_simulation_detections(frame)
        
        try:
            results = self.model(frame, verbose=False)
            detections = []
            
            logger.debug(f"YOLO结果数量: {len(results)}")
            
            for result in results:
                boxes = result.boxes
                logger.debug(f"检测框数量: {len(boxes) if boxes is not None else 0}")
                if boxes is not None:
                    for box in boxes:
                        # 获取检测信息
                        x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                        confidence = box.conf[0].cpu().numpy()
                        class_id = int(box.cls[0].cpu().numpy())
                        
                        # 获取类别名称
                        class_name = self.model.names[class_id]
                        
                        # 计算边界框
                        x, y, w, h = int(x1), int(y1), int(x2-x1), int(y2-y1)
                        
                        # 评估风险
                        is_risk, risk_level = self.assess_risk(class_name, x, y, w, h, confidence)
                        
                        detection = {
                            'class': class_name,
                            'confidence': float(confidence),
                            'x': x,
                            'y': y,
                            'width': w,
                            'height': h,
                            'risk': is_risk,
                            'risk_level': risk_level
                        }
                        
                        detections.append(detection)
                        
                        # 更新统计
                        if class_name == 'person':
                            self.detection_stats['person_count'] += 1
                        elif class_name in ['bicycle', 'motorcycle', 'car', 'truck', 'bus']:
                            self.detection_stats['vehicle_count'] += 1
                        
                        if is_risk:
                            self.detection_stats['risk_alerts'] += 1
            
            self.detection_stats['total_detections'] += len(detections)
            return detections
            
        except Exception as e:
            logger.error(f"YOLO检测失败: {e}")
            return self.generate_simulation_detections(frame)
    
    def generate_simulation_detections(self, frame):
        """生成模拟检测结果"""
        detections = []
        self.simulation_frame_count += 1
        
        # 模拟不同的检测场景
        frame_cycle = self.simulation_frame_count % 200
        
        # 模拟人员检测
        if frame_cycle < 50:
            num_persons = random.randint(1, 3)
            for i in range(num_persons):
                x = random.randint(50, 500)
                y = random.randint(100, 300)
                confidence = random.uniform(0.7, 0.95)
                
                is_risk, risk_level = self.assess_risk('person', x, y, 50, 100, confidence)
                
                detections.append({
                    'class': 'person',
                    'confidence': confidence,
                    'x': x,
                    'y': y,
                    'width': 50,
                    'height': 100,
                    'risk': is_risk,
                    'risk_level': risk_level
                })
                
                self.detection_stats['person_count'] += 1
                if is_risk:
                    self.detection_stats['risk_alerts'] += 1
        
        # 模拟井盖检测
        if frame_cycle % 30 == 0:
            detections.append({
                'class': 'manhole',
                'confidence': random.uniform(0.8, 0.95),
                'x': random.randint(200, 400),
                'y': random.randint(300, 400),
                'width': 60,
                'height': 60,
                'risk': False,
                'risk_level': '低'
            })
        
        # 模拟电瓶车检测
        if 80 < frame_cycle < 120:
            confidence = random.uniform(0.75, 0.9)
            x = random.randint(100, 400)
            y = random.randint(200, 350)
            
            is_risk, risk_level = self.assess_risk('bicycle', x, y, 80, 40, confidence)
            
            detections.append({
                'class': 'bicycle',
                'confidence': confidence,
                'x': x,
                'y': y,
                'width': 80,
                'height': 40,
                'risk': is_risk,
                'risk_level': risk_level
            })
            
            self.detection_stats['vehicle_count'] += 1
            if is_risk:
                self.detection_stats['risk_alerts'] += 1
        
        # 模拟溺水点检测（高风险）
        if frame_cycle > 150 and frame_cycle % 50 == 0:
            detections.append({
                'class': 'drowning',
                'confidence': random.uniform(0.85, 0.95),
                'x': random.randint(150, 350),
                'y': random.randint(250, 350),
                'width': 30,
                'height': 30,
                'risk': True,
                'risk_level': '高'
            })
            
            self.detection_stats['risk_alerts'] += 1
        
        self.detection_stats['total_detections'] += len(detections)
        return detections
    
    def assess_risk(self, class_name, x, y, w, h, confidence):
        """评估检测目标的风险等级"""
        is_risk = False
        risk_level = '低'
        
        if class_name in self.risk_rules:
            rule = self.risk_rules[class_name]
            
            # 置信度检查
            if confidence < rule['confidence_threshold']:
                return False, '低'
            
            # 特殊区域检查
            if 'risk_areas' in rule:
                for risk_area in rule['risk_areas']:
                    rx, ry, rw, rh = risk_area
                    # 检查是否在风险区域内
                    if (x + w > rx and x < rx + rw and 
                        y + h > ry and y < ry + rh):
                        is_risk = True
                        risk_level = '高'
                        break
            
            # 数量检查
            if class_name == 'person' and self.detection_stats['person_count'] > rule['max_safe_count']:
                is_risk = True
                risk_level = '中' if risk_level == '低' else risk_level
        
        # 特殊类别的风险评估
        if class_name == 'drowning':
            is_risk = True
            risk_level = '高'
        elif class_name in ['motorcycle', 'car', 'truck']:
            if confidence > 0.8:
                is_risk = True
                risk_level = '中'
        
        return is_risk, risk_level
    
    def process_video_stream(self, source='simulation', file_path=None):
        """处理视频流"""
        logger.info(f"开始处理视频流: {source}")
        
        # 连接到Qt应用程序
        if not self.connect_to_qt():
            logger.error("无法连接到Qt应用程序，退出")
            return
        
        self.running = True
        cap = None
        
        try:
            # 根据源类型初始化视频捕获
            if source == 'file' and file_path:
                if not Path(file_path).exists():
                    logger.error(f"视频文件不存在: {file_path}")
                    return
                
                cap = cv2.VideoCapture(file_path)
                if not cap.isOpened():
                    logger.error(f"无法打开视频文件: {file_path}")
                    return
                
                fps = cap.get(cv2.CAP_PROP_FPS)
                total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
                logger.info(f"视频文件信息 - FPS: {fps}, 总帧数: {total_frames}")
                
                frame_delay = 1.0 / fps if fps > 0 else 1.0 / 30
                
            elif source == 'camera':
                cap = cv2.VideoCapture(0)  # 默认摄像头
                if not cap.isOpened():
                    logger.error("无法打开摄像头")
                    return
                frame_delay = 1.0 / 30
                
            else:  # simulation
                frame_delay = 1.0 / 30
            
            while self.running:
                if source == 'file' and cap:
                    # 从视频文件读取帧
                    ret, frame = cap.read()
                    if not ret:
                        # 视频结束，重新开始
                        cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                        ret, frame = cap.read()
                        if not ret:
                            logger.error("无法从视频文件读取帧")
                            break
                    
                elif source == 'camera' and cap:
                    # 从摄像头读取帧
                    ret, frame = cap.read()
                    if not ret:
                        logger.error("无法从摄像头读取帧")
                        break
                        
                else:  # simulation
                    # 创建模拟帧
                    frame = np.zeros((480, 640, 3), dtype=np.uint8)
                    frame.fill(50)  # 灰色背景
                    
                    # 在帧上绘制一些内容
                    cv2.putText(frame, f"Frame: {self.detection_stats['total_frames']}", 
                               (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
                    cv2.putText(frame, datetime.now().strftime("%H:%M:%S"), 
                               (10, 70), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
                
                # 目标检测
                detections = self.detect_objects_yolo(frame)
                
                # 发送检测结果（包含帧数据）
                if detections:
                    self.send_detection_results(detections, frame)
                    logger.info(f"检测到 {len(detections)} 个目标")
                else:
                    # 即使没有检测结果，也发送空结果以保持连接
                    self.send_detection_results([], frame)
                
                self.detection_stats['total_frames'] += 1
                
                # 控制帧率
                time.sleep(frame_delay)
                
        except KeyboardInterrupt:
            logger.info("接收到中断信号，停止检测")
        except Exception as e:
            logger.error(f"视频处理出错: {e}")
        finally:
            if cap:
                cap.release()
            self.cleanup()
    
    def cleanup(self):
        """清理资源"""
        self.running = False
        if self.socket:
            self.socket.close()
        
        # 打印统计信息
        runtime = datetime.now() - self.detection_stats['start_time']
        logger.info("检测统计信息:")
        logger.info(f"  运行时间: {runtime}")
        logger.info(f"  总帧数: {self.detection_stats['total_frames']}")
        logger.info(f"  总检测数: {self.detection_stats['total_detections']}")
        logger.info(f"  人员检测: {self.detection_stats['person_count']}")
        logger.info(f"  车辆检测: {self.detection_stats['vehicle_count']}")
        logger.info(f"  风险警报: {self.detection_stats['risk_alerts']}")
        
        logger.info("资源清理完成")

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='无人机视频AI识别系统')
    parser.add_argument('--model', type=str, help='YOLO模型路径')
    parser.add_argument('--host', type=str, default='localhost', 
                       help='Qt应用程序主机地址')
    parser.add_argument('--port', type=int, default=8888, 
                       help='TCP通信端口')
    parser.add_argument('--source', type=str, default='simulation',
                       choices=['simulation', 'camera', 'file'],
                       help='视频源类型')
    parser.add_argument('--file', '--file_path', type=str, dest='file_path',
                       help='视频文件路径（当source为file时使用）')
    parser.add_argument('--verbose', action='store_true',
                       help='详细日志输出')
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    # 参数验证
    if args.source == 'file' and not args.file_path:
        logger.error("当视频源为文件时，必须指定--file_path参数")
        return
    
    logger.info("启动无人机视频AI识别系统")
    logger.info(f"YOLO可用: {YOLO_AVAILABLE}")
    logger.info(f"连接参数: {args.host}:{args.port}")
    logger.info(f"视频源: {args.source}")
    if args.file_path:
        logger.info(f"视频文件: {args.file_path}")
    
    # 创建检测器
    detector = DroneVideoDetector(
        model_path=args.model,
        host=args.host,
        port=args.port
    )
    
    # 开始处理
    try:
        detector.process_video_stream(args.source, args.file_path)
    except Exception as e:
        logger.error(f"系统运行出错: {e}")
    
    logger.info("系统已退出")

if __name__ == '__main__':
    main() 