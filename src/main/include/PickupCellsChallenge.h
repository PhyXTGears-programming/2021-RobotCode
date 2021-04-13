#pragma once

#define MAX_ERROR_FOR_MATCH 1000
#define WIN_SCORE 10
#define YOUNG_BLOCK_LIMIT 32
#define OLD_BLOCK_LIMIT 64

namespace challenge {
    enum class Layout {
        CONFUSED,
        A_RED,
        A_BLUE,
        B_RED,
        B_BLUE,
    };
}
