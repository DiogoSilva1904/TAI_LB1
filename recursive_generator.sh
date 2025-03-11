#!/bin/bash

# Parameters
INPUT_FILE="sequences/sequence2.txt"  # Initial training file
K=5             # Context size
ALPHA=0.1        # Smoothing parameter
PRIOR="As armas e os barões assinalados, que da lollol"
SIZE=10000        # Generated text size
THRESHOLD=0.6    # Candidate selection threshold
ITERATIONS=3     # Number of recursive cycles

# Clean up old files
rm -f k_values_*.csv entropy_values.csv

for (( i=1; i<=$ITERATIONS; i++ ))
do
    echo "Iteration $i: Training model on $INPUT_FILE"
    ./generator train -f "$INPUT_FILE" -k "$K" -a "$ALPHA"

    echo "Iteration $i: Generating text..."
    GENERATED_FILE="./generated_text/generated_text_$i.txt"
    WRITE_FILE="k_values_$i.csv"

    ./generator generate -k "$K" -a "$ALPHA" -p "$PRIOR" -s "$SIZE" -t "$THRESHOLD" -w "$WRITE_FILE" -d 0 > "$GENERATED_FILE"
    
    # Remove first and last lines (headers, summary)
    sed -i '1d;$d' "$GENERATED_FILE"

    echo "Iteration $i complete. Using $GENERATED_FILE as input for next training."
    INPUT_FILE="$GENERATED_FILE" 
done

echo "Recursive text generation complete."
