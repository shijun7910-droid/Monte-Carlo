#!/usr/bin/env python3
"""
Advanced distribution plotting for simulation results.
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from scipy import stats
import argparse
from pathlib import Path

class DistributionPlotter:
    def __init__(self):
        plt.style.use('seaborn-v0_8-darkgrid')
        self.colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd']
    
    def load_data(self, filepath):
        """Load data from CSV file."""
        return pd.read_csv(filepath)
    
    def plot_advanced_distribution(self, data_series, title="Distribution Analysis", 
                                  output_file=None):
        """Create advanced distribution analysis plots."""
        if data_series is None or len(data_series) == 0:
            print("No data to plot")
            return
        
        fig = plt.figure(figsize=(16, 12))
        fig.suptitle(title, fontsize=16, fontweight='bold')
        
        # 1. Histogram with KDE and fitted distributions
        ax1 = plt.subplot(3, 3, 1)
        self._plot_histogram_with_fits(ax1, data_series)
        
        # 2. Box plot with outliers
        ax2 = plt.subplot(3, 3, 2)
        self._plot_boxplot(ax2, data_series)
        
        # 3. Violin plot
        ax3 = plt.subplot(3, 3, 3)
        self._plot_violin(ax3, data_series)
        
        # 4. ECDF plot
        ax4 = plt.subplot(3, 3, 4)
        self._plot_ecdf(ax4, data_series)
        
        # 5. Q-Q plot
        ax5 = plt.subplot(3, 3, 5)
        self._plot_qq(ax5, data_series)
        
        # 6. P-P plot
        ax6 = plt.subplot(3, 3, 6)
        self._plot_pp(ax6, data_series)
        
        # 7. Cumulative histogram
        ax7 = plt.subplot(3, 3, 7)
        self._plot_cumulative_histogram(ax7, data_series)
        
        # 8. Kernel density comparison
        ax8 = plt.subplot(3, 3, 8)
        self._plot_kernel_density_comparison(ax8, data_series)
        
        # 9. Statistical summary
        ax9 = plt.subplot(3, 3, 9)
        self._plot_statistical_summary(ax9, data_series)
        
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Plot saved to {output_file}")
        
        plt.show()
    
    def _plot_histogram_with_fits(self, ax, data):
        """Plot histogram with fitted distributions."""
        # Histogram
        n, bins, patches = ax.hist(data, bins=50, density=True, alpha=0.6, 
                                  color=self.colors[0], edgecolor='black')
        
        # Kernel Density Estimate
        from scipy.stats import gaussian_kde
        kde = gaussian_kde(data)
        x_range = np.linspace(data.min(), data.max(), 1000)
        ax.plot(x_range, kde(x_range), 'r-', linewidth=2, label='KDE')
        
        # Fit normal distribution
        mu, std = stats.norm.fit(data)
        normal_pdf = stats.norm.pdf(x_range, mu, std)
        ax.plot(x_range, normal_pdf, 'g--', linewidth=2, label='Normal fit')
        
        # Fit lognormal if data positive
        if data.min() > 0:
            shape, loc, scale = stats.lognorm.fit(data)
            lognorm_pdf = stats.lognorm.pdf(x_range, shape, loc, scale)
            ax.plot(x_range, lognorm_pdf, 'b:', linewidth=2, label='Lognormal fit')
        
        ax.set_xlabel('Value')
        ax.set_ylabel('Density')
        ax.set_title('Histogram with Distribution Fits')
        ax.legend()
        ax.grid(True, alpha=0.3)
    
    def _plot_boxplot(self, ax, data):
        """Plot box plot with outliers highlighted."""
        bp = ax.boxplot(data, vert=True, patch_artist=True,
                       boxprops=dict(facecolor=self.colors[1], alpha=0.7),
                       medianprops=dict(color='red', linewidth=2))
        
        # Calculate and mark outliers
        Q1 = np.percentile(data, 25)
        Q3 = np.percentile(data, 75)
        IQR = Q3 - Q1
        lower_bound = Q1 - 1.5 * IQR
        upper_bound = Q3 + 1.5 * IQR
        
        outliers = data[(data < lower_bound) | (data > upper_bound)]
        if len(outliers) > 0:
            ax.plot([1]*len(outliers), outliers, 'ro', alpha=0.6, label='Outliers')
        
        ax.set_ylabel('Value')
        ax.set_title('Box Plot with Outliers')
        ax.grid(True, alpha=0.3)
        if len(outliers) > 0:
            ax.legend()
    
    def _plot_violin(self, ax, data):
        """Plot violin plot."""
        parts = ax.violinplot(data, showmeans=True, showmedians=True)
        
        # Customize colors
        for pc in parts['bodies']:
            pc.set_facecolor(self.colors[2])
            pc.set_alpha(0.7)
        
        parts['cmeans'].set_color('red')
        parts['cmedians'].set_color('green')
        
        ax.set_ylabel('Value')
        ax.set_title('Violin Plot')
        ax.grid(True, alpha=0.3)
    
    def _plot_ecdf(self, ax, data):
        """Plot Empirical Cumulative Distribution Function."""
        sorted_data = np.sort(data)
        ecdf = np.arange(1, len(sorted_data) + 1) / len(sorted_data)
        
        ax.plot(sorted_data, ecdf, 'b-', linewidth=2)
        ax.set_xlabel('Value')
        ax.set_ylabel('Cumulative Probability')
        ax.set_title('Empirical CDF')
        ax.grid(True, alpha=0.3)
    
    def _plot_qq(self, ax, data):
        """Plot Q-Q plot."""
        stats.probplot(data, dist="norm", plot=ax)
        ax.set_title('Q-Q Plot (Normal)')
        ax.grid(True, alpha=0.3)
    
    def _plot_pp(self, ax, data):
        """Plot P-P plot."""
        data_sorted = np.sort(data)
        n = len(data_sorted)
        theoretical = np.arange(1, n + 1) / n
        
        # Theoretical quantiles from normal distribution
        mu, std = stats.norm.fit(data)
        norm_quantiles = stats.norm.ppf(theoretical, mu, std)
        
        ax.plot(norm_quantiles, data_sorted, 'bo', alpha=0.5)
        ax.plot([data.min(), data.max()], [data.min(), data.max()], 'r--', linewidth=2)
        ax.set_xlabel('Theoretical Quantiles')
        ax.set_ylabel('Sample Quantiles')
        ax.set_title('P-P Plot')
        ax.grid(True, alpha=0.3)
    
    def _plot_cumulative_histogram(self, ax, data):
        """Plot cumulative histogram."""
        n, bins, patches = ax.hist(data, bins=50, cumulative=True, 
                                  density=True, alpha=0.7, 
                                  color=self.colors[3], edgecolor='black')
        ax.set_xlabel('Value')
        ax.set_ylabel('Cumulative Probability')
        ax.set_title('Cumulative Histogram')
        ax.grid(True, alpha=0.3)
    
    def _plot_kernel_density_comparison(self, ax, data):
        """Plot kernel density with different bandwidths."""
        x_range = np.linspace(data.min(), data.max(), 1000)
        
        bandwidths = [0.1, 0.5, 1.0, 'scott', 'silverman']
        for i, bw in enumerate(bandwidths):
            if isinstance(bw, str):
                kde = stats.gaussian_kde(data, bw_method=bw)
            else:
                kde = stats.gaussian_kde(data, bw_method=bw)
            
            label = f'BW={bw}' if isinstance(bw, (int, float)) else f'BW={bw}'
            ax.plot(x_range, kde(x_range), label=label, alpha=0.7)
        
        ax.set_xlabel('Value')
        ax.set_ylabel('Density')
        ax.set_title('KDE with Different Bandwidths')
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)
    
    def _plot_statistical_summary(self, ax, data):
        """Plot statistical summary as text."""
        stats_text = f"""
