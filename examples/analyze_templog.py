#!/usr/bin/env python3
"""
ESP32 Temperature Logger - Data Analysis Example

This script reads the templog.csv file from the SD card and creates
visualizations of the temperature data.

Requirements:
    pip install pandas matplotlib

Usage:
    python analyze_templog.py templog.csv
"""

import sys
import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime, timedelta

def load_data(filename):
    """Load temperature data from CSV file."""
    print(f"Loading data from {filename}...")
    df = pd.read_csv(filename)
    
    # Convert timestamp to datetime
    df['DateTime'] = pd.to_datetime(df['Timestamp'], unit='s')
    
    print(f"Loaded {len(df)} measurements")
    return df

def analyze_data(df):
    """Perform basic analysis on temperature data."""
    print("\n=== Data Analysis ===")
    
    # Find temperature columns
    temp_cols = [col for col in df.columns if col.endswith('_TempC')]
    
    print(f"Number of sensors: {len(temp_cols)}")
    print(f"Time range: {df['DateTime'].min()} to {df['DateTime'].max()}")
    print(f"Duration: {df['DateTime'].max() - df['DateTime'].min()}")
    
    print("\nTemperature Statistics:")
    for col in temp_cols:
        sensor_num = col.split('_')[0]
        print(f"\n{sensor_num}:")
        print(f"  Min:  {df[col].min():.2f}°C")
        print(f"  Max:  {df[col].max():.2f}°C")
        print(f"  Mean: {df[col].mean():.2f}°C")
        print(f"  Std:  {df[col].std():.2f}°C")

def plot_data(df, output_file='temperature_plot.png'):
    """Create visualization of temperature data."""
    print(f"\nCreating plot...")
    
    # Find temperature columns
    temp_cols = [col for col in df.columns if col.endswith('_TempC')]
    
    # Create plot
    fig, ax = plt.subplots(figsize=(12, 6))
    
    for col in temp_cols:
        sensor_num = col.replace('_TempC', '')
        ax.plot(df['DateTime'], df[col], marker='o', markersize=3, label=sensor_num)
    
    ax.set_xlabel('Time')
    ax.set_ylabel('Temperature (°C)')
    ax.set_title('Temperature Logger Data')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # Format x-axis
    plt.xticks(rotation=45)
    plt.tight_layout()
    
    plt.savefig(output_file, dpi=150)
    print(f"Plot saved to {output_file}")
    
    # Show plot
    plt.show()

def calculate_heating_metrics(df, flow_sensor='Sensor0', return_sensor='Sensor1'):
    """
    Calculate heating system metrics.
    Assumes Sensor0 is flow temperature and Sensor1 is return temperature.
    """
    flow_col = f'{flow_sensor}_TempC'
    return_col = f'{return_sensor}_TempC'
    
    if flow_col not in df.columns or return_col not in df.columns:
        print("\nWarning: Cannot calculate heating metrics - missing sensors")
        return
    
    print("\n=== Heating System Analysis ===")
    
    # Temperature difference (spread)
    df['Spread'] = df[flow_col] - df[return_col]
    
    print(f"Flow Temperature (Vorlauf):")
    print(f"  Mean: {df[flow_col].mean():.2f}°C")
    print(f"  Range: {df[flow_col].min():.2f}°C to {df[flow_col].max():.2f}°C")
    
    print(f"\nReturn Temperature (Rücklauf):")
    print(f"  Mean: {df[return_col].mean():.2f}°C")
    print(f"  Range: {df[return_col].min():.2f}°C to {df[return_col].max():.2f}°C")
    
    print(f"\nTemperature Spread (Spreizung):")
    print(f"  Mean: {df['Spread'].mean():.2f}°C")
    print(f"  Range: {df['Spread'].min():.2f}°C to {df['Spread'].max():.2f}°C")
    
    # Create spread plot
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8))
    
    # Plot temperatures
    ax1.plot(df['DateTime'], df[flow_col], label='Flow (Vorlauf)', color='red')
    ax1.plot(df['DateTime'], df[return_col], label='Return (Rücklauf)', color='blue')
    ax1.set_ylabel('Temperature (°C)')
    ax1.set_title('Flow and Return Temperatures')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Plot spread
    ax2.plot(df['DateTime'], df['Spread'], color='green')
    ax2.set_xlabel('Time')
    ax2.set_ylabel('Temperature Spread (°C)')
    ax2.set_title('Temperature Spread (Spreizung)')
    ax2.grid(True, alpha=0.3)
    
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig('heating_analysis.png', dpi=150)
    print("\nHeating analysis plot saved to heating_analysis.png")
    plt.show()

def main():
    if len(sys.argv) < 2:
        print("Usage: python analyze_templog.py templog.csv")
        sys.exit(1)
    
    filename = sys.argv[1]
    
    try:
        # Load data
        df = load_data(filename)
        
        # Analyze data
        analyze_data(df)
        
        # Plot data
        plot_data(df)
        
        # Calculate heating metrics if applicable
        calculate_heating_metrics(df)
        
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()
