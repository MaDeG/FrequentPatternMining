#!/bin/bash
for dataset_path in datasets/*.dat
do
        dataset=$(basename $dataset_path .dat)
        mkdir -p results-releases/$dataset
        for minsup in 25 75
        do
                for threads in 1 16
                do
                        for release in releases/*
                        do
                                echo "Running release $release dataset $dataset_path with $threads threads and $minsup minsup"
                                for i in {0..4}
                                do
                                        (time $release -i $dataset_path -o -t $threads -s $minsup) 2>> ~/results-releases/$dataset/${release}_${threads}_${minsup} 1> /dev/null
                                done
                        done
                done
        done
done

sudo shutdown -P 0