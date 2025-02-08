import pandas as pd

def read_custom_csv(file_path):
    """Reads the CSV with floating-point values and varying row lengths"""
    data = []
    
    with open(file_path, 'r') as f:
        for line in f:
            # Clean line and convert to floats
            row = [float(x.strip()) for x in line.split(',') if x.strip()]
            data.append(row)
    
    # Create DataFrame with appropriate columns
    max_cols = max(len(row) for row in data)
    columns = [f'col_{i}' for i in range(max_cols)]
    
    return pd.DataFrame(data, columns=columns)

# Usage
df = read_custom_csv('output.csv')

# Display information about the data
print(f"Dataset shape: {df.shape}")
print("\nFirst 5 rows:")
print(df.head())
print("\nSummary statistics:")
print(df.describe())

# Optional: Save to a regular CSV with consistent columns
df.to_csv('processed_output.csv', index=False)

