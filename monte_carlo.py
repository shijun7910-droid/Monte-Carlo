"""
Monte Carlo simulation for exchange rate forecasting
Author: [Your Name]
"""

import numpy as np
import pandas as pd
from typing import Tuple, Optional


class CurrencyMonteCarlo:
    """
    Monte Carlo simulation for currency exchange rates using Geometric Brownian Motion
    """
    
    def __init__(self, initial_rate: float = 75.0, mu: float = 0.05, sigma: float = 0.15):
        """
        Initialize the Monte Carlo model
        
        Parameters:
        -----------
        initial_rate : float
            Initial exchange rate
        mu : float
            Expected annual return (drift)
        sigma : float
            Annual volatility
        """
        self.initial_rate = initial_rate
        self.mu = mu
        self.sigma = sigma
        self.simulations = None
        self.predicted_rates = None
        
    def simulate(self, days: int = 30, n_simulations: int = 1000, random_seed: Optional[int] = None) -> np.ndarray:
        """
        Run Monte Carlo simulation
        
        Parameters:
        -----------
        days : int
            Number of days to forecast
        n_simulations : int
            Number of simulation paths
        random_seed : int, optional
            Random seed for reproducibility
            
        Returns:
        --------
        np.ndarray
            Array of simulated paths with shape (n_simulations, days)
        """
        if random_seed is not None:
            np.random.seed(random_seed)
            
        # Convert annual parameters to daily
        dt = 1 / 252  # Assuming 252 trading days per year
        n_steps = days
        
        # Generate random paths
        z = np.random.standard_normal((n_simulations, n_steps - 1))
        
        # Initialize array for simulations
        self.simulations = np.zeros((n_simulations, n_steps))
        self.simulations[:, 0] = self.initial_rate
        
        # Generate paths using Geometric Brownian Motion
        for t in range(1, n_steps):
            self.simulations[:, t] = self.simulations[:, t - 1] * np.exp(
                (self.mu - 0.5 * self.sigma**2) * dt 
                + self.sigma * np.sqrt(dt) * z[:, t - 1]
            )
        
        # Store predicted final rates
        self.predicted_rates = self.simulations[:, -1]
        
        return self.simulations
    
    def get_prediction(self, confidence_level: float = 0.95) -> dict:
        """
        Get prediction statistics
        
        Parameters:
        -----------
        confidence_level : float
            Confidence level for confidence interval
            
        Returns:
        --------
        dict
            Dictionary with prediction statistics
        """
        if self.predicted_rates is None:
            raise ValueError("Run simulation first using simulate() method")
            
        mean_prediction = np.mean(self.predicted_rates)
        median_prediction = np.median(self.predicted_rates)
        
        # Calculate confidence interval
        alpha = 1 - confidence_level
        lower_bound = np.percentile(self.predicted_rates, (alpha / 2) * 100)
        upper_bound = np.percentile(self.predicted_rates, (1 - alpha / 2) * 100)
        
        return {
            'mean': mean_prediction,
            'median': median_prediction,
            'std': np.std(self.predicted_rates),
            'min': np.min(self.predicted_rates),
            'max': np.max(self.predicted_rates),
            'confidence_interval': (lower_bound, upper_bound),
            'confidence_level': confidence_level
        }
    
    def calculate_var(self, confidence_level: float = 0.95, initial_investment: float = 10000) -> dict:
        """
        Calculate Value at Risk (VaR)
        
        Parameters:
        -----------
        confidence_level : float
            Confidence level for VaR calculation
        initial_investment : float
            Initial investment amount for calculating monetary VaR
            
        Returns:
        --------
        dict
            Dictionary with VaR metrics
        """
        if self.predicted_rates is None:
            raise ValueError("Run simulation first using simulate() method")
        
        # Calculate returns
        returns = (self.predicted_rates - self.initial_rate) / self.initial_rate
        
        # Calculate VaR
        var_percentile = np.percentile(returns, (1 - confidence_level) * 100)
        var_monetary = initial_investment * abs(var_percentile)
        
        # Calculate Conditional VaR (Expected Shortfall)
        worst_returns = returns[returns <= var_percentile]
        cvar = np.mean(worst_returns) if len(worst_returns) > 0 else var_percentile
        
        return {
            'var_percent': var_percentile * 100,
            'var_amount': var_monetary,
            'cvar_percent': cvar * 100,
            'confidence_level': confidence_level,
            'initial_investment': initial_investment
        }
    
    def calculate_probability(self, target_rate: float) -> float:
        """
        Calculate probability of reaching target rate
        
        Parameters:
        -----------
        target_rate : float
            Target exchange rate
            
        Returns:
        --------
        float
            Probability of reaching or exceeding target rate
        """
        if self.predicted_rates is None:
            raise ValueError("Run simulation first using simulate() method")
            
        return np.mean(self.predicted_rates >= target_rate)


class MultiCurrencyMonteCarlo:
    """
    Monte Carlo simulation for multiple currency pairs
    """
    
    def __init__(self):
        self.models = {}
        
    def add_currency(self, currency_pair: str, initial_rate: float, mu: float, sigma: float):
        """
        Add a currency pair to the simulation
        
        Parameters:
        -----------
        currency_pair : str
            Currency pair symbol (e.g., 'USD/RUB')
        initial_rate : float
            Initial exchange rate
        mu : float
            Expected annual return
        sigma : float
            Annual volatility
        """
        self.models[currency_pair] = CurrencyMonteCarlo(initial_rate, mu, sigma)
        
    def simulate_all(self, days: int = 30, n_simulations: int = 1000, 
                    random_seed: Optional[int] = None) -> dict:
        """
        Simulate all currency pairs
        
        Returns:
        --------
        dict
            Dictionary with simulation results for each currency pair
        """
        results = {}
        for currency_pair, model in self.models.items():
            results[currency_pair] = model.simulate(days, n_simulations, random_seed)
        return results
    
    def get_comparison(self) -> pd.DataFrame:
        """
        Get comparison table of all currency predictions
        
        Returns:
        --------
        pd.DataFrame
            DataFrame with prediction statistics for all currencies
        """
        data = []
        for currency_pair, model in self.models.items():
            if model.predicted_rates is not None:
                stats = model.get_prediction()
                var_stats = model.calculate_var()
                
                data.append({
                    'Currency Pair': currency_pair,
                    'Initial Rate': model.initial_rate,
                    'Predicted Mean': stats['mean'],
                    'Predicted Median': stats['median'],
                    'Std Dev': stats['std'],
                    f"VaR {var_stats['confidence_level']*100}%": var_stats['var_percent'],
                    'Min': stats['min'],
                    'Max': stats['max']
                })
        
        return pd.DataFrame(data)
