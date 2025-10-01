# Examples

This directory contains example scripts for analyzing temperature log data.

## analyze_templog.py

Python script for analyzing and visualizing temperature data from the logger.

### Requirements

```bash
pip install pandas matplotlib
```

### Usage

```bash
python analyze_templog.py path/to/templog.csv
```

### Features

- Loads CSV data from SD card
- Calculates temperature statistics (min, max, mean, std)
- Creates temperature plots
- Analyzes heating system metrics (flow/return temperatures and spread)
- Exports plots as PNG images

### Output

The script generates:
- `temperature_plot.png` - All sensor temperatures over time
- `heating_analysis.png` - Flow/return analysis with temperature spread

### Example Output

```
Loading data from templog.csv...
Loaded 120 measurements

=== Data Analysis ===
Number of sensors: 2
Time range: 2025-01-01 00:00:00 to 2025-01-01 02:00:00
Duration: 2:00:00

Temperature Statistics:

Sensor0:
  Min:  44.50°C
  Max:  46.25°C
  Mean: 45.37°C
  Std:  0.45°C

Sensor1:
  Min:  31.25°C
  Max:  33.50°C
  Mean: 32.38°C
  Std:  0.58°C

=== Heating System Analysis ===
Flow Temperature (Vorlauf):
  Mean: 45.37°C
  Range: 44.50°C to 46.25°C

Return Temperature (Rücklauf):
  Mean: 32.38°C
  Range: 31.25°C to 33.50°C

Temperature Spread (Spreizung):
  Mean: 12.99°C
  Range: 11.80°C to 14.20°C
```

## Adding More Examples

Feel free to add your own analysis scripts here!
