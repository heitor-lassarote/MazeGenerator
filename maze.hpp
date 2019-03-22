#include <optional>

struct cell
{
    int x;
    int y;

    bool operator==(cell const& rhs)
    {
        return this->x == rhs.x && this->y == rhs.y;
    }
};

enum class maze_feature
{
    NOTHING,
    WALL,
};

class maze
{
    size_t rows;
    size_t columns;

    maze_feature** table;
    cell entrance;
    cell exit;

public:
    maze(size_t rows, size_t columns);
    maze(maze const& other);
    maze(maze&& other);
    ~maze();
    maze& operator=(const maze& other);
    maze& operator=(maze&& other);

private:
    void alloc_tables(size_t rows, size_t columns);
    void copy_tables(maze const& other);
    void dispose();
    void generate(std::vector<cell>& cells, size_t idx);

public:
    maze_feature at(size_t y, size_t x) const;
    maze_feature** data() const;
    void generate();
    cell get_entrance() const;
    cell get_exit() const;
    size_t get_rows() const;
    size_t get_columns() const;
    std::optional<cell> get_random_neighbor(std::vector<cell> const& cells, size_t idx);
    cell get_random_cell();
    void add_entrance();
    void add_exit();
};

