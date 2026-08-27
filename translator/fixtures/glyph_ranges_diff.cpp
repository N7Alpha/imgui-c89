#include "imgui.h"

#include <cstdio>

static unsigned long long hash_ranges(const ImWchar* ranges)
{
    unsigned long long hash = 1469598103934665603ULL;
    int count = 0;
    while (ranges[count] != 0)
    {
        hash ^= (unsigned long long)ranges[count++];
        hash *= 1099511628211ULL;
    }
    std::printf("%d %llu\n", count, hash);
    return hash;
}

int main()
{
    ImFontAtlas atlas;
    hash_ranges(atlas.GetGlyphRangesChineseSimplifiedCommon());
    hash_ranges(atlas.GetGlyphRangesJapanese());
    return 0;
}
