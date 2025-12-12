#!/usr/bin/env python3
"""
Visualization script for Monte Carlo simulation results.
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path
import argparse

def plot_simulation_paths(csv_file, num_paths=100):
    """Plot sample simulation paths."""
    df = pd.read_csv(csv_file)
    
    plt.figure(figsize=(12, 6))
    
    # 假设第一列是Path ID
    path_columns = [col for col in df.columns if col.startswith('Step_')]
    
    # 绘制前num_paths条路径
    for i in range(min(num_paths, len(df))):
        path = df.loc[i, path_columns].values
        plt.plot(path, alpha=0.1, color='blue', linewidth=0.5)
    
    # 计算并绘制平均路径
    mean_path = df[path_columns].mean().values
    plt.plot(mean_path, color='red', linewidth=2, label='Mean Path')
    
    # 计算并绘制95%置信区间
    lower_bound = df[path_columns].quantile(0.025).values
    upper_bound = df[path_columns].quantile(0.975).values
    plt.fill_between(range(len(mean_path)), lower_bound, upper_bound, 
                     alpha=0.2, color='red', label='95% CI')
    
    plt.title('Monte Carlo Simulation Paths')
    plt.xlabel('Time Steps')
    plt.ylabel('Exchange Rate')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.savefig('simulation_paths.png', dpi=300, bbox_inches='tight')
    plt.show()

def plot_final_price_distribution(csv_file):
    """Plot histogram of final prices."""
    df = pd.read_csv(csv_file)
    
    if 'FinalPrice' not in df.columns:
        print("Error: CSV file must contain 'FinalPrice' column")
        return
    
    final_prices = df['FinalPrice']
    
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    
    # 直方图
    axes[0].hist(final_prices, bins=50, density=True, alpha=0.7, color='blue', edgecolor='black')
    axes[0].axvline(final_prices.mean(), color='red', linestyle='--', 
                   label=f'Mean: {final_prices.mean():.2f}')
    axes[0].axvline(final_prices.median(), color='green', linestyle='--', 
                   label=f'Median: {final_prices.median():.2f}')
    axes[0].set_xlabel('Final Exchange Rate')
    axes[0].set_ylabel('Density')
    axes[0].set_title('Distribution of Final Prices')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)
    
    # 箱线图
    axes[1].boxplot(final_prices, vert=True)
    axes[1].set_title('Box Plot of Final Prices')
    axes[1].set_ylabel('Exchange Rate')
    axes[1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('final_price_distribution.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    # 打印统计信息
    print("\nFinal Price Statistics:")
    print(f"Mean: {final_prices.mean():.4f}")
    print(f"Median: {final_prices.median():.4f}")
    print(f"Std Dev: {final_prices.std():.4f}")
    print(f"Min: {final_prices.min():.4f}")
    print(f"Max: {final_prices.max():.4f}")
    print(f"95% CI: [{final_prices.quantile(0.025):.4f}, {final_prices.quantile(0.975):.4f}]")

def plot_summary_statistics(summary_file):
    """Plot summary statistics."""
    df = pd.read_csv(summary_file, index_col=0)
    
    if 'Value' not in df.columns:
        print("Error: CSV file must contain 'Value' column")
        return
    
    # 提取百分位数
    percentile_data = df[df.index.str.startswith('Percentile_')]
    percentiles = percentile_data.index.str.replace('Percentile_', '').astype(float)
    values = percentile_data['Value'].values
    
    plt.figure(figsize=(10, 6))
    plt.plot(percentiles, values, 'bo-')
    plt.xlabel('Percentile')
    plt.ylabel('Exchange Rate')
    plt.title('Percentile Distribution of Final Prices')
    plt.grid(True, alpha=0.3)
    plt.savefig('percentile_distribution.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    print("\nSummary Statistics:")
    for stat, value in df.iterrows():
        print(f"{stat}: {value['Value']}")

def main():
    parser = argparse.ArgumentParser(description='Visualize Monte Carlo simulation results')
    parser.add_argument('--paths-file', type=str, help='CSV file containing simulation paths')
    parser.add_argument('--prices-file', type=str, help='CSV file containing final prices')
    parser.add_argument('--summary-file', type=str, help='CSV file containing summary statistics')
    parser.add_argument('--num-paths', type=int, default=100, help='Number of paths to plot')
    
    args = parser.parse_args()
    
    sns.set_style("whitegrid")
    plt.rcParams['figure.figsize'] = (12, 6)
    plt.rcParams['font.size'] = 12
    
    if args.paths_file:
        print(f"Plotting simulation paths from {args.paths_file}")
        plot_simulation_paths(args.paths_file, args.num_paths)
    
    if args.prices_file:
        print(f"\nPlotting final price distribution from {args.prices_file}")
        plot_final_price_distribution(args.prices_file)
    
    if args.summary_file:
        print(f"\nPlotting summary statistics from {args.summary_file}")
        plot_summary_statistics(args.summary_file)

if __name__ == '__main__':
    main()
