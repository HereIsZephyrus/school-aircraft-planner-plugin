#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
无人机视频AI识别系统启动脚本
简化启动流程和配置管理
"""

import os
import sys
import subprocess
import argparse
import json
from pathlib import Path

def check_dependencies():
    """检查依赖是否安装"""
    required_packages = [
        'cv2',
        'numpy', 
        'ultralytics'
    ]
    
    missing_packages = []
    
    for package in required_packages:
        try:
            __import__(package)
        except ImportError:
            missing_packages.append(package)
    
    if missing_packages:
        print(f"缺少依赖包: {', '.join(missing_packages)}")
        print("请运行: pip install -r requirements.txt")
        return False
    
    return True

def setup_environment():
    """设置运行环境"""
    script_dir = Path(__file__).parent
    
    # 确保日志目录存在
    log_dir = script_dir / "logs"
    log_dir.mkdir(exist_ok=True)
    
    # 确保模型目录存在
    model_dir = script_dir / "models"
    model_dir.mkdir(exist_ok=True)
    
    return script_dir

def load_config():
    """加载配置文件"""
    script_dir = Path(__file__).parent
    config_file = script_dir / "config.json"
    
    default_config = {
        "host": "localhost",
        "port": 8888,
        "model_path": "",
        "video_source": "simulation",
        "log_level": "INFO",
        "detection_settings": {
            "confidence_threshold": 0.5,
            "nms_threshold": 0.4,
            "max_detections": 100
        },
        "risk_settings": {
            "person_max_count": 5,
            "risk_areas": [],
            "auto_alert": True
        }
    }
    
    if config_file.exists():
        try:
            with open(config_file, 'r', encoding='utf-8') as f:
                config = json.load(f)
            # 合并默认配置
            for key, value in default_config.items():
                if key not in config:
                    config[key] = value
            return config
        except Exception as e:
            print(f"配置文件加载失败: {e}")
            print("使用默认配置")
    
    # 保存默认配置
    with open(config_file, 'w', encoding='utf-8') as f:
        json.dump(default_config, f, indent=2, ensure_ascii=False)
    
    return default_config

def run_detection(config, args):
    """运行检测脚本"""
    script_dir = Path(__file__).parent
    detection_script = script_dir / "yolo_detection.py"
    
    if not detection_script.exists():
        print(f"检测脚本不存在: {detection_script}")
        return False
    
    # 构建命令行参数
    cmd = [sys.executable, str(detection_script)]
    
    # 从配置文件添加参数
    cmd.extend(["--host", config["host"]])
    cmd.extend(["--port", str(config["port"])])
    cmd.extend(["--source", config["video_source"]])
    
    if config["model_path"]:
        cmd.extend(["--model", config["model_path"]])
    
    # 从命令行参数覆盖配置
    if args.host:
        cmd[-4] = args.host
    if args.port:
        cmd[-2] = str(args.port)
    if args.model:
        cmd.extend(["--model", args.model])
    if args.verbose:
        cmd.append("--verbose")
    
    print(f"启动检测脚本: {' '.join(cmd)}")
    
    try:
        # 运行检测脚本
        process = subprocess.run(cmd, check=True)
        return process.returncode == 0
    except subprocess.CalledProcessError as e:
        print(f"检测脚本运行失败: {e}")
        return False
    except KeyboardInterrupt:
        print("\n检测脚本被用户中断")
        return True

def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description='无人机视频AI识别系统启动器',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
使用示例:
  python run_detection.py                    # 使用默认配置启动
  python run_detection.py --host 192.168.1.100  # 指定主机地址
  python run_detection.py --port 9999       # 指定端口
  python run_detection.py --model yolo11s.pt # 指定模型文件
  python run_detection.py --verbose         # 详细日志输出
        """
    )
    
    parser.add_argument('--host', type=str, help='Qt应用程序主机地址')
    parser.add_argument('--port', type=int, help='TCP通信端口') 
    parser.add_argument('--model', type=str, help='YOLO模型文件路径')
    parser.add_argument('--verbose', action='store_true', help='详细日志输出')
    parser.add_argument('--check-deps', action='store_true', help='仅检查依赖')
    parser.add_argument('--setup', action='store_true', help='仅设置环境')
    
    args = parser.parse_args()
    
    print("=" * 60)
    print("无人机视频AI识别系统启动器")
    print("=" * 60)
    
    # 检查依赖
    print("1. 检查依赖...")
    if not check_dependencies():
        return 1
    print("   ✓ 依赖检查通过")
    
    if args.check_deps:
        print("依赖检查完成")
        return 0
    
    # 设置环境
    print("2. 设置环境...")
    script_dir = setup_environment()
    print(f"   ✓ 工作目录: {script_dir}")
    
    if args.setup:
        print("环境设置完成")
        return 0
    
    # 加载配置
    print("3. 加载配置...")
    config = load_config()
    print(f"   ✓ 主机: {config['host']}")
    print(f"   ✓ 端口: {config['port']}")
    print(f"   ✓ 视频源: {config['video_source']}")
    
    # 运行检测
    print("4. 启动检测系统...")
    success = run_detection(config, args)
    
    if success:
        print("\n✓ 检测系统运行完成")
        return 0
    else:
        print("\n✗ 检测系统运行失败")
        return 1

if __name__ == '__main__':
    sys.exit(main()) 