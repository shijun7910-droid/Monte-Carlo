import pandas as pd
import numpy as np
import scipy.stats as stats
from pathlib import Path
import argparse
import json

class ResultAnalyzer:
    def __init__(self):
        self.results = {}
    
    def load_results(self, filepath):
        """Load simulation results from file."""
        try:
            df = pd.read_csv(filepath)
            return df
        except Exception as e:
            print(f"Error loading {filepath}: {e}")
            return None
    
    def calculate_statistics(self, data_series):
        """Calculate comprehensive statistics."""
        if data_series is None or len(data_series) == 0:
            return {}
        
        stats_dict = {
            'count': len(data_series),
            'mean': float(data_series.mean()),
            'median': float(data_series.median()),
            'std': float(data_series.std()),
            'variance': float(data_series.var()),
            'min': float(data_series.min()),
            'max': float(data_series.max()),
            'range': float(data_series.max() - data_series.min()),
            'iqr': float(data_series.quantile(0.75) - data_series.quantile(0.25)),
            'skewness': float(data_series.skew()),
            'kurtosis': float(data_series.kurtosis()),
            'cv': float(data_series.std() / data_series.mean() if data_series.mean() != 0 else 0)
        }
        
        # Add percentiles
        percentiles = [0.01, 0.05, 0.10, 0.25, 0.50, 0.75, 0.90, 0.95, 0.99]
        for p in percentiles:
            stats_dict[f'percentile_{int(p*100)}'] = float(data_series.quantile(p))
        
        return stats_dict
    
    def calculate_risk_metrics(self, returns_series, risk_free_rate=0.03):
        """Calculate financial risk metrics."""
        if returns_series is None or len(returns_series) == 0:
            return {}
        
        # Basic metrics
        mean_return = returns_series.mean()
        std_return = returns_series.std()
        
        # Sort for VaR calculation
        sorted_returns = np.sort(returns_series)
        
        risk_metrics = {
            'mean_return': float(mean_return),
            'std_return': float(std_return),
            'annualized_return': float(mean_return * 252),
            'annualized_volatility': float(std_return * np.sqrt(252)),
            'sharpe_ratio': float((mean_return - risk_free_rate/252) / std_return * np.sqrt(252) 
                                if std_return != 0 else 0),
            'sortino_ratio': float(self._calculate_sortino_ratio(returns_series, risk_free_rate)),
            'var_90': float(sorted_returns[int(0.10 * len(sorted_returns))]),
            'var_95': float(sorted_returns[int(0.05 * len(sorted_returns))]),
            'var_99': float(sorted_returns[int(0.01 * len(sorted_returns))]),
            'cvar_90': float(sorted_returns[:int(0.10 * len(sorted_returns))].mean()),
            'cvar_95': float(sorted_returns[:int(0.05 * len(sorted_returns))].mean()),
            'cvar_99': float(sorted_returns[:int(0.01 * len(sorted_returns))].mean()),
            'max_drawdown': float(self._calculate_max_drawdown(returns_series)),
            'calmar_ratio': float(self._calculate_calmar_ratio(returns_series))
        }
        
        return risk_metrics
    
    def _calculate_sortino_ratio(self, returns, risk_free_rate):
        """Calculate Sortino ratio (downside risk adjusted)."""
        downside_returns = returns[returns < risk_free_rate/252]
        if len(downside_returns) == 0:
            return 0
        
        downside_std = downside_returns.std()
        if downside_std == 0:
            return 0
        
        return (returns.mean() - risk_free_rate/252) / downside_std * np.sqrt(252)
    
    def _calculate_max_drawdown(self, returns):
        """Calculate maximum drawdown."""
        cumulative = (1 + returns).cumprod()
        running_max = cumulative.expanding().max()
        drawdown = (cumulative - running_max) / running_max
        return drawdown.min()
    
    def _calculate_calmar_ratio(self, returns):
        """Calculate Calmar ratio (return to max drawdown)."""
        max_dd = self._calculate_max_drawdown(returns)
        if max_dd == 0:
            return 0
        return (returns.mean() * 252) / abs(max_dd)
    
    def test_normality(self, data_series):
        """Test if data follows normal distribution."""
        if len(data_series) < 8:
            return {'insufficient_data': True}
        
        # Shapiro-Wilk test (good for small samples)
        shapiro_stat, shapiro_p = stats.shapiro(data_series)
        
        # Anderson-Darling test
        anderson_result = stats.anderson(data_series, dist='norm')
        
        # Kolmogorov-Smirnov test
        ks_stat, ks_p = stats.kstest(data_series, 'norm', 
                                     args=(data_series.mean(), data_series.std()))
        
        return {
            'shapiro_wilk': {
                'statistic': float(shapiro_stat),
                'p_value': float(shapiro_p),
                'normal': shapiro_p > 0.05
            },
            'anderson_darling': {
                'statistic': float(anderson_result.statistic),
                'critical_values': anderson_result.critical_values.tolist(),
                'significance_levels': anderson_result.significance_level.tolist(),
                'normal': anderson_result.statistic < anderson_result.critical_values[2]  # 5% level
            },
            'kolmogorov_smirnov': {
                'statistic': float(ks_stat),
                'p_value': float(ks_p),
                'normal': ks_p > 0.05
            }
        }
    
    def analyze_convergence(self, prices_series, batch_sizes=None):
        """Analyze Monte Carlo convergence."""
        if batch_sizes is None:
            batch_sizes = [2, 4, 8, 16, 32]
        
        n = len(prices_series)
        convergence_results = {}
        
        for batch_size in batch_sizes:
            if n // batch_size < 2:
                continue
            
            # Split into batches
            batch_means = []
            for i in range(batch_size):
                start = i * (n // batch_size)
                end = (i + 1) * (n // batch_size) if i < batch_size - 1 else n
                batch = prices_series[start:end]
                batch_means.append(b
