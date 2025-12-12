import numpy as np
import pandas as pd
from typing import Tuple, Optional, Dict, Any, Union
from dataclasses import dataclass


# 常量定义（提升可维护性）
TRADING_DAYS_PER_YEAR = 252


@dataclass
class PredictionResult:
    """Data class for structured prediction output (optional but recommended)"""
    mean: float
    median: float
    std: float
    min_val: float
    max_val: float
    confidence_interval: Tuple[float, float]
    confidence_level: float


@dataclass
class VaRResult:
    """Data class for VaR output"""
    var_percent: float
    var_amount: float
    cvar_percent: float
    confidence_level: float
    initial_investment: float


class CurrencyMonteCarlo:
    """
    Monte Carlo simulation for currency exchange rates using Geometric Brownian Motion (GBM).
    
    The model follows: dS/S = μ dt + σ dW
    Discretized: S[t] = S[t-1] * exp((μ - 0.5σ²)dt + σ√dt * Z), Z ~ N(0,1)
    """
    
    def __init__(
        self,
        initial_rate: float = 75.0,
        mu: float = 0.05,
        sigma: float = 0.15,
        trading_days_per_year: int = TRADING_DAYS_PER_YEAR
    ):
        """
        Initialize the Monte Carlo model
        
        Parameters:
        -----------
        initial_rate : float
            Initial exchange rate (must be > 0)
        mu : float
            Expected annual return (drift)
        sigma : float
            Annual volatility (must be >= 0)
        trading_days_per_year : int
            Number of trading days per year (default: 252)
        """
        if initial_rate <= 0:
            raise ValueError("initial_rate must be positive")
        if sigma < 0:
            raise ValueError("sigma must be non-negative")
        if trading_days_per_year <= 0:
            raise ValueError("trading_days_per_year must be positive")
            
        self.initial_rate = float(initial_rate)
        self.mu = float(mu)
        self.sigma = float(sigma)
        self.trading_days_per_year = int(trading_days_per_year)
        
        self._simulations: Optional[np.ndarray] = None
        self._predicted_rates: Optional[np.ndarray] = None

    @property
    def simulations(self) -> Optional[np.ndarray]:
        """Return simulations (read-only)"""
        return self._simulations.copy() if self._simulations is not None else None

    @property
    def predicted_rates(self) -> Optional[np.ndarray]:
        """Return predicted final rates (read-only)"""
        return self._predicted_rates.copy() if self._predicted_rates is not None else None

    def simulate(
        self,
        days: int = 30,
        n_simulations: int = 1000,
        random_seed: Optional[int] = None,
        return_paths: bool = True
    ) -> np.ndarray:
        """
        Run Monte Carlo simulation using vectorized GBM.
        
        Parameters:
        -----------
        days : int
            Number of days to forecast (must be >= 1)
        n_simulations : int
            Number of simulation paths (must be >= 1)
        random_seed : int, optional
            Random seed for reproducibility
        return_paths : bool
            Whether to return full paths (True) or just final rates (False)
            
        Returns:
        --------
        np.ndarray
            Simulated paths of shape (n_simulations, days) if return_paths=True,
            else final rates of shape (n_simulations,)
        """
        if days < 1:
            raise ValueError("days must be at least 1")
        if n_simulations < 1:
            raise ValueError("n_simulations must be at least 1")

        dt = 1.0 / self.trading_days_per_year
        n_steps = days

        # Use Generator for better randomness control (NumPy >= 1.17)
        rng = np.random.default_rng(random_seed)

        # Generate all random shocks at once: shape (n_simulations, n_steps - 1)
        if n_steps > 1:
            z = rng.standard_normal(size=(n_simulations, n_steps - 1))
        else:
            z = np.empty((n_simulations, 0))

        # Vectorized GBM: avoid Python loop
        # log(S_t) = log(S_0) + (μ - 0.5σ²)t + σ * W_t
        # W_t = cumsum(√dt * Z)
        drift = (self.mu - 0.5 * self.sigma ** 2) * dt
        diffusion = self.sigma * np.sqrt(dt)

        # Cumulative sum of shocks for each path
        if n_steps > 1:
            # Shape: (n_simulations, n_steps - 1)
            incremental_log_returns = drift + diffusion * z
            # Prepend zeros for t=0, then cumulative sum
            log_returns = np.concatenate(
                [np.zeros((n_simulations, 1)), incremental_log_returns], 
                axis=1
            ).cumsum(axis=1)
        else:
            log_returns = np.zeros((n_simulations, 1))

        # Compute paths: S_t = S_0 * exp(log_returns)
        log_initial = np.log(self.initial_rate)
        paths = np.exp(log_initial + log_returns)  # Shape: (n_simulations, n_steps)

        self._simulations = paths
        self._predicted_rates = paths[:, -1]

        return paths if return_paths else paths[:, -1]

    def get_prediction(self, confidence_level: float = 0.95) -> PredictionResult:
        """Get prediction statistics as a structured object."""
        if self._predicted_rates is None:
            raise RuntimeError("Call simulate() before get_prediction()")

        if not (0 < confidence_level < 1):
            raise ValueError("confidence_level must be between 0 and 1")

        pred = self._predicted_rates
        mean_val = np.mean(pred)
        median_val = np.median(pred)
        std_val = np.std(pred, ddof=1)  # Sample std
        min_val = np.min(pred)
        max_val = np.max(pred)

        alpha = 1.0 - confidence_level
        lower = np.percentile(pred, 100 * alpha / 2)
        upper = np.percentile(pred, 100 * (1 - alpha / 2))

        return PredictionResult(
            mean=mean_val,
            median=median_val,
            std=std_val,
            min_val=min_val,
            max_val=max_val,
            confidence_interval=(lower, upper),
            confidence_level=confidence_level
        )

    def calculate_var(
        self,
        confidence_level: float = 0.95,
        initial_investment: float = 10000.0
    ) -> VaRResult:
        """Calculate Value at Risk (VaR) and Conditional VaR (Expected Shortfall)."""
        if self._predicted_rates is None:
            raise RuntimeError("Call simulate() before calculate_var()")

        if not (0 < confidence_level < 1):
            raise ValueError("confidence_level must be between 0 and 1")
        if initial_investment <= 0:
            raise ValueError("initial_investment must be positive")

        # Returns: (S_T - S_0) / S_0
        returns = (self._predicted_rates - self.initial_rate) / self.initial_rate

        # VaR: loss not exceeded with confidence_level probability
        var_quantile = np.percentile(returns, 100 * (1 - confidence_level))
        var_amount = initial_investment * abs(var_quantile)

        # CVaR: average of worst (1 - confidence_level) returns
        tail_mask = returns <= var_quantile
        tail_returns = returns[tail_mask]
        cvar = np.mean(tail_returns) if len(tail_returns) > 0 else var_quantile

        return VaRResult(
            var_percent=var_quantile * 100,
            var_amount=var_amount,
            cvar_percent=cvar * 100,
            confidence_level=confidence_level,
            initial_investment=initial_investment
        )

    def calculate_probability(self, target_rate: float) -> float:
        """Calculate probability of exchange rate >= target_rate at horizon."""
        if self._predicted_rates is None:
            raise RuntimeError("Call simulate() before calculate_probability()")

        if target_rate <= 0:
            raise ValueError("target_rate must be positive")

        return float(np.mean(self._predicted_rates >= target_rate))