Statistical Summary:
{'='*30}
Count: {len(data):,}
Mean: {data.mean():.4f}
Std Dev: {data.std():.4f}
Skewness: {data.skew():.4f}
Kurtosis: {data.kurtosis():.4f}
Min: {data.min():.4f}
25%: {data.quantile(0.25):.4f}
50%: {data.quantile(0.50):.4f}
75%: {data.quantile(0.75):.4f}
Max: {data.max():.4f}
IQR: {data.quantile(0.75) - data.quantile(0.25):.4f}
CV: {data.std()/data.mean()*100 if data.mean() != 0 else 0:.2f}%
"""
        
        ax.text(0.1, 0.5, stats_text, transform=ax.transAxes,
                fontsize=9, verticalalignment='center',
                bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
        
        ax.set_axis_off()
        ax.set_title('Statistical Summary')

def main():
    parser = argparse.ArgumentParser(description='Plot advanced distribution analysis')
    parser.add_argument('input_file', type=str, help='Input CSV file')
    parser.add_argument('--column', type=str, default=None, 
                       help='Column to analyze (uses first column if not specified)')
    parser.add_argument('--output', type=str, default=None,
                       help='Output file for plot')
    parser.add_argument('--title', type=str, default='Distribution Analysis',
                       help='Plot title')
    
    args = parser.parse_args()
    
    plotter = DistributionPlotter()
    
    # Load data
    df = plotter.load_data(args.input_file)
    
    # Select column
    if args.column:
        if args.column in df.columns:
            data = df[args.column]
        else:
            print(f"Column '{args.column}' not found. Available columns: {list(df.columns)}")
            return
    else:
        data = df.iloc[:, 0]  # Use first column
    
    # Create plot
    plotter.plot_advanced_distribution(data, args.title, args.output)

if __name__ == "__main__":
    main()
