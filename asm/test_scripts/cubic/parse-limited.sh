#!/bin/bash

LOG_DIR="klee-out-2" # Directory containing .ktest files
THRESHOLD=5          # Threshold for limited-valued variables

declare -A variables

parse_ktest_file() {
    ktest_file=$1
    echo "Processing file: $ktest_file"
    
    local name=""
    while read -r line; do
        # Detect variable names
        if [[ $line == *"name:"* ]]; then
            name=$(echo "$line" | awk -F": " '{print $2}' | tr -d "'")
            echo "DEBUG: Found variable name: $name"
        # Detect variable integer values
        elif [[ $line == *"int :"* ]]; then
            value=$(echo "$line" | awk -F": " '{print $2}' | xargs)
            echo "DEBUG: Found variable value: $value"

            # Append value to the appropriate variable
            if [[ -n "$name" && -n "$value" ]]; then
                variables["$name"]+="$value "
                echo "DEBUG: Updated variables[$name]: ${variables[$name]}"
                name="" # Reset name after processing
            fi
        fi
    done < <(ktest-tool "$ktest_file")
}

identify_limited_valued_variables() {
    echo "Identified limited-valued variables:"
    for var in "${!variables[@]}"; do
        # Count unique values for the variable
        unique_values=$(echo "${variables[$var]}" | tr ' ' '\n' | sort | uniq | wc -l)
        echo "DEBUG: Variable $var has $unique_values unique values"
        if [ "$unique_values" -le "$THRESHOLD" ]; then
            echo "Variable: $var"
            echo "Values: $(echo "${variables[$var]}" | tr ' ' '\n' | sort | uniq)"
            echo
        fi
    done
}

# Main Execution
export -f parse_ktest_file
export -f identify_limited_valued_variables

# Process all .ktest files in the log directory
find "$LOG_DIR" -name '*.ktest' -exec bash -c 'parse_ktest_file "$0"' {} \;

# Display all variables and their values
echo "All variables and their values:"
for var in "${!variables[@]}"; do
    echo "Variable: $var"
    echo "Values: ${variables[$var]}"
    echo
done

# Identify limited-valued variables
identify_limited_valued_variables
