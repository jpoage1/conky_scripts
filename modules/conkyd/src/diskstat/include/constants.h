#pragma once

// const int COL_WIDTH = 20;
// const int COL_WIDTH_1 = 30;
const int CHAR_WIDTH_PX = 10;                // approximate font width
const int COL_WIDTH_1 = 30 * CHAR_WIDTH_PX;  // first column
const int COL_WIDTH_2 = 30 * CHAR_WIDTH_PX;  // second column
const int COL_WIDTH = 12 * CHAR_WIDTH_PX;    // remaining columns

constexpr int NUM_COLUMNS = 6;
const int column_widths[NUM_COLUMNS] = {20, 20, 15,
                                        15, 10, 10};  // customize per column
