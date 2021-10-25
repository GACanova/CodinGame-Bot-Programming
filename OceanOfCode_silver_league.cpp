#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <map>


using namespace std;
const int WIDTH = 15;
const int HEIGHT = 15;
const int N = WIDTH * HEIGHT;
const int MAX_DEPTH_CHECK_POSITION = 5;
const string allDirections = "ENSW";

struct Coordinate 
{
    int x; int y; 
};

inline bool operator == (const Coordinate& p1, const Coordinate& p2)
{
    if(p1.x != p2.x) 
        return false;
    if(p1.y != p2.y)
        return false; 
    return true;
}

inline bool operator<(const Coordinate& p1, const Coordinate& p2)
{
    if(p1.x != p2.x) 
    {
        return p1.x < p2.x;
    } 
    else 
    {
        return p1.y < p2.y;
    }
}

inline Coordinate operator + (const Coordinate& p1, const Coordinate& p2)
{
    Coordinate newCoordinate = {p1.x + p2.x, p2.y + p2.y};
    return newCoordinate;
}

inline Coordinate operator - (const Coordinate& p1, const Coordinate& p2)
{
    Coordinate newCoordinate = {p1.x - p2.x, p2.y - p2.y};
    return newCoordinate;
}

typedef vector<vector<bool>> ListOfMaps;

template <typename Out>
void split(const std::string &s, char delimiter, Out result)
{
    std::istringstream iss(s);
    std::string item;

    while(std::getline(iss, item, delimiter))
        *result++ = item;
}

std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> elems;
    split(s, delimiter, std::back_inserter(elems));
    return elems;
}

class Orders
{
    public:
        string order;
        Coordinate torpedo;
        Coordinate mine;
        int sector{-1};
        int movment{-1};
        int attack{-1};
        char direction;

        Orders(const string orderLine)
        {
            if(orderLine != "")
                decomposeOrders(orderLine);
        }

        void decomposeOrders(const string orders)
        {
            vector<string> ordersList = split(orders, '|');
            vector<string> movmentOrder = split(ordersList[0], ' ');
            string movmentString = movmentOrder[0];
            
            if(movmentString == "MOVE")
            {
                direction = movmentOrder[1][0];
                movment = 0;
                sector = -1;
            }
            else if(movmentString == "SILENCE")
            {   
                direction = 'X';
                movment = 1;
                sector = -1;
            }
            else if(movmentString == "SURFACE")
            {
                direction = 'X';
                sector = stoi(movmentOrder[1]);
                movment = 2;
            }
            else if(movmentString == "SONAR")
            {
                direction = 'X';
                sector = stoi(movmentOrder[1]);
                movment = 3;
            }

            vector<string> attackOrder;
            string attackString = "";

            if(ordersList.size() > 1)
            {
                attackOrder = split(ordersList[1], ' ');
                attackString = attackOrder[0];

            }

            if(attackString == "TORPEDO")
            {
                attack = 0;
                torpedo = {stoi(attackOrder[1]), stoi(attackOrder[2])};
            }
            else if(attackString == "TRIGGER")
            {
                attack = 1;
                mine = {stoi(attackOrder[1]), stoi(attackOrder[2])};
            }
            else
            {
                attack = -1;
                torpedo.x = -1;
                torpedo.y = -1;
                mine.x = -1;
                mine.y = -1;
            }
        }

        void update(string orderLine)
        {
            if(orderLine != "" && orderLine != "NA")
                decomposeOrders(orderLine);
        }
};

int manhattanDistance(const Coordinate r1, const Coordinate r2)
{
    return abs(r1.x - r2.x) + abs(r1.y - r2.y);
}

int getIndex(const Coordinate r)
{
    return (r.y  + 1) * (WIDTH + 2) + (r.x + 1);
}

Coordinate moveToDirection(const Coordinate r, const char direction)
{
    Coordinate newCoordinate = {r.x + (direction == 'E') - (direction == 'W'), r.y + (direction == 'S') - (direction == 'N')};
    return newCoordinate;
}

