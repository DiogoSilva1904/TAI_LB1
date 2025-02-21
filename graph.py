import matplotlib.pyplot as plt
import pandas as pd

# Read the context counts CSV file
data = pd.read_csv('context_counts.csv', header=None, names=['Context', 'Symbol', 'Count'])

# Group by context and plot the symbol counts
contexts = data['Context'].unique()

# Create a bar plot for each context
for context in contexts:
    context_data = data[data['Context'] == context]
    plt.figure(figsize=(10, 6))
    plt.bar(context_data['Symbol'], context_data['Count'])
    plt.title(f'Context: {context}')
    plt.xlabel('Symbol')
    plt.ylabel('Count')
    plt.xticks(rotation=90)  # Rotate x-axis labels for readability
    plt.show()
