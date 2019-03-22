#include <mutex>
#include <thread>
#include <vector>

#include <ncurses.h>

#include "maze.hpp"
#include "maze_solver.hpp"

enum visited_by
{
    NONE = 0,
    DFS = 1,
    BFS = 2,
};

static std::mutex visit_lock;

char const nothing = ' ';
char const visit   = '!';
char const wall    = '#';

void visit_at(maze const& table, int visited_by, int y, int x, int** vis)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    visit_lock.lock();
    vis[y][x] |= visited_by;
    
    switch (vis[y][x])
    {
        case visited_by::NONE:
            attron(COLOR_PAIR(1));
            mvaddch(y, x, table.at(y, x) == maze_feature::NOTHING ? nothing : wall);
            break;
        case visited_by::DFS | visited_by::BFS:
            attron(COLOR_PAIR(4));
            mvaddch(y, x, visit);
            break;
        case visited_by::DFS:
            attron(COLOR_PAIR(2));
            mvaddch(y, x, visit);
            break;
        case visited_by::BFS:
            attron(COLOR_PAIR(3));
            mvaddch(y, x, visit);
            break;
    }

    refresh();
    visit_lock.unlock();
}

void draw(
    std::vector<cell> const& solution,
    visited_by visited_by,
    maze const& table,
    int** vis)
{
    for (cell const& c : solution)
    {
        visit_at(table, visited_by, c.y, c.x, vis);
    }
}

void draw_maze(maze const& table)
{
    for (size_t i = 0; i < table.get_rows(); ++i)
    {
        for (size_t j = 0; j < table.get_columns(); ++j)
        {
            mvaddch(i, j, table.at(i, j) == maze_feature::NOTHING ? nothing : wall);
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
    maze m{max_lines, max_columns};
    m.generate();

    bfs_solver bfs{m};
    dfs_solver dfs{m};
    std::vector<cell> bfs_s = bfs.run(m.get_entrance(), m.get_exit()); 
    std::vector<cell> dfs_s = dfs.run(m.get_entrance(), m.get_exit()); 

    int** vis = new int*[max_lines];
    for (size_t i = 0; i < max_lines; ++i)
    {
        vis[i] = new int[max_columns];
        for (size_t j = 0; j < max_columns; ++j)
        {
            vis[i][j] = visited_by::NONE;
        }
    }

    draw_maze(m);
    std::thread bfs_t(draw, bfs_s, visited_by::BFS, std::ref(m), vis);
    std::thread dfs_t(draw, dfs_s, visited_by::DFS, std::ref(m), vis);
    bfs_t.join();
    dfs_t.join();

    for (size_t i = 0; i < max_lines; ++i)
    {
        delete[] vis[i];
        vis[i] = nullptr;
    }

    delete[] vis;
    vis = nullptr;

    getch();
    refresh();
    endwin();

    return 0;
}