bool isOutOfBounds(const Coordinate r)
{
    if(r.x < 0)
        return true;
    if(r.x >= WIDTH)
        return true;
    if(r.y < 0)
        return true;
    if(r.y >= HEIGHT)
        return true;
    return false;
}

bool checkPlace(const Coordinate r, const vector<bool> &map_)
{
    return map_[getIndex(r)];
}

void changeMapState(vector<bool> &map_, const Coordinate r, const bool state)
{
    map_[getIndex(r)] = state;
}

vector<bool> resetMap(const vector<bool> &map_)
{
    vector<bool> newMap = map_;
    return newMap;
}

int getSectorIndex(const Coordinate r)
{
    if(r.x < 0 || r.y < 0)
        return -1;
    return (r.x / 5) + 3*(r.y / 5) + 1;
}

vector<Coordinate> getSectCoordinates(const int sector)
{
    vector<Coordinate> out;

    for(int y = 0; y < 5; y++)
        for(int x = 0; x < 5; x++)
            out.push_back({x + 5 * ((sector - 1) % 3), y + 5 * ((sector - 1)/3)});    

    return out;
}

vector<Coordinate> getNearByCoordinates(const vector<bool> &map_, const Coordinate r, const int minDistance, const int maxDistance)
{
    vector<Coordinate> out;

    for(int y = r.y - maxDistance; y <= r.y + maxDistance; y++)
        for(int x = r.x - maxDistance; x <= r.x + maxDistance; x++)
        {

            const Coordinate newPoint = {x, y};

            if(!isOutOfBounds(newPoint) && !checkPlace(newPoint, map_))
            {
                int distance = manhattanDistance(r, newPoint);
                
                if(distance <= maxDistance && distance >= minDistance)
                {
                    out.push_back(newPoint);
                }
            }
        }
    return out;
}

/* This function checks if the sub will run into a dead end */
bool isThereWayOut(Coordinate r, const int depth, const vector<bool> &map_, bool &flag)
{
    if(checkPlace(r, map_))
        return false;
    else if(!depth)
        return true;

    vector<bool> tmpMap = map_;
    changeMapState(tmpMap, r, true);

    for(int i = -1; i < 2; i++)
    {
        if( (r.x + i) < WIDTH && (r.x + i) >= 0 )
        {
            Coordinate newCoordinate = {r.x + i, r.y};
            flag |= isThereWayOut(newCoordinate, depth - 1, tmpMap, flag);
        }

        if( (r.y + i) < HEIGHT && (r.y + i) >= 0 )
        {
            Coordinate newCoordinate = {r.x, r.y + i};
            flag |= isThereWayOut(newCoordinate, depth - 1, tmpMap, flag);
        }
    }
    return flag;
}

bool isDirectionPossible(const Coordinate r, const char direction, const vector<bool> &map_)
{
    Coordinate newCoordinate = moveToDirection(r, direction);

    if(checkPlace(newCoordinate, map_))
        return false;
    
    bool flag = false;
    if(!isThereWayOut(newCoordinate, MAX_DEPTH_CHECK_POSITION, map_, flag))
        return false;

    return true;
}

map<Coordinate, ListOfMaps> allStartingPossibilities(const vector<bool> &map_)
{
    map<Coordinate, ListOfMaps> out;

    for(int x = 0; x < WIDTH; x++)
        for(int y = 0; y < HEIGHT; y++)
        {
            Coordinate r = {x, y};
            vector<bool> copy_map = map_;

            if(!checkPlace(r, map_))
                out[r].push_back(copy_map); 
        }
    return out;     
}

map<Coordinate, ListOfMaps> resetAllMaps(map<Coordinate, ListOfMaps> &possibilities, const vector<bool> &map_)
{
    map<Coordinate, ListOfMaps> out;

    for(std::map<Coordinate, ListOfMaps>::iterator iter = possibilities.begin(); iter != possibilities.end(); ++iter)
    {
        Coordinate r =  iter->first;
        out[r].push_back(map_);
    }
    return out;
}

