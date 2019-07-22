// wfc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <values.h>

#include "pch.h"
#include <vector>
#include <map>
#include <assert.h>
#include <chrono>
#include <iostream>

struct Coord
{
    int x, y;
    Coord& operator +=(const Coord& o) {
        x+=o.x;
        y+=o.y;
    }
};

int GetAngle(Coord dir)
{
    if (dir.x == 1)
        return 0;
    if (dir.y == 1)
        return 1;
    if (dir.x == -1)
        return 2;
    if (dir.y == -1)
        return 3;
    assert(0);
    return -1;
}
/* GetHook: index = 
      3
    0   2
      1
*/
int GetHook(Coord dir)
{
    if (dir.x == 1)
        return 2;
    if (dir.y == 1)
        return 3;
    if (dir.x == -1)
        return 0;
    if (dir.y == -1)
        return 1;
    assert(0);
    return -1;
}

struct Building{
    Building (const char* id, int w, int h) :
        id(id), width(w), height(h)
    {
    }
    size_t size() const { return width * height; }
    bool IsAngleEdge(Coord dir, int tileIndex) const
    {
        if (dir.x == 1)
            return tileIndex % width == width - 1;
        if (dir.y == 1)
            return tileIndex / width == height - 1;
        if (dir.x == -1)
            return tileIndex % width == 0;
        if (dir.y == -1)
            return tileIndex < width;
        
        return false;
    }
    bool IsHookEdge(Coord dir, int tileIndex) const
    {
        return IsAngleEdge({-dir.x, -dir.y}, tileIndex);
    }
    bool IsCorrectTile(Coord dir, int tileIndex1, int tileIndex2) const
    {
        Coord c({tileIndex1 % width, tileIndex1 / width});
        
        c += dir;
        
        return c.x >= 0 && c.x < width && c.y >= 0 && c.y < height && tileIndex2 == (c.x +  c.y *width );
    }

    char getId(int tileIndex) const
    {
        if (width == 1 && height == 1) return id[0];
        if (tileIndex == 0) return 'Ú';
        if (tileIndex == width - 1) return '¿';
        if (tileIndex == width * height - 1) return 'Ù';
        if (tileIndex == width * (height - 1)) return 'À';
        if (tileIndex % width == 0 || tileIndex % width == width - 1) return '³';
        if (tileIndex < width || tileIndex > width * (height - 1)) return 'Ä';
        return id[0];
    }
    const char *id;
    int width, height;
    
};


// struct Tile
// {
// //    char mBitmap;
//     const Building &mBuilding;
//     const int idx;
// //    char mKeys[4];
// };

// typedef std::vector<Tile> tiles_t;
typedef std::vector<Building> buildings_t;

struct BuildingSet {
    BuildingSet(const buildings_t&b)
    {
        assert(!b.empty());
        int i = 0;
        for (auto& bi : b) {
            std::cout << "Register " << bi.id << " @ " << i << " size=" << bi.size() << std::endl;
            auto n = mBuildings.emplace(std::piecewise_construct,
                            std::forward_as_tuple(i),
                            std::forward_as_tuple(bi));
            // _unused(n);
            assert(n.second);
            i += bi.size();
        }
        tileCount = i;
    }
    size_t size() const { return tileCount; }
    char getSymbol(int tileIndex) const
    {
        auto b = mBuildings.upper_bound(tileIndex);
        b--;
        return b->second.getId(tileIndex - b->first);

    }
    // const Building& getBuildingFromTileIndex(int idx) const
    // {
    //     auto i = mBuildings._bound(idx);
    //     assert(i != mBuildings.end());
    // 
    //     return i->second;
    // }
    bool TileCompatible(int tileIndex1, int tileIndex2, Coord dir) const
    {
        assert (dir.x == 0 || dir.y == 0);
        bool compat1 = false;
        bool compat2 = false;
        bool compat = false;

        auto b1 = mBuildings.upper_bound(tileIndex1);
        b1--;
//        assert(b1 != mBuildings.end());
        assert(tileIndex1 >= b1->first);

        if (tileIndex2 >= b1->first && tileIndex2 < b1->first + b1->second.size() ) {
            compat1 = compat = b1->second.IsCorrectTile(dir, tileIndex1 - b1->first, tileIndex2 - b1->first);
        }
        if (!compat && b1->second.IsAngleEdge(dir, tileIndex1 - b1->first) ) {
            
            auto b2 = mBuildings.upper_bound(tileIndex2);
            b2--;
            assert(b2 != mBuildings.end());
            
            compat2 = compat = b2->second.IsHookEdge(dir, tileIndex2 - b2->first);
        }
        
        if (false) {
            auto b1 = mBuildings.upper_bound(tileIndex1);
            b1--;
            auto b2 = mBuildings.upper_bound(tileIndex2);
            b2--;

            std::cout << (compat ? "ok                    " : "") << 
                b1->second.getId(tileIndex1-b1->first) << 
                "(" << tileIndex1 << "," << b1->first << ") ~ " <<
                b2->second.getId(tileIndex2-b2->first) << 
                "(" << tileIndex2 << "," << b2->first << ") dir=" << 
                dir.x << "," << dir.y << 
                " __ " << compat1 << "," << b1->second.IsAngleEdge(dir, tileIndex1 - b1->first) << "," << b2->second.IsHookEdge(dir, tileIndex2 - b2->first)
                << std::endl;
        }

        return compat;

    }

