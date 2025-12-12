import numpy as np
import pandas as pd
import argparse
import os
import sys
from datetime import datetime, timedelta
import warnings
import traceback
from typing import Optional, Dict, List, Any

# 忽略警告信息
warnings.filterwarnings('ignore')

# 导入项目模块
try:
    from monte_carlo import CurrencyMonteCarlo, MultiCurrencyMonteCarlo
    from data_loader import ExchangeRateDataLoader
    from visualization import MonteCarloVisualizer
    IMPORT_SUCCESS = True
except ImportError as e:
    print(f"❌ Error importing project modules: {e}")
    print("Please ensure all required files are in the same directory:")
    print("  - monte_carlo.py")
    print("  - data_loader.py")
    print("  - visualization.py")
    IMPORT_SUCCESS = False


class SimulationManager:
    """管理蒙特卡洛模拟的类"""
    
    def __init__(self):
        """初始化模拟管理器"""
        self.results_dir = "simulation_results"
        self.plots_dir = "plots"
        self.data_dir = "data"
        
        # 创建必要的目录
        self._create_directories()
        
        # 初始化组件
        self.data_loader = ExchangeRateDataLoader()
        self.visualizer = MonteCarloVisualizer()
        
    def _create_directories(self):
        """创建必要的目录"""
        directories = [self.results_dir, self.plots_dir, self.data_dir]
        for directory in directories:
            os.makedirs(directory, exist_ok=True)
            print(f"📁 Created directory: {directory}")
    
    def _generate_timestamp(self) -> str:
        """生成时间戳字符串"""
        return datetime.now().strftime("%Y%m%d_%H%M%S")
    
    def _save_results(self, data: Any, filename: str, subdir: str = None):
        """保存结果到文件"""
        try:
            if subdir:
                save_dir = os.path.join(self.results_dir, subdir)
                os.makedirs(save_dir, exist_ok=True)
                filepath = os.path.join(save_dir, filename)
            else:
                filepath = os.path.join(self.results_dir, filename)
            
            if isinstance(data, pd.DataFrame):
                data.to_csv(filepath, index=False)
                print(f"💾 Results saved to: {filepath}")
            elif isinstance(data, dict):
                import json
                with open(filepath.replace('.csv', '.json'), 'w') as f:
                    json.dump(data, f, indent=2, default=str)
                print(f"💾 Results saved to: {filepath.replace('.csv', '.json')}")
            else:
                print(f"⚠️ Unsupported data type for saving: {type(data)}")
                
        except Exception as e:
            print(f"❌ Error saving results: {e}")


def print_header(title: str):
    """打印漂亮的标题"""
    print("\n" + "═" * 70)
    print(f"📊 {title}")
    print("═" * 70)


def print_section(title: str):
    """打印章节标题"""
    print("\n" + "─" * 60)
    print(f"▶ {title}")
    print("─" * 60)


def print_success(message: str):
    """打印成功消息"""
    print(f"✅ {message}")


def print_warning(message: str):
    """打印警告消息"""
    print(f"⚠️ {message}")


def print_error(message: str):
    """打印错误消息"""
    print(f"❌ {message}")


def get_user_input(prompt: str, default=None, input_type=str):
    """获取用户输入，支持默认值"""
    if default is not None:
        prompt = f"{prompt} [{default}]: "
    else:
        prompt = f"{prompt}: "
    
    while True:
        try:
            user_input = input(prompt).strip()
            if not user_input and default is not None:
                return default
            
            if input_type == float:
                return float(user_input)
            elif input_type == int:
                return int(user_input)
            elif input_type == bool:
                return user_input.lower() in ['y', 'yes', 'true', '1']
            else:
                return user_input
                
        except ValueError:
            print_error("Invalid input. Please try again.")


