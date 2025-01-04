// wfc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <values.h>

#include "pch.h"
#include <vector>
#include <map>
#include <set>
#include <assert.h>
#include <chrono>
#include <iostream>

template <typename T> struct XY
{
    T x, y;
    XY<T>& operator +=(const XY<int>& o) {
        x+=o.x;
        y+=o.y;
        return *this;
    }
    XY<T>& operator +(const XY<int>& o) const {
        XY<T> n = {x+o.x, y+o.y};
        XY<T> &nref = n;
        return nref;
    }
};

struct Room {
    unsigned int n, s, e, w;
};
typedef XY<unsigned int> Coord;
typedef XY<int> Dir;

int GetAngle(Dir dir)
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
int GetHook(Dir dir)
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
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool IsAngleEdge(Dir dir, unsigned int tileIndex) const
    {
        if (dir.x > 0)
            return (tileIndex % width) + 1 == width;
        if (dir.y > 0)
            return tileIndex / width + 1 == height;
        if (dir.x < 0)
            return tileIndex % width == 0;
        if (dir.y < 0)
            return tileIndex / width == 0;
        assert(false);
        return false;
    }
    bool IsHookEdge(Dir dir, unsigned int tileIndex) const
    {
        return IsAngleEdge({-dir.x, -dir.y}, tileIndex);
    }
    bool IsCorrectTile(Dir dir, unsigned int tileIndex1, unsigned int tileIndex2) const
    {
        Coord c({tileIndex1 % width, tileIndex1 / width});
        
        c += dir;
        
        return c.x >= 0 && c.x < width && c.y >= 0 && c.y < height && tileIndex2 == (c.x +  c.y *width );
    }

    char getId(unsigned int tileIndex) const
    {
        if (width == 1 && height == 1) return id[0];
        if (tileIndex == 0) return 'Ú';
        if (tileIndex + 1 == width ) return '¿';
        if (tileIndex + 1 == width * height) return 'Ù';
        if (tileIndex + width == width * height) return 'À';
        if (tileIndex % width == 0 || tileIndex % width + 1 == width) return '³';
        if (tileIndex < width || tileIndex + width > width * height) return 'Ä';
        return id[0];
    }
    void GetPossibleEdgeTiles(Room& room, Dir& dir, std::set<unsigned int>& possible) const
    {
        std::cout << "Requested " << id << " edge tiles" /* << room.x << "," << room.y */ << 
            " towards " << dir.x << "," << dir.y << std::endl;
        assert(room.n != 0 && room.s != 0 && room.w != 0 && room.e != 0);
        if (dir.x > 0 ) {
            if (room.e >= width) // edge
                for(int i = 0; i < height; i += width) possible.insert(i*width);
        } else if (dir.x < 0) {
            if (room.w >= width) // Right Edge
                for(int i = 1; i <= height; i += width) possible.insert(i*width - 1);
        } else if (dir.y > 0) {
            if (room.n >= height) // Bottom edge
                for(int i = 1; i <= width; i++) possible.insert(height * width - i);
        } else if (dir.y < 0) { // Top edge
            if (room.s >= height)
                for(int i = 0; i < width; i++) possible.insert(i);
        } else {
            assert(false);
        }
    }

    const char *id;
    uint8_t width, height;
    
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
    bool TileCompatible(unsigned int tileIndex1, unsigned int tileIndex2, Dir dir) const
    {
        assert (dir.x == 0 || dir.y == 0);
        bool compat1 = false;
        bool compat2 = false;
        bool compat = false;

        auto b1 = mBuildings.upper_bound(tileIndex1);
        b1--;
//        assert(b1 != mBuildings.end());
        assert(tileIndex1 >= b1->first);

        // Is same building?
        if (tileIndex2 >= b1->first && tileIndex2 < b1->first + b1->second.size() ) {
            compat1 = compat = b1->second.IsCorrectTile(dir, tileIndex1 - b1->first, tileIndex2 - b1->first);
        }
        // Can it be adjacent building?
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
                << "," << compat2
                << std::endl;
        }

        return compat;

    }
