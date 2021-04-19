#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <windows.h>

#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#define DISABLE_NEWLINE_AUTO_RETURN 0x0008

enum Cell {
    ASHES, YOUNG, OLD, BEGIN, FIRE, END
};

using v = std::vector<Cell>;
using vv = std::vector<v>;

template<class Container, class... Type>
void extend(Container& c, Type&& ... args)
{
    if constexpr (sizeof...(args) > 1)
        c.reserve(c.size() + sizeof...(args));

    (c.push_back(std::forward<Type>(args)), ...);
}

int randint(int min, int max) {
    int n = max - min + 1;
    int remainder = RAND_MAX % n;
    int x;

    do {
        x = rand();
    } while (x >= RAND_MAX - remainder);

    return min + x % n;
}

vv initWorld(int width, int height, bool ashes = false, bool young = false, bool launchInCorner = false, bool bigLaunchInCorner = false) {
	vv world;
	
	if (ashes) {
		for (int x = 0; x < width; x++) {
			v column;

			for (int y = 0; y < height; y++) {
				column.push_back(ASHES);
			}

			world.push_back(column);
		}
	}
    else if (young) {
        for (int x = 0; x < width; x++) {
            v column;

            for (int y = 0; y < height; y++) {
                column.push_back(YOUNG);
            }

            world.push_back(column);
        }
    }
    else if (launchInCorner) {
        for (int x = 0; x < width; x++) {
            v column;

            for (int y = 0; y < height; y++) {
                if (x == 0 && y == 0) {
                    column.push_back(BEGIN);
                }
                else {
                    column.push_back(YOUNG);
                }
            }

            world.push_back(column);
        }
    }
    else if (bigLaunchInCorner) {
        for (int x = 0; x < width; x++) {
            v column;

            for (int y = 0; y < height; y++) {
                if (x <= width / 10 && y <= height / 10) {
                    column.push_back(BEGIN);
                }
                else {
                    column.push_back(YOUNG);
                }
            }

            world.push_back(column);
        }
    }

	return world;
}

v getNeighbors(const vv& world, int x, int y) {
    v neighbors;

    if (x > 0 && y > 0 && x < world.size() - 1 && y < world[0].size() - 1) {
        extend(neighbors,
            world[x - 1][y - 1],
            world[x][y - 1],
            world[x + 1][y - 1],
            world[x + 1][y],
            world[x + 1][y + 1],
            world[x][y + 1],
            world[x - 1][y + 1],
            world[x - 1][y]);
    }
    else if (x == 0 && y == 0 && x < world.size() - 1 && y < world[0].size() - 1) {
        extend(neighbors,
            world[x + 1][y],
            world[x + 1][y + 1],
            world[x][y + 1]);
    }
    else if (x > 0 && y == 0 && x < world.size() - 1 && y < world[0].size() - 1) {
        extend(neighbors,
            world[x - 1][y],
            world[x - 1][y + 1],
            world[x][y + 1],
            world[x + 1][y + 1],
            world[x + 1][y]);
    }
    else if (x > 0 && y == 0 && x == world.size() - 1 && y < world[0].size() - 1) {
        extend(neighbors,
            world[x - 1][y],
            world[x - 1][y + 1],
            world[x][y + 1]);
    }
    else if (x == 0 && y > 0 && x < world.size() - 1 && y < world[0].size() - 1) {
        extend(neighbors,
            world[x][y - 1],
            world[x + 1][y - 1],
            world[x + 1][y],
            world[x + 1][y + 1],
            world[x][y + 1]);
    }
    else if (x > 0 && y > 0 && x == world.size() - 1 && y < world[0].size() - 1) {
        extend(neighbors,
            world[x][y - 1],
            world[x - 1][y - 1],
            world[x - 1][y],
            world[x - 1][y + 1],
            world[x][y + 1]);
    }
    else if (x == 0 && y > 0 && x < world.size() - 1 && y == world[0].size() - 1) {
        extend(neighbors,
            world[x][y - 1],
            world[x + 1][y - 1],
            world[x + 1][y]);
    }
    else if (x > 0 && y > 0 && x < world.size() - 1 && y == world[0].size() - 1) {
        extend(neighbors,
            world[x - 1][y],
            world[x - 1][y - 1],
            world[x][y - 1],
            world[x + 1][y - 1],
            world[x + 1][y]);
    }
    else if (x > 0 && y > 0 && x == world.size() - 1 && y == world[0].size() - 1) {
        extend(neighbors,
            world[x - 1][y],
            world[x - 1][y - 1],
            world[x][y - 1]);
    }
    return neighbors;
}

