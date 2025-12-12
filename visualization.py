"""
Visualization module for Monte Carlo simulation results
Author: [Your Name]
"""

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from typing import Optional
import matplotlib


class MonteCarloVisualizer:
    """
    Visualization tools for Monte Carlo simulation results
    """
    
    def __init__(self, figsize=(12, 8)):
        """
        Initialize the visualizer
        
        Parameters:
        -----------
        figsize : tuple
            Figure size (width, height)
        """
        self.figsize = figsize
        plt.style.use('seaborn-v0_8-darkgrid')
        
    def plot_simulation_paths(self, simulations: np.ndarray, n_paths: int = 100, 
                            title: str = "Monte Carlo Simulation Paths", 
                            save_path: Optional[str] = None):
        """
        Plot Monte Carlo simulation paths
        
        Parameters:
        -----------
        simulations : np.ndarray
            Array of simulated paths
        n_paths : int
            Number of paths to display (for clarity)
        title : str
            Plot title
        save_path : str, optional
            Path to save the figure
        """
        fig, ax = plt.subplots(figsize=self.figsize)
        
        # Plot a subset of paths for clarity
        if n_paths < simulations.shape[0]:
            indices = np.random.choice(simulations.shape[0], n_paths, replace=False)
            paths_to_plot = simulations[indices]
        else:
            paths_to_plot = simulations
        
        # Plot individual paths
        for i in range(paths_to_plot.shape[0]):
            ax.plot(paths_to_plot[i], alpha=0.1, color='blue', linewidth=0.5)
        
        # Calculate and plot mean path
        mean_path = np.mean(simulations, axis=0)
        ax.plot(mean_path, color='red', linewidth=2.5, label='Mean Path')
        
        # Calculate and plot confidence intervals
        lower_bound = np.percentile(simulations, 5, axis=0)
        upper_bound = np.percentile(simulations, 95, axis=0)
        ax.fill_between(range(len(mean_path)), lower_bound, upper_bound, 
                       alpha=0.2, color='red', label='90% Confidence Interval')
        
        ax.set_xlabel('Days')
        ax.set_ylabel('Exchange Rate')
        ax.set_title(title)
        ax.legend()
        ax.grid(True, alpha=0.3)
        
        # Add text box with statistics
        final_rates = simulations[:, -1]
        stats_text = f"""
        Initial Rate: {simulations[0, 0]:.2f}
        Final Rate Mean: {np.mean(final_rates):.2f}
        Final Rate Std: {np.std(final_rates):.2f}
        Min Final Rate: {np.min(final_rates):.2f}
        Max Final Rate: {np.max(final_rates):.2f}
        """
        
        plt.text(0.02, 0.98, stats_text, transform=ax.transAxes, fontsize=10,
                verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
        
        plt.tight_layout()
        
        if save_path:
            plt.savefig(save_path, dpi=300, bbox_inches='tight')
            print(f"Figure saved to {save_path}")
        
        plt.show()
    
    def plot_final_distribution(self, predicted_rates: np.ndarray, 
                              initial_rate: float,
                              title: str = "Predicted Exchange Rate Distribution",
                              save_path: Optional[str] = None):
        """
        Plot distribution of predicted final rates
        
        Parameters:
        -----------
        predicted_rates : np.ndarray
            Array of predicted final rates
        initial_rate : float
            Initial exchange rate for reference
        title : str
            Plot title
        save_path : str, optional
            Path to save the figure
        """
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
        
        # Plot 1: Histogram with KDE
        ax1.hist(predicted_rates, bins=50, density=True, alpha=0.7, color='skyblue', edgecolor='black')
        
        # Add vertical lines
        ax1.axvline(initial_rate, color='red', linestyle='--', linewidth=2, label=f'Initial: {initial_rate:.2f}')
        ax1.axvline(np.mean(predicted_rates), color='green', linestyle='--', linewidth=2, 
                   label=f'Mean: {np.mean(predicted_rates):.2f}')
        
        # Add shaded confidence intervals
        ci_90 = np.percentile(predicted_rates, [5, 95])
        ax1.axvspan(ci_90[0], ci_90[1], alpha=0.2, color='green', label='90% CI')
        
        ax1.set_xlabel('Exchange Rate')
        ax1.set_ylabel('Probability Density')
        ax1.set_title('Distribution of Predicted Rates')
        ax1.legend()
        ax1.grid(True, alpha=0.3)
        
        # Add statistics text
        stats_text = f"""
        Mean: {np.mean(predicted_rates):.2f}
        Median: {np.median(predicted_rates):.2f}
        Std: {np.std(predicted_rates):.2f}
        Skew: {pd.Series(predicted_rates).skew():.3f}
        Kurtosis: {pd.Series(predicted_rates).kurtosis():.3f}
        """
        
        ax1.text(0.02, 0.98, stats_text, transform=ax1.transAxes, fontsize=9,
                verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
        
        # Plot 2: Cumulative distribution
        sorted_rates = np.sort(predicted_rates)
        cdf = np.arange(1, len(sorted_rates) + 1) / len(sorted_rates)
        
        ax2.plot(sorted_rates, cdf, linewidth=2, color='darkblue')
        ax2.fill_between(sorted_rates, cdf, alpha=0.3, color='lightblue')
        
        # Add reference lines
        ax2.axhline(0.5, color='red', linestyle='--', alpha=0.5, label='Median')
        ax2.axhline(0.05, color='orange', linestyle='--', alpha=0.5, label='5% VaR')
        ax2.axhline(0.95, color='orange', linestyle='--', alpha=0.5, label='95% CI')
        
        ax2.set_xlabel('Exchange Rate')
        ax2.set_ylabel('Cumulative Probability')
        ax2.set_title('Cumulative Distribution Function')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        plt.suptitle(title, fontsize=16, fontweight='bold')
        plt.tight_layout()
        
        if save_path:
            plt.savefig(save_path, dpi=300, bbox_inches='tight')
            print(f"Figure saved to {save_path}")
        
        plt.show()
    
    def plot_risk_metrics(self, model, initial_investment: float = 10000,
                         save_path: Optional[str] = None):
        """
        Plot Value at Risk and other risk metrics
        
        Parameters:
        -----------
        model : CurrencyMonteCarlo
            Monte Carlo model with simulation results
        initial_investment : float
            Initial investment amount
        save_path : str, optional
            Path to save the figure
        """
        if model.predicted_rates is None:
            raise ValueError("Run simulation first")
        
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(14, 10))
        
        # Plot 1: VaR at different confidence levels
        confidence_levels = np.arange(0.90, 0.995, 0.01)
        var_values = []
        
        for cl in confidence_levels:
            var_stats = model.calculate_var(cl, initial_investment)
            var_values.append(var_stats['var_percent'])
        
        ax1.plot(confidence_levels * 100, var_values, marker='o', linewidth=2, color='darkred')
        ax1.fill_between(confidence_levels * 100, var_values, alpha=0.3, color='lightcoral')
        
        ax1.set_xlabel('Confidence Level (%)')
        ax1.set_ylabel('Value at Risk (%)')
        ax1.set_title('VaR vs Confidence Level')
        ax1.grid(True, alpha=0.3)
        
        # Add VaR 95% marker
        var_95 = model.calculate_var(0.95, initial_investment)
        ax1.axvline(95, color='blue', linestyle='--', alpha=0.7, 
                   label=f"VaR 95%: {var_95['var_percent']:.2f}%")
        ax1.legend()
        
        # Plot 2: Loss distribution
        returns = (model.predicted_rates - model.initial_rate) / model.initial_rate * 100
        ax2.hist(returns, bins=50, density=True, alpha=0.7, color='lightblue', edgecolor='black')
        
        # Add VaR lines
        ax2.axvline(var_95['var_percent'], color='red', linestyle='--', linewidth=2,
                   label=f"VaR 95%: {var_95['var_percent']:.2f}%")
        
        ax2.axvspan(-100, var_95['var_percent'], alpha=0.2, color='red', label='Worst 5% Losses')
        ax2.axvline(0, color='black', linestyle='-', linewidth=1, alpha=0.5)
        
        ax2.set_xlabel('Return (%)')
        ax2.set_ylabel('Probability Density')
        ax2.set_title('Return Distribution with VaR')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        # Plot 3: Probability of reaching target rates
        target_rates = np.linspace(model.initial_rate * 0.8, model.initial_rate * 1.2, 20)
        probabilities = []
        
        for target in target_rates:
            prob = model.calculate_probability(target)
            probabilities.append(prob * 100)
        
        ax3.plot(target_rates, probabilities, marker='o', linewidth=2, color='darkgreen')
        ax3.fill_between(target_rates, probabilities, alpha=0.3, color='lightgreen')
        
        ax3.axvline(model.initial_rate, color='red', linestyle='--', alpha=0.7,
                   label=f'Current: {model.initial_rate:.2f}')
        ax3.axhline(50, color='black', linestyle='--', alpha=0.5, label='50% Probability')
        
        ax3.set_xlabel('Target Exchange Rate')
        ax3.set_ylabel('Probability (%)')
        ax3.set_title('Probability of Reaching Target Rate')
        ax3.legend()
        ax3.grid(True, alpha=0.3)
        
        # Plot 4: Risk-return scatter
        mean_returns = (np.mean(model.predicted_rates) - model.initial_rate) / model.initial_rate * 100
        volatility = np.std(model.predicted_rates) / model.initial_rate * 100
        
        ax4.scatter(volatility, mean_returns, s=200, color='darkblue', alpha=0.7, 
                   edgecolors='black', linewidth=2)
        
        # Add reference lines
        ax4.axhline(0, color='black', linestyle='-', alpha=0.3)
        ax4.axvline(0, color='black', linestyle='-', alpha=0.3)
        
        ax4.set_xlabel('Volatility (%)')
        ax4.set_ylabel('Expected Return (%)')
        ax4.set_title('Risk-Return Profile')
        ax4.grid(True, alpha=0.3)
        
        # Add text annotation
        scatter_text = f"""
        Expected Return: {mean_returns:.2f}%
        Volatility: {volatility:.2f}%
        Sharpe Ratio: {mean_returns/volatility:.3f}
        """
        
        ax4.text(0.02, 0.98, scatter_text, transform=ax4.transAxes, fontsize=9,
                verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
        
        plt.suptitle('Risk Analysis Dashboard', fontsize=16, fontweight='bold')
        plt.tight_layout()
        
        if save_path:
            plt.savefig(save_path, dpi=300, bbox_inches='tight')
            print(f"Figure saved to {save_path}")
        
        plt.show()
    
    def plot_multi_currency_comparison(self, multi_model, 
                                      save_path: Optional[str] = None):
        """
        Plot comparison of multiple currency pairs
        
        Parameters:
        -----------
        multi_model : MultiCurrencyMonteCarlo
            Multi-currency Monte Carlo model
        save_path : str, optional
            Path to save the figure
        """
        comparison_df = multi_model.get_comparison()
        
        if comparison_df.empty:
            print("No simulation results available")
            return
        
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 12))
        
        # Plot 1: Initial vs Predicted rates
        x = range(len(comparison_df))
        width = 0.35
        
        ax1.bar([i - width/2 for i in x], comparison_df['Initial Rate'], width, 
               label='Initial Rate', color='skyblue', alpha=0.7)
        ax1.bar([i + width/2 for i in x], comparison_df['Predicted Mean'], width,
               label='Predicted Mean', color='lightcoral', alpha=0.7)
        
        ax1.set_xlabel('Currency Pair')
        ax1.set_ylabel('Exchange Rate')
        ax1.set_title('Initial vs Predicted Exchange Rates')
        ax1.set_xticks(x)
        ax1.set_xticklabels(comparison_df['Currency Pair'], rotation=45)
        ax1.legend()
        ax1.grid(True, alpha=0.3, axis='y')
        
        # Add value labels on bars
        for i, (init, pred) in enumerate(zip(comparison_df['Initial Rate'], 
                                           comparison_df['Predicted Mean'])):
            ax1.text(i - width/2, init + init*0.01, f'{init:.2f}', 
                    ha='center', va='bottom', fontsize=8)
            ax1.text(i + width/2, pred + pred*0.01, f'{pred:.2f}', 
                    ha='center', va='bottom', fontsize=8)
        
        # Plot 2: Risk comparison (VaR)
        ax2.bar(x, comparison_df['VaR 95.0%'], color='lightgreen', alpha=0.7)
        
        ax2.set_xlabel('Currency Pair')
        ax2.set_ylabel('VaR 95% (%)')
        ax2.set_title('Value at Risk Comparison')
        ax2.set_xticks(x)
        ax2.set_xticklabels(comparison_df['Currency Pair'], rotation=45)
        ax2.grid(True, alpha=0.3, axis='y')
        
        # Add value labels on bars
        for i, var in enumerate(comparison_df['VaR 95.0%']):
            ax2.text(i, var + 0.1, f'{var:.1f}%', ha='center', va='bottom', fontsize=8)
        
        # Plot 3: Volatility comparison
        volatility_colors = plt.cm.RdYlGn_r(comparison_df['Std Dev'] / comparison_df['Std Dev'].max())
        ax3.bar(x, comparison_df['Std Dev'], color=volatility_colors, alpha=0.7)
        
        ax3.set_xlabel('Currency Pair')
        ax3.set_ylabel('Standard Deviation')
        ax3.set_title('Volatility Comparison')
        ax3.set_xticks(x)
        ax3.set_xticklabels(comparison_df['Currency Pair'], rotation=45)
        ax3.grid(True, alpha=0.3, axis='y')
        
        # Add value labels on bars
        for i, std in enumerate(comparison_df['Std Dev']):
            ax3.text(i, std + std*0.01, f'{std:.3f}', ha='center', va='bottom', fontsize=8)
        
        # Plot 4: Risk-return scatter for all currencies
        for idx, row in comparison_df.iterrows():
            ax4.scatter(row['Std Dev'], 
                       (row['Predicted Mean'] - row['Initial Rate']) / row['Initial Rate'] * 100,
                       s=row['VaR 95.0%'] * 50,  # Bubble size based on VaR
                       label=row['Currency Pair'],
                       alpha=0.6,
                       edgecolors='black')
            
            # Add currency pair label
            ax4.annotate(row['Currency Pair'], 
                        (row['Std Dev'], 
                         (row['Predicted Mean'] - row['Initial Rate']) / row['Initial Rate'] * 100),
                        textcoords="offset points",
                        xytext=(0,10),
                        ha='center',
                        fontsize=9)
        
        ax4.axhline(0, color='black', linestyle='--', alpha=0.3)
        ax4.axvline(0, color='black', linestyle='--', alpha=0.3)
        
        ax4.set_xlabel('Volatility (Standard Deviation)')
        ax4.set_ylabel('Expected Return (%)')
        ax4.set_title('Risk-Return Profile: All Currencies')
        ax4.grid(True, alpha=0.3)
        
        plt.suptitle('Multi-Currency Analysis Dashboard', fontsize=16, fontweight='bold')
        plt.tight_layout()
        
        if save_path:
            plt.savefig(save_path, dpi=300, bbox_inches='tight')
            print(f"Figure saved to {save_path}")
        
        plt.show()


def main():
    """Example usage of the visualizer"""
    # Create sample data
    np.random.seed(42)
    n_simulations = 1000
    n_days = 30
    
    # Generate sample simulations
    simulations = np.zeros((n_simulations, n_days))
    initial_rate = 75.0
    
    for i in range(n_simulations):
        # Simple random walk
        simulations[i, 0] = initial_rate
        for j in range(1, n_days):
            simulations[i, j] = simulations[i, j-1] * (1 + np.random.normal(0.0005, 0.01))
    
    # Create visualizer instance
    visualizer = MonteCarloVisualizer()
    
    # Plot simulation paths
    visualizer.plot_simulation_paths(simulations, n_paths=50, 
                                   title="Sample Monte Carlo Simulation")
    
    # Plot final distribution
    predicted_rates = simulations[:, -1]
    visualizer.plot_final_distribution(predicted_rates, initial_rate,
                                     title="Sample Distribution Analysis")


if __name__ == "__main__":
    main()