def main_single_currency():
    """运行单货币模拟"""
    print_header("CURRENCY MONTE CARLO SIMULATION")
    
    # 初始化模拟管理器
    manager = SimulationManager()
    
    # 获取用户输入或使用默认值
    print_section("SIMULATION CONFIGURATION")
    
    initial_rate = get_user_input("Enter initial exchange rate", 75.0, float)
    forecast_days = get_user_input("Enter forecast horizon (days)", 30, int)
    n_simulations = get_user_input("Enter number of simulations", 5000, int)
    
    print("\nSelect drift level (expected annual return):")
    print("1. Low (μ = 2%) - Stable economy")
    print("2. Medium (μ = 5%) - Normal growth")
    print("3. High (μ = 10%) - High growth")
    print("4. Negative (μ = -2%) - Economic decline")
    
    drift_choice = get_user_input("Enter choice (1-4)", "2")
    drift_map = {'1': 0.02, '2': 0.05, '3': 0.10, '4': -0.02}
    mu = drift_map.get(drift_choice, 0.05)
    
    print("\nSelect volatility level:")
    print("1. Low (σ = 10%) - Stable currency")
    print("2. Medium (σ = 15%) - Normal currency")
    print("3. High (σ = 25%) - Volatile currency")
    print("4. Very High (σ = 40%) - Emerging market")
    
    vol_choice = get_user_input("Enter choice (1-4)", "2")
    volatility_map = {'1': 0.10, '2': 0.15, '3': 0.25, '4': 0.40}
    sigma = volatility_map.get(vol_choice, 0.15)
    
    # 初始化模型
    print_section("MODEL INITIALIZATION")
    print(f"Initial Rate: {initial_rate:.4f}")
    print(f"Expected Annual Return (μ): {mu:.2%}")
    print(f"Annual Volatility (σ): {sigma:.2%}")
    print(f"Forecast Horizon: {forecast_days} days")
    print(f"Number of Simulations: {n_simulations:,}")
    
    model = CurrencyMonteCarlo(
        initial_rate=initial_rate,
        mu=mu,
        sigma=sigma
    )
    
    # 运行模拟
    print_section("RUNNING SIMULATION")
    print("Running Monte Carlo simulation...")
    
    simulations = model.simulate(
        days=forecast_days,
        n_simulations=n_simulations,
        random_seed=42  # 保证可重现性
    )
    
    print_success(f"Simulations completed: {simulations.shape[0]:,} paths")
    print_success(f"Time horizon: {simulations.shape[1]} days")
    
    # 获取预测结果
    print_section("PREDICTION RESULTS")
    
    predictions = model.get_prediction(confidence_level=0.95)
    
    results_summary = pd.DataFrame({
        'Metric': [
            'Current Rate',
            'Predicted Mean',
            'Predicted Median',
            'Standard Deviation',
            'Minimum',
            'Maximum',
            '95% CI Lower',
            '95% CI Upper'
        ],
        'Value': [
            f"{model.initial_rate:.4f}",
            f"{predictions['mean']:.4f}",
            f"{predictions['median']:.4f}",
            f"{predictions['std']:.4f}",
            f"{predictions['min']:.4f}",
            f"{predictions['max']:.4f}",
            f"{predictions['confidence_interval'][0]:.4f}",
            f"{predictions['confidence_interval'][1]:.4f}"
        ],
        'Change %': [
            '0.00%',
            f"{(predictions['mean'] - model.initial_rate)/model.initial_rate*100:.2f}%",
            f"{(predictions['median'] - model.initial_rate)/model.initial_rate*100:.2f}%",
            '-',
            '-',
            '-',
            '-',
            '-'
        ]
    })
    
    print("\n📈 Prediction Summary:")
    print(results_summary.to_string(index=False))
    
    # 计算风险指标
    print_section("RISK ANALYSIS")
    
    investment_amount = get_user_input("Enter investment amount for risk analysis", 10000, float)
    
    var_results = model.calculate_var(
        confidence_level=0.95,
        initial_investment=investment_amount
    )
    
    risk_summary = pd.DataFrame({
        'Risk Metric': [
            'Value at Risk (95%)',
            'Conditional VaR (95%)',
            'Expected Shortfall',
            'Worst-case Loss',
            'Best-case Gain'
        ],
        'Percentage': [
            f"{var_results['var_percent']:.2f}%",
            f"{var_results['cvar_percent']:.2f}%",
            f"{var_results['cvar_percent']:.2f}%",
            f"{np.min((model.predicted_rates - model.initial_rate)/model.initial_rate*100):.2f}%",
            f"{np.max((model.predicted_rates - model.initial_rate)/model.initial_rate*100):.2f}%"
        ],
        'Monetary Amount': [
            f"${var_results['var_amount']:.2f}",
            f"${var_results['var_amount'] * 1.1:.2f}",
            f"${var_results['var_amount']:.2f}",
            f"${investment_amount * np.min((model.predicted_rates - model.initial_rate)/model.initial_rate)/100:.2f}",
            f"${investment_amount * np.max((model.predicted_rates - model.initial_rate)/model.initial_rate)/100:.2f}"
        ]
    })
    
    print("\n⚠️ Risk Summary:")
    print(risk_summary.to_string(index=False))
    
    # 计算达到目标汇率的概率
    print_section("TARGET PROBABILITY ANALYSIS")
    
    target_rate = get_user_input(
        f"Enter target exchange rate to analyze (current: {model.initial_rate:.2f})",
        model.initial_rate * 1.05,
        float
    )
    
    probability = model.calculate_probability(target_rate)
    print(f"\n🎯 Probability of reaching {target_rate:.2f}: {probability:.2%}")
    
    if probability > 0.7:
        print("   → High probability of reaching target")
    elif probability > 0.3:
        print("   → Moderate probability")
    else:
        print("   → Low probability")
    
    # 生成可视化
    print_section("VISUALIZATION")
    
    generate_plots = get_user_input("Generate visualizations? (y/n)", "y", bool)
    
    if generate_plots:
        timestamp = manager._generate_timestamp()
        prefix = f"single_currency_{timestamp}"
        
        print("\nGenerating visualizations...")
        
        try:
            # 1. 模拟路径图
            plot1_path = os.path.join(manager.plots_dir, f"{prefix}_simulation_paths.png")
            manager.visualizer.plot_simulation_paths(
                simulations,
                n_paths=100,
                title=f"Monte Carlo Simulation - {forecast_days} Day Forecast",
                save_path=plot1_path
            )
            print_success(f"Simulation paths saved to: {plot1_path}")
            
            # 2. 最终分布图
            plot2_path = os.path.join(manager.plots_dir, f"{prefix}_distribution.png")
            manager.visualizer.plot_final_distribution(
                model.predicted_rates,
                model.initial_rate,
                title="Predicted Exchange Rate Distribution",
                save_path=plot2_path
            )
            print_success(f"Distribution plot saved to: {plot2_path}")
            
            # 3. 风险分析图
            plot3_path = os.path.join(manager.plots_dir, f"{prefix}_risk_analysis.png")
            manager.visualizer.plot_risk_metrics(
                model,
                initial_investment=investment_amount,
                save_path=plot3_path
            )
            print_success(f"Risk analysis saved to: {plot3_path}")
            
        except Exception as e:
            print_error(f"Error generating plots: {e}")
    
    # 保存结果
    print_section("SAVING RESULTS")
    
    save_results = get_user_input("Save simulation results? (y/n)", "y", bool)
    
    if save_results:
        timestamp = manager._generate_timestamp()
        
        # 保存模拟数据
        sim_df = pd.DataFrame({
            'Simulation': np.repeat(range(n_simulations), forecast_days),
            'Day': np.tile(range(forecast_days), n_simulations),
            'Exchange_Rate': simulations.flatten()
        })
        
        manager._save_results(sim_df, f"simulation_data_{timestamp}.csv", "simulations")
        
        # 保存预测结果
        pred_df = pd.DataFrame([{
            'Timestamp': timestamp,
            'Initial_Rate': model.initial_rate,
            'Drift': mu,
            'Volatility': sigma,
            'Forecast_Days': forecast_days,
            'N_Simulations': n_simulations,
            'Predicted_Mean': predictions['mean'],
            'Predicted_Median': predictions['median'],
            'Std_Dev': predictions['std'],
            'CI_Lower': predictions['confidence_interval'][0],
            'CI_Upper': predictions['confidence_interval'][1],
            'VaR_95': var_results['var_percent'],
            'Target_Rate': target_rate,
            'Target_Probability': probability
        }])
        
        manager._save_results(pred_df, f"prediction_results_{timestamp}.csv", "predictions")
    
    print_header("SIMULATION COMPLETED")
    print("✅ Single currency simulation completed successfully!")


