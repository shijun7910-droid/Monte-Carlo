import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
from typing import Optional, Dict, List, Tuple, Union
import matplotlib
from matplotlib.patches import Rectangle, Circle
from matplotlib.lines import Line2D
from scipy import stats
import warnings

# 忽略警告
warnings.filterwarnings('ignore')

# 设置中文字体（如果需要）
# plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial']
# plt.rcParams['axes.unicode_minus'] = False

class MonteCarloVisualizer:
    """
    Advanced visualization tools for Monte Carlo simulation results
    """
    
    def __init__(self, figsize=(12, 8), style='seaborn-v0_8-darkgrid', 
                 color_palette='viridis', dpi=100):
        """
        Initialize the visualizer
        
        Parameters:
        -----------
        figsize : tuple
            Figure size (width, height)
        style : str
            Matplotlib style
        color_palette : str
            Color palette for plots
        dpi : int
            DPI for saved figures
        """
        self.figsize = figsize
        self.dpi = dpi
        self.color_palette = color_palette
        
        # 设置样式
        plt.style.use(style)
        sns.set_palette(color_palette)
        
        # 定义颜色
        self.colors = {
            'primary': '#2E86AB',      # 深蓝色
            'secondary': '#A23B72',    # 紫色
            'accent': '#F18F01',       # 橙色
            'success': '#73AB84',      # 绿色
            'danger': '#C73E1D',       # 红色
            'warning': '#F0C808',      # 黄色
            'info': '#048BA8',         # 青色
            'dark': '#2F2D2E',         # 深灰色
            'light': '#F5F5F5',        # 浅灰色
            'paths': '#4A6FA5',        # 路径颜色
            'mean': '#C73E1D',         # 均值线颜色
            'ci': '#F18F01',           # 置信区间颜色
        }
    
    def _create_figure(self, nrows=1, ncols=1, figsize=None, **kwargs):
        """创建图形和坐标轴"""
        if figsize is None:
            figsize = self.figsize
        fig, axes = plt.subplots(nrows, ncols, figsize=figsize, **kwargs)
        if nrows == 1 and ncols == 1:
            axes = np.array([axes])
        return fig, axes.flatten() if isinstance(axes, np.ndarray) else axes
    
    def _add_grid(self, ax, alpha=0.1):
        """添加网格"""
        ax.grid(True, alpha=alpha, linestyle='--', linewidth=0.5)
    
    def _add_watermark(self, fig, text="Monte Carlo Simulation", alpha=0.1, fontsize=30):
        """添加水印"""
        fig.text(0.5, 0.5, text, fontsize=fontsize, color='gray', alpha=alpha,
                ha='center', va='center', rotation=30, transform=fig.transFigure)
    
    def _save_figure(self, fig, save_path, tight_layout=True):
        """保存图形"""
        if save_path:
            if tight_layout:
                fig.tight_layout()
            fig.savefig(save_path, dpi=self.dpi, bbox_inches='tight', 
                       facecolor=fig.get_facecolor())
            print(f"📊 Figure saved to: {save_path}")
    
    def _add_statistics_box(self, ax, stats_dict, position='top_right', fontsize=9):
        """添加统计信息框"""
        stats_text = "\n".join([f"{k}: {v:.4f}" if isinstance(v, float) else f"{k}: {v}"
                               for k, v in stats_dict.items()])
        
        if position == 'top_right':
            x, y, va, ha = 0.98, 0.98, 'top', 'right'
        elif position == 'top_left':
            x, y, va, ha = 0.02, 0.98, 'top', 'left'
        elif position == 'bottom_right':
            x, y, va, ha = 0.98, 0.02, 'bottom', 'right'
        else:  # bottom_left
            x, y, va, ha = 0.02, 0.02, 'bottom', 'left'
        
        ax.text(x, y, stats_text, transform=ax.transAxes, fontsize=fontsize,
                verticalalignment=va, horizontalalignment=ha,
                bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    
    def plot_simulation_paths(self, simulations: np.ndarray, 
                            n_paths: int = 100,
                            show_mean: bool = True,
                            show_ci: bool = True,
                            ci_levels: List[float] = [0.9, 0.95, 0.99],
                            title: str = "Monte Carlo Simulation Paths",
                            xlabel: str = "Days",
                            ylabel: str = "Exchange Rate",
                            save_path: Optional[str] = None) -> plt.Figure:
        """
        Plot Monte Carlo simulation paths with enhanced features
        
        Parameters:
        -----------
        simulations : np.ndarray
            Array of simulated paths
        n_paths : int
            Number of paths to display
        show_mean : bool
            Whether to show mean path
        show_ci : bool
            Whether to show confidence intervals
        ci_levels : List[float]
            Confidence interval levels to display
        title : str
            Plot title
        xlabel : str
            X-axis label
        ylabel : str
            Y-axis label
        save_path : str, optional
            Path to save the figure
            
        Returns:
        --------
        plt.Figure
            The created figure
        """
        fig, ax = self._create_figure()
        
        # 计算基本统计量
        final_rates = simulations[:, -1]
        stats = {
            'Initial Rate': simulations[0, 0],
            'Final Rate Mean': np.mean(final_rates),
            'Final Rate Std': np.std(final_rates),
            'Min Final Rate': np.min(final_rates),
            'Max Final Rate': np.max(final_rates),
            'Paths': simulations.shape[0],
            'Days': simulations.shape[1]
        }
        
        # 绘制路径子集
        if n_paths < simulations.shape[0]:
            indices = np.random.choice(simulations.shape[0], n_paths, replace=False)
            paths_to_plot = simulations[indices]
        else:
            paths_to_plot = simulations
        
        # 绘制路径（使用透明度渐变）
        for i in range(paths_to_plot.shape[0]):
            alpha = 0.05 + 0.95 * (i / paths_to_plot.shape[0])  # 渐变透明度
            ax.plot(paths_to_plot[i], alpha=alpha * 0.3, 
                   color=self.colors['paths'], linewidth=0.8)
        
        # 绘制均值路径
        if show_mean:
            mean_path = np.mean(simulations, axis=0)
            ax.plot(mean_path, color=self.colors['mean'], linewidth=3, 
                   label=f'Mean Path', zorder=10)
        
        # 绘制置信区间
        if show_ci and ci_levels:
            mean_path = np.mean(simulations, axis=0)
            
            # 按置信水平排序（从大到小）
            ci_levels_sorted = sorted(ci_levels, reverse=True)
            alphas = np.linspace(0.1, 0.3, len(ci_levels_sorted))
            
            for i, ci_level in enumerate(ci_levels_sorted):
                alpha_val = 1 - ci_level
                lower_bound = np.percentile(simulations, (alpha_val/2)*100, axis=0)
                upper_bound = np.percentile(simulations, (1-alpha_val/2)*100, axis=0)
                
                ax.fill_between(range(len(mean_path)), lower_bound, upper_bound,
                              alpha=alphas[i], color=self.colors['ci'],
                              label=f'{ci_level:.0%} CI')
        
        # 设置标签和标题
        ax.set_xlabel(xlabel, fontsize=12)
        ax.set_ylabel(ylabel, fontsize=12)
        ax.set_title(title, fontsize=14, fontweight='bold', pad=20)
        
        # 添加图例
        if show_mean or (show_ci and ci_levels):
            ax.legend(loc='upper left', fontsize=10, framealpha=0.9)
        
        # 添加网格
        self._add_grid(ax)
        
        # 添加统计信息框
        self._add_statistics_box(ax, stats, position='top_right')
        
        # 添加水印
        self._add_watermark(fig)
        
        # 保存图形
        self._save_figure(fig, save_path)
        
        return fig
    
    def plot_distribution_analysis(self, predicted_rates: np.ndarray,
                                 initial_rate: float,
                                 show_kde: bool = True,
                                 show_qq: bool = True,
                                 show_boxplot: bool = True,
                                 title: str = "Distribution Analysis",
                                 save_path: Optional[str] = None) -> plt.Figure:
        """
        Comprehensive distribution analysis with multiple plots
        
        Parameters:
        -----------
        predicted_rates : np.ndarray
            Array of predicted final rates
        initial_rate : float
            Initial exchange rate
        show_kde : bool
            Whether to show KDE plot
        show_qq : bool
            Whether to show Q-Q plot
        show_boxplot : bool
            Whether to show box plot
        title : str
            Plot title
        save_path : str, optional
            Path to save the figure
            
        Returns:
        --------
        plt.Figure
            The created figure
        """
        # 确定子图数量
        n_plots = 1 + show_kde + show_qq + show_boxplot
        if n_plots == 1:
            fig, axes = self._create_figure(1, 1)
            axes = [axes]
        else:
            fig, axes = self._create_figure(2, 2)
        
        plot_idx = 0
        
        # 1. 直方图与KDE
        ax = axes[plot_idx]
        n_bins = min(50, int(np.sqrt(len(predicted_rates))))
        
        # 绘制直方图
        hist, bins, patches = ax.hist(predicted_rates, bins=n_bins, density=True,
                                     alpha=0.7, color=self.colors['primary'],
                                     edgecolor='white', linewidth=1)
        
        # 添加KDE
        if show_kde:
            kde = stats.gaussian_kde(predicted_rates)
            x_kde = np.linspace(np.min(predicted_rates), np.max(predicted_rates), 1000)
            ax.plot(x_kde, kde(x_kde), color=self.colors['danger'], 
                   linewidth=2.5, label='KDE')
        
        # 添加参考线
        ax.axvline(initial_rate, color=self.colors['accent'], linestyle='--',
                  linewidth=2, label=f'Initial: {initial_rate:.2f}')
        ax.axvline(np.mean(predicted_rates), color=self.colors['success'], 
                  linestyle='--', linewidth=2, label=f'Mean: {np.mean(predicted_rates):.2f}')
        
        # 添加置信区间阴影
        ci_95 = np.percentile(predicted_rates, [2.5, 97.5])
        ax.axvspan(ci_95[0], ci_95[1], alpha=0.2, color=self.colors['info'],
                  label='95% CI')
        
        ax.set_xlabel('Exchange Rate', fontsize=11)
        ax.set_ylabel('Density', fontsize=11)
        ax.set_title('Histogram with KDE', fontsize=12)
        ax.legend(fontsize=9)
        self._add_grid(ax)
        plot_idx += 1
        
        # 2. 累计分布函数
        if plot_idx < len(axes):
            ax = axes[plot_idx]
            sorted_rates = np.sort(predicted_rates)
            cdf = np.arange(1, len(sorted_rates) + 1) / len(sorted_rates)
            
            ax.plot(sorted_rates, cdf, linewidth=2.5, color=self.colors['secondary'])
            ax.fill_between(sorted_rates, cdf, alpha=0.3, color=self.colors['secondary'])
            
            # 添加参考线
            ax.axhline(0.5, color='red', linestyle='--', alpha=0.5, label='Median')
            ax.axhline(0.05, color='orange', linestyle='--', alpha=0.5, label='5% VaR')
            ax.axhline(0.95, color='orange', linestyle='--', alpha=0.5, label='95% CI')
            
            ax.set_xlabel('Exchange Rate', fontsize=11)
            ax.set_ylabel('Cumulative Probability', fontsize=11)
            ax.set_title('Cumulative Distribution Function', fontsize=12)
            ax.legend(fontsize=9)
            self._add_grid(ax)
            plot_idx += 1
        
        # 3. Q-Q图
        if show_qq and plot_idx < len(axes):
            ax = axes[plot_idx]
            stats.probplot(predicted_rates, dist="norm", plot=ax)
            ax.get_lines()[0].set_marker('o')
            ax.get_lines()[0].set_markersize(4)
            ax.get_lines()[0].set_alpha(0.6)
            ax.get_lines()[1].set_color('red')
            ax.get_lines()[1].set_linewidth(2)
            
            ax.set_title('Q-Q Plot (Normal Distribution)', fontsize=12)
            self._add_grid(ax)
            plot_idx += 1
        
        # 4. 箱线图
        if show_boxplot and plot_idx < len(axes):
            ax = axes[plot_idx]
            box_data = [predicted_rates]
            
            bp = ax.boxplot(box_data, patch_artist=True, widths=0.6,
                           medianprops=dict(color='red', linewidth=2),
                           boxprops=dict(facecolor=self.colors['primary'], alpha=0.7),
                           whiskerprops=dict(color='black', linestyle='--'),
                           capprops=dict(color='black'),
                           flierprops=dict(marker='o', markersize=5, alpha=0.5))
            
            # 添加异常值统计
            q1 = np.percentile(predicted_rates, 25)
            q3 = np.percentile(predicted_rates, 75)
            iqr = q3 - q1
            lower_bound = q1 - 1.5 * iqr
            upper_bound = q3 + 1.5 * iqr
            outliers = predicted_rates[(predicted_rates < lower_bound) | 
                                      (predicted_rates > upper_bound)]
            
            ax.text(0.5, 0.95, f'Outliers: {len(outliers)}',
                   transform=ax.transAxes, ha='center', va='top',
                   bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
            
            ax.set_xticklabels(['Predicted Rates'])
            ax.set_ylabel('Exchange Rate', fontsize=11)
            ax.set_title('Box Plot with Outliers', fontsize=12)
            self._add_grid(ax)
        
        # 计算统计信息
        stats_dict = {
            'Mean': np.mean(predicted_rates),
            'Median': np.median(predicted_rates),
            'Std Dev': np.std(predicted_rates),
            'Skewness': stats.skew(predicted_rates),
            'Kurtosis': stats.kurtosis(predicted_rates),
            'Jarque-Bera': stats.jarque_bera(predicted_rates)[0],
            'JB p-value': stats.jarque_bera(predicted_rates)[1]
        }
        
        # 在主图上添加统计信息
        main_ax = axes[0]
        self._add_statistics_box(main_ax, stats_dict, position='top_left')
        
        fig.suptitle(title, fontsize=16, fontweight='bold', y=1.02)
        
        # 添加水印
        self._add_watermark(fig)
        
        # 保存图形
        self._save_figure(fig, save_path)
        
        return fig
    
    def plot_risk_dashboard(self, model,
                          initial_investment: float = 10000,
                          show_var_curve: bool = True,
                          show_loss_dist: bool = True,
                          show_target_prob: bool = True,
                          show_risk_return: bool = True,
                          title: str = "Risk Analysis Dashboard",
                          save_path: Optional[str] = None) -> plt.Figure:
        """
        Comprehensive risk analysis dashboard
        
        Parameters:
        -----------
        model : CurrencyMonteCarlo
            Monte Carlo model with simulation results
        initial_investment : float
            Initial investment amount
        show_var_curve : bool
            Whether to show VaR curve
        show_loss_dist : bool
            Whether to show loss distribution
        show_target_prob : bool
            Whether to show target probability
        show_risk_return : bool
            Whether to show risk-return profile
        title : str
            Dashboard title
        save_path : str, optional
            Path to save the figure
            
        Returns:
        --------
        plt.Figure
            The created figure
        """
        if model.predicted_rates is None:
            raise ValueError("Run simulation first")
        
        # 确定子图数量
        plots_enabled = [show_var_curve, show_loss_dist, 
                        show_target_prob, show_risk_return]
        n_plots = sum(plots_enabled)
        
        if n_plots == 0:
            raise ValueError("At least one plot must be enabled")
        
        # 创建子图
        if n_plots <= 2:
            fig, axes = self._create_figure(1, n_plots, figsize=(6*n_plots, 5))
        else:
            fig, axes = self._create_figure(2, 2, figsize=(12, 10))
            axes = axes.flatten()
        
        ax_idx = 0
        
        # 1. VaR曲线
        if show_var_curve:
            ax = axes[ax_idx]
            confidence_levels = np.linspace(0.90, 0.99, 20)
            var_values = []
            cvar_values = []
            
            for cl in confidence_levels:
                var_stats = model.calculate_var(cl, initial_investment)
                var_values.append(var_stats['var_percent'])
                cvar_values.append(var_stats['cvar_percent'])
            
            ax.plot(confidence_levels * 100, var_values, marker='o', 
                   linewidth=2, color=self.colors['danger'], label='VaR')
            ax.plot(confidence_levels * 100, cvar_values, marker='s',
                   linewidth=2, color=self.colors['warning'], label='CVaR')
            
            # 标记95%置信水平
            var_95 = model.calculate_var(0.95, initial_investment)
            ax.axvline(95, color='blue', linestyle='--', alpha=0.7)
            ax.text(95, min(var_values), f"VaR={var_95['var_percent']:.1f}%",
                   ha='center', va='top', bbox=dict(boxstyle='round', facecolor='white', alpha=0.8))
            
            ax.set_xlabel('Confidence Level (%)', fontsize=11)
            ax.set_ylabel('Risk (%)', fontsize=11)
            ax.set_title('VaR and CVaR Curves', fontsize=12)
            ax.legend(fontsize=9)
            self._add_grid(ax)
            ax_idx += 1
        
        # 2. 损失分布
        if show_loss_dist:
            ax = axes[ax_idx]
            returns = (model.predicted_rates - model.initial_rate) / model.initial_rate * 100
            
            # 绘制直方图
            n_bins = min(50, int(np.sqrt(len(returns))))
            ax.hist(returns, bins=n_bins, density=True, alpha=0.7,
                   color=self.colors['primary'], edgecolor='white', linewidth=1)
            
            # 添加VaR线
            var_95 = model.calculate_var(0.95, initial_investment)
            ax.axvline(var_95['var_percent'], color=self.colors['danger'],
                      linestyle='--', linewidth=2.5, label=f"VaR 95%")
            
            # 阴影显示最差损失
            worst_returns = returns[returns <= var_95['var_percent']]
            if len(worst_returns) > 0:
                ax.hist(worst_returns, bins=n_bins, density=True, alpha=0.7,
                       color=self.colors['danger'], edgecolor='white', linewidth=1,
                       label='Worst 5% Losses')
            
            ax.axvline(0, color='black', linestyle='-', linewidth=1, alpha=0.5)
            
            ax.set_xlabel('Return (%)', fontsize=11)
            ax.set_ylabel('Density', fontsize=11)
            ax.set_title('Return Distribution with VaR', fontsize=12)
            ax.legend(fontsize=9)
            self._add_grid(ax)
            ax_idx += 1
        
        # 3. 目标概率
        if show_target_prob:
            ax = axes[ax_idx]
            target_rates = np.linspace(model.initial_rate * 0.7, 
                                     model.initial_rate * 1.3, 30)
            probabilities = []
            
            for target in target_rates:
                prob = model.calculate_probability(target)
                probabilities.append(prob * 100)
            
            ax.plot(target_rates, probabilities, marker='o', linewidth=2,
                   color=self.colors['success'], markersize=4)
            ax.fill_between(target_rates, probabilities, alpha=0.3,
                           color=self.colors['success'])
            
            # 标记重要水平
            for prob_threshold in [10, 50, 90]:
                # 找到最接近的概率对应的汇率
                prob_diff = np.abs(np.array(probabilities) - prob_threshold)
                closest_idx = np.argmin(prob_diff)
                if prob_diff[closest_idx] < 5:  # 容差5%
                    ax.axhline(prob_threshold, color='gray', linestyle=':', alpha=0.5)
                    ax.text(target_rates[closest_idx], prob_threshold,
                           f'{prob_threshold}%', ha='right', va='bottom')
            
            ax.axvline(model.initial_rate, color=self.colors['accent'],
                      linestyle='--', linewidth=2, label='Current Rate')
            
            ax.set_xlabel('Target Exchange Rate', fontsize=11)
            ax.set_ylabel('Probability (%)', fontsize=11)
            ax.set_title('Probability of Reaching Target', fontsize=12)
            ax.legend(fontsize=9)
            self._add_grid(ax)
            ax_idx += 1
        
        # 4. 风险-回报分析
        if show_risk_return:
            ax = axes[ax_idx]
            returns_pct = (model.predicted_rates - model.initial_rate) / model.initial_rate * 100
            
            mean_return = np.mean(returns_pct)
            volatility = np.std(returns_pct)
            sharpe_ratio = mean_return / volatility if volatility > 0 else 0
            
            # 散点图显示风险回报分布
            ax.scatter(volatility, mean_return, s=300, color=self.colors['secondary'],
                      alpha=0.8, edgecolors='black', linewidth=2, zorder=10,
                      label=f'Portfolio (Sharpe: {sharpe_ratio:.2f})')
            
            # 添加有效前沿示意线
            frontier_vol = np.linspace(volatility * 0.5, volatility * 2, 20)
            frontier_ret = sharpe_ratio * frontier_vol
            ax.plot(frontier_vol, frontier_ret, '--', color='gray', alpha=0.5,
                   label='Efficient Frontier')
            
            ax.axhline(0, color='black', linestyle='-', alpha=0.3)
            ax.axvline(0, color='black', linestyle='-', alpha=0.3)
            
            # 添加风险分类区域
            if volatility > 0:
                risk_levels = [
                    (0, volatility*0.5, 'Low Risk', 'lightgreen'),
                    (volatility*0.5, volatility*1.5, 'Medium Risk', 'lightyellow'),
                    (volatility*1.5, volatility*2.5, 'High Risk', 'lightcoral')
                ]
                
                for lower, upper, label, color in risk_levels:
                    ax.axvspan(lower, upper, alpha=0.1, color=color)
                    ax.text((lower+upper)/2, mean_return*0.5, label,
                           ha='center', va='center', rotation=90, fontsize=8)
            
            ax.set_xlabel('Volatility (Risk) (%)', fontsize=11)
            ax.set_ylabel('Expected Return (%)', fontsize=11)
            ax.set_title('Risk-Return Profile', fontsize=12)
            ax.legend(fontsize=9, loc='upper left')
            self._add_grid(ax)
        
        fig.suptitle(title, fontsize=16, fontweight='bold', y=1.02)
        
        # 添加水印
        self._add_watermark(fig, text="Risk Analysis", alpha=0.08)
        
        # 保存图形
        self._save_figure(fig, save_path)
        
        return fig
    
    def plot_multi_currency_dashboard(self, multi_model,
                                    metrics: List[str] = ['initial', 'predicted', 'var', 'volatility'],
                                    sort_by: str = 'expected_return',
                                    show_heatmap: bool = True,
                                    title: str = "Multi-Currency Analysis Dashboard",
                                    save_path: Optional[str] = None) -> plt.Figure:
        """
        Advanced multi-currency comparison dashboard
        
        Parameters:
        -----------
        multi_model : MultiCurrencyMonteCarlo
            Multi-currency model
        metrics : List[str]
            Metrics to display
        sort_by : str
            Sort currencies by this metric
        show_heatmap : bool
            Whether to show correlation heatmap
        title : str
            Dashboard title
        save_path : str, optional
            Path to save the figure
            
        Returns:
        --------
        plt.Figure
            The created figure
        """
        comparison_df = multi_model.get_comparison()
        
        if comparison_df.empty:
            raise ValueError("No simulation results available")
        
        # 计算额外指标
        comparison_df['Expected Return %'] = ((comparison_df['Predicted Mean'] - 
                                             comparison_df['Initial Rate']) / 
                                            comparison_df['Initial Rate'] * 100)
        comparison_df['Return/Risk Ratio'] = comparison_df['Expected Return %'] / comparison_df['Std Dev']
        
        # 排序
        if sort_by in comparison_df.columns:
            comparison_df = comparison_df.sort_values(sort_by, ascending=False)
        
        # 创建子图
        n_subplots = len(metrics) + (1 if show_heatmap else 0)
        fig, axes = self._create_figure(2, max(2, (n_subplots + 1) // 2), figsize=(15, 10))
        axes = axes.flatten()
        
        plot_idx = 0
        currency_names = comparison_df['Currency Pair'].values
        x_pos = np.arange(len(currency_names))
        
        # 绘制每个指标的条形图
        metric_configs = {
            'initial': {
                'data': comparison_df['Initial Rate'],
                'title': 'Initial Exchange Rates',
                'ylabel': 'Rate',
                'color': self.colors['primary']
            },
            'predicted': {
                'data': comparison_df['Predicted Mean'],
                'title': 'Predicted Exchange Rates',
                'ylabel': 'Rate',
                'color': self.colors['secondary']
            },
            'var': {
                'data': comparison_df['VaR 95.0%'],
                'title': 'Value at Risk (95%)',
                'ylabel': 'VaR (%)',
                'color': self.colors['danger']
            },
            'volatility': {
                'data': comparison_df['Std Dev'],
                'title': 'Volatility (Standard Deviation)',
                'ylabel': 'Std Dev',
                'color': self.colors['warning']
            },
            'expected_return': {
                'data': comparison_df['Expected Return %'],
                'title': 'Expected Return',
                'ylabel': 'Return (%)',
                'color': self.colors['success']
            },
            'return_risk': {
                'data': comparison_df['Return/Risk Ratio'],
                'title': 'Return/Risk Ratio',
                'ylabel': 'Ratio',
                'color': self.colors['info']
            }
        }
        
        for metric in metrics:
            if metric in metric_configs and plot_idx < len(axes):
                config = metric_configs[metric]
                ax = axes[plot_idx]
                
                # 创建条形图
                bars = ax.bar(x_pos, config['data'], color=config['color'],
                             alpha=0.7, edgecolor='white', linewidth=1.5)
                
                # 添加数值标签
                for bar in bars:
                    height = bar.get_height()
                    ax.text(bar.get_x() + bar.get_width()/2., height + 0.01*height,
                           f'{height:.2f}', ha='center', va='bottom', fontsize=8)
                
                # 设置坐标轴
                ax.set_xticks(x_pos)
                ax.set_xticklabels(currency_names, rotation=45, ha='right')
                ax.set_ylabel(config['ylabel'], fontsize=11)
                ax.set_title(config['title'], fontsize=12)
                self._add_grid(ax, alpha=0.2)
                
                plot_idx += 1
        
        # 添加热力图（相关性矩阵）
        if show_heatmap and plot_idx < len(axes):
            ax = axes[plot_idx]
            
            # 选择数值列进行相关性分析
            numeric_cols = comparison_df.select_dtypes(include=[np.number]).columns
            if len(numeric_cols) > 1:
                corr_matrix = comparison_df[numeric_cols].corr()
                
                # 创建热力图
                im = ax.imshow(corr_matrix, cmap='RdYlBu', vmin=-1, vmax=1,
                              aspect='auto')
                
                # 添加文本
                for i in range(len(corr_matrix)):
                    for j in range(len(corr_matrix)):
                        text = ax.text(j, i, f'{corr_matrix.iloc[i, j]:.2f}',
                                      ha="center", va="center", 
                                      color="white" if abs(corr_matrix.iloc[i, j]) > 0.5 else "black",
                                      fontsize=9)
                
                # 设置坐标轴
                ax.set_xticks(range(len(numeric_cols)))
                ax.set_yticks(range(len(numeric_cols)))
                ax.set_xticklabels([col[:10] for col in numeric_cols], rotation=45, ha='right')
                ax.set_yticklabels([col[:10] for col in numeric_cols])
                ax.set_title('Correlation Matrix', fontsize=12)
                
                # 添加颜色条
                plt.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
            
            plot_idx += 1
        
        # 添加雷达图（如果还有空间）
        if plot_idx < len(axes):
            ax = axes[plot_idx]
            self._create_radar_chart(ax, comparison_df)
        
        fig.suptitle(title, fontsize=18, fontweight='bold', y=1.02)
        
        # 添加总结信息
        summary_text = f"""
        Total Currencies: {len(comparison_df)}
        Best Performer: {comparison_df.iloc[0]['Currency Pair']}
        Worst Performer: {comparison_df.iloc[-1]['Currency Pair']}
        Avg Expected Return: {comparison_df['Expected Return %'].mean():.2f}%
        Avg Volatility: {comparison_df['Std Dev'].mean():.4f}
        """
        
        fig.text(0.02, 0.02, summary_text, fontsize=10,
                bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8),
                transform=fig.transFigure)
        
        # 添加水印
        self._add_watermark(fig, text="Multi-Currency", alpha=0.06)
        
        # 保存图形
        self._save_figure(fig, save_path)
        
        return fig
    
    def _create_radar_chart(self, ax, df):
        """创建雷达图"""
        try:
            # 选择前5个货币对
            n_currencies = min(5, len(df))
            currencies = df.head(n_currencies)
            
            # 选择要显示的指标
            metrics = ['Expected Return %', 'VaR 95.0%', 'Std Dev', 'Predicted Mean']
            metrics = [m for m in metrics if m in df.columns]
            
            if len(metrics) < 3:
                ax.text(0.5, 0.5, "Insufficient data\nfor radar chart",
                       ha='center', va='center', fontsize=12)
                ax.set_title('Radar Chart', fontsize=12)
                return
            
            # 标准化数据
            normalized_data = []
            for metric in metrics:
                col_data = currencies[metric].values
                min_val, max_val = col_data.min(), col_data.max()
                if max_val > min_val:
                    normalized = (col_data - min_val) / (max_val - min_val)
                else:
                    normalized = np.ones_like(col_data) * 0.5
                normalized_data.append(normalized)
            
            normalized_data = np.array(normalized_data)
            
            # 计算角度
            angles = np.linspace(0, 2*np.pi, len(metrics), endpoint=False).tolist()
            angles += angles[:1]  # 闭合图形
            
            # 绘制每个货币对
            for i in range(n_currencies):
                values = normalized_data[:, i].tolist()
                values += values[:1]  # 闭合图形
                
                ax.plot(angles, values, linewidth=2, 
                       label=currencies.iloc[i]['Currency Pair'])
                ax.fill(angles, values, alpha=0.1)
            
            # 设置雷达图
            ax.set_xticks(angles[:-1])
            ax.set_xticklabels([m[:15] for m in metrics])
            ax.set_ylim(0, 1)
            ax.set_title('Performance Radar Chart', fontsize=12)
            ax.legend(loc='upper right', fontsize=8, bbox_to_anchor=(1.3, 1.0))
            ax.grid(True, alpha=0.3)
            
        except Exception as e:
            ax.text(0.5, 0.5, f"Error creating radar chart:\n{str(e)}",
                   ha='center', va='center', fontsize=10, color='red')
            ax.set_title('Radar Chart (Error)', fontsize=12)
    
    def plot_time_series_analysis(self, historical_data: pd.DataFrame,
                                simulation_data: np.ndarray,
                                title: str = "Time Series Analysis",
                                save_path: Optional[str] = None) -> plt.Figure:
        """
        Plot historical data with simulation forecasts
        
        Parameters:
        -----------
        historical_data : pd.DataFrame
            Historical exchange rate data
        simulation_data : np.ndarray
            Monte Carlo simulation data
        title : str
            Plot title
        save_path : str, optional
            Path to save the figure
            
        Returns:
        --------
        plt.Figure
            The created figure
        """
        fig, axes = self._create_figure(2, 2, figsize=(14, 10))
        
        # 1. 历史数据与预测
        ax1 = axes[0]
        
        # 绘制历史数据
        if 'Exchange Rate' in historical_data.columns:
            historical_data['Exchange Rate'].plot(ax=ax1, linewidth=2,
                                                color=self.colors['primary'],
                                                label='Historical Data')
        
        # 绘制预测区间
        forecast_days = simulation_data.shape[1]
        last_historical_date = historical_data.index[-1]
        forecast_dates = pd.date_range(start=last_historical_date, 
                                      periods=forecast_days+1, freq='D')[1:]
        
        mean_forecast = np.mean(simulation_data, axis=0)
        ci_lower = np.percentile(simulation_data, 2.5, axis=0)
        ci_upper = np.percentile(simulation_data, 97.5, axis=0)
        
        ax1.plot(forecast_dates, mean_forecast, linewidth=2.5,
                color=self.colors['danger'], label='Mean Forecast')
        ax1.fill_between(forecast_dates, ci_lower, ci_upper,
                        alpha=0.2, color=self.colors['ci'], label='95% CI')
        
        ax1.set_xlabel('Date', fontsize=11)
        ax1.set_ylabel('Exchange Rate', fontsize=11)
        ax1.set_title('Historical Data with Forecast', fontsize=12)
        ax1.legend(fontsize=9)
        ax1.grid(True, alpha=0.3)
        
        # 2. 滚动统计量
        ax2 = axes[1]
        
        if 'Exchange Rate' in historical_data.columns:
            rolling_mean = historical_data['Exchange Rate'].rolling(window=20).mean()
            rolling_std = historical_data['Exchange Rate'].rolling(window=20).std()
            
            ax2.plot(historical_data.index, rolling_mean, linewidth=2,
                    color=self.colors['success'], label='20-Day Moving Avg')
            ax2.fill_between(historical_data.index,
                            rolling_mean - rolling_std,
                            rolling_mean + rolling_std,
                            alpha=0.2, color=self.colors['info'],
                            label='±1 Std Dev')
            
            ax2.set_xlabel('Date', fontsize=11)
            ax2.set_ylabel('Exchange Rate', fontsize=11)
            ax2.set_title('Rolling Statistics', fontsize=12)
            ax2.legend(fontsize=9)
            ax2.grid(True, alpha=0.3)
        
        # 3. 收益率分布
        ax3 = axes[2]
        
        if 'Exchange Rate' in historical_data.columns:
            returns = historical_data['Exchange Rate'].pct_change().dropna() * 100
            
            # 直方图
            n_bins = min(50, int(np.sqrt(len(returns))))
            ax3.hist(returns, bins=n_bins, density=True, alpha=0.7,
                    color=self.colors['primary'], edgecolor='white', linewidth=1)
            
            # 正态分布拟合
            if len(returns) > 10:
                mu, std = returns.mean(), returns.std()
                x = np.linspace(returns.min(), returns.max(), 100)
                p = stats.norm.pdf(x, mu, std)
                ax3.plot(x, p, 'k', linewidth=2, label='Normal Fit')
            
            ax3.axvline(0, color='black', linestyle='--', alpha=0.5)
            ax3.set_xlabel('Daily Return (%)', fontsize=11)
            ax3.set_ylabel('Density', fontsize=11)
            ax3.set_title('Return Distribution', fontsize=12)
            ax3.legend(fontsize=9)
            ax3.grid(True, alpha=0.3)
        
        # 4. 预测误差分析
        ax4 = axes[3]
        
        # 计算预测误差统计
        if len(mean_forecast) > 1:
            forecast_errors = simulation_data - mean_forecast[np.newaxis, :]
            mae = np.mean(np.abs(forecast_errors))
            mse = np.mean(forecast_errors**2)
            rmse = np.sqrt(mse)
            
            # 绘制误差分布
            final_errors = forecast_errors[:, -1]
            ax4.hist(final_errors, bins=30, density=True, alpha=0.7,
                    color=self.colors['warning'], edgecolor='white', linewidth=1)
            
            ax4.axvline(0, color='black', linestyle='--', linewidth=2, alpha=0.7)
            ax4.set_xlabel('Forecast Error', fontsize=11)
            ax4.set_ylabel('Density', fontsize=11)
            ax4.set_title(f'Forecast Error Distribution\nMAE: {mae:.4f}, RMSE: {rmse:.4f}', 
                         fontsize=12)
            ax4.grid(True, alpha=0.3)
        
        fig.suptitle(title, fontsize=16, fontweight='bold', y=1.02)
        
        # 添加水印
        self._add_watermark(fig, text="Time Series", alpha=0.06)
        
        # 保存图形
        self._save_figure(fig, save_path)
        
        return fig


def create_sample_data():
    """创建示例数据用于测试"""
    np.random.seed(42)
    
    # 创建历史数据
    dates = pd.date_range(start='2023-01-01', end='2023-12-31', freq='D')
    n_days = len(dates)
    
    # 模拟历史汇率（随机游走）
    historical_rates = np.zeros(n_days)
    historical_rates[0] = 75.0
    
    for i in range(1, n_days):
        historical_rates[i] = historical_rates[i-1] * (1 + np.random.normal(0.0002, 0.01))
    
    historical_data = pd.DataFrame({
        'Exchange Rate': historical_rates
    }, index=dates)
    
    # 创建模拟数据
    n_simulations = 1000
    forecast_days = 30
    
    simulations = np.zeros((n_simulations, forecast_days))
    initial_rate = historical_rates[-1]
    
    for i in range(n_simulations):
        simulations[i, 0] = initial_rate
        for j in range(1, forecast_days):
            simulations[i, j] = simulations[i, j-1] * (1 + np.random.normal(0.0005, 0.015))
    
    return historical_data, simulations, initial_rate


def main():
    """测试可视化模块"""
    print("Testing Monte Carlo Visualizer...")
    
    # 创建示例数据
    historical_data, simulations, initial_rate = create_sample_data()
    predicted_rates = simulations[:, -1]
    
    # 创建可视化器实例
    visualizer = MonteCarloVisualizer(
        figsize=(14, 8),
        style='seaborn-v0_8-whitegrid',
        color_palette='husl'
    )
    
    # 测试各种可视化
    print("\n1. Plotting simulation paths...")
    visualizer.plot_simulation_paths(
        simulations,
        n_paths=100,
        title="USD/RUB Monte Carlo Simulation",
        save_path="simulation_paths.png"
    )
    
    print("\n2. Plotting distribution analysis...")
    visualizer.plot_distribution_analysis(
        predicted_rates,
        initial_rate,
        title="Predicted Rate Distribution Analysis",
        save_path="distribution_analysis.png"
    )
    
    print("\n3. Testing multi-currency dashboard...")
    # 创建模拟的多货币数据
    class MockModel:
        def __init__(self, predicted_rates, initial_rate):
            self.predicted_rates = predicted_rates
            self.initial_rate = initial_rate
        
        def calculate_var(self, confidence_level, initial_investment):
            returns = (self.predicted_rates - self.initial_rate) / self.initial_rate * 100
            var_percentile = np.percentile(returns, (1 - confidence_level) * 100)
            return {
                'var_percent': var_percentile,
                'cvar_percent': var_percentile * 0.9  # 简化
            }
        
        def calculate_probability(self, target_rate):
            return np.mean(self.predicted_rates >= target_rate)
    
    model = MockModel(predicted_rates, initial_rate)
    
    print("\n4. Plotting risk dashboard...")
    visualizer.plot_risk_dashboard(
        model,
        initial_investment=10000,
        title="Risk Analysis Dashboard",
        save_path="risk_dashboard.png"
    )
    
    print("\n5. Plotting time series analysis...")
    visualizer.plot_time_series_analysis(
        historical_data,
        simulations,
        title="Historical and Forecast Analysis",
        save_path="time_series_analysis.png"
    )
    
    print("\n✅ All visualizations created successfully!")


if __name__ == "__main__":
    main()
