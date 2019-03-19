#include <ncurses.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <random>
#include <thread>
#include <vector>

struct cell
{
    int x;
    int y;
};

void draw_line_x(int x0, int x1, int y, char c)
{
    for (int x = std::min(x0, x1); x < std::max(x0, x1); ++x)
    {
        mvaddch(y, x, c);
        //system("sleep 0.1s");
        //refresh();
    }
}

void draw_line_y(int x, int y0, int y1, char c)
{
    for (int y = std::min(y0, y1); y < std::max(y0, y1); ++y)
    {
        mvaddch(y, x, c);
        //system("sleep 0.1s");
        //refresh();
    }
}

cell get_random_cell()
{
    static std::random_device rng;
    std::uniform_int_distribution<int> gen(0, 50);
    cell c;
    c.x = gen(rng);
    c.y = gen(rng);
    return c;
}

std::optional<cell> get_random_neighbor(std::vector<cell> const& cells, size_t idx, char** table)
{
    cell last = cells[idx];
    static std::random_device rng;
    int dirs[4] = { 0, 1, 2, 3 };
    std::shuffle(dirs, dirs + 4, rng);

    int first_x = 2;
    int first_y = 2;
    int last_x = 48;
    int last_y = 48;
    for (size_t i = 0; i < 4; ++i)
    {
        //std::cout << "Dir = " << dirs[i] << " pt = " << "{" << last.x << "," << last.y << "}\n";
        switch (dirs[i])
        {
            case 0:
                if (last.y >= first_y && table[last.y - 2][last.x] == '#')
                {
                    table[last.y - 1][last.x] = ' ';
                    mvaddch(last.y, last.x, table[last.y][last.x]);
                    refresh();
                    last.y -= 2;
                    return std::optional<cell>{last};
                }
                break;
            case 1:
                if (last.x < last_x && table[last.y][last.x + 2] == '#')
                {
                    table[last.y][last.x + 1] = ' ';
                    mvaddch(last.y, last.x, table[last.y][last.x]);
                    refresh();
                    last.x += 2;
                    return std::optional<cell>{last};
                }
                break;
            case 2:
                if (last.y < last_y && table[last.y + 2][last.x] == '#')
                {
                    table[last.y + 1][last.x] = ' ';
                    mvaddch(last.y, last.x, table[last.y][last.x]);
                    refresh();
                    last.y += 2;
                    return std::optional<cell>{last};
                }
                break;
            case 3:
                if (last.x >= first_x && table[last.y][last.x - 2] == '#')
                {
                    table[last.y][last.x - 1] = ' ';
                    mvaddch(last.y, last.x, table[last.y][last.x]);
                    refresh();
                    last.x -= 2;
                    return std::optional<cell>{last};
                }
                break;
        }
    }

    return std::nullopt;
}

void generate(std::vector<cell>& cells, size_t idx, char** table)
{
    if (cells.empty())
    {
        return;
    }

    std::optional<cell> n;
    do
    {
        n = get_random_neighbor(cells, idx, table);
        //std::cout << (n.has_value() ? "std::optional<cell>" : "std::nullopt") << '\n';
        if (n.has_value())
        {
            cell c = n.value();

            int i = cells.size();
            cells.push_back(c);
            //std::cout << "Table[" << c.y << "][" << c.x << "]\n";
            table[c.y][c.x] = ' ';
            mvaddch(c.y, c.x, table[c.y][c.x]);
            system("sleep 0.01s");
            refresh();
            generate(cells, i, table);
        }
        else
        {
            return;
        }
    }
    while (n.has_value());
}

void add_border(char** table)
{
    for (size_t i = 0; i < 50; ++i)
    {
        table[i][0] = '#';
    }

    for (size_t j = 0; j < 50; ++j)
    {
        table[0][j] = '#';
    }

    for (size_t i = 0; i < 50; ++i)
    {
        table[i][50 - 1] = '#';
    }

    for (size_t j = 0; j < 50; ++j)
    {
        table[50 - 1][j] = '#';
    }
}

int main()
{
    initscr();
    cbreak();
    noecho();

    raw();
    clear();

    constexpr int max_lines   = 50;
    constexpr int max_columns = 50;
    std::vector<cell> cells;
    cells.push_back(get_random_cell());

    //char table[max_lines][max_columns];
    char** table = new char*[max_lines];
    for (int i = 0; i < max_lines; ++i)
    {
        table[i] = new char[max_columns];
        for (int j = 0; j < max_columns; ++j)
        {
            table[i][j] = '#';
            mvaddch(i, j, '#');
        }
    }

    generate(cells, 0, table);
    add_border(table);
    
    for (int i = 0; i < max_lines; ++i)
    {
        for (int j = 0; j < max_columns; ++j)
        {
            mvaddch(i, j, table[i][j]);
        }

        delete[] table[i];
    }

    delete[] table;

    getch();
    refresh();
    endwin();

    return 0;
}

