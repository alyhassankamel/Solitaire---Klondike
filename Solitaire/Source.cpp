#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <map>
#include <stack>
#include "raylib.h"

using namespace std;
struct insideCard
{
    string rank;
    string suit;
    insideCard(string r, string s)
    {
        rank = r;
        suit = s;
    }

    insideCard() = default;
};
struct CardNode
{
    // CardNode structure representing a node in the doubly linked list
    insideCard* card; // Represent the card as a string (e.g., "Ace of Hearts")
    CardNode* prev;   // Pointer to the previous card in the list
    CardNode* next;   // Pointer to the next card in the list
    bool faceUp;

    // Constructor to initialize the card and set prev/next to nullptr
    CardNode(string ranks, string shapes, bool facingUp = false)
    {
        card = new insideCard(ranks, shapes);
        prev = next = nullptr;
        this->faceUp = facingUp;
    }
    ~CardNode()
    {
        delete card; // Clean up memory
        card = nullptr;
    }
};
struct ClickableCard
{
    Rectangle bounds;
    int tableauIndex;
    int cardsCount;
    ClickableCard(Vector2 position, int from, int count = 1)
    {
        bounds = { position.x, position.y, 80, 109 };
        tableauIndex = from;
        cardsCount = count;
    }
};
struct GameState
{
    vector<CardNode*> tableaus[7];   // Current state of tableau piles
    vector<CardNode*> foundation[4]; // Current state of foundation piles
    vector<CardNode*> waste;         // Current state of the waste pile
    vector<CardNode*> stock;         // Current state of the stock pile

    GameState() = default;
};
class Solitaire
{
public:
    vector<CardNode*> tableau[7];     // Doubly linked lists for 7 tableau piles
    vector<CardNode*> stock;          // Doubly linked list for the stock
    vector<CardNode*> waste;          // Doubly linked list for the waste
    vector<CardNode*> foundations[4]; // Doubly linked lists for the 4 foundation piles
    stack<GameState> undoStack;
    stack<ClickableCard*> drawStack;
    ClickableCard* selected = nullptr;
    Texture2D atlas;

    Solitaire()
    {
        initializeDeck();
        shuffleDeck();
        setupTableau();
    }

    ~Solitaire()
    {
        // Clean up dynamically allocated memory in the game
        clearPile(stock);
        clearPile(waste);

        for (int i = 0; i < 7; ++i)
        {
            clearPile(tableau[i]);
        }

        for (int i = 0; i < 4; ++i)
        {
            clearPile(foundations[i]);
        }
    }

    void clearPile(vector<CardNode*>& pile)
    {
        for (CardNode* card : pile)
        {
            delete card; // Delete each card
        }
        pile.clear(); // Clear the vector
    }