map<Coordinate, ListOfMaps> findMatchingPattern(map<Coordinate, ListOfMaps> &possibilities, const vector<bool> &defaultMap, const char direction)
{
    map<Coordinate, ListOfMaps> out;

    for(std::map<Coordinate, ListOfMaps>::iterator iter = possibilities.begin(); iter != possibilities.end(); ++iter)
    {
        const Coordinate r =  iter->first;
        const ListOfMaps list = iter->second;
        const Coordinate path = moveToDirection(r, direction);

        if(!isOutOfBounds(path) && !checkPlace(path, defaultMap))
        {
            for(auto &map_ : list)
            {
                if(!checkPlace(path, map_))
                {
                    vector<bool> newMap = map_;
                    changeMapState(newMap, path, true);
                    out[path].push_back(newMap);
                }
            }
        }
    }
    return out;
}

map<Coordinate, ListOfMaps> addNewPossibilities(map<Coordinate, ListOfMaps> &possibilities, const vector<bool> &defaultMap, const char direction)
{
    map<Coordinate, ListOfMaps> out;

    for(std::map<Coordinate, ListOfMaps>::iterator iter = possibilities.begin(); iter != possibilities.end(); ++iter)
    {
        const Coordinate r = iter->first;
        const ListOfMaps list = iter->second;

        for(auto &d : allDirections)
        {
            Coordinate path = r;

            for(int i = 0; i < 4; i++)
            {
                path = moveToDirection(path, d);

                if(!isOutOfBounds(path) && !checkPlace(path, defaultMap))
                {
                    for(auto &map_ : list)
                    {
                        if(!checkPlace(path, map_))
                        {
                            vector<bool> newMap = map_;
                            changeMapState(newMap, path, true);
                            out[path].push_back(newMap);
                        }
                    }
                }
            }
        }
    }
    return out;
}

map<Coordinate, ListOfMaps> refinePossibilitiesBySector(map<Coordinate, ListOfMaps> &possibilities, const int sector)
{
    map<Coordinate, ListOfMaps> out;
    const auto sectorCoordinates = getSectCoordinates(sector);

    for(auto &xy : sectorCoordinates)
    {
        if (possibilities.count(xy))
        {
            auto tmp = possibilities.extract(xy);
            tmp.key() = xy;
            out.insert(std::move(tmp));
        }
    }
    return out;
}

map<Coordinate, ListOfMaps> refinePossibilitiesByDistance(map<Coordinate, ListOfMaps> &possibilities, const vector<bool> &defaultMap, const Coordinate r)
{
    map<Coordinate, ListOfMaps> out;
    const auto nearByCoordinates = getNearByCoordinates(defaultMap, r, 0, 4);

    for(auto &xy : nearByCoordinates)
    {
        if (possibilities.count(xy))
        {
            auto tmp = possibilities.extract(xy);
            tmp.key() = xy;
            out.insert(std::move(tmp));
        }
    }
    return out;
}

vector<Coordinate> getFirstNeighbors(const Coordinate r, bool withDiagonals)
{
    vector<Coordinate> out;
    Coordinate neigh;

    for(int i = -1; i < 2; i += 2)
    {
        neigh.x = r.x + i;
        neigh.y = r.y;
        out.push_back(neigh);

        neigh.x = r.x;
        neigh.y = r.y + i;
        out.push_back(neigh);

        if(withDiagonals)
        {
            neigh.x = r.x + i;
            neigh.y = r.y + i;
            out.push_back(neigh);

            neigh.x = r.x + i;
            neigh.y = r.y - i;
            out.push_back(neigh);
        }
    }
    return out;
}

map<Coordinate, ListOfMaps> refinePossibilitiesByCoordinates(map<Coordinate, ListOfMaps> &possibilities, const vector<bool> &defaultMap, const vector<Coordinate> &coordinates)
{
    map<Coordinate, ListOfMaps> out;

    for(auto &xy : coordinates)
    {
        if(!isOutOfBounds(xy) && !checkPlace(xy, defaultMap))
        {
            if (possibilities.count(xy))
            {
                auto tmp = possibilities.extract(xy);
                tmp.key() = xy;
                out.insert(std::move(tmp));
            }
        }
    }
    return out;
}

