#include <windows.h>
#include <SFML/Graphics.hpp>
#include <random>
#include <chrono>
#include <iostream>
#include <string>

constexpr int WIDTH = 800, HEIGHT = 600; // screen and image dimensions
constexpr int FRAME_RATE_LIMIT = 25;
constexpr int LIVE = 1, DEAD = 0;
constexpr double LIFE_PROPABILITY = 0.05;
const std::string CONWAY_RULE = "B3/S23";
const std::string REPLICATOR_RULE = "B1357/S1357";
const std::string SEEDS_RULE = "B2/S";
const std::string B25_S4_REPLICATOR_RULE = "B25/S4";
const std::string LIFE_WITHOUT_DEATH = "B3/S012345678";
const std::string LIFE_RULE = "B3/S23";
const std::string LIFE_34_RULE = "B34/S34";
const std::string DIAMOEBA_RULE = "B35678/S5678";
const std::string TWO_BY_TWO_RULE = "B36/S125";
const std::string HIGHLIFE_RULE = "B36/S23";
const std::string DAY_AND_NIGHT_RULE = "B3678/S34678";
const std::string MORLEY_RULE = "B368/S245";
const std::string ANNEAL_RULE = "B4678/S35678";
const std::string MAZE_RULE = "B3/S12345";
const std::string MAZECTRIC_RULE = "B3/S1234";
const std::string RULE = MAZECTRIC_RULE; // pick one of the above strings or write yours (RESPICTING FORMAT)

std::pair<std::string, std::string> parseRule(const std::string& rule);
uint16_t createMask(const std::string& rule);
int applyRules(int cell, int neighbors, const uint16_t b_mask, const uint16_t s_mask);
void setBoardPixels(std::vector<std::uint8_t>& pixels, const std::vector<int>& board,
    sf::Color liveColor = sf::Color::White, sf::Color deadColor = sf::Color::Black);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    sf::RenderWindow window(sf::VideoMode({ WIDTH, HEIGHT }), "Conway's Game of Life");
    window.setFramerateLimit(FRAME_RATE_LIMIT);
    // One sprite for all the board
    std::vector<std::uint8_t> boardPixels(WIDTH * HEIGHT * 4);
    sf::Texture boardTexture(sf::Vector2u(WIDTH, HEIGHT));
    sf::Sprite boardSprite(boardTexture);

    // Convert rules to B and S masks for fast bitwise operations
    auto [b, s] = parseRule(RULE);
    uint16_t b_mask = createMask(b);
    uint16_t s_mask = createMask(s);

    // Create board and initialize with random cells
    std::vector<int> board(WIDTH * HEIGHT, 0);  // life or dead cells
    std::vector<int> nCount(WIDTH * HEIGHT, 0); // neighbors count for each cell
    std::mt19937 randEng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::bernoulli_distribution lifeDeath(LIFE_PROPABILITY);
    for (unsigned int y = 1; y < HEIGHT - 1; y++)
    {
        for (unsigned int x = 1; x < WIDTH - 1; x++)
        {
            board[y * WIDTH + x] = lifeDeath(randEng);
        }
    }
    setBoardPixels(boardPixels, board);
    boardTexture.update(boardPixels.data());

    // Main loop
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Count all neighbours
        for (unsigned int y = 1; y < HEIGHT-1; y++)
        {
            for (unsigned int x = 1; x < WIDTH-1; x++)
            {
                nCount[y * WIDTH + x] = 0;
                for (unsigned int ny = y-1; ny <= y+1; ny++)
                {
                    for (unsigned int nx = x-1; nx <= x+1; nx++)
                    {
                        if (ny == y && nx == x)
                            continue;
                        if(board[ny * WIDTH + nx])
                            nCount[y * WIDTH + x] ++;
                    }
                }
            }
        }

        // Rules
        for (unsigned int y = 1; y < HEIGHT - 1; y++)
        {
            for (unsigned int x = 1; x < WIDTH - 1; x++)
            {
                
                int &cell = board[y * WIDTH + x];
                int& neighbors = nCount[y * WIDTH + x];
                cell = applyRules(cell, neighbors, b_mask, s_mask);
            }
        }

        // Update sprite image
        setBoardPixels(boardPixels, board);
        boardTexture.update(boardPixels.data());

        window.clear();
        window.draw(boardSprite);
        window.display();
    }
    return 0;
}

std::pair<std::string, std::string> parseRule(const std::string& rule)
{
    std::string b = "";
    std::string s = "";

    size_t slash = rule.find('/');
    if (slash != std::string::npos) {
        if (slash > 1) //if B rule exists
            b = rule.substr(1, slash - 1); //get it

        if (slash + 2 <= rule.size()) //if S rule exist
            s = rule.substr(slash + 2); //get it
    }

    return { b, s };
}

uint16_t createMask(const std::string& rule)
{
    // 8-bits, each bit represents neighbour count
    //                      mask = 000000000
    // neighbors representation => 876543210
    // 
    // e.g. B014: (if neighbors count is 0, 1 or 4)
    // mask = 000000000
    // mask |= 1 << 0  => 000000001
    // mask |= 1 << 1  => 000000011
    // mask |= 1 << 4  => 000010011
    uint16_t mask = 0;
    for (char c : rule)
    {
        if (std::isdigit(c))
        {
            mask |= (1 << (c - '0'));
        }
    }
    return mask;
}

int applyRules(int cell, int neighbors, const uint16_t b_mask, const uint16_t s_mask)
{
    // Check if the bit corresponding to neighbors is set in the rule mask.
    // e.g. 1: mask = 000000010 (mask for only 1, not 2), neighbors = 2
    //         mask >> neighbors = 000000010 >> 2 = 000000000
    //         000000000 & 000000001 = 000000000 => returns DEAD
    // e.g. 2: mask = 011000000 (mask for both 6 and 7), neighbors = 6
    //         mask >> neighbors = 011000000 >> 6 = 000000011
    //         000000011 & 000000001 = 000000001 => returns LIVE
    if (cell == DEAD)
        return ((b_mask >> neighbors) & 1) ? LIVE : DEAD; // Born conditions
    else
        return ((s_mask >> neighbors) & 1) ? LIVE : DEAD; // Survival conditions
}

void setBoardPixels(std::vector<std::uint8_t>& pixels, const std::vector<int>& board,
    sf::Color liveColor, sf::Color deadColor)
{
    for (unsigned int y = 1; y < HEIGHT - 1; ++y)
    {
        for (unsigned int x = 1; x < WIDTH - 1; ++x)
        {
            std::size_t i = (y * WIDTH + x) * 4;
            sf::Color color = board[y * WIDTH + x] ? liveColor : deadColor;
            pixels[i + 0] = color.r;
            pixels[i + 1] = color.g;
            pixels[i + 2] = color.b;
            pixels[i + 3] = color.a;
        }
    }
}

/////// OLD VERSIONS /////////

int applyRules(int cell, int neighbors)
{
    if (cell == DEAD)
        return (neighbors == 3) ? LIVE : DEAD; // Born conditions
    else
        return (neighbors == 2 || neighbors == 3) ? LIVE : DEAD; // Survival conditions
}

void setBoardPixels(sf::Image& boardImage, const std::vector<int>& board,
    sf::Color liveColor, sf::Color deadColor)
{
    for (unsigned int y = 1; y < HEIGHT - 1; y++)
    {
        for (unsigned int x = 1; x < WIDTH - 1; x++)
        {
            boardImage.setPixel({ x,y }, board[y * WIDTH + x] ? liveColor : deadColor);
        }
    }
}
