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
vector<bool> defaultMap;

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
    Coordinate newCoordinate = {p1.x + p2.x, p1.y + p2.y};
    return newCoordinate;
}

inline Coordinate operator - (const Coordinate& p1, const Coordinate& p2)
{
    Coordinate newCoordinate = {p1.x - p2.x, p1.y - p2.y};
    return newCoordinate;
}

inline ostream &operator << (ostream& os, const Coordinate& p1)
{
    os << p1.x << " " << p1.y;
    return os;
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
        Coordinate r;
        int sector;
        char direction;

        Orders() : order(""), r({-1, -1}), sector(-1), direction('X'){}

        void updateData(string orderString)
        {
            if(orderString != "")
            {
                vector<string> orderComponents = split(orderString, ' ');

                if(orderComponents[0] != "MSG")
                {
                    order = orderComponents[0];

                    if(order == "MOVE" || order == "MINE")
                        direction = orderComponents[1][0];

                    if(order == "TORPEDO" || order == "TRIGGER")
                        r = {stoi(orderComponents[1]), stoi(orderComponents[2])};

                    if(order == "SURFACE" || order == "SONAR")
                        sector = stoi(orderComponents[1]);

                }
            }
        }
};

vector<Orders> decomposeOrders(string stringLine)
{
    const auto orderList = split(stringLine, '|');
    vector<Orders> out;

    for(auto &order : orderList)
    {   
        Orders tmp;
        tmp.updateData(order);
        out.push_back(tmp);
    }
    return out;
}

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