    // Initialize the deck and store it as a doubly linked list
    void initializeDeck()
    {
        string suits[] = { "Hearts", "Diamonds", "Clubs", "Spades" };
        string ranks[] = { "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };

        for (const auto& suit : suits)
        {
            for (const auto& rank : ranks)
            {
                string rr = rank;
                string ss = suit;
                stock.push_back(new CardNode(rr, ss)); // Add new card as node in stock
            }
        }
    }

    /*void debugDeck() {
        cout << "Deck cards:" << endl;
        for (CardNode* node : stock) {
            cout << node->card->rank << " of " << node->card->suit << endl;
        }
    }
    */

    // Shuffle the deck using random number generation
    void shuffleDeck()
    {
        random_device rd;
        mt19937 g(rd());
        shuffle(stock.begin(), stock.end(), g);
    }

    void saveState()
    {
        GameState currentState;

        for (int i = 0; i < 7; ++i)
        {
            currentState.tableaus[i].clear();
            for (CardNode* card : tableau[i])
            {
                // Create a new CardNode for the state
                CardNode* newCard = new CardNode(card->card->rank, card->card->suit, card->faceUp);
                currentState.tableaus[i].push_back(newCard);
            }
        }

        for (int i = 0; i < 4; ++i)
        {
            currentState.foundation[i] = foundations[i]; // Assuming similar deep copy for foundations
        }

        // Similar deep copy for waste and stock
        currentState.waste = waste;
        currentState.stock = stock;

        undoStack.push(currentState);
    }

    void undo()
    {
        if (undoStack.empty())
        {
            return;
        }

        GameState previousState = undoStack.top();
        undoStack.pop();

        // Restore tableau states
        for (int i = 0; i < 7; ++i)
        {
            tableau[i] = previousState.tableaus[i];

            // Restore the faceUp state for each card
            for (size_t j = 0; j < tableau[i].size(); ++j)
            {
                tableau[i][j]->faceUp = previousState.tableaus[i][j]->faceUp;
            }
        }

        // Restore foundation states
        for (int i = 0; i < 4; ++i)
        {
            foundations[i] = previousState.foundation[i];
        }

        selected = nullptr;
        // Restore waste and stock states
        waste = previousState.waste;
        stock = previousState.stock;
    }

    // Set up the tableau, each pile is a doubly linked list
    void setupTableau()
    {
        for (int i = 0; i < 7; ++i)
        {
            for (int j = 0; j <= i; ++j)
            {
                CardNode* card = stock.back(); // Take the card from the end of the stock
                stock.pop_back();              // Remove it from the stock
                card->faceUp = (j == i);       // Face up only the topmost card
                tableau[i].push_back(card);    // Add the card to the tableau pile
            }
        }
        saveState();
    }

    // when user clicks on stock
    void stockWaste()
    {
        // used all stock cards
        saveState();
        if (stock.empty() && !waste.empty())
        {
            while (!waste.empty())
            {
                stock.push_back(waste.back());
                waste.pop_back();
            }
            return;
        }
        // no waste and no stock
        if (waste.empty() && stock.empty())
            return;
        // regular case:
        if (!stock.empty())
        {
            waste.push_back(stock.front()); //////
            waste.back()->faceUp = true;
            stock.erase(stock.begin());
        }
    }

    bool foundationValid(CardNode* from, int indexwhichff)
    {
        if (indexwhichff < 0 || indexwhichff >= 4)
            return false;
        if (!from || !from->card)
            return false; // Handle null pointer case
        if (foundations[indexwhichff].empty())
        { // clear foundation only add aces
            // return *from.card.begin() == 'A';
            return from->card->rank == "A";
            // foundations[indexwhichff].front() = from.card;
        }

        // Ensure suits match
        if (foundations[indexwhichff].back()->card->suit != from->card->suit)
        {
            return false; // Suits dont match
        }

        static const map<string, int> rankValues = {// this map gives integer valuse to rank to help in comparisons.
                                                    {"A", 1},
                                                    {"2", 2},
                                                    {"3", 3},
                                                    {"4", 4},
                                                    {"5", 5},
                                                    {"6", 6},
                                                    {"7", 7},
                                                    {"8", 8},
                                                    {"9", 9},
                                                    {"10", 10},
                                                    {"J", 11},
                                                    {"Q", 12},
                                                    {"K", 13} };

        // change ranks of from card and current one in foundation to integers
        int fromRank = rankValues.at(from->card->rank);
        int foundationRank = rankValues.at(foundations[indexwhichff].back()->card->rank);

        // if from is 2 and foundation rank is A, foundation + A = 2, then will return true
        return fromRank == foundationRank + 1;
    }

    /*void moveToFoundation(CardNode* from, int indexwhichff)
    {
        // if the user tries to move a card that is not on
        //  the outmost level or is not valid move returns
        if (indexwhichff < 0 || indexwhichff >= 4)
            return;

        if (from->next || !foundationValid(from, indexwhichff))
            return;

        saveState();

        for (auto& pile : tableau)
        {
            if (!pile.empty() && pile.back() == from)
            {
                pile.pop_back(); // Remove the card from the tableau pile
                if (!pile.empty())
                {
                    // Flip the new top card face-up if it exists
                    pile.back()->faceUp = true;
                }
                break;
            }
        }
        foundations[indexwhichff].push_back(from);

        // if move not valid returns.
    }*/

    bool tableauValid(CardNode* from, vector<CardNode*>& toPile)
    {
        if (toPile.empty())
        {
            return from->card->rank == "K"; // Only Kings can start an empty tableau pile
        }

        static const map<string, int> rankValues = {
            {"A", 1}, {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}, {"6", 6}, {"7", 7}, {"8", 8}, {"9", 9}, {"10", 10}, {"J", 11}, {"Q", 12}, {"K", 13} };

        int fromRank = rankValues.at(from->card->rank);
        int toRank = rankValues.at(toPile.back()->card->rank);

        // Alternating colors and descending rank
        return (fromRank + 1 == toRank) &&
            ((from->card->suit == "Hearts" || from->card->suit == "Diamonds") !=
                (toPile.back()->card->suit == "Hearts" || toPile.back()->card->suit == "Diamonds"));
    }

    void moveTableauToTableau(int fromIndex, int toIndex, int numCardsToMove)
    {
        if (fromIndex < 0 || fromIndex >= 7 || toIndex < 0 || toIndex >= 7)
        {
            cout << "Invalid tableau indices." << endl;
            return;
        }

        if (tableau[fromIndex].empty())
        {
            cout << "No cards to move from the source tableau." << endl;
            return;
        }

        if (numCardsToMove < 1 || numCardsToMove > tableau[fromIndex].size())
        {
            cout << "Invalid number of cards to move." << endl;
            return;
        }

        // Check that all cards to be moved are face up
        auto startIt = tableau[fromIndex].end() - numCardsToMove;
        for (auto it = startIt; it != tableau[fromIndex].end(); ++it)
        {
            if (!(*it)->faceUp)
            {
                cout << "Cannot move cards: a card in the sequence is face down." << endl;
                return;
            }
        }

        // Validate the move
        CardNode* firstCardToMove = *startIt;
        if (!tableau[toIndex].empty())
        {
            CardNode* targetCard = tableau[toIndex].back();
            if (!tableauValid(firstCardToMove, tableau[toIndex]))
            {
                cout << "Invalid move: Cards do not follow alternating color and descending order rules." << endl;
                return;
            }
        }
        else if (firstCardToMove->card->rank != "K")
        {
            cout << "Invalid move: Only Kings can be placed on an empty tableau." << endl;
            return;
        }

        saveState();

        // Perform the move in one efficient operation
        tableau[toIndex].insert(tableau[toIndex].end(), startIt, tableau[fromIndex].end());
        tableau[fromIndex].erase(startIt, tableau[fromIndex].end());

        // Flip the next card in the source tableau if needed
        if (!tableau[fromIndex].empty() && !tableau[fromIndex].back()->faceUp)
        {
            tableau[fromIndex].back()->faceUp = true;
        }

        cout << "Moved " << numCardsToMove << " card(s) from tableau " << (fromIndex + 1)
            << " to tableau " << (toIndex + 1) << "." << endl;
    }

    void moveTableauToFoundation(int fromIndex, int toIndex)
    {
        if (fromIndex < 0 || fromIndex >= 7 || toIndex < 0 || toIndex >= 4)
        {
            cout << "Invalid indices." << endl;
            return;
        }

        if (tableau[fromIndex].empty())
        {
            cout << "No card." << endl;
            return;
        }

        CardNode* cardToMove = tableau[fromIndex].back();
        if (!foundationValid(cardToMove, toIndex))
        {
            cout << "Invalid move." << endl;
            return;
        }

        saveState();

        // Perform the move
        foundations[toIndex].push_back(cardToMove);
        tableau[fromIndex].pop_back();

        // Flip the next card in the source tableau if needed
        if (!tableau[fromIndex].empty() && !tableau[fromIndex].back()->faceUp)
        {
            tableau[fromIndex].back()->faceUp = true;
        }

        cout << "Moved " << cardToMove->card->rank << " of " << cardToMove->card->suit
            << " from tableau " << (fromIndex + 1) << " to foundation " << (toIndex + 1) << "." << endl;
    }

    void moveWasteToTableau(int index)
    {
        if (!tableauValid(waste.back(), tableau[index]) || waste.empty())
        {
            return;
        }
        saveState();
        if (tableau[index].empty())
        {
            tableau[index].push_back(waste.back());
        }
        else
        {
            tableau[index].back()->next = waste.back();
            tableau[index].push_back(waste.back());
        }
        waste.pop_back();
    }

    void moveWasteToFoundation(int index)
    {
        if (!foundationValid(waste.back(), index) || waste.empty())
            return;
        saveState();

        if (foundations[index].empty())
        {
            foundations[index].push_back(waste.back());
        }
        else
        {
            foundations[index].back()->next = waste.back();
            foundations[index].push_back(waste.back());
        }
        waste.pop_back();
    }

    void moveFoundationToTableau(int fromIndex, int toIndex)
    {
        if (fromIndex < 0 || fromIndex >= 4 || toIndex < 0 || toIndex >= 7)
        {
            return;
        }
        if (foundations[fromIndex].empty() || !tableauValid(foundations[fromIndex].back(), tableau[toIndex]))
        {
            return;
        }
        saveState();
        tableau[toIndex].push_back(foundations[fromIndex].back());
        foundations[fromIndex].pop_back();
    }

    void decideMoveType(Vector2 mousePos)
    {
        if (mousePos.x >= 100 && mousePos.x < 800)
        {
            int targetTableau = (mousePos.x - 100) / 100;
            if (selected->tableauIndex == -1)
            {
                moveWasteToTableau(targetTableau);
            }
            else if (selected->tableauIndex > 6)
            {
                moveFoundationToTableau(selected->tableauIndex - 7, targetTableau);
            }
            else
            {
                moveTableauToTableau(selected->tableauIndex, targetTableau, selected->cardsCount);
            }
        }
        else if (mousePos.x >= 800)
        {
            int foundationIndex = (mousePos.y - 10) / 119;

            if (selected->tableauIndex == -1)
            {
                moveWasteToFoundation(foundationIndex);
            }
            else
            {
                moveTableauToFoundation(selected->tableauIndex, foundationIndex);
            }
        }
        selected = nullptr;
    }

    // when the user clicks reset button it restarts the game
    void resetGame()
    {
        // delete all cards
        saveState();
        clearPile(waste);
        clearPile(stock);
        for (int i = 0; i < 4; i++)
        {
            clearPile(foundations[i]);
        }
        for (int i = 0; i < 7; i++)
        {
            clearPile(tableau[i]);
        }
        selected = nullptr;
        // and then start over

        initializeDeck();
        shuffleDeck();
        setupTableau();
    }

    bool gameIsWon()
    {
        for (const auto& foundation : foundations)
        {
            if (foundation.size() != 13) // chechs 13 cards A-K in each foundation
                return false;
        }
        return true;
    }

    void DrawFront(insideCard* card, Vector2 position, int from, int count = 1)
    {
        map<string, int> suits = {
            {"Hearts", 0},
            {"Clubs", 1},
            {"Diamonds", 2},
            {"Spades", 3},
        };
        map<string, int> ranks = {
            {"2", 0},
            {"3", 1},
            {"4", 2},
            {"5", 3},
            {"6", 4},
            {"7", 5},
            {"8", 6},
            {"9", 7},
            {"10", 8},
            {"J", 9},
            {"Q", 10},
            {"K", 11},
            {"A", 12},  
        };
        DrawTexturePro(atlas, { ranks[card->rank] * 140.0f + 8, suits[card->suit] * 188.0f + 8, 132, 180 }, { position.x, position.y, 80, 109 }, { 0, 0 }, 0.0f, WHITE);
        ClickableCard* clickableCard = new ClickableCard(position, from, count);
        drawStack.push(clickableCard);
    }

    void DrawBack(Vector2 position)
    {
        DrawTexturePro(atlas, { 1828, 572, 132, 180 }, { position.x, position.y, 80, 109 }, { 0, 0 }, 0.0f, WHITE);
    }
};
int main()
{
    Solitaire game;
    
    const int screenWidth = 900;
    const int screenHeight = 486;
    const Color BG_GREEN = { 52, 162, 73, 255 };
    const Color DARK_GREEN = { 43, 123, 59, 255 };

    InitWindow(screenWidth, screenHeight, "Solitaire");
    game.atlas = LoadTexture("cards.png");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (CheckCollisionPointRec(mousePos, { 10, 10, 80, 109 }))
            {
                game.stockWaste();
            }
            else if (CheckCollisionPointRec(mousePos, { 10, 396, 80, 80 }))
            {
                game.resetGame();
            }
            else if (CheckCollisionPointRec(mousePos, { 10, 306, 80, 80 }))
            {
                game.undo();
            }
            else if (game.selected)
            {
                game.decideMoveType(mousePos);
            }
            else
            {
                while (!game.drawStack.empty())
                {
                    if (CheckCollisionPointRec(mousePos, game.drawStack.top()->bounds))
                    {
                        Vector2 position = { game.drawStack.top()->bounds.x, game.drawStack.top()->bounds.y };
                        game.selected = new ClickableCard(position, game.drawStack.top()->tableauIndex, game.drawStack.top()->cardsCount);
                        break;
                    }
                    else
                    {
                        game.drawStack.pop();
                    }
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            game.selected = nullptr;
        }

        BeginDrawing();

        // Initialize Background
        ClearBackground(BG_GREEN);
        DrawRectangle(0, 0, 100, 500, DARK_GREEN);
        DrawRectangle(800, 0, 100, 500, DARK_GREEN);

        // Foundations & Stock Background
        DrawTexturePro(game.atlas, { 1828, 196, 132, 180 }, { 810, 10, 80, 109 }, { 0, 0 }, 0.0f, WHITE);
        DrawTexturePro(game.atlas, { 1828, 196, 132, 180 }, { 810, 129, 80, 109 }, { 0, 0 }, 0.0f, WHITE);
        DrawTexturePro(game.atlas, { 1828, 196, 132, 180 }, { 810, 248, 80, 109 }, { 0, 0 }, 0.0f, WHITE);
        DrawTexturePro(game.atlas, { 1828, 196, 132, 180 }, { 810, 367, 80, 109 }, { 0, 0 }, 0.0f, WHITE);
        DrawTexturePro(game.atlas, { 1828, 196, 132, 180 }, { 10, 10, 80, 109 }, { 0, 0 }, 0.0f, WHITE);

        // Undo & Reset Buttons
        DrawTexturePro(game.atlas, { 1968, 572, 132, 132 }, { 10, 306, 80, 80 }, { 0, 0 }, 0.0f, WHITE);
        DrawTexturePro(game.atlas, { 1968, 384, 132, 132 }, { 10, 396, 80, 80 }, { 0, 0 }, 0.0f, WHITE);

        stack<ClickableCard*>().swap(game.drawStack); // Clear drawStack before drawing new cards

        // Draw Stock & Waste
        if (!game.stock.empty())
        {
            game.DrawBack({ 10, 10 });
        }

        if (!game.waste.empty())
        {
            game.DrawFront(game.waste.back()->card, { 10, 129 }, -1);
        }

        // Draw Foundations
        for (int i = 0; i < 4; i++)
        {
            if (!game.foundations[i].empty())
            {
                game.DrawFront(game.foundations[i].back()->card, { 810, i * 119.0f + 10 }, 7 + i);
            }
        }

        // Draw Tableaus
        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < game.tableau[i].size(); j++)
            {
                if (!game.tableau[i][j]->faceUp)
                {
                    game.DrawBack({ i * 100.0f + 110, j * 30.0f + 10 });
                }
                else
                {
                    game.DrawFront(game.tableau[i][j]->card, { i * 100.0f + 110, j * 30.0f + 10 }, i, game.tableau[i].size() - j);
                }
            }
        }

        // Selection Outline
        if (game.selected)
        {
            Vector2 position = { game.selected->bounds.x, game.selected->bounds.y };
            int numOfCards = game.selected->cardsCount;
            DrawTexturePro(game.atlas, { 1828, 8, 132, 50 }, { position.x, position.y, 80, 30 }, { 0, 0 }, 0.0f, WHITE);
            for (int i = 1; i < numOfCards; i++)
            {
                DrawTexturePro(game.atlas, { 1828, 16, 132, 50 }, { position.x, position.y + 30.0f * i, 80, 30 }, { 0, 0 }, 0.0f, WHITE);
            }
            DrawTexturePro(game.atlas, { 1828, 58, 132, 130 }, { position.x, position.y + 30.0f * numOfCards, 80, 79 }, { 0, 0 }, 0.0f, WHITE);
        }

        EndDrawing();
    }

    UnloadTexture(game.atlas);
    CloseWindow();

    return 0;
}
