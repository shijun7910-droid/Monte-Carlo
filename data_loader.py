import pandas as pd
import numpy as np
import yfinance as yf
from datetime import datetime, timedelta
from typing import Optional, Tuple, Dict, List
import os
import warnings

# 忽略警告信息
warnings.filterwarnings('ignore')

class ExchangeRateDataLoader:
    """
    Load and preprocess exchange rate data from multiple sources
    """
    
    # 货币对映射表（雅虎财经代码）
    CURRENCY_PAIRS = {
        'USD/RUB': 'RUB=X',      # USD to RUB
        'EUR/USD': 'EURUSD=X',   # EUR to USD
        'USD/JPY': 'JPY=X',      # USD to JPY
        'GBP/USD': 'GBPUSD=X',   # GBP to USD
        'USD/CNY': 'CNY=X',      # USD to CNY
        'AUD/USD': 'AUDUSD=X',   # AUD to USD
        'USD/CAD': 'CAD=X',      # USD to CAD
        'USD/CHF': 'CHF=X',      # USD to CHF
        'NZD/USD': 'NZDUSD=X',   # NZD to USD
    }
    
    @classmethod
    def get_yahoo_symbol(cls, currency_pair: str) -> str:
        """
        获取雅虎财经的货币对符号
        
        Parameters:
        -----------
        currency_pair : str
            货币对名称，如 'USD/RUB'
            
        Returns:
        --------
        str
            雅虎财经符号
        """
        # 如果已经提供了雅虎符号，直接返回
        if '=' in currency_pair:
            return currency_pair
        
        # 从映射表中查找
        return cls.CURRENCY_PAIRS.get(currency_pair.upper(), f"{currency_pair.replace('/', '')}=X")
    
    @classmethod
    def load_from_yahoo(cls, currency_pair: str, 
                       start_date: str = '2023-01-01', 
                       end_date: Optional[str] = None, 
                       interval: str = '1d',
                       auto_adjust: bool = True) -> pd.DataFrame:
        """
        从雅虎财经加载汇率数据
        
        Parameters:
        -----------
        currency_pair : str
            货币对符号 (如 'USD/RUB' 或 'USDRUB=X')
        start_date : str
            开始日期 'YYYY-MM-DD' 格式
        end_date : str, optional
            结束日期 'YYYY-MM-DD' 格式（默认为今天）
        interval : str
            数据间隔 ('1d', '1h', '1wk', '1mo'等)
        auto_adjust : bool
            是否自动调整价格
            
        Returns:
        --------
        pd.DataFrame
            包含汇率数据的DataFrame
        """
        try:
            # 获取雅虎财经符号
            yahoo_symbol = cls.get_yahoo_symbol(currency_pair)
            print(f"Fetching data for {currency_pair} ({yahoo_symbol}) from Yahoo Finance...")
            
            # 下载数据
            ticker = yf.Ticker(yahoo_symbol)
            data = ticker.history(start=start_date, end=end_date, 
                                 interval=interval, auto_adjust=auto_adjust)
            
            if data.empty:
                print(f"⚠️ No data found for {currency_pair} ({yahoo_symbol})")
                print(f"尝试的日期范围: {start_date} to {end_date or datetime.now().date()}")
                return pd.DataFrame()
            
            # 处理列名
            if 'Close' in data.columns:
                close_col = 'Close'
            elif 'Adj Close' in data.columns:
                close_col = 'Adj Close'
            else:
                close_col = data.columns[0]
            
            # 创建新的DataFrame
            df = pd.DataFrame({
                'Exchange Rate': data[close_col]
            })
            df.index.name = 'Date'
            
            # 移除NaN值
            df = df.dropna()
            
            print(f"✅ Successfully loaded {len(df)} records for {currency_pair}")
            print(f"📅 Date range: {df.index[0].date()} to {df.index[-1].date()}")
            print(f"💰 Rate range: {df['Exchange Rate'].min():.4f} - {df['Exchange Rate'].max():.4f}")
            
            return df
            
        except Exception as e:
            print(f"❌ Error loading data from Yahoo Finance for {currency_pair}: {str(e)}")
            return pd.DataFrame()
    
    @classmethod
    def load_multiple_currencies(cls, currency_pairs: List[str], 
                               start_date: str = '2023-01-01',
                               end_date: Optional[str] = None,
                               interval: str = '1d') -> Dict[str, pd.DataFrame]:
        """
        同时加载多个货币对的数据
        
        Parameters:
        -----------
        currency_pairs : List[str]
            货币对列表
        start_date : str
            开始日期
        end_date : str, optional
            结束日期
        interval : str
            数据间隔
            
        Returns:
        --------
        Dict[str, pd.DataFrame]
            字典，键为货币对名称，值为汇率数据
        """
        results = {}
        
        for pair in currency_pairs:
            print(f"\nLoading data for {pair}...")
            data = cls.load_from_yahoo(pair, start_date, end_date, interval)
            if not data.empty:
                results[pair] = data
            else:
                print(f"Skipping {pair} due to data loading failure")
        
        print(f"\n✅ Successfully loaded {len(results)} out of {len(currency_pairs)} currency pairs")
        return results
    
    @classmethod
    def load_from_csv(cls, filepath: str, 
                     date_col: str = 'Date',
                     rate_col: str = 'Exchange Rate') -> pd.DataFrame:
        """
        从CSV文件加载汇率数据
        
        Parameters:
        -----------
        filepath : str
            CSV文件路径
        date_col : str
            日期列名
        rate_col : str
            汇率列名
            
        Returns:
        --------
        pd.DataFrame
            包含汇率数据的DataFrame
        """
        try:
            if not os.path.exists(filepath):
                print(f"❌ File not found: {filepath}")
                return pd.DataFrame()
            
            print(f"📂 Loading data from {filepath}...")
            
            # 尝试自动检测分隔符
            with open(filepath, 'r') as f:
                first_line = f.readline()
            
            # 检测分隔符
            if ',' in first_line:
                delimiter = ','
            elif ';' in first_line:
                delimiter = ';'
            elif '\t' in first_line:
                delimiter = '\t'
            else:
                delimiter = ','  # 默认使用逗号
            
            # 读取CSV文件
            data = pd.read_csv(filepath, delimiter=delimiter, parse_dates=[date_col])
            
            # 设置日期索引
            if date_col in data.columns:
                data = data.set_index(date_col)
                data.index.name = 'Date'
            else:
                print(f"❌ Date column '{date_col}' not found in file")
                return pd.DataFrame()
            
            # 重命名汇率列
            if rate_col in data.columns:
                data = data[[rate_col]].copy()
                data.columns = ['Exchange Rate']
            else:
                # 尝试自动找到数值列
                numeric_cols = data.select_dtypes(include=[np.number]).columns
                if len(numeric_cols) > 0:
                    data = data[[numeric_cols[0]]].copy()
                    data.columns = ['Exchange Rate']
                    print(f"⚠️ Using column '{numeric_cols[0]}' as exchange rate")
                else:
                    print("❌ No numeric column found for exchange rate")
                    return pd.DataFrame()
            
            # 移除NaN值
            data = data.dropna()
            
            print(f"✅ Successfully loaded {len(data)} records")
            print(f"📅 Date range: {data.index[0].date()} to {data.index[-1].date()}")
            
            return data
            
        except Exception as e:
            print(f"❌ Error loading CSV file {filepath}: {str(e)}")
            return pd.DataFrame()
    
    @classmethod
    def calculate_statistics(cls, data: pd.DataFrame, 
                           column: str = 'Exchange Rate',
                           trading_days_per_year: int = 252) -> Dict:
        """
        从汇率数据计算基本统计量
        
        Parameters:
        -----------
        data : pd.DataFrame
            汇率数据
        column : str
            包含汇率的列名
        trading_days_per_year : int
            每年交易天数
            
        Returns:
        --------
        Dict
            包含统计量的字典
        """
        if data.empty:
            print("⚠️ No data to calculate statistics")
            return {}
        
        rates = data[column].dropna()
        
        if len(rates) < 2:
            print("⚠️ Insufficient data for statistics (need at least 2 data points)")
            return {}
        
        # 计算日收益率
        returns = rates.pct_change().dropna()
        log_returns = np.log(rates / rates.shift(1)).dropna()
        
        # 计算基本统计量
        statistics = {
            # 基本统计
            'current_rate': float(rates.iloc[-1]),
            'mean_rate': float(rates.mean()),
            'median_rate': float(rates.median()),
            'min_rate': float(rates.min()),
            'max_rate': float(rates.max()),
            'std_rate': float(rates.std()),
            
            # 收益率统计
            'mean_daily_return': float(returns.mean()),
            'std_daily_return': float(returns.std()),
            'skew_daily_return': float(returns.skew()),
            'kurtosis_daily_return': float(returns.kurtosis()),
            
            # 对数收益率统计
            'mean_log_return': float(log_returns.mean()),
            'std_log_return': float(log_returns.std()),
            
            # 年化统计量
            'volatility_daily': float(returns.std()),
            'volatility_annual': float(returns.std() * np.sqrt(trading_days_per_year)),
            'drift_daily': float(returns.mean()),
            'drift_annual': float(returns.mean() * trading_days_per_year),
            'sharpe_ratio': float(returns.mean() / returns.std() * np.sqrt(trading_days_per_year)) if returns.std() > 0 else 0,
            
            # 数据信息
            'total_days': len(rates),
            'total_returns': len(returns),
            'start_date': rates.index[0].date(),
            'end_date': rates.index[-1].date(),
            'date_range_days': (rates.index[-1] - rates.index[0]).days,
        }
        
        return statistics
    
    @classmethod
    def prepare_training_data(cls, data: pd.DataFrame, 
                            forecast_days: int = 30,
                            column: str = 'Exchange Rate',
                            return_type: str = 'log') -> Tuple[np.ndarray, float, Dict]:
        """
        为蒙特卡洛模拟准备数据
        
        Parameters:
        -----------
        data : pd.DataFrame
            历史汇率数据
        forecast_days : int
            预测天数
        column : str
            包含汇率的列名
        return_type : str
            收益率类型 ('log' 或 'simple')
            
        Returns:
        --------
        Tuple[np.ndarray, float, Dict]
            (历史收益率数组, 最新汇率, 统计信息字典)
        """
        if data.empty:
            print("⚠️ No data for training")
            return np.array([]), 0.0, {}
        
        rates = data[column].dropna()
        
        if len(rates) < 2:
            print("⚠️ Insufficient data for training (need at least 2 data points)")
            return np.array([]), 0.0, {}
        
        latest_rate = float(rates.iloc[-1])
        
        # 计算收益率
        if return_type == 'log':
            # 对数收益率 (更适合GBM模型)
            returns = np.log(rates / rates.shift(1)).dropna().values
        else:
            # 简单收益率
            returns = rates.pct_change().dropna().values
        
        # 计算训练数据的统计信息
        training_stats = {
            'n_observations': len(returns),
            'return_mean': float(np.mean(returns)),
            'return_std': float(np.std(returns)),
            'return_min': float(np.min(returns)),
            'return_max': float(np.max(returns)),
            'latest_rate': latest_rate,
            'return_type': return_type
        }
        
        print(f"✅ Prepared {len(returns)} return observations for training")
        print(f"   Latest rate: {latest_rate:.4f}")
        print(f"   Return mean: {training_stats['return_mean']:.6f}")
        print(f"   Return std: {training_stats['return_std']:.6f}")
        
        return returns, latest_rate, training_stats
    
    @classmethod
    def analyze_volatility(cls, data: pd.DataFrame, 
                          column: str = 'Exchange Rate',
                          window_days: int = 20) -> pd.DataFrame:
        """
        分析波动性，计算滚动波动率
        
        Parameters:
        -----------
        data : pd.DataFrame
            汇率数据
        column : str
            包含汇率的列名
        window_days : int
            滚动窗口天数
            
        Returns:
        --------
        pd.DataFrame
            包含波动率分析的DataFrame
        """
        if data.empty:
            return pd.DataFrame()
        
        rates = data[column].dropna()
        returns = rates.pct_change().dropna()
        
        # 计算滚动波动率
        rolling_volatility = returns.rolling(window=window_days).std() * np.sqrt(252)
        
        # 创建结果DataFrame
        result = pd.DataFrame({
            'Rate': rates,
            'Daily Return': returns,
            f'Rolling {window_days}D Volatility (Annualized)': rolling_volatility
        })
        
        # 计算波动率统计
        vol_stats = {
            'mean_volatility': float(rolling_volatility.mean()),
            'median_volatility': float(rolling_volatility.median()),
            'max_volatility': float(rolling_volatility.max()),
            'min_volatility': float(rolling_volatility.min()),
            'current_volatility': float(rolling_volatility.iloc[-1]) if not pd.isna(rolling_volatility.iloc[-1]) else 0.0
        }
        
        return result, vol_stats
    
    @classmethod
    def detect_outliers(cls, data: pd.DataFrame, 
                       column: str = 'Exchange Rate',
                       z_score_threshold: float = 3.0) -> pd.DataFrame:
        """
        检测汇率数据中的异常值
        
        Parameters:
        -----------
        data : pd.DataFrame
            汇率数据
        column : str
            包含汇率的列名
        z_score_threshold : float
            Z-score阈值
            
        Returns:
        --------
        pd.DataFrame
            包含异常值信息
        """
        if data.empty:
            return pd.DataFrame()
        
        rates = data[column].dropna()
        returns = rates.pct_change().dropna()
        
        # 计算Z-score
        mean_return = returns.mean()
        std_return = returns.std()
        
        if std_return == 0:
            print("⚠️ Zero standard deviation, cannot detect outliers")
            return pd.DataFrame()
        
        z_scores = (returns - mean_return) / std_return
        
        # 识别异常值
        outliers_mask = np.abs(z_scores) > z_score_threshold
        outliers = returns[outliers_mask]
        
        if len(outliers) > 0:
            print(f"🔍 Found {len(outliers)} outliers (Z-score > {z_score_threshold}):")
            for date, ret in outliers.items():
                z_score = z_scores[date]
                print(f"   {date.date()}: Return={ret:.4%}, Z-score={z_score:.2f}")
        else:
            print(f"✅ No outliers found (Z-score > {z_score_threshold})")
        
        return pd.DataFrame({
            'Date': outliers.index,
            'Return': outliers.values,
            'Z_Score': [z_scores[date] for date in outliers.index]
        })
    
    @classmethod
    def create_sample_data(cls, currency_pair: str = 'USD/RUB',
                          start_date: str = '2023-01-01',
                          end_date: str = '2023-12-31',
                          initial_rate: float = 75.0,
                          drift: float = 0.0002,
                          volatility: float = 0.01) -> pd.DataFrame:
        """
        创建样本汇率数据用于测试
        
        Parameters:
        -----------
        currency_pair : str
            货币对名称
        start_date : str
            开始日期
        end_date : str
            结束日期
        initial_rate : float
            初始汇率
        drift : float
            日漂移率
        volatility : float
            日波动率
            
        Returns:
        --------
        pd.DataFrame
            样本汇率数据
        """
        try:
            dates = pd.date_range(start=start_date, end=end_date, freq='B')  # 工作日频率
            np.random.seed(42)
            
            rates = [initial_rate]
            for i in range(1, len(dates)):
                # 使用几何布朗运动生成汇率
                daily_return = drift + volatility * np.random.randn()
                new_rate = rates[-1] * np.exp(daily_return - 0.5 * volatility**2)
                rates.append(new_rate)
            
            data = pd.DataFrame({
                'Date': dates,
                'Exchange Rate': rates,
                'Currency': currency_pair
            })
            data.set_index('Date', inplace=True)
            
            print(f"✅ Created sample data for {currency_pair}")
            print(f"   Date range: {start_date} to {end_date}")
            print(f"   Initial rate: {initial_rate:.4f}")
            print(f"   Drift: {drift:.6f}, Volatility: {volatility:.6f}")
            print(f"   Total records: {len(data)}")
            
            return data
            
        except Exception as e:
            print(f"❌ Error creating sample data: {str(e)}")
            return pd.DataFrame()
    
    @classmethod
    def save_to_csv(cls, data: pd.DataFrame, filepath: str):
        """
        保存汇率数据到CSV文件
        
        Parameters:
        -----------
        data : pd.DataFrame
            汇率数据
        filepath : str
            保存路径
        """
        try:
            # 确保目录存在
            os.makedirs(os.path.dirname(filepath), exist_ok=True)
            
            # 保存到CSV
            data.reset_index().to_csv(filepath, index=False)
            print(f"✅ Data saved to {filepath}")
            
        except Exception as e:
            print(f"❌ Error saving data to {filepath}: {str(e)}")
    
    @classmethod
    def generate_report(cls, data: pd.DataFrame, 
                       currency_pair: str = "Unknown",
                       column: str = 'Exchange Rate') -> str:
        """
        生成数据报告
        
        Parameters:
        -----------
        data : pd.DataFrame
            汇率数据
        currency_pair : str
            货币对名称
        column : str
            包含汇率的列名
            
        Returns:
        --------
        str
            格式化的报告字符串
        """
        if data.empty:
            return "No data available for report"
        
        stats = cls.calculate_statistics(data, column)
        
        report = f"""
╔══════════════════════════════════════════════════════════════╗
║                    EXCHANGE RATE DATA REPORT                 ║
╠══════════════════════════════════════════════════════════════╣
║ Currency Pair: {currency_pair:45} ║
║ Date Range: {stats['start_date']} to {stats['end_date']:28} ║
║ Total Days: {stats['total_days']:44} ║
╠══════════════════════════════════════════════════════════════╣
║                         RATE STATISTICS                      ║
╠══════════════════════════════════════════════════════════════╣
║ Current Rate:          {stats['current_rate']:10.4f}                  ║
║ Mean Rate:            {stats['mean_rate']:10.4f}                  ║
║ Median Rate:          {stats['median_rate']:10.4f}                  ║
║ Minimum Rate:         {stats['min_rate']:10.4f}                  ║
║ Maximum Rate:         {stats['max_rate']:10.4f}                  ║
║ Standard Deviation:   {stats['std_rate']:10.4f}                  ║
╠══════════════════════════════════════════════════════════════╣
║                      RETURN STATISTICS                       ║
╠══════════════════════════════════════════════════════════════╣
║ Daily Mean Return:    {stats['mean_daily_return']:10.4%}                ║
║ Daily Std Return:     {stats['std_daily_return']:10.4%}                ║
║ Annual Volatility:    {stats['volatility_annual']:10.4%}                ║
║ Annual Drift:         {stats['drift_annual']:10.4%}                ║
║ Sharpe Ratio:         {stats['sharpe_ratio']:10.4f}                  ║
║ Skewness:             {stats['skew_daily_return']:10.4f}                  ║
║ Kurtosis:             {stats['kurtosis_daily_return']:10.4f}                  ║
╚══════════════════════════════════════════════════════════════╝
"""
        return report


