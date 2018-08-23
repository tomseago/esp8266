#pragma once

// Expects to be included after LEDGeometry.h

uint16_t BTFrow0[] = { 14, 13, 12, 11, 10 };
uint16_t BTFrow1[] = { 15, 33, 32, 31,  9 };
uint16_t BTFrow2[] = { 16, 34, 47, 46, 45, 30,  8};
uint16_t BTFrow3[] = { 17, 35, 48, 57, 56, 55, 44, 29, 7};
uint16_t BTFrow4[] = { 18, 36, 49, 58, 60, 54, 43, 28, 6};
uint16_t BTFrow5[] = { 19, 37, 50, 59,  5, 53, 42, 27, 5};
uint16_t BTFrow6[] = { 20, 38, 51, 40, 41, 26,  4 };
uint16_t BTFrow7[] = { 21, 39, 24, 25, 3};
uint16_t BTFrow8[] = { 22, 23,  0,  1, 2};

uint16_t* BTFrows[] = {
    BTFrow0,
    BTFrow1,
    BTFrow2,
    BTFrow3,
    BTFrow4,
    BTFrow5,
    BTFrow6,
    BTFrow7,
    BTFrow8,
};

uint16_t BTFcolCounts[] = {
    5,
    5,
    7,
    9,
    9,
    9,
    7,
    5,
    5,
};

LEDArtRowGeometry geomBackToFront("Back to Front", 9, BTFrows, BTFcolCounts);


uint16_t STSrow0[] = { 16, 17, 18, 19, 20};
uint16_t STSrow1[] = { 15, 35, 36, 37, 21};
uint16_t STSrow2[] = { 14, 34, 48, 49, 50, 38, 22};
uint16_t STSrow3[] = { 13, 33, 47, 57, 58, 59, 51, 39, 23};
uint16_t STSrow4[] = { 12, 32, 46, 56, 60,  2, 40, 24,  0};
uint16_t STSrow5[] = { 11, 31, 45, 55, 54, 53, 41,  2,  1};
uint16_t STSrow6[] = { 10, 30, 44, 43, 42, 26, 2};
uint16_t STSrow7[] = {  9, 29, 28, 27,  3};
uint16_t STSrow8[] = {  8,  7,  6,  7,  4};

uint16_t* STSrows[] = {
    STSrow0,
    STSrow1,
    STSrow2,
    STSrow3,
    STSrow4,
    STSrow5,
    STSrow6,
    STSrow7,
    STSrow8,
};

uint16_t STScolCounts[] = {
    5,
    5,
    7,
    9,
    9,
    9,
    7,
    5,
    5,
};

LEDArtRowGeometry geomSideToSide("Side To Side", 9, STSrows, STScolCounts);