#if 0
    void GetPossibleTiles(unsigned int tileIndex, const Room& room, std::set<unsigned int>& possible) const
    {
        assert(room.s != 0 && room.n != 0 && room.e != 0 && room.w != 0);

        auto b = mBuildings.upper_bound(tileIndex);
        b--;
        int width =  b->second.getWidth();
        int height =  b->second.getHeight();
        Dir c = {
            (tileIndex - b->first) % width,
            (tileIndex - b->first) / width};

        // If the next tile in the direction is within the same building, just return it
        // It is the only tile possible
        if (room.x > 0) c.x++;
        if (room.y > 0) c.y++;
        if (room.x < 0) c.x--;
        if (room.y < 0) c.y--;
        if (room.x >= 0 && room.y >=0 && 
            room.x < width &&
            room.y < height) {
                possible.insert(b->first + room.y*width + room.x);
                return;
        }
        
        // Otherwise, it can be an edge tile of any other building
        
        for (auto&i : mBuildings) {
            i.second.GetPossibleEdgeTiles(room, c, possible);
        }
    }
#endif
    void GetPossibleTiles(const Room& room, std::set<unsigned int>& possible) const
    {
        assert(room.s != 0 && room.n != 0 && room.e != 0 && room.w != 0);

        for (const auto &b : mBuildings) {
            if (b.getWidth() >= room.e + room.w && b.getHeight() >= room.n + room.s) {
                unsigned xmin = MIM(room.w, b.getWidth() - room.w);
                for (auto i : 
                    pt.insert(b.first + x * b.getWidth() + y);
            }
        }
        
        auto b = mBuildings.upper_bound(tileIndex);
        b--;
        int width =  b->second.getWidth();
        int height =  b->second.getHeight();
        Dir c = {
            (tileIndex - b->first) % width,
            (tileIndex - b->first) / width};

        // If the next tile in the direction is within the same building, just return it
        // It is the only tile possible
        if (room.x > 0) c.x++;
        if (room.y > 0) c.y++;
        if (room.x < 0) c.x--;
        if (room.y < 0) c.y--;
        if (room.x >= 0 && room.y >=0 && 
            room.x < width &&
            room.y < height) {
                possible.insert(b->first + room.y*width + room.x);
                return;
        }
        
        // Otherwise, it can be an edge tile of any other building
        
        for (auto&i : mBuildings) {
            i.second.GetPossibleEdgeTiles(room, c, possible);
        }
    }
    // const buildings_t& mBuildings;
    std::map<unsigned int, const Building&> mBuildings;
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


static const int tileCount = 3;
Tile mTiles[tileCount] = {
    {' ',{2,2,2,2}},
    {'X',{1,1,1,1}},
    {'.',{3,3,3,3}},

};

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

GetAngle: index = 
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
        #if 0
        for (auto &c : mCoef)
        {
            c.resize(tileCount, true);
        }
        #endif

        mSumCoef.resize(width*height, tileCount);
        mTotalSum = width * height * tileCount;
        std::cout << "tileCount = " << tileCount << std::endl;
    }


    unsigned int GetTileAtIndex(Coord coord) const
    {
        unsigned int idx = (coord.y * mWidth + coord.x) * tileCount;
        int res = UINT_MAX;
        for (unsigned int i = 0; i < tileCount; i++)
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
        unsigned int minEntropy = UINT_MAX;
        static std::vector<Coord> minEntropyCoords;
        minEntropyCoords.clear();
        for (unsigned int y = 0; y < mHeight; y++)
        {
            for (unsigned int x = 0; x < mWidth; x++)
            {
                unsigned int coef = mSumCoef[y * mWidth + x];
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
        unsigned int idx = (coord.y * mWidth + coord.x)*tileCount;
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
        for (unsigned int i = 0; i < tileCount; i++)
        {
            if (mCoef[idx + i])
                potentials[potentialIndex++] = i;
        }
        assert(potentialIndex);
        int selected = potentials[rand() % potentialIndex];
        Collapse(coord, selected);
    }
    unsigned int GetPossibleTiles(const Coord& coord, int* possibleTiles)
    {
        unsigned int res = 0;
        unsigned int idx = (coord.y * mWidth + coord.x)*tileCount;
        //auto & v = mCoef[coord.y * mWidth + coord.x];
//        std::cout << "possibleTiles @ " << coord.x << "," << coord.y << ": ";


        Dir validDirs[4];
        int validDirCount = 0;
        GetValidDirs(currentCoord, validDirs, validDirCount);
        std::set<unsigned int> pt;
        Room room = getRoom(coord);
        mBuildings.GetPossibleTiles(room, pt);


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
    unsigned int GetPossibleTiles(const Coord& coord, const Dir& dir, int* possibleTiles)
    {
        unsigned int res = 0;
        unsigned int idx = (coord.y * mWidth + coord.x)*tileCount;
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
    const Room getRoom(const Coord& coord, const Coord& dir) const
    {
        return { mHeight - coord.y, coord.y, mWidth - coord.x, coord.x };
    }
    unsigned int GetPossibleTiles(const Coord& coord, const Coord& dir, 
        unsigned int* curPossibleTiles, 
        unsigned int curPossibleTileCount, unsigned int* possibleTiles) const
    {
        unsigned int res = 0;
        Room room = getRoom(coord, dir);
        // int tileIndex;
        std::set<unsigned int> pt;
        for (unsigned int i = 0; i < curPossibleTileCount; i++)
        {
            mBuildings.GetPossibleTiles(curPossibleTiles[i], room, dir, pt);
        }
        for(auto i: pt) possibleTiles[res++] = i;
        return pt.size();
    }
    void GetValidDirs(Coord coord, Dir *dest, int& coordCount)
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
        unsigned int idx = (coord.y * mWidth + coord.x)*tileCount;
        //auto & v = mCoef[coord.y * mWidth + coord.x];
        assert(mCoef[idx + tileIndex]);
        mCoef[idx + tileIndex] = 0;
        mSumCoef[coord.y * mWidth + coord.x] --;
        mTotalSum--;
    }
    void Propagate(Coord coord)
    {
        //static std::vector<Coord> coords;
        coords.clear();
        coords.push_back(coord);
        while (coords.size())
        {
            Coord currentCoord = coords.back();
            coords.pop_back();
            
            int curPossibleTiles[tileCount];
            int curPossibleTileCount = GetPossibleTiles(currentCoord, curPossibleTiles);

            Dir validDirs[4];
            int validDirCount = 0;
            GetValidDirs(currentCoord, validDirs, validDirCount);
            for (int d = 0 ; d < validDirCount ; d++)
            {
                Dir dir = validDirs[d];
                int otherPossibleTiles[tileCount];
                int otherPossibleTileCount = GetPossibleTiles(currentCoord, dir,
                        curPossibleTiles, curPossibleTileCount, otherPossibleTiles);
                Coord otherCoord = { currentCoord.x + dir.x, currentCoord.y + dir.y };
                std::cout << "@ " << otherCoord.x << "," << otherCoord.y << " " << 
                    otherPossibleTileCount << " ... ";
                for (int otherTileIndex = 0;otherTileIndex< otherPossibleTileCount;otherTileIndex++)
                {
                    unsigned int otherTile = otherPossibleTiles[otherTileIndex];
                    std::cout << otherTile << ",";
                    bool tileCompatible = false;
                    for (int curTileIndex = 0;curTileIndex< curPossibleTileCount;curTileIndex++)
                    {
                        unsigned int curTile = curPossibleTiles[curTileIndex];
                        tileCompatible |= mBuildings.TileCompatible(curTile, otherTile, dir);
                    }
                    if (!tileCompatible)
                    {
                        Constrain(otherCoord, otherTile);
                        coords.push_back(otherCoord);
                    }
                }
                    std::cout << std::endl;
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
        for (unsigned int y = 0; y < mHeight; y++)
        {
            for (unsigned int x = 0; x < mWidth; x++)
            {
                unsigned int tileIndex = GetTileAtIndex(Coord{ x,y });
                assert(tileIndex < tileCount);

                printf("%c", mBuildings.getSymbol(tileIndex));
            }
            printf("\n");
        }
    }

    const BuildingSet& mBuildings;
    const size_t tileCount;
    unsigned int mWidth, mHeight;
    std::vector<bool> mCoef;
    std::vector<unsigned short> mSumCoef;
    unsigned int mTotalSum;
    std::vector<Coord> coords;
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


/*
    int runCount = 1;
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
