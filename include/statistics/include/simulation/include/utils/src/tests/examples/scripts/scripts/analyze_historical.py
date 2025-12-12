#!/usr/bin/env python3
"""
Analyze historical currency data and calibrate model parameters.
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import yfinance as yf
from datetime import datetime, timedelta

def download_historical_data(currency_pair='USDRUB=X', start_date='2023-01-01', end_date=None):
    """Download historical currency data from Yahoo Finance."""
    if end_date is None:
        end_date = datetime.now().strftime('%Y-%R-%d')
    
    print(f"Downloading {currency_pair} data from {start_date} to {end_date}")
    
    ticker = yf.Ticker(currency_pair)
    df = ticker.history(start=start_date, end=end_date)
    
    if df.empty:
        print(f"No data found for {currency_pair}")
        return None
    
    # 保存到CSV
    df.to_csv(f'historical_{currency_pair.replace("=", "")}.csv')
    print(f"Data saved to historical_{currency_pair.replace('=', '')}.csv")
    
    return df

def calculate_returns(df, price_col='Close'):
    """Calculate logarithmic returns."""
    prices = df[price_col].dropna()
    returns = np.log(prices / prices.shift(1)).dropna()
    
    return returns

def calibrate_gbm_parameters(df, price_col='Close', trading_days=252):
    """Calibrate GBM parameters from historical data."""
    returns = calculate_returns(df, price_col)
    
    # 计算日回报率的统计量
    daily_mean = returns.mean()
    daily_std = returns.std()
    
    # 年化参数
    annual_drift = daily_mean * trading_days
    annual_volatility = daily_std * np.sqrt(trading_days)
    
    initial_price = df[price_col].iloc[-1]
    
    return {
        'initial_price': initial_price,
        'drift': annual_drift,
        'volatility': annual_volatility,
        'daily_mean_return': daily_mean,
        'daily_std_return': daily_std,
        'sharpe_ratio': daily_mean / daily_std if daily_std != 0 else 0
    }

def plot_historical_data(df, currency_pair):
    """Plot historical currency data."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    # 价格走势
    axes[0, 0].plot(df.index, df['Close'], linewidth=1)
    axes[0, 0].set_title(f'{currency_pair} Historical Price')
    axes[0, 0].set_ylabel('Exchange Rate')
    axes[0, 0].grid(True, alpha=0.3)
    
    # 日回报率
    returns = calculate_returns(df)
    axes[0, 1].plot(returns.index, returns, linewidth=0.5)
    axes[0, 1].axhline(y=0, color='r', linestyle='--', alpha=0.3)
    axes[0, 1].set_title('Daily Returns')
    axes[0, 1].set_ylabel('Log Return')
    axes[0, 1].grid(True, alpha=0.3)
    
    # 回报率分布
    axes[1, 0].hist(returns, bins=50, density=True, alpha=0.7, edgecolor='black')
    axes[1, 0].set_title('Return Distribution')
    axes[1, 0].set_xlabel('Return')
    axes[1, 0].set_ylabel('Density')
    axes[1, 0].grid(True, alpha=0.3)
    
    # 滚动波动率（30天窗口）
    rolling_vol = returns.rolling(window=30).std() * np.sqrt(252)
    axes[1, 1].plot(rolling_vol.index, rolling_vol, linewidth=1)
    axes[1, 1].set_title('Rolling Annualized Volatility (30-day window)')
    axes[1, 1].set_ylabel('Volatility')
    axes[1, 1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f'historical_analysis_{currency_pair.replace("=", "")}.png', 
                dpi=300, bbox_inches='tight')
    plt.show()

def generate_config_file(params, currency_pair, output_file=None):
    """Generate configuration file for C++ simulation."""
    if output_file is None:
        output_file = f'config_{currency_pair.replace("=", "")}.json'
    
    config = {
        "currency_pair": currency_pair,
        "simulation": {
            "model": "GBM",
            "parameters": {
                "initial_price": float(params['initial_price']),
                "drift": float(params['drift']),
                "volatility": float(params['volatility']),
                "risk_free_rate": 0.03
            },
            "simulation_params": {
                "num_simulations": 10000,
                "num_steps": 252,
                "time_horizon_days": 365
            },
            "calibration_data": {
                "historical_days": len(params.get('returns', [])),
                "daily_mean_return": float(params['daily_mean_return']),
                "daily_std_return": float(params['daily_std_return']),
                "sharpe_ratio": float(params['sharpe_ratio'])
            }
        }
    }
    
    # 保存为JSON
    import json
    with open(output_file, 'w') as f:
        json.dump(config, f, indent=2)
    
    print(f"Configuration saved to {output_file}")
    return config

def main():
    # 示例：分析USD/RUB汇率
    currency_pair = 'USDRUB=X'
    
    # 下载数据
    df = download_historical_data(currency_pair, start_date='2023-01-01')
    
    if df is not None and not df.empty:
        # 校准参数
        params = calibrate_gbm_parameters(df)
        
        print("\nCalibrated GBM Parameters:")
        print(f"Initial Price: {params['initial_price']:.4f}")
        print(f"Annual Drift (μ): {params['drift']:.4f} ({params['drift']*100:.2f}%)")
        print(f"Annual Volatility (σ): {params['volatility']:.4f} ({params['volatility']*100:.2f}%)")
        print(f"Sharpe Ratio: {params['sharpe_ratio']:.4f}")
        
        # 生成配置文件
        generate_config_file(params, currency_pair)
        
        # 绘制图表
        plot_historical_data(df, currency_pair)

if __name__ == '__main__':
    main()
