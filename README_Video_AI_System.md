# 无人机视频AI识别系统

## 概述

本系统为现有的无人机模拟平台新增了实时视频显示和AI智能识别功能，可以在模拟飞行过程中：

- 显示模拟的无人机视频流
- 使用YOLO11进行实时目标检测
- 识别人流、井盖、电瓶车、溺水点等目标
- 提供风险评估和预警功能
- 显示检测结果统计信息

## 系统架构

### C++ Qt端
- `VideoDisplayWidget`: 视频显示和结果展示控件
- `VideoManager`: 视频管理器，处理视频流和AI通信
- 集成到现有的右侧停靠窗口中

### Python AI端
- `yolo_detection.py`: YOLO11目标检测脚本
- 支持实时视频分析和风险评估
- 通过TCP Socket与Qt应用通信

## 安装配置

### 1. Python环境配置

```bash
# 创建Python虚拟环境（推荐）
python -m venv drone_ai_env

# 激活虚拟环境
# Windows:
drone_ai_env\Scripts\activate
# Linux/Mac:
source drone_ai_env/bin/activate

# 安装依赖
cd python
pip install -r requirements.txt
```

### 2. YOLO模型下载

首次运行时，系统会自动下载YOLO11模型文件。如需手动下载：

```bash
# 下载预训练模型
python -c "from ultralytics import YOLO; YOLO('yolo11n.pt')"
```

### 3. 项目编译

在Qt项目中添加新文件到CMakeLists.txt或.pro文件：

```cmake
# 添加到源文件列表
src/gui/VideoDisplayWidget.h
src/gui/VideoDisplayWidget.cpp
src/core/VideoManager.h
src/core/VideoManager.cpp
```

## 使用方法

### 1. 启动系统

1. 编译并运行Qt应用程序
2. 系统会自动初始化视频管理器
3. 在右侧停靠窗口中可以看到"无人机视频"面板

### 2. 开始视频识别

1. 点击左侧"飞行模拟"分组中的"开始"按钮
2. 系统会自动启动视频流和AI识别
3. Python识别脚本会在后台启动
4. 视频和检测结果会实时显示在右侧面板

### 3. 查看识别结果

在视频显示面板中可以看到：
- **视频显示区域**: 显示模拟的无人机视频
- **检测结果列表**: 显示识别到的目标和置信度
- **风险预警区域**: 显示当前风险状态和统计信息
- **控制按钮**: 开始/停止、清除结果、保存结果

### 4. 风险预警

系统会根据检测结果自动评估风险：
- **绿色正常**: 无风险检测
- **橙色警告**: 检测到中等风险目标
- **红色警报**: 检测到高风险目标（如溺水点）

## 功能特性

### 检测目标类型

| 目标类型 | 图标 | 风险等级 | 说明 |
|---------|------|----------|------|
| 人员 | 👤 | 低-高 | 根据数量和位置评估 |
| 井盖 | 🔘 | 低 | 基础设施检测 |
| 电瓶车 | 🛵 | 中 | 交通工具检测 |
| 溺水点 | 💧 | 高 | 紧急情况检测 |

### 风险评估规则

- **人员聚集**: 超过5人时触发中风险警报
- **危险区域**: 特定区域内的目标会提升风险等级
- **置信度阈值**: 低置信度检测会被过滤
- **特殊目标**: 溺水点等特殊目标直接标记为高风险

### 数据统计

系统会实时统计：
- 检测帧数和总检测数
- 人员数量和车辆数量
- 风险警报次数
- 检测准确率等指标

## 技术细节

### 通信协议

Qt端和Python端通过TCP Socket通信，使用JSON格式传输数据：

```json
{
  "timestamp": "2024-12-19T10:30:00",
  "frame_id": 1234,
  "detections": [
    {
      "class": "person",
      "confidence": 0.85,
      "x": 100,
      "y": 150,
      "width": 50,
      "height": 100,
      "risk": true,
      "risk_level": "中"
    }
  ],
  "stats": {
    "total_frames": 1234,
    "total_detections": 56,
    "person_count": 12,
    "vehicle_count": 3,
    "risk_alerts": 2
  }
}
```

### 模拟模式

当YOLO不可用时，系统会自动切换到模拟模式：
- 生成模拟检测数据
- 模拟不同场景的目标出现
- 保持界面功能完整

### 性能优化

- 使用30FPS的视频流处理
- 异步处理检测结果
- 限制结果列表长度防止内存溢出
- 支持GPU加速（如果可用）

## 故障排除

### 常见问题

1. **Python脚本无法启动**
   - 检查Python路径配置
   - 确认所需依赖已安装
   - 查看应用程序日志

2. **TCP连接失败**
   - 检查端口8888是否被占用
   - 确认防火墙设置
   - 重启应用程序

3. **YOLO模型加载失败**
   - 检查网络连接
   - 手动下载模型文件
   - 使用模拟模式

4. **检测结果不准确**
   - 调整置信度阈值
   - 更新风险评估规则
   - 使用更大的YOLO模型

### 日志文件

系统会生成以下日志文件：
- Qt应用日志: 在应用程序输出中
- Python识别日志: `python/yolo_detection.log`

## 扩展开发

### 添加新的检测类型

1. 在`DetectionType`枚举中添加新类型
2. 更新`VideoManager::processDetectionData`中的类别映射
3. 在Python端添加相应的检测逻辑
4. 更新风险评估规则

### 自定义风险规则

在`python/yolo_detection.py`中修改`risk_rules`字典：

```python
self.risk_rules = {
    'person': {
        'max_safe_count': 10,  # 调整安全人数阈值
        'risk_areas': [(x, y, w, h)],  # 定义风险区域
        'confidence_threshold': 0.7  # 调整置信度阈值
    }
}
```

### 集成真实摄像头

1. 修改`VideoManager`的摄像头捕获逻辑
2. 在Python端添加摄像头输入支持
3. 配置摄像头参数和编码格式

## 版本信息

- 版本: 1.0.0
- 兼容性: Qt 5.15+, Python 3.8+
- YOLO版本: YOLOv11
- 更新日期: 2024-12-19

## 技术支持

如有问题请检查：
1. 系统日志输出
2. Python依赖是否完整安装
3. 网络连接是否正常
4. 模型文件是否下载成功 