void transition(const vv& oldWorld, vv& newWorld, bool burningCellIncreaseProbability = false) {
    for (int x = 0; x < oldWorld.size(); x++) {
        for (int y = 0; y < oldWorld[0].size(); y++) {
            v neighbors{ getNeighbors(oldWorld, x, y) };
            Cell cell = oldWorld[x][y];

            if (cell == YOUNG) {
                if (std::find(neighbors.begin(), neighbors.end(), BEGIN) != neighbors.end() && randint(1, 100) == 1) {
                    newWorld[x][y] = BEGIN;
                }
                else if (std::find(neighbors.begin(), neighbors.end(), FIRE) != neighbors.end() && randint(1, 50) == 1) {
                    newWorld[x][y] = BEGIN;
                }
                else if (std::find(neighbors.begin(), neighbors.end(), END) != neighbors.end() && randint(1, 100) == 1) {
                    newWorld[x][y] = BEGIN;
                }
                else if (randint(1, 200) == 1) {
                    newWorld[x][y] = OLD;
                }
                else {
                    newWorld[x][y] = YOUNG;
                }
            }
            else if (cell == OLD) {
                if (std::find(neighbors.begin(), neighbors.end(), BEGIN) != neighbors.end() && randint(1, 10) == 1) {
                    newWorld[x][y] = BEGIN;
                }
                else if (std::find(neighbors.begin(), neighbors.end(), FIRE) != neighbors.end() && randint(1, 5) == 1) {
                    newWorld[x][y] = BEGIN;
                }
                else if (std::find(neighbors.begin(), neighbors.end(), END) != neighbors.end() && randint(1, 10) == 1) {
                    newWorld[x][y] = BEGIN;
                }
                else if (std::count(neighbors.begin(), neighbors.end(), OLD) >= 5 && randint(1, 20000) == 1) {
                    newWorld[x][y] = BEGIN;
                }
                else {
                    newWorld[x][y] = OLD;
                }
            }
            else if (cell == BEGIN) {
                if (randint(1, 10) == 1) {
                    newWorld.at(x).at(y) = FIRE;
                }
                else {
                    newWorld[x][y] = BEGIN;
                }
            }
            else if (cell == FIRE) {
                if (randint(1, 10) == 1) {
                    newWorld[x][y] = END;
                }
                else {
                    newWorld[x][y] = FIRE;
                }
            }
            else if (cell == END) {
                if (randint(1, 10) == 1) {
                    newWorld[x][y] = ASHES;
                }
                else {
                    newWorld[x][y] = END;
                }
            }
            else if (cell == ASHES) {
                if (randint(1, 1000) == 1) {
                    newWorld[x][y] = YOUNG;
                }
                else {
                    newWorld[x][y] = ASHES;
                }
            }
        }
    }
}

void printColored(int r, int g, int b, char c) {
    std::cout << "\x1b[48;2;" << r << ";" << g << ";" << b << "m" << c << "\x1b[0m";
}

void printWorld(const vv& world) {
    for (int y = 0; y < world[0].size(); y++) {
        for (int x = 0; x < world.size(); x++) {
            Cell cell = world[x][y];

            if (cell == ASHES) {
                printColored(128, 128, 128, ' '); // gray
            }
            else if (cell == YOUNG) {
                printColored(102, 255, 102, ' '); // light green
            }
            else if (cell == OLD) {
                printColored(0, 153, 0, ' '); // dark green
            }
            else if (cell == BEGIN) {
                printColored(255, 255, 0, ' '); // yellow
            }
            else if (cell == FIRE) {
                printColored(255, 128, 0, ' '); // orange
            }
            else if (cell == END) {
                printColored(255, 0, 0, ' '); // red
            }
        }
        std::cout << std::endl;
    }
}

int main() {
    DWORD l_mode;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hStdout, &l_mode);
    SetConsoleMode(hStdout, l_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);

    //vv oldWorld{ initWorld(100, 15, true, false, false, false) }; // ashes
    //vv oldWorld{ initWorld(10, 10, false, true, false, false) }; // young
    //vv oldWorld{ initWorld(200, 25, false, false, true, false) }; // launch in corner
    vv oldWorld{ initWorld(5, 5, false, false, false, true) }; // big launch in corner

    vv newWorld(oldWorld);    

    auto begin = std::chrono::steady_clock::now();

    for (int i = 0; i <= 1000; i++) {
        transition(oldWorld, newWorld);
        std::cout << "Step " << i << std::endl;
        printWorld(newWorld);
        oldWorld.swap(newWorld);
        //using namespace std::chrono_literals;
        //std::this_thread::sleep_for(0.01s);
    }

    auto end = std::chrono::steady_clock::now();

    std::cout << "Temps pour faire 1000 itérations : " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "s" << std::endl;
}