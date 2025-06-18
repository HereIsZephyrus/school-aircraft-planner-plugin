#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
无人机视频AI检测 - PyCharm运行脚本
简化版本，直接运行即可
"""

import sys
import os
from pathlib import Path

# 添加当前目录到路径
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from yolo_detection import DroneVideoDetector
import logging

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(message)s'
)
logger = logging.getLogger(__name__)

def main():
    """主函数 - 直接在这里设置您的视频路径"""
    
    # ================== 配置区域 ==================
    # 视频路径设置
    VIDEO_PATH = r"F:\git\school-aircraft-planner-plugin\resources\video\VID_20250617094821.wmv"
    OUTPUT_PATH = r"F:\git\school-aircraft-planner-plugin\resources\video\VID_20250617094821_analyzed.mp4"
    
    # 水域区域设置 (如果视频中有水域，设置坐标)
    # 格式: [(x, y, width, height), ...]
    WATER_AREAS = []  # 暂时不设置，如需要请修改为: [(300, 200, 400, 300)]
    
    # 运行设置
    SHOW_REALTIME = True   # 是否显示实时检测窗口
    SAVE_VIDEO = True      # 是否保存检测结果视频
    # =============================================
    
    print("🚁 无人机视频AI检测系统")
    print("=" * 50)
    print(f"📹 输入视频: {Path(VIDEO_PATH).name}")
    print(f"💾 输出视频: {'保存' if SAVE_VIDEO else '不保存'}")
    print(f"👁️  实时显示: {'开启' if SHOW_REALTIME else '关闭'}")
    print(f"🌊 水域区域: {len(WATER_AREAS)}个")
    print()
    print("⚠️  风险规则:")
    print("   • 人数 > 30 = 高风险(红色)")
    print("   • 距水域 < 2米 = 高风险(红色)")
    print("   • 井盖/车辆 = 仅统计")
    print("   • 按ESC或Q键退出")
    print("=" * 50)
    
    # 检查文件
    if not Path(VIDEO_PATH).exists():
        print(f"❌ 错误: 视频文件不存在")
        print(f"   路径: {VIDEO_PATH}")
        return
    
    # 创建检测器
    try:
        print("🔧 初始化AI检测器...")
        detector = DroneVideoDetector()
        print("✅ 检测器初始化成功")
    except Exception as e:
        print(f"❌ 检测器初始化失败: {e}")
        return
    
    # 开始处理
    try:
        print("🚀 开始视频处理...")
        
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
            print("\n✅ 处理完成!")
            if SAVE_VIDEO:
                print(f"📁 输出文件: {Path(OUTPUT_PATH).name}")
        else:
            print("\n❌ 处理失败")
            
    except KeyboardInterrupt:
        print("\n⏹️  用户中断")
    except Exception as e:
        print(f"\n❌ 运行错误: {e}")
    
    print("🔚 程序结束")

if __name__ == '__main__':
    main() 