class MultiCurrencyMonteCarlo:
    """
    Monte Carlo simulation for multiple currency pairs.
    Supports correlation modeling (future extension).
    """
    
    def __init__(self):
        self.models: Dict[str, CurrencyMonteCarlo] = {}

    def add_currency(
        self,
        currency_pair: str,
        initial_rate: float,
        mu: float = 0.05,
        sigma: float = 0.15,
        trading_days_per_year: int = TRADING_DAYS_PER_YEAR
    ) -> None:
        """
        Add a currency pair to the simulation.
        
        Parameters:
        -----------
        currency_pair : str
            Currency pair symbol (e.g., 'USD/RUB')
        initial_rate, mu, sigma, trading_days_per_year
            See CurrencyMonteCarlo.__init__
        """
        if not currency_pair or not isinstance(currency_pair, str):
            raise ValueError("currency_pair must be a non-empty string")
        self.models[currency_pair] = CurrencyMonteCarlo(
            initial_rate=initial_rate,
            mu=mu,
            sigma=sigma,
            trading_days_per_year=trading_days_per_year
        )

    def simulate_all(
        self,
        days: int = 30,
        n_simulations: int = 1000,
        random_seed: Optional[int] = None,
        return_paths: bool = True
    ) -> Dict[str, np.ndarray]:
        """
        Simulate all currency pairs.
        
        Note: Currently assumes independence. For correlated simulation,
        override this method or extend with Cholesky decomposition.
        """
        if not self.models:
            raise ValueError("No currency pairs added. Use add_currency() first.")
            
        results = {}
        # Use same RNG for all to ensure reproducibility across runs
        base_rng = np.random.default_rng(random_seed)
        
        for i, (currency_pair, model) in enumerate(self.models.items()):
            # Seed each simulation deterministically if global seed given
            local_seed = int(base_rng.integers(0, 2**32 - 1)) if random_seed is not None else None
            results[currency_pair] = model.simulate(
                days=days,
                n_simulations=n_simulations,
                random_seed=local_seed,
                return_paths=return_paths
            )
        return results

    def get_comparison(self) -> pd.DataFrame:
        """Get comparison table of all currency predictions."""
        if not self.models:
            return pd.DataFrame()  # Empty DataFrame

        records = []
        for currency_pair, model in self.models.items():
            if model._predicted_rates is None:
                continue
            
            pred = model.get_prediction()
            var = model.calculate_var()

            records.append({
                'Currency Pair': currency_pair,
                'Initial Rate': model.initial_rate,
                'Predicted Mean': pred.mean,
                'Predicted Median': pred.median,
                'Std Dev': pred.std,
                f"VaR {int(var.confidence_level*100)}% (%)": var.var_percent,
                'Min': pred.min_val,
                'Max': pred.max_val,
                '95% CI Lower': pred.confidence_interval[0],
                '95% CI Upper': pred.confidence_interval[1],
            })

        if not records:
            return pd.DataFrame()
        
        return pd.DataFrame(records).round(4)
