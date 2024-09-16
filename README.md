# ACK-Scalable Method (ASM): Scalably check the multi-ACK properties of Linux TCP congestion control algorithm implementations


This repository contains the source code and experiment scripts for paper "Scalable Verification of Multi-ACK Properties in Loss-Based Congestion Control Implementations" published in ICNP 2024. 

## Introduction

Congestion control algorithms, such as RENO and CUBIC, are vital for the Internet. However, numerous bugs have been discovered and reported in the Congestion Control Algorithm Implementations (CCAIs), even in those that have been extensively tested and used on the Internet for years, such as Linux RENO and Linux CUBIC. Some of these bugs have potentially severe impacts on the performance and stability of the Internet. Unfortunately, current CCAI testing and verification methods are inadequate for proving the absence of bugs, require substantial verification expertise, or are not scalable to a large number of acknowledgment packets (ACKs) that trigger CCAI actions. To address all these shortcomings, we propose an ACK Scalable Method, called ASM, and conduct experiments with two representative loss-based CCAIs, Linux RENO and CUBIC.

## Instructions

* Install [KLEE](https://klee-se.org/) to ~/klee_build
* Download ASM code to ~/asm

## Experiments

There are three groups of experiments.

### Linux RENO experiments

* The scripts are located in the ~/asm/test_script/aimd directory.
* `plan_eq.sh` is the script to check the equivalence between the aggregation algorithm and the original RENO.
* `plan_agg_property.sh` is the script to check the maximum increment properties of RENO using the aggregation algorithm.
* `plan_ori_property.sh` is the script to check the maximum increment properties of RENO using the original RENO.

### Linux CUBIC experiments

* The scripts are located in the ~/asm/test_script/cubic directory.
* `plan_eq.sh` is the script to check the equivalence between the aggregation algorithm and the original CUBIC.
* `plan_agg_property.sh` is the script to check the maximum increment properties of CUBIC using the aggregation algorithm.
* `plan_ori_property.sh` is the script to check the maximum increment properties of CUBIC using the original CUBIC.
* `plan_property_target.sh` is the script to check the maximum increment properties of CUBIC in extreme network environments.

### Linux RENO and CUBIC experiments

* The scripts are located in the ~/asm/test_script/friendliness directory.
* `plan.sh` is the script to check the friendliness property of CUBIC.
