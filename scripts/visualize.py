import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path
import argparse
import sys
import json

# Set style
plt.style.use('seaborn-v0_8-darkgrid')
sns.set_palette("husl")

class ResultVisualizer:
    def __init__(self, results_dir="results"):
        self.results_dir = Path(results_dir)
        self.results_dir.mkdir(exist_ok=True)
        
    def load_simulation_data(self, price_file):
        """Load simulation results from CSV file."""
        try:
            df = pd.read_csv(price_file)
            return df
        except Exception as e:
            print(f"Error loading {price_file}: {e}")
            return None
    
    def plot_price_distribution(self, df, title="Simulation Results", output_file=None):
        """Plot distribution of simulated prices."""
        if df is None or df.empty:
            print("No data to plot")
            return
        
        fig, axes = plt.subplots(2, 2, figsize=(15, 10))
        fig.suptitle(title, fontsize=16, fontweight='bold')
        
        # Histogram
        ax = axes[0, 0]
        ax.hist(df.iloc[:, 0], bins=50, alpha=0.7, color='steelblue', edgecolor='black')
        ax.axvline(df.iloc[:, 0].mean(), color='red', linestyle='--', linewidth=2, 
                   label=f'Mean: {df.iloc[:, 0].mean():.2f}')
        ax.axvline(df.iloc[:, 0].median(), color='green', linestyle='--', linewidth=2,
                   label=f'Median: {df.iloc[:, 0].median():.2f}')
        ax.set_xlabel('Price')
        ax.set_ylabel('Frequency')
        ax.set_title('Price Distribution')
        ax.legend()
        ax.grid(True, alpha=0.3)
        
        # Box plot
        ax = axes[0, 1]
        ax.boxplot(df.iloc[:, 0], vert=True)
        ax.set_ylabel('Price')
        ax.set_title('Box Plot')
        ax.grid(True, alpha=0.3)
        
        # QQ plot for normality check
        ax = axes[1, 0]
        from scipy import stats
        stats.probplot(df.iloc[:, 0], dist="norm", plot=ax)
        ax.set_title('Q-Q Plot (Normality Check)')
        ax.grid(True, alpha=0.3)
        
        # Kernel Density Estimate
        ax = axes[1, 1]
        sns.kdeplot(data=df.iloc[:, 0], ax=ax, fill=True, alpha=0.5)
        ax.set_xlabel('Price')
        ax.set_ylabel('Density')
        ax.set_title('Density Estimate')
        ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Plot saved to {output_file}")
        
        plt.show()
        
        # Print statistics
        print("\n" + "="*50)
        print(f"Statistics for {title}:")
        print("="*50)
        print(f"Count: {len(df):,}")
        print(f"Mean: {df.iloc[:, 0].mean():.4f}")
        print(f"Median: {df.iloc[:, 0].median():.4f}")
        print(f"Std Dev: {df.iloc[:, 0].std():.4f}")
        print(f"Min: {df.iloc[:, 0].min():.4f}")
        print(f"Max: {df.iloc[:, 0].max():.4f}")
        print(f"25th percentile: {df.iloc[:, 0].quantile(0.25):.4f}")
        print(f"75th percentile: {df.iloc[:, 0].quantile(0.75):.4f}")
    
    def plot_simulation_paths(self, paths_file, max_paths=100, output_file=None):
        """Plot sample simulation paths."""
        try:
            df = pd.read_csv(paths_file)
            
            # Extract price columns (skip 'Path' column if present)
            if 'Path' in df.columns:
                price_columns = [col for col in df.columns if col.startswith('Step_')]
                paths = df[price_columns].values
            else:
                paths = df.values
            
            plt.figure(figsize=(12, 6))
            
            # Plot individual paths
            for i in range(min(max_paths, len(paths))):
                plt.plot(paths[i], alpha=0.1, color='blue', linewidth=0.5)
            
            # Plot mean path
            mean_path = np.mean(paths[:max_paths], axis=0)
            plt.plot(mean_path, color='red', linewidth=2, label='Mean Path')
            
            # Plot confidence intervals
            if len(paths) > 1:
                lower_bound = np.percentile(paths[:max_paths], 2.5, axis=0)
                upper_bound = np.percentile(paths[:max_paths], 97.5, axis=0)
                plt.fill_between(range(len(mean_path)), lower_bound, upper_bound,
                               alpha=0.2, color='red', label='95% Confidence Interval')
            
            plt.title(f'Simulation Paths (First {min(max_paths, len(paths))} paths)')
            plt.xlabel('Time Step')
            plt.ylabel('Price')
            plt.legend()
            plt.grid(True, alpha=0.3)
            
            if output_file:
                plt.savefig(output_file, dpi=300, bbox_inches='tight')
                print(f"Paths plot saved to {output_file}")
            
            plt.show()
            
        except Exception as e:
            print(f"Error plotting paths: {e}")
    
    def plot_risk_metrics(self, risk_file, output_file=None):
        """Plot risk metrics from risk analysis file."""
        try:
            df = pd.read_csv(risk_file)
            
            if len(df) < 2:
                print("Not enough data for risk metrics plot")
                return
            
            fig, axes = plt.subplots(2, 2, figsize=(12, 8))
            
            # VaR and CVaR comparison
            if 'VaR_95' in df.columns and 'CVaR_95' in df.columns:
                ax = axes[0, 0]
                metrics = ['VaR_95', 'CVaR_95']
                values = [df['VaR_95'].iloc[0], df['CVaR_95'].iloc[0]]
                bars = ax.bar(metrics, values, color=['lightcoral', 'indianred'])
                ax.set_ylabel('Value')
                ax.set_title('Value at Risk (95%)')
                ax.grid(True, alpha=0.3, axis='y')
                
                # Add value labels on bars
                for bar in bars:
                    height = bar.get_height()
                    ax.text(bar.get_x() + bar.get_width()/2., height,
                           f'{height:.3%}', ha='center', va='bottom')
            
            # Volatility over time (if available)
            if 'Volatility' in df.columns and len(df) > 10:
                ax = axes[0, 1]
                ax.plot(df.index, df['Volatility'], marker='o', linewidth=2)
                ax.set_xlabel('Simulation')
                ax.set_ylabel('Volatility')
                ax.set_title('Volatility Trend')
                ax.grid(True, alpha=0.3)
            
            # Sharpe ratio distribution (if multiple values)
            if 'Sharpe_Ratio' in df.columns:
                ax = axes[1, 0]
                ax.hist(df['Sharpe_Ratio'].dropna(), bins=20, alpha=0.7, edgecolor='black')
                ax.axvline(df['Sharpe_Ratio'].mean(), color='red', linestyle='--',
                          label=f'Mean: {df["Sharpe_Ratio"].mean():.2f}')
                ax.set_xlabel('Sharpe Ratio')
                ax.set_ylabel('Frequency')
                ax.set_title('Sharpe Ratio Distribution')
                ax.legend()
                ax.grid(True, alpha=0.3)
            
            # Maximum drawdown
            if 'Max_Drawdown' in df.columns:
                ax = axes[1, 1]
                drawdowns = df['Max_Drawdown'].dropna()
                if len(drawdowns) > 0:
                    ax.bar(range(len(drawdowns)), drawdowns, alpha=0.7)
                    ax.set_xlabel('Simulation')
                    ax.set_ylabel('Drawdown')
                    ax.set_title('Maximum Drawdown')
                    ax.grid(True, alpha=0.3, axis='y')
            
            plt.tight_layout()
            
            if output_file:
                plt.savefig(output_file, dpi=300, bbox_inches='tight')
                print(f"Risk metrics plot saved to {output_file}")
            
            plt.show()
            
        except Exception as e:
            print(f"Error plotting risk metrics: {e}")
    
    def plot_comparison(self, files, labels=None, output_file=None):
        """Compare multiple simulation results."""
        if labels is None:
            labels = [Path(f).stem for f in files]
        
        fig, axes = plt.subplots(2, 2, figsize=(15, 10))
        fig.suptitle('Simulation Comparison', fontsize=16, fontweight='bold')
        
        all_data = []
        
        for i, (file, label) in enumerate(zip(files, labels)):
            df = self.load_simulation_data(file)
            if df is not None:
                all_data.append((label, df.iloc[:, 0]))
        
        if len(all_data) < 2:
            print("Need at least 2 datasets for comparison")
            return
        
        # Box plot comparison
        ax = axes[0, 0]
        data_to_plot = [data for _, data in all_data]
        ax.boxplot(data_to_plot, labels=[label for label, _ in all_data])
        ax.set_ylabel('Price')
        ax.set_title('Distribution Comparison')
        ax.grid(True, alpha=0.3)
        
        # Mean and CI comparison
        ax = axes[0, 1]
        means = [data.mean() for _, data in all_data]
        stds = [data.std() for _, data in all_data]
        ci_lower = [mean - 1.96 * std / np.sqrt(len(data)) 
                   for mean, std, (_, data) in zip(means, stds, all_data)]
        ci_upper = [mean + 1.96 * std / np.sqrt(len(data)) 
                   for mean, std, (_, data) in zip(means, stds, all_data)]
        
        x_pos = np.arange(len(all_data))
        ax.bar(x_pos, means, yerr=[means[i] - ci_lower[i] for i in range(len(means))],
               capsize=5, alpha=0.7)
        ax.set_xticks(x_pos)
        ax.set_xticklabels([label for label, _ in all_data])
        ax.set_ylabel('Mean Price')
        ax.set_title('Mean with 95% Confidence Intervals')
        ax.grid(True, alpha=0.3)
        
        # Cumulative distribution comparison
        ax = axes[1, 0]
        for label, data in all_data:
            sorted_data = np.sort(data)
            cdf = np.arange(1, len(sorted_data) + 1) / len(sorted_data)
            ax.plot(sorted_data, cdf, label=label, linewidth=2)
        ax.set_xlabel('Price')
        ax.set_ylabel('Cumulative Probability')
        ax.set_title('Cumulative Distribution Functions')
        ax.legend()
        ax.grid(True, alpha=0.3)
        
        # Violin plot for distribution shape
        ax = axes[1, 1]
        parts = ax.violinplot(data_to_plot, showmeans=True, showmedians=True)
        ax.set_xticks(np.arange(1, len(all_data) + 1))
        ax.set_xticklabels([label for label, _ in all_data])
        ax.set_ylabel('Price')
        ax.set_title('Distribution Density (Violin Plot)')
        ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Comparison plot saved to {output_file}")
        
        plt.show()
        
        # Print comparison statistics
        print("\n" + "="*60)
        print("Comparison Statistics:")
        print("="*60)
        print(f"{'Dataset':<20} {'Mean':>10} {'Std Dev':>10} {'Min':>10} {'Max':>10}")
        print("-"*60)
        for label, data in all_data:
            print(f"{label:<20} {data.mean():>10.2f} {data.std():>10.2f} "
                  f"{data.min():>10.2f} {data.max():>10.2f}")