def main_multi_currency():
    """运行多货币模拟"""
    print_header("MULTI-CURRENCY MONTE CARLO SIMULATION")
    
    # 初始化模拟管理器
    manager = SimulationManager()
    
    # 定义可用的货币对及其默认参数
    currency_configs = {
        'USD/RUB': {'rate': 75.0, 'mu': 0.05, 'sigma': 0.15},
        'EUR/USD': {'rate': 1.08, 'mu': 0.02, 'sigma': 0.10},
        'USD/JPY': {'rate': 150.0, 'mu': 0.03, 'sigma': 0.12},
        'GBP/USD': {'rate': 1.26, 'mu': 0.04, 'sigma': 0.14},
        'USD/CNY': {'rate': 7.15, 'mu': 0.03, 'sigma': 0.11},
        'AUD/USD': {'rate': 0.66, 'mu': 0.04, 'sigma': 0.13},
        'USD/CAD': {'rate': 1.35, 'mu': 0.02, 'sigma': 0.09},
    }
    
    print_section("SELECT CURRENCY PAIRS")
    print("Available currency pairs:")
    for i, (pair, config) in enumerate(currency_configs.items(), 1):
        print(f"  {i}. {pair:10} (Rate: {config['rate']:.2f}, Vol: {config['sigma']:.0%})")
    
    print("\nEnter the numbers of currency pairs to simulate (comma-separated)")
    print("Example: 1,3,5 or 'all' for all currencies")
    
    selection = get_user_input("Selection", "1,2,3,4")
    
    if selection.lower() == 'all':
        selected_pairs = list(currency_configs.keys())
    else:
        try:
            indices = [int(idx.strip()) - 1 for idx in selection.split(',')]
            selected_pairs = [list(currency_configs.keys())[i] for i in indices]
        except (ValueError, IndexError):
            print_warning("Invalid selection. Using default: USD/RUB, EUR/USD, USD/JPY, GBP/USD")
            selected_pairs = ['USD/RUB', 'EUR/USD', 'USD/JPY', 'GBP/USD']
    
    print_section("SIMULATION PARAMETERS")
    forecast_days = get_user_input("Enter forecast horizon (days)", 30, int)
    n_simulations = get_user_input("Enter number of simulations per currency", 2000, int)
    
    # 初始化多货币模型
    multi_model = MultiCurrencyMonteCarlo()
    
    print_section("ADDING CURRENCY PAIRS")
    for pair in selected_pairs:
        config = currency_configs[pair]
        multi_model.add_currency(
            currency_pair=pair,
            initial_rate=config['rate'],
            mu=config['mu'],
            sigma=config['sigma']
        )
        print_success(f"Added {pair}: Rate={config['rate']:.2f}, μ={config['mu']:.2%}, σ={config['sigma']:.2%}")
    
    # 运行模拟
    print_section("RUNNING SIMULATIONS")
    print(f"Running simulations for {len(selected_pairs)} currency pairs...")
    
    results = multi_model.simulate_all(
        days=forecast_days,
        n_simulations=n_simulations,
        random_seed=42
    )
    
    print_success(f"Simulations completed for {len(results)} currency pairs")
    
    # 获取比较表格
    print_section("MULTI-CURRENCY COMPARISON")
    
    comparison_df = multi_model.get_comparison()
    
    # 格式化显示
    display_cols = ['Currency Pair', 'Initial Rate', 'Predicted Mean', 'Predicted Median',
                   'Std Dev', 'VaR 95.0%', 'Min', 'Max', 'Expected Return %']
    
    # 计算预期收益率
    comparison_df['Expected Return %'] = ((comparison_df['Predicted Mean'] - 
                                         comparison_df['Initial Rate']) / 
                                        comparison_df['Initial Rate'] * 100)
    
    # 按预期收益率排序
    comparison_df = comparison_df.sort_values('Expected Return %', ascending=False)
    
    print("\n📊 Currency Comparison (sorted by expected return):")
    print(comparison_df[display_cols].to_string(index=False, float_format=lambda x: f"{x:.4f}" if isinstance(x, float) else str(x)))
    
    # 识别最佳和最差货币对
    best_currency = comparison_df.iloc[0]
    worst_currency = comparison_df.iloc[-1]
    
    print(f"\n🏆 Best performer: {best_currency['Currency Pair']}")
    print(f"   Expected return: {best_currency['Expected Return %']:.2f}%")
    print(f"   Volatility: {best_currency['Std Dev']:.4f}")
    print(f"   Risk (VaR 95%): {best_currency['VaR 95.0%']:.2f}%")
    
    print(f"\n📉 Worst performer: {worst_currency['Currency Pair']}")
    print(f"   Expected return: {worst_currency['Expected Return %']:.2f}%")
    print(f"   Volatility: {worst_currency['Std Dev']:.4f}")
    print(f"   Risk (VaR 95%): {worst_currency['VaR 95.0%']:.2f}%")
    
    # 生成可视化
    print_section("VISUALIZATION")
    
    generate_plots = get_user_input("Generate multi-currency visualizations? (y/n)", "y", bool)
    
    if generate_plots:
        timestamp = manager._generate_timestamp()
        prefix = f"multi_currency_{timestamp}"
        
        try:
            # 多货币比较图
            plot_path = os.path.join(manager.plots_dir, f"{prefix}_comparison.png")
            manager.visualizer.plot_multi_currency_comparison(
                multi_model,
                save_path=plot_path
            )
            print_success(f"Multi-currency comparison saved to: {plot_path}")
            
        except Exception as e:
            print_error(f"Error generating plots: {e}")
    
    # 保存结果
    print_section("SAVING RESULTS")
    
    save_results = get_user_input("Save multi-currency results? (y/n)", "y", bool)
    
    if save_results:
        timestamp = manager._generate_timestamp()
        
        # 保存比较表格
        filename = f"multi_currency_comparison_{timestamp}.csv"
        manager._save_results(comparison_df, filename, "multi_currency")
        
        # 保存详细模拟数据
        for pair, sim_data in results.items():
            sim_flat = sim_data.flatten()
            n_paths, n_days = sim_data.shape
            
            sim_df = pd.DataFrame({
                'Currency_Pair': pair,
                'Simulation': np.repeat(range(n_paths), n_days),
                'Day': np.tile(range(n_days), n_paths),
                'Exchange_Rate': sim_flat
            })
            
            safe_pair = pair.replace('/', '_')
            manager._save_results(sim_df, f"{safe_pair}_simulations_{timestamp}.csv", "multi_currency/simulations")
    
    print_header("MULTI-CURRENCY SIMULATION COMPLETED")
    print("✅ Multi-currency analysis completed successfully!")