void calculatePossibilities(map<Coordinate, ListOfMaps> &possibilities, const vector<bool> &defaultMap, const Orders &orders)
{
        switch(orders.movment)
        {
            //MOVE
            case 0:
                possibilities = findMatchingPattern(possibilities, defaultMap, orders.direction);
            break;

            //SILENCE
            case 1:
                possibilities = addNewPossibilities(possibilities, defaultMap, orders.direction);
            break;

            //SURFACE
            case 2:
                possibilities = resetAllMaps(possibilities, defaultMap);
                possibilities = refinePossibilitiesBySector(possibilities, orders.sector);
            break;

            //SONAR
            case 3:
                //possibilities = refinePossibilitiesBySector(possibilities, orders.sector);
            break;
        }

        switch(orders.attack)
        {   
            //TORPEDO
            case 0:
                possibilities = refinePossibilitiesByDistance(possibilities, defaultMap, orders.torpedo);
            break;
        }
}

vector<bool> buildInitialMap(const vector<string> lines)
{
    vector<bool> border(WIDTH + 2, true);
    vector<bool> row;
    row.insert( row.begin(), border.begin(), border.end() );

    for(auto &line : lines)
    {
        row.push_back(true);

        for(auto &c : line)
        {
            row.push_back(c == 'x');
        }

        row.push_back(true);
    }
    row.insert( row.end(), border.begin(), border.end() );
    return row;
}

vector<Coordinate> viableTorpedoCoordinates(const vector<bool> &defaultMap, const Coordinate r)
{
    vector<Coordinate> firePossibilities;
    const auto coordinatesInRange = getNearByCoordinates(defaultMap, r, 1, 4);
    const auto firstNeighbors = getFirstNeighbors(r, true);

    for(auto &x : coordinatesInRange)
    {
        auto it = std::find(firstNeighbors.begin(), firstNeighbors.end(), x);

        if(it == firstNeighbors.end())
            firePossibilities.push_back(x);
    }
    return firePossibilities;
}

Coordinate torpedoAttack(map<Coordinate, ListOfMaps> &possibilities, const vector<bool> &defaultMap, const Coordinate r, const int maxPossibilities)
{
    auto coordinates = viableTorpedoCoordinates(defaultMap, r);
    vector<Coordinate> out;

    for(auto &xy : coordinates)
    {
        if (possibilities.count(xy))
            out.push_back(xy);
    }

    const int nPossibilities = out.size();
    Coordinate fireCoordinate = {-1, -1};

    if(nPossibilities && nPossibilities <= maxPossibilities)
    {
        fireCoordinate = out[rand() % nPossibilities];
        return fireCoordinate;
    }
    return fireCoordinate;
}
    
int moveInSilence(vector<bool> &map_, const Coordinate r, const char direction ,const int maxNumberOfSteps)
{
    int n = 0;
    Coordinate path = r;

    for(n = 0; n < maxNumberOfSteps; n++)
    {
        path = moveToDirection(path, direction);

        bool flag = false;
        if(isOutOfBounds(path) || checkPlace(path, map_) || !isThereWayOut(path, MAX_DEPTH_CHECK_POSITION, map_, flag))
        {
            break;
        }
        else
        {
            changeMapState(map_, path, true);
        }
    }
    return n;
}

bool isThereMineAround(const vector<Coordinate> mineCoordinates, const Coordinate r)
{
    auto firstNeighbors = getFirstNeighbors(r, true);
    firstNeighbors.push_back(r);

    for(auto &m : mineCoordinates)
        for(auto &n : firstNeighbors)
        {
            if(m == n)
                return true;
        }
    return false;
}

bool canPlaceMine(const vector<bool> &map_, const vector<Coordinate> &mineCoordinates, const Coordinate r, const char direction, Coordinate &mineCoordinate, char &mineDirection)
{
    string directions = "EWNS";

    for(auto &d : directions)
    {
        Coordinate newCoordinate = moveToDirection(r, d);
        Coordinate mine;

        if(!isThereMineAround(mineCoordinates, newCoordinate) && !checkPlace(newCoordinate, map_))
        {
            mineCoordinate = newCoordinate;
            mineDirection = d;
            return true;
        }
    }
    return false;
}