    // const buildings_t& mBuildings;
    std::map<int, const Building&>mBuildings;
    size_t tileCount;
};

// +x
// +y
// -x
// -y
/*
static const int tileCount = 10;
Tile mTiles[tileCount] = {
    {'X',{1,1,1,1}},
{'/',{1,0,0,1}},
{'\\',{0,0,1,1}},
{'|',{1,0,0,0}},
{'|',{0,0,1,0}},
{'-',{0,0,0,1}},
{'-',{0,1,0,0}},
{'/',{0,1,1,0}},
{'\\',{1,1,0,0}},
{' ',{0,0,0,0}}
};

/*
static const int tileCount = 3;
Tile mTiles[tileCount] = {
    {' ',{2,2,2,2}},
    {'X',{1,1,1,1}},
    {'.',{3,3,3,3}},

};
*/
// #define _ 16
//    Tile mTiles[] = {
//        {'.',{3,3,3,3}},
//        {'X',{1,1,1,1}},
//        {' ',{2,2,2,2}},
//    //    {' ',{16,16,16,16}},
//    //    {'Ú',{1,4|8|_,2|8|_,4}},    {'¿',{1|4|_,4|8|_,1,8}},
//    //    {'À',{8,1,2|8|_,1|2|_}},    {'Ù',{1|4|_,2,4,1|2|_}},
//    //    {'Ú',{_,4,1,_}},    {'¿',{1,8,_,_}},
//    //    {'À',{_,_,2,4}},    {'Ù',{2,_,_,8}},
//    };
//    static const int tileCount = sizeof(mTiles)/sizeof(mTiles[0]);

/* GetAngle: index = 
      1
    2   0
      3
*/

struct Model
{
    Model(const BuildingSet& buildings, int width, int height)
        : mBuildings(buildings), tileCount(buildings.size()), mWidth(width), mHeight(height)
    {
        mCoef.resize(width*height * tileCount, true);
        /*for (auto&c : mCoef)
        {
            c.resize(tileCount, true);
        }
        */
        mSumCoef.resize(width*height, tileCount);
        mTotalSum = width * height * tileCount;
        std::cout << "tileCount = " << tileCount << std::endl;
    }


    int GetTileAtIndex(Coord coord) const
    {
        int idx = (coord.y * mWidth + coord.x) * tileCount;
        int res = -1;
        for (int i = 0; i < tileCount; i++)
        {
            if (mCoef[idx + i])
            {
                assert(res == -1);
                res = i;
            }
        }
        return res;
    }

