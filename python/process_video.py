#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
无人机视频快速处理脚本
简化的视频AI检测工具

使用方法:
python process_video.py input.mp4 output.mp4

可选参数:
python process_video.py input.mp4 output.mp4 --water-areas "x,y,w,h"
"""

import sys
import os
from pathlib import Path
import argparse

# 添加当前目录到路径
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from yolo_detection import DroneVideoDetector

def main():
    if len(sys.argv) < 3:
        print("使用方法:")
        print("  python process_video.py <输入视频> <输出视频>")
        print("")
        print("示例:")
        print("  python process_video.py drone_video.mp4 analyzed_video.mp4")
        print("")
        print("可选参数:")
        print("  --water-areas \"x,y,w,h;x2,y2,w2,h2\"  设置水域区域")
        print("  --model model.pt                     指定YOLO模型文件")
        print("")
        print("完整示例:")
        print("  python process_video.py input.mp4 output.mp4 --water-areas \"100,200,300,150\"")
        return
    
    # 解析参数
    parser = argparse.ArgumentParser(description='无人机视频AI检测')
    parser.add_argument('input', help='输入视频文件')
    parser.add_argument('output', help='输出视频文件') 
    parser.add_argument('--water-areas', type=str, help='水域区域坐标')
    parser.add_argument('--model', type=str, help='YOLO模型文件')
    
    args = parser.parse_args()
    
    # 检查输入文件
    input_path = Path(args.input)
    if not input_path.exists():
        print(f"❌ 错误：输入文件不存在 - {args.input}")
        return
    
    # 解析水域区域
    water_areas = []
    if args.water_areas:
        try:
            for area_str in args.water_areas.split(';'):
                area_str = area_str.strip()
                if area_str:
                    x, y, w, h = map(int, area_str.split(','))
                    water_areas.append((x, y, w, h))
            print(f"✅ 设置了 {len(water_areas)} 个水域区域: {water_areas}")
        except Exception as e:
            print(f"❌ 水域区域格式错误: {e}")
            print("正确格式: 'x,y,width,height' 例如: '100,200,300,150'")
            return
    
    print("=" * 50)
    print("🚁 无人机视频AI检测系统")
    print("=" * 50)
    print(f"📹 输入视频: {args.input}")
    print(f"💾 输出视频: {args.output}")
    
    if args.model:
        print(f"🤖 AI模型: {args.model}")
    else:
        print("🤖 AI模型: yolo11n.pt (默认)")
    
    if water_areas:
        print(f"🌊 水域区域: {len(water_areas)}个")
    
    print("\n⚠️  风险评估规则:")
    print("   • 人数 > 30人 = 高风险")
    print("   • 距离水域 < 2米 = 高风险") 
    print("   • 井盖和车辆仅统计数量")
    print("")
    
    # 创建检测器
    print("🔧 初始化AI检测器...")
    try:
        detector = DroneVideoDetector(model_path=args.model)
    except Exception as e:
        print(f"❌ 初始化失败: {e}")
        return
    
    # 开始处理
    print("🚀 开始处理视频...")
    success = detector.process_video_to_output(args.input, args.output, water_areas)
    
    print("\n" + "=" * 50)
    if success:
        print("✅ 视频处理完成！")
        print(f"📁 输出文件: {args.output}")
        print("")
        print("📊 检测结果包含:")
        print("   • 人员检测框（绿色/红色）")
        print("   • 车辆检测框（蓝色）") 
        print("   • 井盖检测框（黄色）")
        print("   • 实时统计信息")
        print("   • 风险等级标识")
    else:
        print("❌ 视频处理失败")
        print("请检查:")
        print("   • 输入文件是否为有效视频格式")
        print("   • 是否有足够的存储空间")
        print("   • YOLO模型是否正确安装")

if __name__ == '__main__':
    main() 