bool canTriggerMine(map<Coordinate, ListOfMaps> possibilities, const vector<Coordinate> &mineCoordinates, Coordinate &mineCoordinate, int maxPossibilities)
{
    vector<Coordinate> allPossibleMines;
    vector<Coordinate> inRangePossibilities;

    for(auto &mine : mineCoordinates)
    {
        auto inRangePossibilities = getFirstNeighbors(mine, true);
        inRangePossibilities.push_back(mine);

        for(auto &xy : inRangePossibilities)
        {
            if(possibilities.count(xy))
            {
                allPossibleMines.push_back(mine);
            }

        }
    }

    const int nPossibilities = allPossibleMines.size();

    if(nPossibilities && nPossibilities <= maxPossibilities)
    {
        mineCoordinate = allPossibleMines[rand() % nPossibilities];
        return true;
    }
    return false;
}

Coordinate initialPosition(const vector<bool> &map_, string mode)
{
    Coordinate r;

    if(mode == "CENTER")
    {
        //If there is an island at the center, iterate till finding a viable coordinate as close as possible to the center
        for(int i = 0; i < WIDTH; i++)
        {
            for(int j = -1; j < 2; j+=2)
            {
                r.x = i * j + WIDTH/2;
                r.y = HEIGHT/2 - 1;

                cerr << "INIT " << r.x << " " << r.y << endl;

                bool flag = false;
                if(!checkPlace(r, map_) && isThereWayOut(r, MAX_DEPTH_CHECK_POSITION, map_, flag))
                    return r;
                
                r.x = WIDTH/2;
                r.y = i * j + HEIGHT/2;

                flag = false;
                if(!checkPlace(r, map_) && isThereWayOut(r, MAX_DEPTH_CHECK_POSITION, map_, flag))
                    return r;
            }
        }
    }
    return r;
}

bool navigationSystem(const vector<bool> &map_, char &direction, const Coordinate &r, int movmentMode)
{
    bool isTherePossibleDirection = false;
    
    if(!movmentMode)
    {
        for(auto &d : allDirections)
        {
            if(isDirectionPossible(r, d, map_))
            {
                direction = d;
                isTherePossibleDirection = true;
                break;
            }
        }
    }
    return isTherePossibleDirection;
} 

