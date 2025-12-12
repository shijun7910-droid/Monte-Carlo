"""
Data loading module for exchange rate data
Author: [Your Name]
"""

import pandas as pd
import numpy as np
import yfinance as yf
from typing import Optional, Tuple
import os


class ExchangeRateDataLoader:
    """
    Load and preprocess exchange rate data
    """
    
    @staticmethod
    def load_from_yahoo(currency_pair: str, start_date: str = '2023-01-01', 
                       end_date: Optional[str] = None, interval: str = '1d') -> pd.DataFrame:
        """
        Load exchange rate data from Yahoo Finance
        
        Parameters:
        -----------
        currency_pair : str
            Currency pair symbol (e.g., 'USDRUB=X' for USD/RUB)
        start_date : str
            Start date in 'YYYY-MM-DD' format
        end_date : str, optional
            End date in 'YYYY-MM-DD' format (defaults to today)
        interval : str
            Data interval ('1d', '1h', '1wk', etc.)
            
        Returns:
        --------
        pd.DataFrame
            DataFrame with exchange rate data
        """
        try:
            ticker = yf.Ticker(currency_pair)
            data = ticker.history(start=start_date, end=end_date, interval=interval)
            
            if data.empty:
                print(f"No data found for {currency_pair}")
                return pd.DataFrame()
            
            # Clean up column names
            data = data[['Close']].copy()
            data.columns = ['Exchange Rate']
            data.index.name = 'Date'
            
            print(f"Loaded {len(data)} days of data for {currency_pair}")
            print(f"Date range: {data.index[0].date()} to {data.index[-1].date()}")
            
            return data
            
        except Exception as e:
            print(f"Error loading data from Yahoo Finance: {e}")
            return pd.DataFrame()
    
    @staticmethod
    def load_from_csv(filepath: str) -> pd.DataFrame:
        """
        Load exchange rate data from CSV file
        
        Parameters:
        -----------
        filepath : str
            Path to CSV file
            
        Returns:
        --------
        pd.DataFrame
            DataFrame with exchange rate data
        """
        try:
            data = pd.read_csv(filepath, parse_dates=['Date'], index_col='Date')
            print(f"Loaded {len(data)} records from {filepath}")
            return data
        except Exception as e:
            print(f"Error loading CSV file: {e}")
            return pd.DataFrame()
    
    @staticmethod
    def calculate_statistics(data: pd.DataFrame, column: str = 'Exchange Rate') -> dict:
        """
        Calculate basic statistics from exchange rate data
        
        Parameters:
        -----------
        data : pd.DataFrame
            Exchange rate data
        column : str
            Column name containing exchange rates
            
        Returns:
        --------
        dict
            Dictionary with statistical measures
        """
        if data.empty:
            return {}
        
        rates = data[column].dropna()
        returns = rates.pct_change().dropna()
        
        # Annualize statistics (assuming 252 trading days)
        trading_days = 252
        
        # Calculate volatility (annualized)
        daily_volatility = returns.std()
        annual_volatility = daily_volatility * np.sqrt(trading_days)
        
        # Calculate drift (annualized)
        daily_drift = returns.mean()
        annual_drift = daily_drift * trading_days
        
        statistics = {
            'current_rate': rates.iloc[-1],
            'mean_rate': rates.mean(),
            'min_rate': rates.min(),
            'max_rate': rates.max(),
            'volatility_daily': daily_volatility,
            'volatility_annual': annual_volatility,
            'drift_daily': daily_drift,
            'drift_annual': annual_drift,
            'total_days': len(rates),
            'start_date': rates.index[0].date(),
            'end_date': rates.index[-1].date()
        }
        
        return statistics
    
    @staticmethod
    def prepare_training_data(data: pd.DataFrame, forecast_days: int = 30, 
                            column: str = 'Exchange Rate') -> Tuple[np.ndarray, float]:
        """
        Prepare data for Monte Carlo simulation
        
        Parameters:
        -----------
        data : pd.DataFrame
            Historical exchange rate data
        forecast_days : int
            Number of days to forecast
        column : str
            Column name containing exchange rates
            
        Returns:
        --------
        Tuple[np.ndarray, float]
            Tuple containing historical returns array and latest exchange rate
        """
        if data.empty:
            return np.array([]), 0.0
        
        rates = data[column].dropna()
        returns = rates.pct_change().dropna().values
        
        latest_rate = rates.iloc[-1]
        
        return returns, latest_rate
    
    @staticmethod
    def create_sample_data() -> pd.DataFrame:
        """
        Create sample exchange rate data for testing
        
        Returns:
        --------
        pd.DataFrame
            Sample exchange rate data
        """
        dates = pd.date_range(start='2023-01-01', end='2023-12-31', freq='D')
        np.random.seed(42)
        
        # Generate sample USD/RUB rates
        initial_rate = 75.0
        drift = 0.0002  # Daily drift
        volatility = 0.01  # Daily volatility
        
        rates = [initial_rate]
        for i in range(1, len(dates)):
            daily_return = drift + volatility * np.random.randn()
            new_rate = rates[-1] * (1 + daily_return)
            rates.append(new_rate)
        
        data = pd.DataFrame({
            'Date': dates,
            'Exchange Rate': rates
        })
        data.set_index('Date', inplace=True)
        
        return data
    
    @staticmethod
    def save_sample_data(filepath: str = 'sample_rates.csv'):
        """
        Save sample data to CSV file
        
        Parameters:
        -----------
        filepath : str
            Path to save the sample data
        """
        data = ExchangeRateDataLoader.create_sample_data()
        data.reset_index(inplace=True)
        data.to_csv(filepath, index=False)
        print(f"Sample data saved to {filepath}")


def main():
    """Example usage of the data loader"""
    
    # Create sample data directory
    os.makedirs('example_data', exist_ok=True)
    
    # Save sample data
    ExchangeRateDataLoader.save_sample_data('example_data/sample_rates.csv')
    
    # Load sample data
    data = ExchangeRateDataLoader.load_from_csv('example_data/sample_rates.csv')
    
    if not data.empty:
        # Calculate statistics
        stats = ExchangeRateDataLoader.calculate_statistics(data)
        
        print("\nExchange Rate Statistics:")
        print("-" * 40)
        for key, value in stats.items():
            if isinstance(value, float):
                print(f"{key:20}: {value:.4f}")
            else:
                print(f"{key:20}: {value}")
        
        # Prepare training data
        returns, latest_rate = ExchangeRateDataLoader.prepare_training_data(data)
        print(f"\nLatest exchange rate: {latest_rate:.2f}")
        print(f"Number of return observations: {len(returns)}")


if __name__ == "__main__":
    main()
