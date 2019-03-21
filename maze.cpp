#include <ncurses.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <thread>
#include <unordered_map>
#include <vector>

struct cell
{
    int x;
    int y;

    bool operator==(cell const& rhs)
    {
        return this->x == rhs.x && this->y == rhs.y;
    }
};

enum visited_by
{
    NONE = 0,
    DFS = 1,
    BFS = 2,
};

struct maze
{
    char** table;
    int** visited;
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
    std::uniform_int_distribution<int> gen(0, 24);
    cell c;
    c.x = gen(rng) * 2 + 1;
    c.y = gen(rng) * 2 + 1;
    return c;
}

std::optional<cell> get_random_neighbor(std::vector<cell> const& cells, size_t idx, maze& table)
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
        switch (dirs[i])
        {
            case 0:
                if (last.y >= first_y && table.table[last.y - 2][last.x] == '#')
                {
                    table.table[last.y - 1][last.x] = ' ';
                    last.y -= 2;
                    return std::optional<cell>{last};
                }
                break;
            case 1:
                if (last.x < last_x && table.table[last.y][last.x + 2] == '#')
                {
                    table.table[last.y][last.x + 1] = ' ';
                    last.x += 2;
                    return std::optional<cell>{last};
                }
                break;
            case 2:
                if (last.y < last_y && table.table[last.y + 2][last.x] == '#')
                {
                    table.table[last.y + 1][last.x] = ' ';
                    last.y += 2;
                    return std::optional<cell>{last};
                }
                break;
            case 3:
                if (last.x >= first_x && table.table[last.y][last.x - 2] == '#')
                {
                    table.table[last.y][last.x - 1] = ' ';
                    last.x -= 2;
                    return std::optional<cell>{last};
                }
                break;
        }
    }

    return std::nullopt;
}

void generate(std::vector<cell>& cells, size_t idx, maze& table)
{
    if (cells.empty())
    {
        return;
    }

    std::optional<cell> n;
    do
    {
        n = get_random_neighbor(cells, idx, table);
        if (n.has_value())
        {
            cell c = n.value();

            int i = cells.size();
            cells.push_back(c);
            table.table[c.y][c.x] = ' ';
            generate(cells, i, table);
        }
        else
        {
            return;
        }
    }
    while (n.has_value());
}

void add_border(maze& table)
{
    for (size_t i = 0; i < 50; ++i)
    {
        table.table[i][0] = '#';
    }

    for (size_t j = 0; j < 50; ++j)
    {
        table.table[0][j] = '#';
    }

    for (size_t i = 0; i < 50; ++i)
    {
        table.table[i][50 - 1] = '#';
    }

    for (size_t j = 0; j < 50; ++j)
    {
        table.table[50 - 1][j] = '#';
    }
}

cell add_entrance(maze& table)
{
    for (size_t j = 0; j < 50; ++j)
    {
        if (table.table[1][j] == ' ')
        {
            table.table[0][j] = ' ';
            return cell{j, 0};
        }
    }

    return cell{-1, -1};
}

cell add_exit(maze& table)
{
    for (size_t j = 51 - 1; j > 0; --j)
    {
        if (table.table[49][j] == ' ')
        {
            table.table[50][j] = ' ';
            return cell{j, 50};
        }
    }

    return cell{-1, -1};
}

std::mutex visit_lock;
void visit_at(maze& table, int visited_by, int y, int x)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    visit_lock.lock();
    table.visited[y][x] |= visited_by;
    
    switch (table.visited[y][x])
    {
        case visited_by::NONE:
            attron(COLOR_PAIR(1));
            mvaddch(y, x, table.table[y][x]);
            break;
        case visited_by::DFS | visited_by::BFS:
            attron(COLOR_PAIR(4));
            mvaddch(y, x, '!');
            break;
        case visited_by::DFS:
            attron(COLOR_PAIR(2));
            mvaddch(y, x, '!');
            break;
        case visited_by::BFS:
            attron(COLOR_PAIR(3));
            mvaddch(y, x, '!');
            break;
    }

    refresh();
    visit_lock.unlock();
}

