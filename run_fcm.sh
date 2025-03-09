#!/bin/bash

# Path to the sequence file
SEQUENCE_FILE="sequences/sequence2.txt"

# Define k and alpha values to test
K_VALUES=(2 3 4 5 6)  
ALPHA_VALUES=(0.001 0.005 0.01 0.1 0.5 1 3 5 10)  

# Output files
OUTPUT_FILE="fcm_results.txt"
CSV_FILE="alpha_plot.csv"

# Clear previous results
> "$OUTPUT_FILE"
echo "k,alpha,entropy" > "$CSV_FILE"

echo "Testing different k and alpha values"
echo "--------------------------------------"
echo -e "k\talpha\tentropy"

# Loop through different values of k and alpha
for k in "${K_VALUES[@]}"; do
    for alpha in "${ALPHA_VALUES[@]}"; do
        echo "Running: ./fcm $SEQUENCE_FILE -k $k -a $alpha"
        
        # Run FCM model and capture full output
        RESULT=$(./fcm "$SEQUENCE_FILE" -k "$k" -a "$alpha")
        
        # Extract entropy (assuming entropy is the last value in RESULT)
        ENTROPY=$(echo "$RESULT" | awk '{print $NF}')
        
        # Log to console and files
        echo "Result: $RESULT"
        echo -e "$k\t$alpha\t$ENTROPY"
        echo "$k,$alpha,$RESULT" >> "$CSV_FILE"
    done
done

echo "All tests completed! Results saved in $OUTPUT_FILE and $CSV_FILE."