def main():
    """主要演示函数"""
    print("=" * 70)
    print("EXCHANGE RATE DATA LOADER DEMONSTRATION")
    print("=" * 70)
    
    # 创建数据加载器实例
    loader = ExchangeRateDataLoader()
    
    # 选项菜单
    print("\nSelect an option:")
    print("1. Load real data from Yahoo Finance")
    print("2. Create and analyze sample data")
    print("3. Load data from CSV file")
    print("4. Load multiple currencies")
    print("5. Exit")
    
    choice = input("\nEnter your choice (1-5): ")
    
    if choice == '1':
        # 从雅虎财经加载真实数据
        currency = input("Enter currency pair (e.g., USD/RUB): ") or "USD/RUB"
        start_date = input("Enter start date (YYYY-MM-DD): ") or "2023-01-01"
        end_date = input("Enter end date (YYYY-MM-DD, leave empty for today): ") or None
        
        data = loader.load_from_yahoo(currency, start_date, end_date)
        
        if not data.empty:
            # 计算并显示统计量
            stats = loader.calculate_statistics(data)
            
            print("\n" + "-" * 60)
            print("STATISTICS SUMMARY")
            print("-" * 60)
            for key, value in stats.items():
                if isinstance(value, float):
                    if 'volatility' in key or 'drift' in key or 'return' in key:
                        print(f"{key:25}: {value:.4%}")
                    elif 'rate' in key:
                        print(f"{key:25}: {value:.4f}")
                    else:
                        print(f"{key:25}: {value}")
                elif isinstance(value, datetime):
                    print(f"{key:25}: {value.date()}")
                else:
                    print(f"{key:25}: {value}")
            
            # 保存数据
            save_option = input("\nSave data to CSV? (y/n): ")
            if save_option.lower() == 'y':
                filename = input("Enter filename (default: exchange_rates.csv): ") or "exchange_rates.csv"
                loader.save_to_csv(data, filename)
    
    elif choice == '2':
        # 创建并分析样本数据
        print("\nCreating sample data...")
        data = loader.create_sample_data()
        
        if not data.empty:
            # 显示报告
            report = loader.generate_report(data, "USD/RUB (Sample)")
            print(report)
            
            # 分析波动性
            vol_analysis, vol_stats = loader.analyze_volatility(data)
            if not vol_analysis.empty:
                print("\n" + "-" * 60)
                print("VOLATILITY ANALYSIS")
                print("-" * 60)
                for key, value in vol_stats.items():
                    print(f"{key:25}: {value:.4%}")
            
            # 检测异常值
            print("\n" + "-" * 60)
            print("OUTLIER DETECTION")
            print("-" * 60)
            outliers = loader.detect_outliers(data)
            
            # 保存样本数据
            save_option = input("\nSave sample data to CSV? (y/n): ")
            if save_option.lower() == 'y':
                loader.save_to_csv(data, "sample_rates.csv")
    
    elif choice == '3':
        # 从CSV文件加载数据
        filepath = input("Enter CSV file path: ")
        if os.path.exists(filepath):
            data = loader.load_from_csv(filepath)
            
            if not data.empty:
                # 显示报告
                currency = input("Enter currency pair name: ") or "Unknown"
                report = loader.generate_report(data, currency)
                print(report)
        else:
            print(f"❌ File not found: {filepath}")
    
    elif choice == '4':
        # 加载多个货币对
        print("\nAvailable currency pairs:")
        for pair in loader.CURRENCY_PAIRS.keys():
            print(f"  - {pair}")
        
        pairs_input = input("\nEnter currency pairs (comma-separated): ") or "USD/RUB, EUR/USD"
        currency_pairs = [p.strip() for p in pairs_input.split(',')]
        
        start_date = input("Enter start date (YYYY-MM-DD): ") or "2023-01-01"
        
        print(f"\nLoading {len(currency_pairs)} currency pairs...")
        all_data = loader.load_multiple_currencies(currency_pairs, start_date)
        
        if all_data:
            print("\n" + "=" * 60)
            print("MULTI-CURRENCY SUMMARY")
            print("=" * 60)
            
            summary_data = []
            for pair, data in all_data.items():
                if not data.empty:
                    stats = loader.calculate_statistics(data)
                    summary_data.append({
                        'Currency Pair': pair,
                        'Current Rate': stats['current_rate'],
                        'Annual Volatility': stats['volatility_annual'],
                        'Annual Drift': stats['drift_annual'],
                        'Sharpe Ratio': stats['sharpe_ratio'],
                        'Days': stats['total_days']
                    })
            
            # 显示汇总表格
            summary_df = pd.DataFrame(summary_data)
            print("\nSummary Statistics:")
            print(summary_df.to_string(index=False, float_format=lambda x: f"{x:.4f}" if isinstance(x, float) else str(x)))
    
    elif choice == '5':
        print("Exiting...")
        return
    
    else:
        print("❌ Invalid choice")
    
    print("\n" + "=" * 70)
    print("DEMONSTRATION COMPLETED")
    print("=" * 70)


if __name__ == "__main__":
    main()