void dfs_impl(maze& table, cell entrance, cell exit, bool& found, bool** vis)
{
    //table.visited[entrance.y][entrance.x] |= visited_by::DFS;
    visit_at(table, visited_by::DFS, entrance.y, entrance.x);
    if (entrance.x == exit.x && entrance.y == exit.y)
    {
        found = true;
    }

    if (!found && entrance.y > 0 && table.table[entrance.y - 1][entrance.x] == ' ' && !vis[entrance.y - 1][entrance.x])
    {
        vis[entrance.y - 1][entrance.x] = true;
        cell dummy = entrance;
        dummy.y -= 1;
        dfs_impl(table, dummy, exit, found, vis);
    }

    if (!found && entrance.x > 0 && table.table[entrance.y][entrance.x - 1] == ' ' && !vis[entrance.y][entrance.x - 1])
    {
        vis[entrance.y][entrance.x - 1] = true;
        cell dummy = entrance;
        dummy.x -= 1;
        dfs_impl(table, dummy, exit, found, vis);
    }

    if (!found && entrance.y < 51 - 1 && table.table[entrance.y + 1][entrance.x] == ' ' && !vis[entrance.y + 1][entrance.x])
    {
        vis[entrance.y + 1][entrance.x] = true;
        cell dummy = entrance;
        dummy.y += 1;
        dfs_impl(table, dummy, exit, found, vis);
    }

    if (!found && entrance.x < 51 - 1 && table.table[entrance.y][entrance.x + 1] == ' ' && !vis[entrance.y][entrance.x + 1])
    {
        vis[entrance.y][entrance.x + 1] = true;
        cell dummy = entrance;
        dummy.x += 1;
        dfs_impl(table, dummy, exit, found, vis);
    }
}

void dfs(maze& table, cell entrance, cell exit)
{
    bool found = false;
    bool** vis = new bool*[51];
    for (size_t i = 0; i < 51; ++i)
    {
        vis[i] = new bool[51];
        for (size_t j = 0; j < 51; ++j)
        {
            vis[i][j] = false;
        }
    }

    dfs_impl(table, entrance, exit, found, vis);
    for (size_t i = 0; i < 51; ++i)
    {
        delete[] vis[i];
    }

    delete vis;
}

void bfs(maze& table, cell entrance, cell exit)
{
    bool vis[51][51];
    cell u;
    cell v;

    std::queue<cell> q;
    for (size_t i = 0; i < 51; ++i)
    {
        for (size_t j = 0; j < 51; ++j)
        {
            vis[i][j] = false;
        }
    }

    vis[entrance.y][entrance.x] = true;
    //table.visited[entrance.y][entrance.x] |= visited_by::BFS;
    visit_at(table, visited_by::BFS, entrance.y, entrance.x);
    q.push(entrance);

    while (!q.empty())
    {
        auto visit = [exit](auto vis, auto v, auto &q, auto table) -> bool
        {
            bool found = false;
            if (v.x == exit.x && v.y == exit.y)
            {
                found = true;
            }

            if (!vis[v.y][v.x])
            {
                vis[v.y][v.x] = true;
                q.push(v);
                //table.visited[v.y][v.x] |= visited_by::BFS;
                visit_at(table, visited_by::BFS, v.y, v.x);
            }

            return found;
        };

        u = q.front();
        q.pop();
        if (u.y > 0 && table.table[u.y - 1][u.x] == ' ')
        {
            v = u;
            v.y -= 1;
            if (visit(vis, v, q, table))
            {
                return;
            }
        }

        if (u.x > 0 && table.table[u.y][u.x - 1] == ' ')
        {
            v = u;
            v.x -= 1;
            if (visit(vis, v, q, table))
            {
                return;
            }
        }

        if (u.y < 51 - 1 && table.table[u.y + 1][u.x] == ' ')
        {
            v = u;
            v.y += 1;
            if (visit(vis, v, q, table))
            {
                return;
            }
        }

        if (u.x < 51 - 1 && table.table[u.y][u.x + 1] == ' ')
        {
            v = u;
            v.x += 1;
            if (visit(vis, v, q, table))
            {
                return;
            }
        }
    }
}

int main()
{
    initscr();
    cbreak();
    noecho();
    start_color();
    raw();
    clear();

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);

    constexpr int max_lines   = 51;
    constexpr int max_columns = 51;
    std::vector<cell> cells;
    cells.push_back(get_random_cell());

    maze m;
    m.table = new char*[max_lines];
    m.visited = new int*[max_lines];
    for (int i = 0; i < max_lines; ++i)
    {
        m.table[i] = new char[max_columns];
        m.visited[i] = new int[max_columns];
        for (int j = 0; j < max_columns; ++j)
        {
            m.table[i][j] = '#';
            m.visited[i][j] = visited_by::NONE;
        }
    }

    generate(cells, 0, m);
    cell entrance = add_entrance(m);
    cell exit = add_exit(m);
    for (size_t i = 0; i < max_lines; ++i)
    {
        for (size_t j = 0; j < max_columns; ++j)
        {
            mvaddch(i, j, m.table[i][j]);
        }
    }

    std::thread t_bfs(bfs, std::ref(m), entrance, exit);
    std::thread t_dfs(dfs, std::ref(m), entrance, exit);

    t_bfs.join();
    t_dfs.join();
    
    for (int i = 0; i < max_lines; ++i)
    {
        delete[] m.table[i];
        delete[] m.visited[i];
    }

    delete[] m.table;
    delete[] m.visited;

    getch();
    refresh();
    endwin();

    return 0;
}