def main_with_real_data():
    """使用真实数据运行模拟"""
    print_header("MONTE CARLO SIMULATION WITH REAL DATA")
    
    # 初始化模拟管理器
    manager = SimulationManager()
    
    print_section("DATA SOURCE SELECTION")
    print("Select data source:")
    print("1. Yahoo Finance (requires internet connection)")
    print("2. Local CSV file")
    print("3. Generate sample data")
    
    source_choice = get_user_input("Enter choice (1-3)", "1")
    
    if source_choice == '1':
        # 从雅虎财经加载数据
        print_section("YAHOO FINANCE DATA")
        
        print("Available currency pairs:")
        for pair, yahoo_code in ExchangeRateDataLoader.CURRENCY_PAIRS.items():
            print(f"  - {pair:10} ({yahoo_code})")
        
        currency_pair = get_user_input("Enter currency pair (e.g., USD/RUB)", "USD/RUB")
        
        # 获取日期范围
        print("\nSelect date range:")
        print("1. Last 30 days")
        print("2. Last 90 days")
        print("3. Last 180 days")
        print("4. Last 365 days")
        print("5. Custom range")
        
        date_choice = get_user_input("Enter choice (1-5)", "3")
        
        end_date = datetime.now()
        date_ranges = {
            '1': 30,
            '2': 90,
            '3': 180,
            '4': 365
        }
        
        if date_choice in date_ranges:
            days = date_ranges[date_choice]
            start_date = end_date - timedelta(days=days)
        else:
            start_date_str = get_user_input("Enter start date (YYYY-MM-DD)", "2023-01-01")
            end_date_str = get_user_input("Enter end date (YYYY-MM-DD)", end_date.strftime("%Y-%m-%d"))
            start_date = datetime.strptime(start_date_str, "%Y-%m-%d")
            end_date = datetime.strptime(end_date_str, "%Y-%m-%d")
        
        print(f"\n📅 Loading data from {start_date.date()} to {end_date.date()}...")
        
        data = manager.data_loader.load_from_yahoo(
            currency_pair=currency_pair,
            start_date=start_date.strftime("%Y-%m-%d"),
            end_date=end_date.strftime("%Y-%m-%d"),
            interval="1d"
        )
        
    elif source_choice == '2':
        # 从本地CSV文件加载数据
        print_section("LOCAL CSV DATA")
        
        data_dir = manager.data_dir
        csv_files = [f for f in os.listdir(data_dir) if f.endswith('.csv')]
        
        if csv_files:
            print("Available CSV files:")
            for i, file in enumerate(csv_files, 1):
                print(f"  {i}. {file}")
            
            file_choice = get_user_input(f"Select file (1-{len(csv_files)}) or enter path", "1")
            
            try:
                if file_choice.isdigit():
                    file_idx = int(file_choice) - 1
                    filepath = os.path.join(data_dir, csv_files[file_idx])
                else:
                    filepath = file_choice
            except (ValueError, IndexError):
                print_warning("Invalid selection. Using first file.")
                filepath = os.path.join(data_dir, csv_files[0])
        else:
            filepath = get_user_input("Enter CSV file path", "data/exchange_rates.csv")
        
        data = manager.data_loader.load_from_csv(filepath)
        
    else:
        # 生成样本数据
        print_section("SAMPLE DATA GENERATION")
        
        currency_pair = get_user_input("Enter currency pair name", "USD/RUB")
        initial_rate = get_user_input("Enter initial rate", 75.0, float)
        volatility = get_user_input("Enter daily volatility", 0.01, float)
        
        data = manager.data_loader.create_sample_data(
            currency_pair=currency_pair,
            initial_rate=initial_rate,
            volatility=volatility
        )
    
    if data.empty:
        print_error("Failed to load data. Exiting simulation.")
        return
    
    # 分析历史数据
    print_section("HISTORICAL DATA ANALYSIS")
    
    stats = manager.data_loader.calculate_statistics(data)
    
    # 显示关键统计量
    key_stats = pd.DataFrame({
        'Metric': [
            'Current Rate',
            'Mean Rate',
            'Min Rate',
            'Max Rate',
            'Daily Volatility',
            'Annual Volatility',
            'Daily Drift',
            'Annual Drift',
            'Sharpe Ratio',
            'Data Points'
        ],
        'Value': [
            f"{stats['current_rate']:.4f}",
            f"{stats['mean_rate']:.4f}",
            f"{stats['min_rate']:.4f}",
            f"{stats['max_rate']:.4f}",
            f"{stats['volatility_daily']:.4%}",
            f"{stats['volatility_annual']:.4%}",
            f"{stats['drift_daily']:.4%}",
            f"{stats['drift_annual']:.4%}",
            f"{stats['sharpe_ratio']:.4f}",
            f"{stats['total_days']:,}"
        ]
    })
    
    print("\n📊 Historical Statistics:")
    print(key_stats.to_string(index=False))
    
    # 检测异常值
    print_section("OUTLIER DETECTION")
    outliers = manager.data_loader.detect_outliers(data)
    
    if not outliers.empty:
        print(f"\n🔍 Found {len(outliers)} outliers in historical data")
    else:
        print("✅ No significant outliers detected")
    
    # 初始化模型
    print_section("MODEL INITIALIZATION")
    
    use_historical_params = get_user_input("Use historical parameters for simulation? (y/n)", "y", bool)
    
    if use_historical_params:
        initial_rate = stats['current_rate']
        mu = stats['drift_annual']
        sigma = stats['volatility_annual']
        
        print(f"✅ Using historical parameters:")
        print(f"   Initial Rate: {initial_rate:.4f}")
        print(f"   Annual Drift (μ): {mu:.4%}")
        print(f"   Annual Volatility (σ): {sigma:.4%}")
    else:
        initial_rate = get_user_input("Enter initial rate", stats['current_rate'], float)
        mu = get_user_input("Enter annual drift", stats['drift_annual'], float)
        sigma = get_user_input("Enter annual volatility", stats['volatility_annual'], float)
    
    model = CurrencyMonteCarlo(
        initial_rate=initial_rate,
        mu=mu,
        sigma=sigma
    )
    
    # 运行模拟
    print_section("RUNNING SIMULATION")
    
    forecast_days = get_user_input("Enter forecast horizon (days)", 30, int)
    n_simulations = get_user_input("Enter number of simulations", 5000, int)
    
    simulations = model.simulate(
        days=forecast_days,
        n_simulations=n_simulations,
        random_seed=42
    )
    
    print_success(f"Simulations completed: {simulations.shape[0]:,} paths")
    
    # 显示预测结果
    print_section("FORECAST RESULTS")
    
    predictions = model.get_prediction()
    
    forecast_summary = pd.DataFrame({
        'Metric': ['Current', 'Predicted Mean', 'Change %', '95% CI Lower', '95% CI Upper'],
        'Value': [
            f"{initial_rate:.4f}",
            f"{predictions['mean']:.4f}",
            f"{(predictions['mean'] - initial_rate)/initial_rate*100:.2f}%",
            f"{predictions['confidence_interval'][0]:.4f}",
            f"{predictions['confidence_interval'][1]:.4f}"
        ]
    })
    
    print("\n📈 Forecast Summary:")
    print(forecast_summary.to_string(index=False))
    
    # 生成可视化
    print_section("VISUALIZATION")
    
    generate_plots = get_user_input("Generate visualizations? (y/n)", "y", bool)
    
    if generate_plots:
        timestamp = manager._generate_timestamp()
        prefix = f"real_data_{timestamp}"
        
        try:
            # 模拟路径图
            plot1_path = os.path.join(manager.plots_dir, f"{prefix}_simulation_paths.png")
            manager.visualizer.plot_simulation_paths(
                simulations,
                n_paths=100,
                title=f"Monte Carlo Simulation Based on Historical Data",
                save_path=plot1_path
            )
            print_success(f"Simulation paths saved to: {plot1_path}")
            
            # 风险分析图
            plot2_path = os.path.join(manager.plots_dir, f"{prefix}_risk_analysis.png")
            manager.visualizer.plot_risk_metrics(
                model,
                initial_investment=10000,
                save_path=plot2_path
            )
            print_success(f"Risk analysis saved to: {plot2_path}")
            
        except Exception as e:
            print_error(f"Error generating plots: {e}")
    
    # 保存结果
    print_section("SAVING RESULTS")
    
    save_results = get_user_input("Save simulation results? (y/n)", "y", bool)
    
    if save_results:
        timestamp = manager._generate_timestamp()
        
        # 保存历史数据
        hist_filename = f"historical_data_{timestamp}.csv"
        manager._save_results(data.reset_index(), hist_filename, "historical")
        
        # 保存模拟结果
        pred_df = pd.DataFrame([{
            'Timestamp': timestamp,
            'Data_Source': 'Real Data',
            'Initial_Rate': initial_rate,
            'Historical_Drift': stats['drift_annual'],
            'Historical_Volatility': stats['volatility_annual'],
            'Used_Drift': mu,
            'Used_Volatility': sigma,
            'Forecast_Days': forecast_days,
            'N_Simulations': n_simulations,
            'Predicted_Mean': predictions['mean'],
            'Predicted_Median': predictions['median'],
            'CI_Lower': predictions['confidence_interval'][0],
            'CI_Upper': predictions['confidence_interval'][1]
        }])
        
        manager._save_results(pred_df, f"real_data_prediction_{timestamp}.csv", "predictions")
    
    print_header("REAL DATA SIMULATION COMPLETED")
    print("✅ Real data analysis completed successfully!")


