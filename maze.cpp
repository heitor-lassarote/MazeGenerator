#include <algorithm>
#include <optional>
#include <random>
#include <vector>

#include "maze.hpp"

maze::maze(size_t rows, size_t columns)
    : rows(rows), columns(columns)
{
    this->alloc_tables(rows, columns);
}

maze::maze(maze const& other)
    : rows(other.rows), columns(other.columns)
{
    this->copy_tables(other);
}

maze::maze(maze&& other)
    : rows(std::move(other.rows))
    , columns(std::move(other.columns))
    , table(std::move(other.table))
{
}

maze::~maze()
{
    this->dispose();
}

maze& maze::operator=(maze const& other)
{
    if (this != &other)
    {
        this->dispose();
        this->copy_tables(other);
    }

    return *this;
}

maze& maze::operator=(maze&& other)
{
    if (this != &other)
    {
        this->rows = std::move(other.rows);
        this->columns = std::move(other.columns);
        this->table = std::move(other.table);
    }

    return *this;
}

void maze::alloc_tables(size_t rows, size_t columns)
{
    this->table = new maze_feature*[rows];
    for (size_t i = 0; i < rows; ++i)
    {
        this->table[i] = new maze_feature[columns];
        for (size_t j = 0; j < columns; ++j)
        {
            this->table[i][j] = maze_feature::WALL;
        }
    }
}

void maze::copy_tables(maze const& other)
{
    this->table = new maze_feature*[other.rows];
    for (size_t i = 0; i < other.rows; ++i)
    {
        this->table[i] = new maze_feature[other.columns];
        for (size_t j = 0; j < other.columns; ++j)
        {
            this->table[i][j] = other.table[i][j];
        }
    }
}

void maze::dispose()
{
    for (size_t i = 0; i < this->rows; ++i)
    {
        delete[] this->table[i];
        this->table[i] = nullptr;
    }

    delete[] this->table;
    this->table = nullptr;
}

maze_feature maze::at(size_t y, size_t x) const
{
    return this->table[y][x];
}

maze_feature** maze::data() const
{
    return this->table;
}

size_t maze::get_rows() const
{
    return this->rows;
}

size_t maze::get_columns() const
{
    return this->columns;
}

cell maze::get_entrance() const
{
    return this->entrance;
}

cell maze::get_exit() const
{
    return this->exit;
}

std::optional<cell> maze::get_random_neighbor(std::vector<cell> const& cells, size_t idx)
{
    cell last = cells[idx];
    static std::random_device rng;
    int dirs[4] = { 0, 1, 2, 3 };
    std::shuffle(dirs, dirs + 4, rng);

    int first_x = 2;
    int first_y = 2;
    int last_x = this->columns - 3;
    int last_y = this->columns - 3;
    for (size_t i = 0; i < 4; ++i)
    {
        switch (dirs[i])
        {
            case 0:
                if (last.y >= first_y && this->table[last.y - 2][last.x] == maze_feature::WALL)
                {
                    this->table[last.y - 1][last.x] = maze_feature::NOTHING;
                    last.y -= 2;
                    return std::optional<cell>{last};
                }
                break;
            case 1:
                if (last.x < last_x && this->table[last.y][last.x + 2] == maze_feature::WALL)
                {
                    this->table[last.y][last.x + 1] = maze_feature::NOTHING;
                    last.x += 2;
                    return std::optional<cell>{last};
                }
                break;
            case 2:
                if (last.y < last_y && this->table[last.y + 2][last.x] == maze_feature::WALL)
                {
                    this->table[last.y + 1][last.x] = maze_feature::NOTHING;
                    last.y += 2;
                    return std::optional<cell>{last};
                }
                break;
            case 3:
                if (last.x >= first_x && this->table[last.y][last.x - 2] == maze_feature::WALL)
                {
                    this->table[last.y][last.x - 1] = maze_feature::NOTHING;
                    last.x -= 2;
                    return std::optional<cell>{last};
                }
                break;
        }
    }

    return std::nullopt;
}

void maze::generate()
{
    std::vector<cell> cells;
    cells.push_back(this->get_random_cell());
    this->generate(cells, 0);
    this->add_entrance();
    this->add_exit();
}

void maze::generate(std::vector<cell>& cells, size_t idx)
{
    if (cells.empty())
    {
        return;
    }

    std::optional<cell> n;
    do
    {
        n = this->get_random_neighbor(cells, idx);
        if (n.has_value())
        {
            cell c = n.value();

            int i = cells.size();
            cells.push_back(c);
            this->table[c.y][c.x] = maze_feature::NOTHING;
            generate(cells, i);
        }
    }
    while (n.has_value());
}

cell maze::get_random_cell()
{
    static std::random_device rng;
    std::uniform_int_distribution<int> gen(0, 24);
    cell c;
    c.x = gen(rng) * 2 + 1;
    c.y = gen(rng) * 2 + 1;
    return c;
}

void maze::add_entrance()
{
    for (size_t j = 0; j < this->columns - 1; ++j)
    {
        if (this->table[1][j] == maze_feature::NOTHING)
        {
            this->table[0][j] = maze_feature::NOTHING;
            this->entrance = cell{j, 0};
            return;
        }
    }

    this->entrance = cell{-1, -1};
}

void maze::add_exit()
{
    for (size_t j = this->columns - 1; j > 0; --j)
    {
        if (this->table[this->columns - 2][j] == maze_feature::NOTHING)
        {
            this->table[this->columns - 1][j] = maze_feature::NOTHING;
            this->exit = cell{j, this->columns - 1};
            return;
        }
    }

    this->exit = cell{-1, -1};
}

