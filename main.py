"""
Main application for Currency Monte Carlo Simulation
Author: [Your Name]
"""

import numpy as np
import pandas as pd
import argparse
import os
from datetime import datetime, timedelta

# Import project modules
from monte_carlo import CurrencyMonteCarlo, MultiCurrencyMonteCarlo
from data_loader import ExchangeRateDataLoader
from visualization import MonteCarloVisualizer


def main_single_currency():
    """Run single currency simulation"""
    print("=" * 60)
    print("CURRENCY MONTE CARLO SIMULATION")
    print("=" * 60)
    
    # Initialize model
    print("\nInitializing Monte Carlo model...")
    model = CurrencyMonteCarlo(
        initial_rate=75.0,  # Example: USD/RUB rate
        mu=0.05,           # 5% annual expected return
        sigma=0.15         # 15% annual volatility
    )
    
    print(f"Initial Rate: {model.initial_rate}")
    print(f"Expected Annual Return (μ): {model.mu*100:.1f}%")
    print(f"Annual Volatility (σ): {model.sigma*100:.1f}%")
    
    # Run simulation
    print("\nRunning Monte Carlo simulation...")
    simulations = model.simulate(
        days=30,           # Forecast 30 days
        n_simulations=5000, # 5000 simulation paths
        random_seed=42     # For reproducibility
    )
    
    print(f"Simulations completed: {simulations.shape[0]} paths")
    print(f"Time horizon: {simulations.shape[1]} days")
    
    # Get predictions
    print("\n" + "-" * 40)
    print("PREDICTION RESULTS")
    print("-" * 40)
    
    predictions = model.get_prediction(confidence_level=0.95)
    for key, value in predictions.items():
        if key == 'confidence_interval':
            ci_low, ci_high = value
            print(f"{key:25}: [{ci_low:.2f}, {ci_high:.2f}]")
        elif isinstance(value, float):
            print(f"{key:25}: {value:.4f}")
        else:
            print(f"{key:25}: {value}")
    
    # Calculate risk metrics
    print("\n" + "-" * 40)
    print("RISK ANALYSIS")
    print("-" * 40)
    
    var_results = model.calculate_var(
        confidence_level=0.95,
        initial_investment=10000
    )
    
    for key, value in var_results.items():
        if isinstance(value, float):
            if 'percent' in key:
                print(f"{key:25}: {value:.2f}%")
            elif 'amount' in key:
                print(f"{key:25}: ${value:.2f}")
            else:
                print(f"{key:25}: {value:.4f}")
        else:
            print(f"{key:25}: {value}")
    
    # Calculate probability of reaching target
    target_rate = 80.0
    probability = model.calculate_probability(target_rate)
    print(f"\nProbability of reaching {target_rate:.2f}: {probability*100:.1f}%")
    
    # Create visualizer
    visualizer = MonteCarloVisualizer()
    
    # Generate visualizations
    print("\nGenerating visualizations...")
    
    # Plot 1: Simulation paths
    visualizer.plot_simulation_paths(
        simulations,
        n_paths=100,
        title="USD/RUB Monte Carlo Simulation (30-day forecast)",
        save_path="simulation_paths.png"
    )
    
    # Plot 2: Final distribution
    visualizer.plot_final_distribution(
        model.predicted_rates,
        model.initial_rate,
        title="USD/RUB: Predicted Exchange Rate Distribution",
        save_path="final_distribution.png"
    )
    
    # Plot 3: Risk metrics
    visualizer.plot_risk_metrics(
        model,
        initial_investment=10000,
        save_path="risk_analysis.png"
    )
    
    print("\nVisualizations saved as PNG files")
    print("=" * 60)