def main():
    parser = argparse.ArgumentParser(description='Visualize Monte Carlo simulation results')
    parser.add_argument('--prices-file', type=str, help='CSV file containing final prices')
    parser.add_argument('--paths-file', type=str, help='CSV file containing simulation paths')
    parser.add_argument('--risk-file', type=str, help='CSV file containing risk metrics')
    parser.add_argument('--compare-files', nargs='+', help='Multiple files to compare')
    parser.add_argument('--output-dir', type=str, default='results/visualizations',
                       help='Output directory for plots')
    parser.add_argument('--title', type=str, default='Simulation Results',
                       help='Title for plots')
    
    args = parser.parse_args()
    
    # Create output directory
    output_dir = Path(args.output_dir)
    output_dir.mkdir(exist_ok=True, parents=True)
    
    visualizer = ResultVisualizer()
    
    # Generate timestamp for filenames
    from datetime import datetime
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    # Plot based on provided arguments
    if args.prices_file:
        print(f"Loading price data from {args.prices_file}")
        df = visualizer.load_simulation_data(args.prices_file)
        if df is not None:
            output_file = output_dir / f"distribution_{timestamp}.png"
            visualizer.plot_price_distribution(df, args.title, str(output_file))
    
    if args.paths_file:
        print(f"\nLoading path data from {args.paths_file}")
        output_file = output_dir / f"paths_{timestamp}.png"
        visualizer.plot_simulation_paths(args.paths_file, output_file=str(output_file))
    
    if args.risk_file:
        print(f"\nLoading risk metrics from {args.risk_file}")
        output_file = output_dir / f"risk_metrics_{timestamp}.png"
        visualizer.plot_risk_metrics(args.risk_file, output_file=str(output_file))
    
    if args.compare_files and len(args.compare_files) >= 2:
        print(f"\nComparing {len(args.compare_files)} datasets")
        output_file = output_dir / f"comparison_{timestamp}.png"
        visualizer.plot_comparison(args.compare_files, output_file=str(output_file))
    
    if not any([args.prices_file, args.paths_file, args.risk_file, args.compare_files]):
        print("No input files specified. Use --help for usage information.")
        print("\nExample usage:")
        print("  python visualize.py --prices-file results/simulation_prices.csv")
        print("  python visualize.py --compare-files sim1.csv sim2.csv sim3.csv")
        print("  python visualize.py --paths-file results/paths.csv --risk-file results/risk.csv")

if __name__ == "__main__":
    main()
