import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from scipy import stats
import sys

def plot_gbm_paths():
    """绘制GBM模型生成的路径"""
    try:
        df = pd.read_csv('gbm_paths.csv')
        plt.figure(figsize=(12, 6))
        
        # 绘制前10条路径
        for i in range(1, min(11, len(df.columns))):
            plt.plot(df['Time'], df.iloc[:, i], alpha=0.6, linewidth=1)
        
        plt.xlabel('时间 (年)')
        plt.ylabel('汇率')
        plt.title('GBM模型 - 蒙特卡洛模拟路径')
        plt.grid(True, alpha=0.3)
        plt.savefig('gbm_paths.png', dpi=150, bbox_inches='tight')
        plt.show()
        
    except FileNotFoundError:
        print("错误: 未找到 gbm_paths.csv 文件")
        print("请先运行C++程序生成数据")

def plot_distribution():
    """绘制汇率分布图"""
    try:
        # 从CSV读取最终汇率（这里需要根据实际数据调整）
        # 实际应用中，应该从C++程序输出中获取
        
        # 示例：生成测试数据
        np.random.seed(42)
        final_rates = np.random.lognormal(mean=0, sigma=0.15, size=10000)
        
        fig, axes = plt.subplots(1, 2, figsize=(14, 5))
        
        # 直方图
        axes[0].hist(final_rates, bins=50, density=True, alpha=0.7, color='skyblue')
        axes[0].set_xlabel('最终汇率')
        axes[0].set_ylabel('频率')
        axes[0].set_title('最终汇率分布直方图')
        axes[0].grid(True, alpha=0.3)
        
        # 箱线图
        axes[1].boxplot(final_rates, vert=True, patch_artist=True)
        axes[1].set_ylabel('汇率')
        axes[1].set_title('最终汇率箱线图')
        axes[1].grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig('distribution.png', dpi=150, bbox_inches='tight')
        plt.show()
        
    except Exception as e:
        print(f"错误: {e}")

def calculate_risk_metrics():
    """计算并显示风险指标"""
    try:
        # 示例数据
        np.random.seed(42)
        final_rates = np.random.lognormal(mean=0, sigma=0.15, size=10000)
        
        metrics = {
            '均值': np.mean(final_rates),
            '标准差': np.std(final_rates),
            '最小值': np.min(final_rates),
            '最大值': np.max(final_rates),
            '中位数': np.median(final_rates),
            '5%分位数': np.percentile(final_rates, 5),
            '95%分位数': np.percentile(final_rates, 95),
            '95% VaR': np.percentile(final_rates, 5),  # 对于汇率，VaR是左侧5%
            '95% CVaR': np.mean(final_rates[final_rates <= np.percentile(final_rates, 5)])
        }
        
        print("风险指标统计:")
        print("-" * 40)
        for key, value in metrics.items():
            print(f"{key:<15}: {value:.6f}")
        
    except Exception as e:
        print(f"错误: {e}")

def main():
    print("货币汇率蒙特卡洛模拟结果分析")
    print("=" * 50)
    
    while True:
        print("\n请选择操作:")
        print("1. 绘制GBM路径图")
        print("2. 绘制分布图")
        print("3. 计算风险指标")
        print("4. 全部执行")
        print("5. 退出")
        
        choice = input("\n请输入选择 (1-5): ").strip()
        
        if choice == '1':
            plot_gbm_paths()
        elif choice == '2':
            plot_distribution()
        elif choice == '3':
            calculate_risk_metrics()
        elif choice == '4':
            plot_gbm_paths()
            plot_distribution()
            calculate_risk_metrics()
        elif choice == '5':
            print("程序退出")
            break
        else:
            print("无效选择，请重新输入")

if __name__ == "__main__":
    main()
