# The analysis of falling data of accelerometer
Data are collected from the IMU, **LSM6DS3TR-C** installed within seeed XIAO nRf52840. The sum of square equation will be used on the 3 axis acceleration to obtain the resultant acceleration vector for the analysis. 

```
Norm = SQRT(ACCEL_X**2 + ACCEL_Y**2 + ACCEL_Z**2);
```

## Overview of data retrieved
![Falling Data Full](./Images/fullData.png "Falling Data Full")
**Legend**
1) Walking
2) Walking Up and Down Stairs
3) Dropping of IMU
4) Sitting and Standing up from plastic chair
5) Shaking of IMU
6) Falling from the chair
7) Fall forward

**Note**
1) The device designed to be used as a keychain, thus the device is placed in the back pocket of our pants during data collection.
2) The shaking of IMU is recorded with it held on the hand.

With that we will include the rate of change in acceleration, the second derivative in the rate of change in acceleration, and moving average of a window of 50 data point,

![Falling Data Full Expand](./Images/fullDataExpand.png "Falling Data Full Expand")

A few highest rate of change of acceleration will be recorded for analysis.

## Analysing **Walking**
![Walking Data 1](./Images/WalkingData1.png "Walking Data 1")
**Zooming in:**\
![Walking Data 2](./Images/WalkingData2.png "Walking Data 2")
**Note**
1) Random points:

|point| Norm | f'(x) | f''(x) |
|-----|------|-------|--------|
|2101| 0.85        | 0.336068328 | 0.00305213  |
|2202| 1.25876924  | 0.271906121 | 0.003399633 |
|2325| 1.393448959 | 0.397809776 | 0.003501889 |

## Analysing **Stairs**
![Stairs Data 1](./Images/StairsData1.png "Stairs Data 1")
From the graph we could observe two different section where the norm acceleration for going up the stairs is way lower than going down the stairs. Thus, we will analyse the going down of stairs instead. 
**Legend**
1) Walking up then down the stairs
2) Limping up then down the stairs
3) Limping up then down the stairs (Other Leg)
4) Running up then down the stairs

### Walking up then down the stairs
![Stairs Data 2](./Images/StairsData2.png "Stairs Data 2")
**Note**
|point| Norm | f'(x) | f''(x) |
|-----|------|-------|--------|
|26704| 0.595399026 | 1.453343034 | 0.015052862 |
|26900| 0.672978454 | 1.922880154 | 0.017114909 |
|27242| 0.904488806 | 0.175432193 | 0.00063385  |

### Limping up and then down the stairs
![Stairs Data 3](./Images/StairsData3.png "Stairs Data 3")
**Note**
|point| Norm | f'(x) | f''(x) |
|-----|------|-------|--------|
|34018| 5.320620265 | 2.319318931 | 0.034806461 |
|34223| 6.612503308 | 2.080303042 | 0.028609112 |
|34375| 4.503987123 | 1.345020579 | 0.015369838 |