def main_multi_currency():
    """Run multi-currency simulation"""
    print("=" * 60)
    print("MULTI-CURRENCY MONTE CARLO SIMULATION")
    print("=" * 60)
    
    # Initialize multi-currency model
    multi_model = MultiCurrencyMonteCarlo()
    
    # Add currency pairs with realistic parameters
    print("\nAdding currency pairs...")
    
    # USD/RUB (US Dollar / Russian Ruble)
    multi_model.add_currency(
        currency_pair="USD/RUB",
        initial_rate=75.0,
        mu=0.05,     # 5% expected annual return
        sigma=0.15   # 15% annual volatility
    )
    
    # EUR/USD (Euro / US Dollar)
    multi_model.add_currency(
        currency_pair="EUR/USD",
        initial_rate=1.08,
        mu=0.02,     # 2% expected annual return
        sigma=0.10   # 10% annual volatility
    )
    
    # USD/CNY (US Dollar / Chinese Yuan)
    multi_model.add_currency(
        currency_pair="USD/CNY",
        initial_rate=7.15,
        mu=0.03,     # 3% expected annual return
        sigma=0.12   # 12% annual volatility
    )
    
    # GBP/USD (British Pound / US Dollar)
    multi_model.add_currency(
        currency_pair="GBP/USD",
        initial_rate=1.26,
        mu=0.04,     # 4% expected annual return
        sigma=0.14   # 14% annual volatility
    )
    
    print(f"Added {len(multi_model.models)} currency pairs")
    
    # Run simulations for all currencies
    print("\nRunning simulations for all currency pairs...")
    results = multi_model.simulate_all(
        days=30,
        n_simulations=2000,
        random_seed=42
    )
    
    print(f"Simulations completed for {len(results)} currency pairs")
    
    # Get comparison table
    print("\n" + "-" * 60)
    print("MULTI-CURRENCY COMPARISON")
    print("-" * 60)
    
    comparison_df = multi_model.get_comparison()
    
    # Format DataFrame for better display
    display_df = comparison_df.copy()
    display_df = display_df.round(4)
    
    # Set column display options
    pd.set_option('display.max_columns', None)
    pd.set_option('display.width', None)
    pd.set_option('display.max_colwidth', None)
    
    print("\nPrediction Summary:")
    print(display_df.to_string(index=False))
    
    # Save comparison to CSV
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_filename = f"currency_comparison_{timestamp}.csv"
    comparison_df.to_csv(csv_filename, index=False)
    print(f"\nComparison table saved to: {csv_filename}")
    
    # Create visualizer
    visualizer = MonteCarloVisualizer()
    
    # Generate multi-currency visualization
    print("\nGenerating multi-currency visualization...")
    visualizer.plot_multi_currency_comparison(
        multi_model,
        save_path="multi_currency_comparison.png"
    )
    
    print("\nMulti-currency analysis completed!")
    print("=" * 60)


def main_with_real_data():
    """Run simulation with real data from Yahoo Finance"""
    print("=" * 60)
    print("MONTE CARLO SIMULATION WITH REAL DATA")
    print("=" * 60)
    
    # Initialize data loader
    data_loader = ExchangeRateDataLoader()
    
    # Load data for USD/RUB
    print("\nLoading real exchange rate data from Yahoo Finance...")
    currency_pair = "USDRUB=X"  # USD/RUB on Yahoo Finance
    
    # Calculate date range (last 180 days)
    end_date = datetime.now()
    start_date = end_date - timedelta(days=180)
    
    data = data_loader.load_from_yahoo(
        currency_pair=currency_pair,
        start_date=start_date.strftime("%Y-%m-%d"),
        end_date=end_date.strftime("%Y-%m-%d"),
        interval="1d"
    )
    
    if data.empty:
        print("Failed to load data. Using sample data instead.")
        data = data_loader.create_sample_data()
    else:
        print(f"Successfully loaded {len(data)} days of data")
    
    # Calculate statistics from historical data
    print("\n" + "-" * 40)
    print("HISTORICAL DATA ANALYSIS")
    print("-" * 40)
    
    stats = data_loader.calculate_statistics(data)
    
    for key, value in stats.items():
        if isinstance(value, float):
            print(f"{key:20}: {value:.6f}")
        elif isinstance(value, datetime):
            print(f"{key:20}: {value.date()}")
        else:
            print(f"{key:20}: {value}")
    
    # Initialize model with parameters from historical data
    print("\n" + "-" * 40)
    print("INITIALIZING MODEL WITH HISTORICAL PARAMETERS")
    print("-" * 40)
    
    initial_rate = stats['current_rate']
    mu = stats['drift_annual']
    sigma = stats['volatility_annual']
    
    print(f"Initial Rate: {initial_rate:.4f}")
    print(f"Historical Annual Drift: {mu*100:.2f}%")
    print(f"Historical Annual Volatility: {sigma*100:.2f}%")
    
    model = CurrencyMonteCarlo(
        initial_rate=initial_rate,
        mu=mu,
        sigma=sigma
    )
    
    # Run simulation
    print("\nRunning Monte Carlo simulation with historical parameters...")
    simulations = model.simulate(
        days=30,
        n_simulations=5000,
        random_seed=42
    )
    
    # Display results
    predictions = model.get_prediction()
    
    print("\n" + "-" * 40)
    print("30-DAY FORECAST")
    print("-" * 40)
    print(f"Current Rate: {initial_rate:.4f}")
    print(f"Predicted Mean (30 days): {predictions['mean']:.4f}")
    print(f"Expected Change: {(predictions['mean'] - initial_rate)/initial_rate*100:.2f}%")
    
    ci_low, ci_high = predictions['confidence_interval']
    print(f"95% Confidence Interval: [{ci_low:.4f}, {ci_high:.4f}]")
    
    # Risk analysis
    var_results = model.calculate_var(confidence_level=0.95, initial_investment=10000)
    print(f"\nRisk Metrics (95% confidence):")
    print(f"Value at Risk: {var_results['var_percent']:.2f}%")
    print(f"Expected Shortfall: {var_results['cvar_percent']:.2f}%")
    
    # Create visualizer
    visualizer = MonteCarloVisualizer()
    
    # Generate visualizations
    print("\nGenerating visualizations...")
    
    # Plot simulation paths
    visualizer.plot_simulation_paths(
        simulations,
        n_paths=100,
        title=f"{currency_pair} - Monte Carlo Simulation (Based on Historical Data)",
        save_path="real_data_simulation.png"
    )
    
    # Plot risk metrics
    visualizer.plot_risk_metrics(
        model,
        initial_investment=10000,
        save_path="real_data_risk_analysis.png"
    )
    
    print("\nReal data analysis completed!")
    print("=" * 60)


