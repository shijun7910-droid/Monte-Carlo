
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
                batch_means.append(batch.mean())
            
            # Calculate batch statistics
            batch_mean = np.mean(batch_means)
            batch_std = np.std(batch_means, ddof=1)
            standard_error = batch_std / np.sqrt(batch_size)
            relative_error = standard_error / abs(batch_mean) if batch_mean != 0 else standard_error
            
            convergence_results[f'batch_{batch_size}'] = {
                'batch_mean': float(batch_mean),
                'batch_std': float(batch_std),
                'standard_error': float(standard_error),
                'relative_error': float(relative_error),
                'converged': relative_error < 0.01
            }
        
        return convergence_results
    
    def generate_report(self, filepath, output_dir="reports"):
        """Generate comprehensive analysis report."""
        df = self.load_results(filepath)
        if df is None:
            return
        
        # Create output directory
        output_path = Path(output_dir)
        output_path.mkdir(exist_ok=True, parents=True)
        
        filename = Path(filepath).stem
        report = {
            'file': filepath,
            'timestamp': pd.Timestamp.now().isoformat(),
            'data_summary': {
                'rows': len(df),
                'columns': len(df.columns),
                'column_names': df.columns.tolist()
            }
        }
        
        # Analyze each column
        for column in df.columns:
            series = df[column]
            
            column_report = {
                'statistics': self.calculate_statistics(series),
                'normality_tests': self.test_normality(series),
                'convergence_analysis': self.analyze_convergence(series)
            }
            
            # If column looks like returns, add risk metrics
            if series.mean() < 1 and series.std() < 1:  # Heuristic for returns
                column_report['risk_metrics'] = self.calculate_risk_metrics(series)
            
            report[column] = column_report
        
        # Save report as JSON
        json_file = output_path / f"{filename}_analysis.json"
        with open(json_file, 'w') as f:
            json.dump(report, f, indent=2, default=str)
        
        # Save summary as CSV
        summary_data = []
        for column, analysis in report.items():
            if column not in ['file', 'timestamp', 'data_summary']:
                stats = analysis.get('statistics', {})
                summary_data.append({
                    'column': column,
                    'mean': stats.get('mean', np.nan),
                    'median': stats.get('median', np.nan),
                    'std': stats.get('std', np.nan),
                    'min': stats.get('min', np.nan),
                    'max': stats.get('max', np.nan),
                    'skewness': stats.get('skewness', np.nan),
                    'kurtosis': stats.get('kurtosis', np.nan),
                    'normal_distribution': analysis.get('normality_tests', {})
                                               .get('shapiro_wilk', {})
                                               .get('normal', False)
                })
        
        summary_df = pd.DataFrame(summary_data)
        csv_file = output_path / f"{filename}_summary.csv"
        summary_df.to_csv(csv_file, index=False)
        
        print(f"Report saved to {json_file}")
        print(f"Summary saved to {csv_file}")
        
        # Print key findings
        self._print_summary(report)
    
    def _print_summary(self, report):
        """Print summary of analysis results."""
        print("\n" + "="*60)
        print("ANALYSIS SUMMARY")
        print("="*60)
        
        for column, analysis in report.items():
            if column not in ['file', 'timestamp', 'data_summary']:
                print(f"\nColumn: {column}")
                print("-"*40)
                
                stats = analysis.get('statistics', {})
                if stats:
                    print(f"Statistics:")
                    print(f"  Count: {stats.get('count', 'N/A'):,}")
                    print(f"  Mean: {stats.get('mean', 'N/A'):.4f}")
                    print(f"  Std Dev: {stats.get('std', 'N/A'):.4f}")
                    print(f"  Min: {stats.get('min', 'N/A'):.4f}")
                    print(f"  Max: {stats.get('max', 'N/A'):.4f}")
                
                normality = analysis.get('normality_tests', {})
                if normality and not normality.get('insufficient_data', False):
                    sw_test = normality.get('shapiro_wilk', {})
                    print(f"\nNormality Test (Shapiro-Wilk):")
                    print(f"  Statistic: {sw_test.get('statistic', 'N/A'):.4f}")
                    print(f"  p-value: {sw_test.get('p_value', 'N/A'):.4f}")
                    print(f"  Normal distribution: {'Yes' if sw_test.get('normal', False) else 'No'}")
                
                risk_metrics = analysis.get('risk_metrics', {})
                if risk_metrics:
                    print(f"\nRisk Metrics:")
                    print(f"  Annualized Return: {risk_metrics.get('annualized_return', 'N/A'):.2%}")
                    print(f"  Annualized Volatility: {risk_metrics.get('annualized_volatility', 'N/A'):.2%}")
                    print(f"  Sharpe Ratio: {risk_metrics.get('sharpe_ratio', 'N/A'):.2f}")
                    print(f"  95% VaR: {risk_metrics.get('var_95', 'N/A'):.2%}")
                    print(f"  95% CVaR: {risk_metrics.get('cvar_95', 'N/A'):.2%}")
                    print(f"  Max Drawdown: {risk_metrics.get('max_drawdown', 'N/A'):.2%}")

def main():
    parser = argparse.ArgumentParser(description='Analyze Monte Carlo simulation results')
    parser.add_argument('input_file', type=str, help='Input CSV file with simulation results')
    parser.add_argument('--output-dir', type=str, default='reports',
                       help='Output directory for analysis reports')
    parser.add_argument('--column', type=str, help='Specific column to analyze')
    parser.add_argument('--risk-free-rate', type=float, default=0.03,
                       help='Risk-free rate for risk metrics calculation')
    
    args = parser.parse_args()
    
    analyzer = ResultAnalyzer()
    
    # Generate comprehensive report
    analyzer.generate_report(args.input_file, args.output_dir)

if __name__ == "__main__":
    main()
