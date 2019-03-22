#include <chrono>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

#include <ncurses.h>

#include "maze.hpp"
#include "maze_solver.hpp"

dfs_solver::dfs_solver(maze const& table)
    : table(table)
{
}

std::vector<cell> dfs_solver::run(cell entrance, cell exit)
{
    this->found = false;
    this->solution.clear();
    this->visited.resize(table.get_rows());
    for (size_t i = 0; i < table.get_rows(); ++i)
    {
        this->visited[i].resize(table.get_columns());
    }

    this->dfs_impl(entrance, exit);
    return this->solution;
}

void dfs_solver::dfs_impl(cell entrance, cell exit)
{
    this->solution.push_back(entrance);
    this->visited[entrance.y][entrance.x] = true;
    if (entrance.x == exit.x && entrance.y == exit.y)
    {
        this->found = true;
    }

    if (!this->found && entrance.y < this->table.get_rows() - 1
        && this->table.at(entrance.y + 1, entrance.x) == maze_feature::NOTHING
        && !this->visited[entrance.y + 1][entrance.x])
    {
        cell dummy = entrance;
        dummy.y += 1;
        this->dfs_impl(dummy, exit);
    }

    if (!this->found && entrance.x < this->table.get_columns() - 1
        && this->table.at(entrance.y, entrance.x + 1) == maze_feature::NOTHING
        && !this->visited[entrance.y][entrance.x + 1])
    {
        cell dummy = entrance;
        dummy.x += 1;
        this->dfs_impl(dummy, exit);
    }

    if (!this->found && entrance.y > 0
        && this->table.at(entrance.y - 1, entrance.x) == maze_feature::NOTHING
        && !this->visited[entrance.y - 1][entrance.x])
    {
        cell dummy = entrance;
        dummy.y -= 1;
        this->dfs_impl(dummy, exit);
    }

    if (!this->found && entrance.x > 0
        && this->table.at(entrance.y, entrance.x - 1) == maze_feature::NOTHING
        && !this->visited[entrance.y][entrance.x - 1])
    {
        cell dummy = entrance;
        dummy.x -= 1;
        this->dfs_impl(dummy, exit);
    }
}

bfs_solver::bfs_solver(maze const& table)
    : table(table)
{
}

std::vector<cell> bfs_solver::run(cell entrance, cell exit)
{
    std::vector<std::vector<bool>> vis;
    cell u;
    cell v;

    vis.resize(this->table.get_rows());
    for (size_t i = 0; i < this->table.get_rows(); ++i)
    {
        vis[i].resize(this->table.get_columns());
    }

    vis[entrance.y][entrance.x] = true;
    this->solution.push_back(entrance);
    std::queue<cell> q;
    q.push(entrance);

    while (!q.empty())
    {
        auto visit = [this, &vis, &exit, &q](cell const& v) -> bool
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
                this->solution.push_back(v);
            }

            return found;
        };

        u = q.front();
        q.pop();
        if (u.y > 0 && this->table.at(u.y - 1, u.x) == maze_feature::NOTHING)
        {
            v = u;
            v.y -= 1;
            if (visit(v))
            {
                return this->solution;
            }
        }

        if (u.x > 0 && table.at(u.y, u.x - 1) == maze_feature::NOTHING)
        {
            v = u;
            v.x -= 1;
            if (visit(v))
            {
                return this->solution;
            }
        }

        if (u.y < table.get_rows() - 1 && table.at(u.y + 1, u.x) == maze_feature::NOTHING)
        {
            v = u;
            v.y += 1;
            if (visit(v))
            {
                return this->solution;
            }
        }

        if (u.x < table.get_columns() - 1 && table.at(u.y, u.x + 1) == maze_feature::NOTHING)
        {
            v = u;
            v.x += 1;
            if (visit(v))
            {
                return this->solution;
            }
        }
    }

    return this->solution;
}