def main_interactive():
    """交互式模式"""
    print_header("INTERACTIVE MONTE CARLO SIMULATION")
    
    print("Welcome to the interactive Monte Carlo simulation!")
    print("This mode will guide you through the entire simulation process.")
    
    # 选择模拟类型
    print_section("SELECT SIMULATION TYPE")
    print("What type of simulation would you like to run?")
    print("1. Single currency simulation")
    print("2. Multi-currency comparison")
    print("3. Simulation with real data")
    print("4. Quick demo (all features)")
    
    sim_type = get_user_input("Enter choice (1-4)", "1")
    
    if sim_type == '1':
        main_single_currency()
    elif sim_type == '2':
        main_multi_currency()
    elif sim_type == '3':
        main_with_real_data()
    elif sim_type == '4':
        print("\n🚀 Running quick demo with all features...")
        
        # 运行单货币模拟
        print("\n" + "="*70)
        print("DEMO: Single Currency Simulation")
        print("="*70)
        main_single_currency()
        
        # 运行多货币模拟
        print("\n" + "="*70)
        print("DEMO: Multi-Currency Simulation")
        print("="*70)
        main_multi_currency()
        
        # 运行真实数据模拟
        print("\n" + "="*70)
        print("DEMO: Real Data Simulation")
        print("="*70)
        main_with_real_data()
        
        print_header("DEMO COMPLETED")
        print("✅ All demo simulations completed successfully!")
    else:
        print_error("Invalid choice. Exiting.")


