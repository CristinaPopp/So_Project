#!/bin/bash

if [ $# -ne 1 ]; then
    echo "numar gresit de argumente"
    echo "USAGE: bash $0 <char>"
    exit 1
fi
ch=$1
count=0

while IFS = read -r line || [[ -n "$line" ]]; do
    aux = $(echo "$line" | grep -E '^[A-Z][a-zA-Z0-9, -]*(\.|\?|\!)$' | grep -vE ',[ ]*si[ ]*' | grep -vE '[ ]*si[ ]*,')
    if [[ -n "$aux" && $line == *"$ch"* ]]; then
        ((count++))
    fi
done

echo $count