def main_interactive():
    """Interactive mode for user input"""
    print("=" * 60)
    print("INTERACTIVE MONTE CARLO SIMULATION")
    print("=" * 60)
    
    print("\nConfigure your simulation:")
    
    # Get user inputs
    try:
        initial_rate = float(input("Enter initial exchange rate (e.g., 75.0): "))
        forecast_days = int(input("Enter forecast horizon (days, e.g., 30): "))
        n_simulations = int(input("Enter number of simulations (e.g., 5000): "))
        
        print("\nSelect volatility level:")
        print("1. Low (σ = 10%) - Stable currency")
        print("2. Medium (σ = 15%) - Normal currency")
        print("3. High (σ = 25%) - Volatile currency")
        print("4. Very High (σ = 40%) - Emerging market currency")
        
        vol_choice = input("Enter choice (1-4): ")
        
        volatility_map = {'1': 0.10, '2': 0.15, '3': 0.25, '4': 0.40}
        sigma = volatility_map.get(vol_choice, 0.15)
        
        print(f"\nSelected volatility: {sigma*100:.0f}%")
        
        # Initialize model
        model = CurrencyMonteCarlo(
            initial_rate=initial_rate,
            mu=0.05,  # Default expected return
            sigma=sigma
        )
        
        # Run simulation
        print(f"\nRunning {n_simulations} simulations for {forecast_days} days...")
        simulations = model.simulate(
            days=forecast_days,
            n_simulations=n_simulations,
            random_seed=42
        )
        
        # Display results
        predictions = model.get_prediction()
        
        print("\n" + "=" * 40)
        print("SIMULATION RESULTS")
        print("=" * 40)
        print(f"Initial Rate: {initial_rate:.2f}")
        print(f"Predicted Rate (mean): {predictions['mean']:.2f}")
        print(f"Standard Deviation: {predictions['std']:.2f}")
        
        ci_low, ci_high = predictions['confidence_interval']
        print(f"95% Confidence Interval: [{ci_low:.2f}, {ci_high:.2f}]")
        
        # Ask for target rate
        target = input(f"\nEnter target rate to check probability (press Enter for {initial_rate*1.05:.2f}): ")
        if not target:
            target_rate = initial_rate * 1.05
        else:
            target_rate = float(target)
        
        probability = model.calculate_probability(target_rate)
        print(f"\nProbability of reaching {target_rate:.2f}: {probability*100:.1f}%")
        
        if probability > 0.7:
            print("→ High probability of reaching target")
        elif probability > 0.3:
            print("→ Moderate probability")
        else:
            print("→ Low probability")
        
        # Ask about visualization
        visualize = input("\nGenerate visualizations? (y/n): ").lower()
        
        if visualize == 'y':
            visualizer = MonteCarloVisualizer()
            
            visualizer.plot_simulation_paths(
                simulations,
                n_paths=100,
                title=f"Exchange Rate Forecast - {forecast_days} days"
            )
            
            visualizer.plot_final_distribution(
                model.predicted_rates,
                initial_rate,
                title="Predicted Rate Distribution"
            )
        
        print("\nSimulation completed!")
        
    except ValueError as e:
        print(f"\nError: Invalid input - {e}")
    except Exception as e:
        print(f"\nError: {e}")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="Currency Monte Carlo Simulation")
    parser.add_argument('--mode', type=str, default='single',
                       choices=['single', 'multi', 'real', 'interactive'],
                       help='Simulation mode')
    parser.add_argument('--save-plots', action='store_true',
                       help='Save plots as PNG files')
    
    args = parser.parse_args()
    
    print("\n" + "=" * 60)
    print("CURRENCY MONTE CARLO SIMULATION PROJECT")
    print("=" * 60)
    print(f"Mode: {args.mode}")
    print(f"Save plots: {args.save_plots}")
    print("=" * 60)
    
    try:
        if args.mode == 'single':
            main_single_currency()
        elif args.mode == 'multi':
            main_multi_currency()
        elif args.mode == 'real':
            main_with_real_data()
        elif args.mode == 'interactive':
            main_interactive()
        else:
            print(f"Unknown mode: {args.mode}")
    
    except KeyboardInterrupt:
        print("\n\nSimulation interrupted by user.")
    except Exception as e:
        print(f"\nError: {e}")
        import traceback
        traceback.print_exc()
    
    print("\nThank you for using Currency Monte Carlo Simulation!")
    print("=" * 60)


if __name__ == "__main__":
    main()
