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
        
        # 更新的风险评估规则
        self.risk_rules = {
            'person': {
                'max_safe_count': 20,  # 超过20人为高风险
                'water_safe_distance': 2.0,  # 水域2米范围内有人为高风险
                'confidence_threshold': 0.3  # 降低阈值，提高检测敏感度
            },
            'bicycle': {
                'count_only': True,  # 仅统计数量
                'confidence_threshold': 0.3  # 降低阈值
            },
            'motorcycle': {
                'count_only': True,  # 仅统计数量
                'confidence_threshold': 0.3  # 降低阈值
            },
            'car': {
                'count_only': True,  # 仅统计数量
                'confidence_threshold': 0.4
            },
            'truck': {
                'count_only': True,  # 仅统计数量
                'confidence_threshold': 0.4
            },
            'bus': {
                'count_only': True,  # 仅统计数量
                'confidence_threshold': 0.4
            },
            'manhole': {
                'count_only': True,  # 仅统计数量
                'confidence_threshold': 0.4
            }
        }
        
        # 水域区域定义 (可根据实际情况调整坐标)
        self.water_areas = [
            # (x, y, width, height) - 可以定义多个水域区域
            # 示例水域区域，您可以根据实际视频内容调整
        ]
        
        # 初始化YOLO模型
        self.model = None
        if YOLO_AVAILABLE:
            try:
                if model_path and Path(model_path).exists():
                    self.model = YOLO(model_path)
                else:
                    # 尝试使用更精确的模型，如果不存在则降级到nano版本
                    try:
                        self.model = YOLO('yolo11s.pt')  # 使用small版本，更精确
                        logger.info("YOLO模型加载成功 (yolo11s.pt - 精确版)")
                    except:
                        self.model = YOLO('yolo11n.pt')  # 降级到nano版本
                        logger.info("YOLO模型加载成功 (yolo11n.pt - 快速版)")
            except Exception as e:
                logger.error(f"YOLO模型加载失败: {e}")
                self.model = None
        
        # 检测统计
        self.detection_stats = {
            'total_frames': 0,
            'total_detections': 0,
            'person_count': 0,
            'bicycle_count': 0,
            'motorcycle_count': 0,
            'car_count': 0,
            'truck_count': 0,
            'bus_count': 0,
            'manhole_count': 0,
            'risk_alerts': 0,
            'start_time': datetime.now()
        }
        
        # 当前帧的检测计数
        self.current_frame_counts = {
            'person': 0,
            'bicycle': 0,
            'motorcycle': 0,
            'car': 0,
            'truck': 0,
            'bus': 0,
            'manhole': 0
        }
        
        # 模拟数据生成器（当YOLO不可用时）
        self.simulation_frame_count = 0

    def calculate_distance(self, point1, point2):
        """计算两点之间的距离（像素距离）"""
        return np.sqrt((point1[0] - point2[0])**2 + (point1[1] - point2[1])**2)
    
    def is_near_water(self, person_center):
        """检查人员是否在水域附近"""
        if not self.water_areas:
            return False
            
        px, py = person_center
        
        for water_area in self.water_areas:
            wx, wy, ww, wh = water_area
            
            # 计算人员中心点到水域边界的最短距离
            closest_x = max(wx, min(px, wx + ww))
            closest_y = max(wy, min(py, wy + wh))
            
            distance_pixels = self.calculate_distance((px, py), (closest_x, closest_y))
            
            # 假设每像素代表约0.02米（可根据实际情况调整）
            distance_meters = distance_pixels * 0.02
            
            if distance_meters <= self.risk_rules['person']['water_safe_distance']:
                return True
                
        return False
    
    def draw_detection_boxes(self, frame, detections):
        """在视频帧上绘制检测框和风险信息"""
        overlay_frame = frame.copy()
        
        # 重置当前帧计数
        self.current_frame_counts = {k: 0 for k in self.current_frame_counts.keys()}
        
        # 绘制检测框
        for detection in detections:
            x = int(detection['x'])
            y = int(detection['y'])
            w = int(detection['width'])
            h = int(detection['height'])
            confidence = detection['confidence']
            class_name = detection['class']
            is_risk = detection.get('risk', False)
            risk_level = detection.get('risk_level', '低')
            
            # 更新当前帧计数
            if class_name in self.current_frame_counts:
                self.current_frame_counts[class_name] += 1
            
            # 选择颜色 (BGR格式)
            if is_risk:
                if risk_level == '高':
                    color = (0, 0, 255)  # 红色 - 高风险
                else:
                    color = (0, 165, 255)  # 橙色 - 中风险
            elif class_name == 'person':
                color = (0, 255, 0)  # 绿色 - 人员
            elif class_name == 'bicycle':
                color = (255, 0, 0)  # 蓝色 - 自行车
            elif class_name == 'motorcycle':
                color = (255, 0, 128)  # 紫蓝色 - 摩托车/电瓶车
            elif class_name in ['car', 'truck', 'bus']:
                color = (0, 128, 255)  # 橙色 - 汽车类
            elif class_name == 'manhole':
                color = (0, 255, 255)  # 黄色 - 井盖
            else:
                color = (128, 128, 128)  # 灰色 - 其他
            
            # 绘制检测框
            thickness = 3 if is_risk else 2
            cv2.rectangle(overlay_frame, (x, y), (x + w, y + h), color, thickness)
            
            # 绘制标签
            label = f"{class_name}"
            if is_risk:
                label += f" [{risk_level}风险]"
                
            label_size = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.6, 2)[0]
            
            # 标签背景
            cv2.rectangle(overlay_frame, 
                         (x, y - label_size[1] - 15), 
                         (x + label_size[0] + 10, y), 
                         color, -1)
            
            # 标签文字
            cv2.putText(overlay_frame, label, (x + 5, y - 8), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        
        # 绘制水域区域（如果定义了）
        for water_area in self.water_areas:
            wx, wy, ww, wh = water_area
            cv2.rectangle(overlay_frame, (wx, wy), (wx + ww, wy + wh), (255, 255, 0), 2)
            cv2.putText(overlay_frame, "Water Area", (wx, wy - 10), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 2)
        
        # 绘制统计信息和风险警告
        self.draw_statistics_overlay(overlay_frame)
        
        return overlay_frame
    
    def draw_statistics_overlay(self, frame):
        """在帧上绘制统计信息"""
        h, w = frame.shape[:2]
        
        # 背景框
        overlay = frame.copy()
        cv2.rectangle(overlay, (10, 10), (450, 280), (0, 0, 0), -1)
        cv2.addWeighted(overlay, 0.7, frame, 0.3, 0, frame)
        
        # 统计信息
        stats_text = [
            f"Frame: {self.detection_stats['total_frames']}",
            f"People: {self.current_frame_counts['person']}",
            f"Bicycles: {self.current_frame_counts['bicycle']}",
            f"Motorcycles: {self.current_frame_counts['motorcycle']}",
            f"Cars: {self.current_frame_counts['car']}",
            f"Trucks: {self.current_frame_counts['truck']}",
            f"Buses: {self.current_frame_counts['bus']}",
            f"Manholes: {self.current_frame_counts['manhole']}",
            f"Total Risks: {self.detection_stats['risk_alerts']}"
        ]
        
        for i, text in enumerate(stats_text):
            y_pos = 35 + i * 25
            color = (255, 255, 255)
            
            # 人员数量超过20时用红色显示
            if "People:" in text and self.current_frame_counts['person'] > 20:
                color = (0, 0, 255)
                
            cv2.putText(frame, text, (20, y_pos), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
        
        # 高风险警告
        if self.current_frame_counts['person'] > 20:
            cv2.putText(frame, "HIGH RISK: >20 PEOPLE!", (20, h - 60), 
                       cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 0, 255), 3)
    
    def process_video_to_output(self, input_path, output_path, water_areas=None):
        """处理视频文件并输出带检测框的新视频"""
        if not Path(input_path).exists():
            logger.error(f"输入视频文件不存在: {input_path}")
            return False
        
        # 设置水域区域
        if water_areas:
            self.water_areas = water_areas
            logger.info(f"设置了 {len(water_areas)} 个水域区域")
        
        # 打开输入视频
        cap = cv2.VideoCapture(input_path)
        if not cap.isOpened():
            logger.error(f"无法打开输入视频: {input_path}")
            return False
        
        # 获取视频信息
        fps = int(cap.get(cv2.CAP_PROP_FPS))
        width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        
        logger.info(f"视频信息: {width}x{height}, {fps}fps, {total_frames}帧")
        
        # 创建输出视频编写器
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))
        
        if not out.isOpened():
            logger.error(f"无法创建输出视频: {output_path}")
            cap.release()
            return False
        
        logger.info(f"开始处理视频: {input_path} -> {output_path}")
        
        try:
            frame_count = 0
            while True:
                ret, frame = cap.read()
                if not ret:
                    break
                
                # 进行目标检测
                detections = self.detect_objects_yolo(frame)
                
                # 绘制检测框和信息
                output_frame = self.draw_detection_boxes(frame, detections)
                
                # 写入输出视频
                out.write(output_frame)
                
                frame_count += 1
                self.detection_stats['total_frames'] = frame_count
                
                # 显示进度
                if frame_count % 30 == 0:
                    progress = (frame_count / total_frames) * 100
                    logger.info(f"处理进度: {frame_count}/{total_frames} ({progress:.1f}%)")
                    
        except Exception as e:
            logger.error(f"处理视频时出错: {e}")
            return False
        finally:
            cap.release()
            out.release()
        
        logger.info(f"视频处理完成！输出文件: {output_path}")
        self.print_final_statistics()
        return True
    
    def process_video_with_realtime_display(self, input_path, output_path=None, water_areas=None):
        """处理视频并实时显示检测结果"""
        if not Path(input_path).exists():
            logger.error(f"输入视频文件不存在: {input_path}")
            return False
        
        # 设置水域区域
        if water_areas:
            self.water_areas = water_areas
            logger.info(f"设置了 {len(water_areas)} 个水域区域")
        
        # 打开输入视频
        cap = cv2.VideoCapture(input_path)
        if not cap.isOpened():
            logger.error(f"无法打开输入视频: {input_path}")
            return False
        
        # 获取视频信息
        fps = int(cap.get(cv2.CAP_PROP_FPS))
        width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        
        logger.info(f"视频信息: {width}x{height}, {fps}fps, {total_frames}帧")
        
        # 如果指定了输出路径，创建视频编写器
        out = None
        if output_path:
            fourcc = cv2.VideoWriter_fourcc(*'mp4v')
            out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))
            if not out.isOpened():
                logger.warning(f"无法创建输出视频: {output_path}")
                out = None
        
        # 创建显示窗口
        window_name = "无人机视频AI检测 - 按ESC或Q退出"
        cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
        cv2.resizeWindow(window_name, 1280, 720)  # 设置窗口大小
        
        logger.info("开始实时检测，按ESC或Q键退出...")
        
        try:
            frame_count = 0
            while True:
                ret, frame = cap.read()
                if not ret:
                    logger.info("视频播放完毕")
                    break
                
                # 进行目标检测
                detections = self.detect_objects_yolo(frame)
                
                # 绘制检测框和信息
                output_frame = self.draw_detection_boxes(frame, detections)
                
                # 如果有输出视频，写入
                if out:
                    out.write(output_frame)
                
                # 显示检测结果
                cv2.imshow(window_name, output_frame)
                
                frame_count += 1
                self.detection_stats['total_frames'] = frame_count
                
                # 显示进度
                if frame_count % 30 == 0:
                    progress = (frame_count / total_frames) * 100
                    logger.info(f"处理进度: {frame_count}/{total_frames} ({progress:.1f}%)")
                
                # 检查按键退出
                key = cv2.waitKey(1) & 0xFF
                if key == 27 or key == ord('q') or key == ord('Q'):  # ESC或Q键退出
                    logger.info("用户按键退出")
                    break
                    
        except KeyboardInterrupt:
            logger.info("接收到中断信号，停止检测")
        except Exception as e:
            logger.error(f"处理视频时出错: {e}")
            return False
        finally:
            cap.release()
            if out:
                out.release()
            cv2.destroyAllWindows()
        
        if output_path and out:
            logger.info(f"视频处理完成！输出文件: {output_path}")
        
        self.print_final_statistics()
        return True
    
    def print_final_statistics(self):
        """打印最终统计信息"""
        logger.info("=== 检测统计报告 ===")
        logger.info(f"总处理帧数: {self.detection_stats['total_frames']}")
        logger.info(f"总检测对象: {self.detection_stats['total_detections']}")
        logger.info(f"人员检测总数: {self.detection_stats['person_count']}")
        logger.info(f"自行车检测总数: {self.detection_stats['bicycle_count']}")
        logger.info(f"摩托车/电瓶车检测总数: {self.detection_stats['motorcycle_count']}")
        logger.info(f"汽车检测总数: {self.detection_stats['car_count']}")
        logger.info(f"卡车检测总数: {self.detection_stats['truck_count']}")
        logger.info(f"公交车检测总数: {self.detection_stats['bus_count']}")
        logger.info(f"井盖检测总数: {self.detection_stats['manhole_count']}")
        logger.info(f"风险警报总数: {self.detection_stats['risk_alerts']}")
        
        runtime = datetime.now() - self.detection_stats['start_time']
        logger.info(f"总处理时间: {runtime}")

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
            
            # 重置当前帧检测计数
            frame_counts = {'person': 0, 'bicycle': 0, 'motorcycle': 0, 'car': 0, 'truck': 0, 'bus': 0, 'manhole': 0}
            
            for result in results:
                boxes = result.boxes
                if boxes is not None:
                    for box in boxes:
                        # 获取检测信息
                        x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                        confidence = box.conf[0].cpu().numpy()
                        class_id = int(box.cls[0].cpu().numpy())
                        
                        # 获取类别名称
                        class_name = self.model.names[class_id]
                        
                        # 映射为我们关心的类别
                        if class_name == 'person':
                            mapped_class = 'person'
                        elif class_name in ['bicycle']:
                            mapped_class = 'bicycle'
                        elif class_name in ['motorcycle', 'motorbike']:
                            mapped_class = 'motorcycle'
                        elif class_name in ['car']:
                            mapped_class = 'car'
                        elif class_name in ['truck']:
                            mapped_class = 'truck'
                        elif class_name in ['bus']:
                            mapped_class = 'bus'
                        # 注意：YOLO默认模型可能没有井盖类别，这里作为示例
                        elif class_name in ['manhole', 'sewer']:
                            mapped_class = 'manhole'
                        else:
                            continue  # 跳过其他类别
                        
                        # 计算边界框
                        x, y, w, h = int(x1), int(y1), int(x2-x1), int(y2-y1)
                        center = (x + w//2, y + h//2)
                        
                        # 更新帧内计数
                        frame_counts[mapped_class] += 1
                        
                        # 评估风险
                        is_risk, risk_level = self.assess_risk_new(mapped_class, center, confidence, frame_counts)
                        
                        detection = {
                            'class': mapped_class,
                            'confidence': float(confidence),
                            'x': x,
                            'y': y,
                            'width': w,
                            'height': h,
                            'risk': is_risk,
                            'risk_level': risk_level
                        }
                        
                        detections.append(detection)
                        
                        # 更新总体统计
                        self.detection_stats[f'{mapped_class}_count'] += 1
                        
                        if is_risk:
                            self.detection_stats['risk_alerts'] += 1
            
            self.detection_stats['total_detections'] += len(detections)
            return detections
            
        except Exception as e:
            logger.error(f"YOLO检测失败: {e}")
            return []
    
    def assess_risk_new(self, class_name, center, confidence, frame_counts):
        """新的风险评估方法"""
        is_risk = False
        risk_level = '低'
        
        if class_name in self.risk_rules:
            rule = self.risk_rules[class_name]
            
            # 置信度检查
            if confidence < rule['confidence_threshold']:
                return False, '低'
            
            # 人员特殊风险评估
            if class_name == 'person':
                # 检查人数是否超过30
                if frame_counts['person'] > rule['max_safe_count']:
                    is_risk = True
                    risk_level = '高'
                
                # 检查是否靠近水域
                if self.is_near_water(center):
                    is_risk = True
                    risk_level = '高'
            
            # 对于仅统计数量的类别，不标记为风险
            elif rule.get('count_only', False):
                is_risk = False
                risk_level = '低'
        
        return is_risk, risk_level
    
    def generate_simulation_detections(self, frame):
     
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
                
                is_risk, risk_level = self.assess_risk_new('person', (x, y), confidence, {'person': 0})
                
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
            
            is_risk, risk_level = self.assess_risk_new('bicycle', (x, y), confidence, {'bicycle': 0})
            
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
            
            self.detection_stats['bicycle_count'] += 1
            if is_risk:
                self.detection_stats['risk_alerts'] += 1
        
       
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
        logger.info(f"  车辆检测: {self.detection_stats['bicycle_count']}")
        logger.info(f"  风险警报: {self.detection_stats['risk_alerts']}")
        
        logger.info("资源清理完成")

def main():
    """主函数"""
    # 直接在代码中设置视频路径和参数
    VIDEO_PATH = r"F:\git\school-aircraft-planner-plugin\resources\video\VID_20250617094821.wmv"
    OUTPUT_PATH = r"F:\git\school-aircraft-planner-plugin\resources\video\VID_20250617094821_analyzed2.mp4"
    
    # 可选：设置水域区域 (x, y, width, height)
    # 如果您知道视频中水域的位置，可以取消注释并设置坐标
    # WATER_AREAS = [(300, 200, 400, 300)]  # 示例坐标，需要根据实际视频调整
    WATER_AREAS = []  # 暂时不设置水域区域
    
    # 运行模式设置
    SHOW_REALTIME = True  # 是否显示实时检测窗口
    SAVE_VIDEO = True     # 是否保存输出视频
    
    logger.info("=" * 60)
    logger.info("🚁 无人机视频AI检测系统 - PyCharm运行模式")
    logger.info("=" * 60)
    logger.info(f"📹 输入视频: {VIDEO_PATH}")
    logger.info(f"💾 输出视频: {OUTPUT_PATH if SAVE_VIDEO else '不保存'}")
    logger.info(f"👁️  实时显示: {'开启' if SHOW_REALTIME else '关闭'}")
    logger.info(f"🌊 水域区域: {len(WATER_AREAS)}个")
    logger.info(f"🤖 YOLO可用: {YOLO_AVAILABLE}")
    
    print("\n⚠️  风险评估规则:")
    print("   • 人数 > 30人 = 高风险(红色)")
    print("   • 距离水域 < 2米 = 高风险(红色)")
    print("   • 井盖和车辆仅统计数量")
    print("   • 按ESC或Q键退出实时显示")
    print()
    
    # 检查视频文件是否存在
    if not Path(VIDEO_PATH).exists():
        logger.error(f"❌ 视频文件不存在: {VIDEO_PATH}")
        return
    
    # 创建检测器
    try:
        logger.info("🔧 初始化AI检测器...")
        detector = DroneVideoDetector()
    except Exception as e:
        logger.error(f"❌ 初始化检测器失败: {e}")
        return
    
    # 开始处理
    try:
        if SHOW_REALTIME:
            # 实时显示模式
            output_path = OUTPUT_PATH if SAVE_VIDEO else None
            success = detector.process_video_with_realtime_display(
                VIDEO_PATH, output_path, WATER_AREAS
            )
        else:
            # 后台处理模式
            success = detector.process_video_to_output(
                VIDEO_PATH, OUTPUT_PATH, WATER_AREAS
            )
        
        if success:
            logger.info("✅ 处理完成！")
            if SAVE_VIDEO:
                logger.info(f"📁 输出文件: {OUTPUT_PATH}")
        else:
            logger.error("❌ 处理失败")
            
    except Exception as e:
        logger.error(f"❌ 系统运行出错: {e}")
    
    logger.info("🔚 系统已退出")

def main_with_args():
    """命令行模式的主函数"""
    parser = argparse.ArgumentParser(description='无人机视频AI识别系统')
    parser.add_argument('--model', type=str, help='YOLO模型路径')
    parser.add_argument('--host', type=str, default='localhost', 
                       help='Qt应用程序主机地址')
    parser.add_argument('--port', type=int, default=8888, 
                       help='TCP通信端口')
    parser.add_argument('--source', type=str, default='simulation',
                       choices=['simulation', 'camera', 'file', 'process_video'],
                       help='视频源类型')
    parser.add_argument('--file', '--file_path', type=str, dest='file_path',
                       help='视频文件路径（当source为file时使用）')
    parser.add_argument('--input', type=str, help='输入视频文件路径')
    parser.add_argument('--output', type=str, help='输出视频文件路径')
    parser.add_argument('--water-areas', type=str, help='水域区域坐标，格式: "x1,y1,w1,h1;x2,y2,w2,h2"')
    parser.add_argument('--verbose', action='store_true',
                       help='详细日志输出')
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    # 处理视频文件模式
    if args.source == 'process_video':
        if not args.input or not args.output:
            logger.error("处理视频模式需要指定 --input 和 --output 参数")
            return
        
        # 解析水域区域
        water_areas = []
        if args.water_areas:
            try:
                for area_str in args.water_areas.split(';'):
                    x, y, w, h = map(int, area_str.split(','))
                    water_areas.append((x, y, w, h))
                logger.info(f"解析到 {len(water_areas)} 个水域区域")
            except Exception as e:
                logger.error(f"解析水域区域失败: {e}")
                return
        
        # 创建检测器并处理视频
        detector = DroneVideoDetector(model_path=args.model)
        success = detector.process_video_to_output(args.input, args.output, water_areas)
        
        if success:
            logger.info("视频处理成功完成！")
        else:
            logger.error("视频处理失败！")
        return
    
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
    # 检查是否有命令行参数
    import sys
    if len(sys.argv) > 1:
        # 有命令行参数，使用原始的命令行模式
        main_with_args()
    else:
        # 没有命令行参数，使用PyCharm直接运行模式
        main() 