    Coord GetMinEntropy()
    {
        int minEntropy = INT_MAX;
        static std::vector<Coord> minEntropyCoords;
        minEntropyCoords.clear();
        for (int y = 0; y < mHeight; y++)
        {
            for (int x = 0; x < mWidth; x++)
            {
                int coef = mSumCoef[y * mWidth + x];
                if (coef == 1)
                    continue;
                if (coef < minEntropy)
                {
                    minEntropy = coef;
                    minEntropyCoords.clear();
                }
                if (coef == minEntropy)
                    minEntropyCoords.push_back({ x, y });
            }
        }
        assert(!minEntropyCoords.empty());
        return minEntropyCoords[rand() % minEntropyCoords.size()];
    }
    bool IsFullyCollapsed()
    {
        return mTotalSum == (mWidth * mHeight);
    }
    void Collapse(Coord coord, int tileIndex)
    {
        int idx = (coord.y * mWidth + coord.x)*tileCount;
        for (unsigned int i = 0; i < tileCount; i++)
        {
            if (mCoef[idx + i])
                mTotalSum--;
            mCoef[idx + i] = 0;
        }
        mCoef[idx + tileIndex] = 1;
        mTotalSum++;
        mSumCoef[coord.y * mWidth + coord.x] = 1;
    }
    void Collapse(Coord coord)
    {
        int potentials[tileCount];
        int potentialIndex = 0;
        int idx = (coord.y * mWidth + coord.x)*tileCount;
        //auto & v = mCoef[coord.y * mWidth + coord.x];
        int cnt = 0;
        for (unsigned int i = 0; i < tileCount; i++)
        {
            if (mCoef[idx + i])
                potentials[potentialIndex++] = i;
        }
        assert(potentialIndex);
        static int rd = 0;
        int selected = potentials[rand() % potentialIndex];
        Collapse(coord, selected);
    }
    int GetPossibleTiles(Coord coord, int* possibleTiles)
    {
        int res = 0;
        int idx = (coord.y * mWidth + coord.x)*tileCount;
        //auto & v = mCoef[coord.y * mWidth + coord.x];
//        std::cout << "possibleTiles @ " << coord.x << "," << coord.y << ": ";
        for (unsigned int i = 0; i < tileCount; i++)
        {
            if (mCoef[idx + i]) {
                possibleTiles[res++] = i;
//                std::cout << i << ",";
            }
        }
//        std::cout << std::endl;
        return res;
    }
    int GetPossibleTiles(Coord coord, Coord dir, int* possibleTiles)
    {
        return GetPossibleTiles(coord, possibleTiles);
    }
    void GetValidDirs(Coord coord, Coord *dest, int& coordCount)
    {
        if (coord.x < (mWidth-1))
            dest[coordCount++] = {1, 0 };
        if (coord.y < (mHeight - 1))
            dest[coordCount++] = { 0, 1 };
        if (coord.x > 0)
            dest[coordCount++] = { -1, 0 };
        if (coord.y > 0)
            dest[coordCount++] = {0, -1};
    }
    void Constrain(Coord coord, int tileIndex)
    {
        int idx = (coord.y * mWidth + coord.x)*tileCount;
        //auto & v = mCoef[coord.y * mWidth + coord.x];
        assert(mCoef[idx + tileIndex]);
        mCoef[idx + tileIndex] = 0;
        mSumCoef[coord.y * mWidth + coord.x] --;
        mTotalSum--;
    }
    void Propagate(Coord coord)
    {
        static std::vector<Coord> coords;
        coords.clear();
        coords.push_back(coord);
        while (coords.size())
        {
            Coord currentCoord = coords.back();
            coords.pop_back();
            
            int curPossibleTiles[tileCount];
            int curPossibleTileCount = GetPossibleTiles(currentCoord, curPossibleTiles);

            Coord validDirs[4];
            int validDirCount = 0;
            GetValidDirs(currentCoord, validDirs, validDirCount);
            for (int d = 0 ; d < validDirCount ; d++)
            {
                Coord dir = validDirs[d];
                Coord otherCoord = { currentCoord.x + dir.x, currentCoord.y + dir.y };
                int otherPossibleTiles[tileCount];
                int otherPossibleTileCount = GetPossibleTiles(otherCoord, otherPossibleTiles);
                for (int otherTileIndex = 0;otherTileIndex< otherPossibleTileCount;otherTileIndex++)
                {
                    int otherTile = otherPossibleTiles[otherTileIndex];
                    bool tileCompatible = false;
                    for (int curTileIndex = 0;curTileIndex< curPossibleTileCount;curTileIndex++)
                    {
                        int curTile = curPossibleTiles[curTileIndex];
                        tileCompatible |= mBuildings.TileCompatible(curTile, otherTile, dir);
                    }
                    if (!tileCompatible)
                    {
                        Constrain(otherCoord, otherTile);
                        coords.push_back(otherCoord);
                    }
                }
            }
        }
    }

    void Run()
    {
        while (!IsFullyCollapsed())
        {
            // std::cout << "------------------------------" << std::endl;
            Coord coord = GetMinEntropy();
            Collapse(coord);
            Propagate(coord);
        }
    }

    void Dump() const
    {
        for (int y = 0; y < mHeight; y++)
        {
            for (int x = 0; x < mWidth; x++)
            {
                int tileIndex = GetTileAtIndex(Coord{ x,y });

                printf("%1.1c", mBuildings.getSymbol(tileIndex));
            }
            printf("\n");
        }
    }

    const BuildingSet& mBuildings;
    const size_t tileCount;
    int mWidth, mHeight;
    std::vector<bool> mCoef;
    std::vector<unsigned short> mSumCoef;
    unsigned int mTotalSum;
};


int main()
{
    srand(57784);
    
//     tiles_t mTiles = {
//         {'.',{3,3,3,3}},
//         {'X',{1,1,1,1}},
//         {' ',{2,2,2,2}},
// //    {' ',{16,16,16,16}},
// //    {'Ú',{1,4|8|_,2|8|_,4}},    {'¿',{1|4|_,4|8|_,1,8}},
// //    {'À',{8,1,2|8|_,1|2|_}},    {'Ù',{1|4|_,2,4,1|2|_}},
// //    {'Ú',{_,4,1,_}},    {'¿',{1,8,_,_}},
// //    {'À',{_,_,2,4}},    {'Ù',{2,_,_,8}},
//     };

    buildings_t buildings = {
//        {"B", 3, 3},
        {"r", 2, 2},
        {"p", 3, 3},
        {"H", 5, 4},
        {"*", 1, 1},
    };
    BuildingSet bs(buildings);
// static const int tileCount = sizeof(mTiles)/sizeof(mTiles[0]);


    int runCount = 1;
/*
    auto t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < runCount; i++)
    {
        Model model(bs, 40, 25);
        model.Run();
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    
    printf("Time per solve : %2.4f seconds\n", float(time_span.count()) / float(runCount));
*/      

    Model model(bs, 18, 18);
    model.Run();
    model.Dump();
    
}