int main()
{
    srand(42);
    int width_;
    int height_;
    int myId;
    cin >> width_ >> height_ >> myId; cin.ignore();

    vector<string> lines;
    for (int i = 0; i < HEIGHT; i++)
    {
        string line;
        getline(cin, line);
        lines.push_back(line);
    }

    bool opponentPositionExists = false;
    bool escapeMode = false;

    const vector<bool> defaultMap = buildInitialMap(lines);
    vector<bool> myMap = defaultMap;    
    char direction;
    string myPrevOrder;
    vector<Coordinate> mineCoordinates;

    Orders myPrevOrders("");
    map<Coordinate, ListOfMaps> myPossibilities = allStartingPossibilities(defaultMap);
    int myPrevLife = 6;

    Orders opponentOrders("");
    map<Coordinate, ListOfMaps> opponenetPossibilities = allStartingPossibilities(defaultMap);
    int oppPrevLife = 6;

    auto startingPosition = initialPosition(defaultMap, "CENTER");

    cout << startingPosition.x << " " << startingPosition.y << endl;

    // game loop
    while(true) 
    {
        int x;
        int y;
        int myLife;
        int oppLife;
        int torpedoCooldown;
        int sonarCooldown;
        int silenceCooldown;
        int mineCooldown;
        cin >> x >> y >> myLife >> oppLife >> torpedoCooldown >> sonarCooldown >> silenceCooldown >> mineCooldown; cin.ignore();
        string sonarResult;
        cin >> sonarResult; cin.ignore();
        string opponentOrdersLine;
        getline(cin, opponentOrdersLine);

        const Coordinate r = {x, y};
        changeMapState(myMap, r, true);

        opponentOrders.update(opponentOrdersLine);
        calculatePossibilities(opponenetPossibilities, defaultMap, opponentOrders);

        //TORPEDO HIT
        if(myPrevOrders.attack == 0 && opponentOrders.movment != 2 && opponentOrders.direction != 'X')
        {
            int oppLifeDifference = oppPrevLife - oppLife;

            if(oppLifeDifference == 2)
            {
                vector<Coordinate> v;
                v.push_back(moveToDirection(myPrevOrders.torpedo, opponentOrders.direction));
                opponenetPossibilities = refinePossibilitiesByCoordinates(opponenetPossibilities, defaultMap, v);
            }
            else if(oppLifeDifference == 1)
            {
                const auto firstNeighborsCoordinates = getFirstNeighbors(myPrevOrders.torpedo, true);
                vector<Coordinate> movedFirstNeighbors;

                for(auto &xy : firstNeighborsCoordinates)
                {
                    movedFirstNeighbors.push_back(moveToDirection(xy, opponentOrders.direction));
                }
                
                opponenetPossibilities = refinePossibilitiesByCoordinates(opponenetPossibilities, defaultMap, movedFirstNeighbors);
            }
        }

        calculatePossibilities(myPossibilities, defaultMap, myPrevOrders);

/*********************** Direction **********************************************************************/
        char direction;
        bool isTherePossibleDirection = navigationSystem(myMap, direction, r, 0);
        const Coordinate myNextCoordinate = moveToDirection(r, direction);

/****************** Torpedo Attack ***************************************************************************/
    Coordinate torpedoCoordinate = {-1, -1};

    if(opponenetPossibilities.size() == 1)
    {
        torpedoCoordinate = torpedoAttack(opponenetPossibilities, defaultMap, myNextCoordinate, 1);
    }
    const bool fireTorpedo = (torpedoCoordinate.x != -1 && torpedoCoordinate.y != -1 && !torpedoCooldown);

/****************** Mine Attack ***************************************************************************/
    //if(!mineCooldown)
    char mineDirection;
    Coordinate mineCoordinate;
    const bool placeMine = canPlaceMine(defaultMap, mineCoordinates, myNextCoordinate, direction, mineCoordinate, mineDirection);
    const bool triggerMine = canTriggerMine(opponenetPossibilities, mineCoordinates, mineCoordinate, 225);


/****************** CharginMode ***************************************************************************/

        string chargingMode = "";

        if(torpedoCooldown)
            chargingMode = "TORPEDO";
        else if(silenceCooldown)
            chargingMode = "SILENCE";
        else
            chargingMode = "MINE";

/****************** My Order ******************************************************************************/

        int myNumberOfPossibilities = myPossibilities.size();

        if((myNumberOfPossibilities && myNumberOfPossibilities <= 3) || (!opponentOrders.attack && myPrevLife != myLife && myPrevOrders.movment != 2))
            escapeMode = true;

        stringstream myOrder;

        if(isTherePossibleDirection)
        {
            if(escapeMode && !silenceCooldown)
            {
                myOrder << "SILENCE " << direction << " " << to_string(moveInSilence(myMap, r, direction, rand() % 4));
                escapeMode = false;
            }
            else
            {
                myOrder << "MOVE " << direction << " " << chargingMode;

                if(fireTorpedo)
                {
                    myOrder << "|TORPEDO " << to_string(torpedoCoordinate.x) << " " << to_string(torpedoCoordinate.y);
                }
                
                if(triggerMine && !mineCooldown)
                {
                    myOrder << "|TRIGGER " << to_string(mineCoordinate.x) << " " << to_string(mineCoordinate.y);
                    mineCoordinates.erase(std::remove(mineCoordinates.begin(), mineCoordinates.end(), mineCoordinate), mineCoordinates.end());
                }
                else if(placeMine && !mineCooldown)
                {
                    myOrder << "|MINE " << mineDirection;
                    mineCoordinates.push_back(mineCoordinate);
                }
            }
        }   
        else
        {
            myOrder << "SURFACE";
            myMap = resetMap(defaultMap);
        }

        cout << myOrder.str() << endl;

        myPrevOrder = myOrder.str();

        if(myPrevOrder == "SURFACE")
            myPrevOrder += ' ' + to_string(getSectorIndex(r));

        myPrevOrders.update(myPrevOrder);

        myPrevLife = myLife;
        oppPrevLife = oppLife;
    }
}