def main():
    """主入口点"""
    if not IMPORT_SUCCESS:
        print_error("Cannot start application due to missing modules.")
        sys.exit(1)
    
    parser = argparse.ArgumentParser(
        description="Currency Monte Carlo Simulation",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python main.py                    # Run single currency simulation
  python main.py --mode multi       # Run multi-currency simulation
  python main.py --mode real        # Run with real data
  python main.py --mode interactive # Interactive mode
  python main.py --save-plots       # Save all plots automatically
        """
    )
    
    parser.add_argument('--mode', type=str, default='single',
                       choices=['single', 'multi', 'real', 'interactive', 'demo'],
                       help='Simulation mode (default: single)')
    parser.add_argument('--save-plots', action='store_true',
                       help='Automatically save all plots')
    parser.add_argument('--quiet', action='store_true',
                       help='Minimal output')
    parser.add_argument('--version', action='version', 
                       version='Currency Monte Carlo Simulator v1.0')
    
    args = parser.parse_args()
    
    try:
        print_header("CURRENCY MONTE CARLO SIMULATOR")
        print(f"Mode: {args.mode}")
        print(f"Timestamp: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print("─" * 70)
        
        if args.mode == 'single':
            main_single_currency()
        elif args.mode == 'multi':
            main_multi_currency()
        elif args.mode == 'real':
            main_with_real_data()
        elif args.mode == 'interactive':
            main_interactive()
        elif args.mode == 'demo':
            print("Running comprehensive demo...")
            # 简化的演示模式
            main_single_currency()
            print("\n" + "="*70 + "\n")
            main_multi_currency()
        else:
            print_error(f"Unknown mode: {args.mode}")
            parser.print_help()
    
    except KeyboardInterrupt:
        print("\n\n⏹️ Simulation interrupted by user.")
    except Exception as e:
        print_error(f"Unexpected error: {e}")
        if not args.quiet:
            traceback.print_exc()
    finally:
        print_header("SIMULATION ENDED")
        print("Thank you for using Currency Monte Carlo Simulator!")
        print("📁 Results are saved in the 'simulation_results' directory")
        print("🖼️  Plots are saved in the 'plots' directory")


if __name__ == "__main__":
    main()
