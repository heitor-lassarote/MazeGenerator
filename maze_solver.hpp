class maze_solver
{
public:
    virtual std::vector<cell> run(cell entrance, cell exit) = 0;
};

class dfs_solver : public maze_solver
{
private:
    bool found;
    maze const& table;
    std::vector<cell> solution;
    std::vector<std::vector<bool>> visited;

public:
    dfs_solver(maze const& table);
    virtual std::vector<cell> run(cell entrance, cell exit);

private:
    void dfs_impl(cell entrance, cell exit);
};

class bfs_solver : public maze_solver
{
private:
    maze const& table;
    std::vector<cell> solution;

public:
    bfs_solver(maze const& table);
    virtual std::vector<cell> run(cell entrance, cell exit);
};