bool isViableCoordinate(const Coordinate r)
{
    if(isOutOfBounds(r))
        return false;
    if(checkPlace(r, defaultMap))
        return false;
    return true;
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
        {
            const Coordinate r = {x + 5 * ((sector - 1) % 3), y + 5 * ((sector - 1)/3)};

            if(isViableCoordinate(r))
                out.push_back(r);    
        }
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
                    out.push_back(newPoint);
                
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

bool checkTrajectory(const Coordinate r, const Coordinate target, const int steps, bool &flag)
{
    if(r == target)
        return true;
    else if(steps == 0 || !isViableCoordinate(r))
        return false;

    Coordinate newCoordinate;

    for(int i = -1; i < 2; i++)
    {
        newCoordinate = {r.x + i, r.y};
        flag |= checkTrajectory(newCoordinate, target, steps - 1, flag);

        newCoordinate = {r.x, r.y + i};
        flag |= checkTrajectory(newCoordinate, target, steps - 1, flag);
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


vector<Coordinate> getAllMapKeys(map<Coordinate, ListOfMaps> &possibilities)
{
    vector<Coordinate> out;

    for(map<Coordinate, ListOfMaps>::iterator iter = possibilities.begin(); iter != possibilities.end(); ++iter)
        out.push_back(iter->first);

    return out;
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

map<Coordinate, ListOfMaps> resetAllMaps(map<Coordinate, ListOfMaps> &possibilities)
{
    map<Coordinate, ListOfMaps> out;

    for(std::map<Coordinate, ListOfMaps>::iterator iter = possibilities.begin(); iter != possibilities.end(); ++iter)
    {
        Coordinate r =  iter->first;
        out[r].push_back(defaultMap);
    }
    return out;
}

map<Coordinate, ListOfMaps> refinePossibilitiesByCoordinates(map<Coordinate, ListOfMaps> &possibilities, const vector<Coordinate> &coordinates)
{
    map<Coordinate, ListOfMaps> out;

    for(auto &xy : coordinates)
    {
        if(possibilities.count(xy))
        {
            auto tmp = possibilities.extract(xy);
            tmp.key() = xy;
            out.insert(std::move(tmp));
        }
    }
    return out;
}

void removeCoordinatesFromPossibilities(map<Coordinate, ListOfMaps> &possibilities, const vector<Coordinate> &coordinates)
{
    for(auto &xy : coordinates)
        if(possibilities.count(xy))
            possibilities.erase(possibilities.find(xy));
}

map<Coordinate, ListOfMaps> findMatchingPattern(map<Coordinate, ListOfMaps> &possibilities, const char direction)
{
    map<Coordinate, ListOfMaps> out;

    for(std::map<Coordinate, ListOfMaps>::iterator iter = possibilities.begin(); iter != possibilities.end(); ++iter)
    {
        const Coordinate r =  iter->first;
        const ListOfMaps list = iter->second;
        const Coordinate path = moveToDirection(r, direction);

        if(isViableCoordinate(r))
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

map<Coordinate, ListOfMaps> addNewPossibilities(map<Coordinate, ListOfMaps> &possibilities, const char direction)
{
    map<Coordinate, ListOfMaps> out = possibilities;

    for(std::map<Coordinate, ListOfMaps>::iterator iter = possibilities.begin(); iter != possibilities.end(); ++iter)
    {
        const Coordinate r = iter->first;
        const ListOfMaps list = iter->second;
        vector<bool> flag(list.size(), true);

        for(auto &d : allDirections)
        {
            Coordinate path = r;

            for(int i = 0; i < 4; i++)
            {
                path = moveToDirection(path, d);

                if(isViableCoordinate(path))
                {
                    int i = 0;
                    for(auto &map_ : list)
                    {
                        if(!checkPlace(path, map_) && flag[i])
                        {
                            vector<bool> newMap = map_;
                            changeMapState(newMap, path, true);
                            out[path].push_back(newMap);
                        }
                        else
                        {
                            flag[i] = false;
                        }
                        i++;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
    return out;
}

map<Coordinate, ListOfMaps> refinePossibilitiesByTorpedoFired(map<Coordinate, ListOfMaps> &possibilities, const vector<bool> &defaultMap, const Coordinate r)
{
    const auto nearByCoordinates = getNearByCoordinates(defaultMap, r, 0, 4);
    vector<Coordinate> coordinates;

    for(auto &xy : nearByCoordinates)
    {
        bool flag = false;

        if(checkTrajectory(xy, r, 4, flag))
            coordinates.push_back(xy);
    }

    return refinePossibilitiesByCoordinates(possibilities, coordinates);
}

vector<Coordinate> getFirstNeighbors(const Coordinate r, bool withDiagonals)
{
    vector<Coordinate> neighbors, out;
    Coordinate neigh;

    for(int i = -1; i < 2; i += 2)
    {
        neigh.x = r.x + i;
        neigh.y = r.y;
        neighbors.push_back(neigh);

        neigh.x = r.x;
        neigh.y = r.y + i;
        neighbors.push_back(neigh);

        if(withDiagonals)
        {
            neigh.x = r.x + i;
            neigh.y = r.y + i;
            neighbors.push_back(neigh);

            neigh.x = r.x + i;
            neigh.y = r.y - i;
            neighbors.push_back(neigh);
        }
    }

    for(auto &xy : neighbors)
    {
        if(isViableCoordinate(xy))
            out.push_back(xy);
    }
    return out;
}

int getNumberOfPossibilities(const vector<bool> &map_, const Coordinate r)
{
    int counter = 0;

    for(auto &d : allDirections)
    {
        Coordinate path = r;

        for(int i = 0; i < 4; i++)
        {
            path = moveToDirection(path, d);

            if(isViableCoordinate(path) && !checkPlace(path, map_))
            {
                counter++;
            }
            else
            {
                break;
            }
        }
    }
    return counter;
}

int checkForEqualMaps(const ListOfMaps &maps)
{
    int counter = 0;

    for(int j = 0; j < maps.size() - 1; j++)
        for(int i = j + 1; i < maps.size(); i++)
            counter += (maps[i] == maps[j]);

    return counter;
}

void calculatePossibilities(map<Coordinate, ListOfMaps> &possibilities, const vector<bool> &defaultMap, const Orders &orders)
{
        if(orders.order == "MOVE")
        {
            possibilities = findMatchingPattern(possibilities, orders.direction);
        }
        else if(orders.order == "SILENCE")
        {
            possibilities = addNewPossibilities(possibilities, orders.direction);
        }
        else if(orders.order == "TORPEDO")
        {
            possibilities = refinePossibilitiesByTorpedoFired(possibilities, defaultMap, orders.r);
        }
        else if(orders.order == "SURFACE")
        {
            possibilities = resetAllMaps(possibilities);
            possibilities = refinePossibilitiesByCoordinates(possibilities, getSectCoordinates(orders.sector));
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


Coordinate maxCoordinatesDensity(const vector<Coordinate> &coordinates)
{
    int max = -1;
    Coordinate max_coordinate;

    for(auto &xy : coordinates)
    {
        const auto neighbors = getFirstNeighbors(xy, true);
        int counter = 0;

        for(auto &n : neighbors)
        {
            auto it = std::find(coordinates.begin(), coordinates.end(), n);

            if(it != coordinates.end())
                counter++;
        }

        if(counter >= max)
        {
            max = counter;
            max_coordinate = xy;        
        }
        return max_coordinate;
    }
}

vector<Coordinate> viableTorpedoCoordinates(const Coordinate r)
{
    vector<Coordinate> firePossibilities, out;
    const auto coordinatesInRange = getNearByCoordinates(defaultMap, r, 1, 4);
    const auto firstNeighbors = getFirstNeighbors(r, true);

    for(auto &x : coordinatesInRange)
    {
        auto it = std::find(firstNeighbors.begin(), firstNeighbors.end(), x);

        if(it == firstNeighbors.end())
            firePossibilities.push_back(x);
    }

    for(auto &xy : firePossibilities)
    {
        bool flag = false;
        if(checkTrajectory(r, xy, 4, flag))
            out.push_back(xy);
    }

    return out;
}

Coordinate torpedoAttack(map<Coordinate, ListOfMaps> &possibilities, const Coordinate r, const int maxPossibilities)
{
    auto coordinates = viableTorpedoCoordinates(r);
    vector<Coordinate> out;

    for(auto &xy : coordinates)
    {
        if (possibilities.count(xy))
            out.push_back(xy);
    }

    const int nPossibilities = out.size();
    Coordinate fireCoordinate = {-1, -1};

    if(nPossibilities > 2)
    {
        fireCoordinate = maxCoordinatesDensity(out);
    }
    else
    {
        if(nPossibilities && nPossibilities <= maxPossibilities)
        {
            fireCoordinate = out[rand() % nPossibilities];
        }
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

bool canPlaceMine(const vector<Coordinate> &mineCoordinates, const Coordinate r, const char direction, Coordinate &mineCoordinate, char &mineDirection)
{
    string directions = "EWNS";

    for(auto &d : directions)
    {
        Coordinate newCoordinate = moveToDirection(r, d);

        if(!isThereMineAround(mineCoordinates, newCoordinate) && isViableCoordinate(newCoordinate))
        {
            mineCoordinate = newCoordinate;
            mineDirection = d;
            return true;
        }
    }
    return false;
}

bool canTriggerMine(map<Coordinate, ListOfMaps> possibilities, const vector<Coordinate> &mineCoordinates, Coordinate &mineCoordinate, Coordinate myCoordinate, int maxPossibilities)
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
                auto it = std::find(inRangePossibilities.begin(), inRangePossibilities.end(), myCoordinate);

                if(it == inRangePossibilities.end())
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
    else if(mode == "RANDOM")
    {
        bool flag = false;

        do
        {
            r.x = rand() % WIDTH;
            r.y = rand() % HEIGHT;
            flag = false;
        
        }
        while(checkPlace(r, map_) || !isThereWayOut(r, MAX_DEPTH_CHECK_POSITION, map_, flag));
    }

    return r;
}

bool navigationSystem(const vector<bool> &map_, char &direction, const Coordinate &r, string movmentMode)
{
    if(movmentMode == "FILL")
    {
        std::vector<std::pair<int , char>> nn;

        for(auto &d : allDirections)
        {
            Coordinate newCoordinate = moveToDirection(r, d);
            int counter = 0;

            if(!checkPlace(newCoordinate, map_))
            {
                for(auto &d2 : allDirections)
                {
                    Coordinate neigh = moveToDirection(newCoordinate, d2);
                    counter += (checkPlace(neigh, map_));
                }

                pair<int , char> p = {counter, d};
                nn.push_back(p);
            }
            
        }

        if(!nn.size())
            return false;

        std::sort(nn.begin(), nn.end(), std::greater<pair<int, char>>());

        for(auto &x : nn)
            cerr << x.second << " " << x.first << endl;

        for(auto &x : nn)
        {
            Coordinate newCoordinate = moveToDirection(r, x.second);

            bool flag = false;
            if(!checkPlace(newCoordinate, map_) && isThereWayOut(newCoordinate, MAX_DEPTH_CHECK_POSITION, map_, flag))
            {
                direction = x.second;
                return true;
            }
        }

    }

    if(movmentMode == "PATTERN")
    {
        for(auto &d : allDirections)
        {
            if(isDirectionPossible(r, d, map_))
            {
                direction = d;
                return true;
            }
        }
    }
    
    return false;
} 

void filterPossibilitiesByHit(map<Coordinate, ListOfMaps> possibilities, const vector<bool> &defaultMap, const int oppLifeDifference, const Coordinate r, char direction)
{
    if(oppLifeDifference == 2)
    {
        vector<Coordinate> v;
        v.push_back(moveToDirection(r, direction));
        possibilities = refinePossibilitiesByCoordinates(possibilities, v);
    }
    else if(oppLifeDifference == 1)
    {
        const auto firstNeighborsCoordinates = getFirstNeighbors(r, true);
        vector<Coordinate> movedFirstNeighbors;

        for(auto &xy : firstNeighborsCoordinates)
        {
            movedFirstNeighbors.push_back(moveToDirection(xy, direction));

        }
        
        possibilities = refinePossibilitiesByCoordinates(possibilities, movedFirstNeighbors);
    }

    if(!oppLifeDifference)
    {
        auto firstNeighborsCoordinates = getFirstNeighbors(r, true);
        firstNeighborsCoordinates.push_back(r);
        removeCoordinatesFromPossibilities(possibilities, firstNeighborsCoordinates);
    }
}

int main()
{
    srand(42);
    //srand(time(NULL));
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
    bool canFilterByHit = true;

    defaultMap = buildInitialMap(lines);
    vector<bool> myMap = defaultMap;    
    char direction;
    string myPrevOrder;
    string myPrevOrderLine;
    vector<Coordinate> mineCoordinates;

    //Orders myPrevOrders("");
    map<Coordinate, ListOfMaps> myPossibilities = allStartingPossibilities(defaultMap);
    int myPrevLife = 6;

    //Orders opponentOrders("");
    map<Coordinate, ListOfMaps> oppPossibilities = allStartingPossibilities(defaultMap);
    int oppPrevLife = 6;
    char oppDirection = 'X';

    auto startingPosition = initialPosition(defaultMap, "RANDOM"); //"CENTER"

    int step_counter = 0;

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

        cerr << "ATÉ 1" << endl;

        const Coordinate r = {x, y};
        changeMapState(myMap, r, true);

        auto oppOrders = decomposeOrders(opponentOrdersLine);

        cerr << "ATÉ 1.5" << endl;


        auto myPrevOrders = decomposeOrders(myPrevOrderLine);

        cerr << "ATÉ 2" << endl;

        for(auto &oppOrder : oppOrders)
        {
            cerr << oppOrder.order << " direction = " << oppOrder.direction << " " << oppOrder.r <<  endl;
            calculatePossibilities(oppPossibilities, defaultMap, oppOrder);
            canFilterByHit &= ((oppOrder.order != "SILENCE") && (oppOrder.order != "SURFACE"));
            
            if(oppOrder.order == "MOVE")
                oppDirection = oppOrder.direction;
        }

        cerr << "ATÉ 9" << endl;

        if(canFilterByHit)
        {
            for(auto &myOrder : myPrevOrders)
            {

                if(myOrder.order == "TORPEDO" || myOrder.order == "TRIGGER")
                {
                    int oppLifeDifference = oppPrevLife - oppLife;

                    filterPossibilitiesByHit(oppPossibilities, defaultMap, oppLifeDifference, myOrder.r, direction);
                }
            }
        }

        for(auto &myOrder : myPrevOrders)
        {                
            calculatePossibilities(myPossibilities, defaultMap, myOrder);

        }
        cerr << "ATÉ 10" << endl;
        
        canFilterByHit = true;
       
        bool activateSonar = false;
        int sectorSonar;

        if(!sonarCooldown && oppPossibilities.size() > 5)
        {
            activateSonar = true;
            auto coordinates = getAllMapKeys(oppPossibilities);
            sectorSonar = getSectorIndex(maxCoordinatesDensity(coordinates));
        }

        if(sonarResult != "NA")
        {
            auto coordinates = getSectCoordinates(sectorSonar);
            vector<Coordinate> movedCoordinates;

            for(auto &xy : coordinates)
                movedCoordinates.push_back(moveToDirection(xy, oppDirection));

            if(sonarResult == "Y")
                oppPossibilities = refinePossibilitiesByCoordinates(oppPossibilities, movedCoordinates);
            else if(sonarResult == "N")
                removeCoordinatesFromPossibilities(oppPossibilities, movedCoordinates);
        }

        int oppTotalMaps = 0;
        for(std::map<Coordinate, ListOfMaps>::iterator iter = oppPossibilities.begin(); iter != oppPossibilities.end(); ++iter)
        {
            Coordinate r =  iter->first;
            ListOfMaps list = iter->second;
            oppTotalMaps += list.size();

            if(list.size() > 100)
                cerr << "EQUAL MAPS-------------> " << checkForEqualMaps(list) << endl;


            cerr << "("  << r.x << "," << r.y << ")  maps= " << list.size() << endl; 
        }

        int myTotalMaps = 0;
        for(std::map<Coordinate, ListOfMaps>::iterator iter = myPossibilities.begin(); iter != myPossibilities.end(); ++iter)
        {
            Coordinate r =  iter->first;
            ListOfMaps list = iter->second;
            myTotalMaps += list.size();
        }

        cerr << "oppTotalMaps=" << oppTotalMaps << endl;
        cerr << "myTotalMaps=" << myTotalMaps << endl;



        if(oppTotalMaps > 1000000)
        {
            oppPossibilities = resetAllMaps(oppPossibilities);
        }

        if(myTotalMaps > 1024)
            myPossibilities = resetAllMaps(myPossibilities);


/*********************** Direction **********************************************************************/
        char direction;
        bool isTherePossibleDirection = navigationSystem(myMap, direction, r, "FILL");
        const Coordinate myNextCoordinate = moveToDirection(r, direction);

/****************** Torpedo Attack ***************************************************************************/
    Coordinate torpedoCoordinate = {-1, -1};

    if(!torpedoCooldown && step_counter > 20)
    {
        if(oppPossibilities.size() <= 10)
        {
            torpedoCoordinate = torpedoAttack(oppPossibilities, myNextCoordinate, 1000);
        }
    }
    const bool fireTorpedo = (torpedoCoordinate.x != -1 && torpedoCoordinate.y != -1 && !torpedoCooldown);

/****************** Mine Attack ***************************************************************************/
    char mineDirection;
    bool triggerMine = false;
    bool placeMine = false;
    Coordinate mineCoordinate;

    if(!mineCooldown)
    {
        placeMine = canPlaceMine(mineCoordinates, myNextCoordinate, direction, mineCoordinate, mineDirection);

        if(oppPossibilities.size() <= 2)
            triggerMine = canTriggerMine(oppPossibilities, mineCoordinates, mineCoordinate, r, 225);
    }

/****************** CharginMode ***************************************************************************/

        string chargingMode = "";

        if(torpedoCooldown)
            chargingMode = "TORPEDO";
        else if(silenceCooldown)
            chargingMode = "SILENCE";
        else if(mineCooldown)
            chargingMode = "MINE";
        else
            chargingMode = "SONAR";

/****************** My Order ******************************************************************************/

        int myNumberOfPossibilities = myPossibilities.size();

        cerr << "MY NUMBER OF POSS " << myNumberOfPossibilities << endl;

        cerr << "GET NUMBER OF POSS " << getNumberOfPossibilities(myMap, r) << endl;


        if(silenceCooldown && myNumberOfPossibilities < 20)
            if(myNumberOfPossibilities < 5 || getNumberOfPossibilities(myMap, r) >= 10)
                escapeMode = true;


        stringstream myOrder;

        if(isTherePossibleDirection)
        {
            if(escapeMode && !silenceCooldown)
            {
                myOrder << "SILENCE " << direction << " " << to_string(moveInSilence(myMap, r, direction, 0)); //rand() % 4)
                escapeMode = false;
            }
            else
            {
                myOrder << "|MOVE " << direction << " " << chargingMode;

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
                
                if(activateSonar)
                {
                    myOrder << "|SONAR " << sectorSonar;
                }
            }
        }   
        else
        {
            myOrder << "SURFACE";
            myMap = resetMap(defaultMap);
            isTherePossibleDirection = true;
        }

        cerr << "END " << myOrder.str() << endl;

        cout << myOrder.str() << endl;

        myPrevOrderLine = myOrder.str();

        if(myPrevOrderLine == "SURFACE")
            myPrevOrderLine += ' ' + to_string(getSectorIndex(r));


        myPrevLife = myLife;
        oppPrevLife = oppLife;
        step_counter++;

    }
}
