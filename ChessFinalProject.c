#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h> 
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define TILE_SIZE  70
#define SCREEN_WIDTH  (TILE_SIZE * 8)
#define SCREEN_HEIGHT (TILE_SIZE * 8)

#define NUM_PIECES  12

typedef enum {
    EMPTY = -1,
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
} PieceType;

PieceType board[8][8] = {
    {BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK},
    {BLACK_PAWN, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,  BLACK_PAWN, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN},
    {EMPTY,      EMPTY,        EMPTY ,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY},
    {EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY},
    {EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY},
    {EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY},
    {WHITE_PAWN, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,  WHITE_PAWN, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN},
    {WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK}
};

SDL_Texture **loadPieceTextures(SDL_Renderer *renderer) {
    static const char *paths[NUM_PIECES] = {
        "pieces/wp.png", "pieces/wn.png",  "pieces/wb.png",
        "pieces/wr.png", "pieces/wq.png", "pieces/wk.png",
        "pieces/bp.png", "pieces/bn.png", "pieces/bb.png",
        "pieces/br.png", "pieces/bq.png", "pieces/bk.png"
    };

    SDL_Texture **textures = malloc(NUM_PIECES * sizeof(*textures));
    if (!textures) {
        printf("Failed to allocate memory for textures.\n");
        return NULL;
    }

    for (int i = 0; i < NUM_PIECES; i++) {
        textures[i] = IMG_LoadTexture(renderer, paths[i]);
        if (!textures[i]) {
            printf("Failed to load %s: %s\n", paths[i], SDL_GetError());
        }
    }

    return textures;
}

void drawChessboard(SDL_Renderer* renderer, SDL_Texture** pieceTextures) {
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderClear(renderer);
      
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            SDL_FRect tile = { col * TILE_SIZE , row * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            if ((row + col) % 2 == 0)
                SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255);
            else
                SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);

            SDL_RenderFillRect(renderer, &tile);

            PieceType piece = board[row][col];
            if (piece != EMPTY && pieceTextures[piece])
                SDL_RenderTexture(renderer, pieceTextures[piece], NULL, &tile);
        }
    }

    SDL_RenderPresent(renderer);
}

bool getTileFromMouse(int x, int y, int *row, int *col) {
    if (x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
     return false;
    *col = x / TILE_SIZE;
    *row = y / TILE_SIZE;
    return true;
}

bool isWhitePiece(PieceType p) {
    return p >= WHITE_PAWN && p <= WHITE_KING;
}

bool isBlackPiece(PieceType p) {
    return p >= BLACK_PAWN && p <= BLACK_KING;
}

bool isSameColor(PieceType a, PieceType b) {
    return (isWhitePiece(a) && isWhitePiece(b)) || (isBlackPiece(a) && isBlackPiece(b));
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return 1;
    }
    

    SDL_Window* window = SDL_CreateWindow("Chess Game", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("Window Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture **pieces = loadPieceTextures(renderer);
    if (!pieces) {
        printf("Error loading piece textures!\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    
    int running = 1;
    SDL_Event e;
    int selectedRow = -1, selectedCol = -1;
    bool whiteTurn = true;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT)
                running = 0;
                  
            else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                int row, col;
                
                if (getTileFromMouse(e.button.x, e.button.y, &row, &col)) {
                    PieceType piece = board[row][col];

                    if (selectedRow == -1) {
                        if ((whiteTurn && isWhitePiece(piece)) || (!whiteTurn && isBlackPiece(piece))) {
                            selectedRow = row;
                            selectedCol = col;
                        }
                    } else {
                        PieceType selPiece = board[selectedRow][selectedCol];
                        bool validMove = false;

                        int dir = (whiteTurn ? -1 : 1);
                        int startRow = (whiteTurn ? 6 : 1);

                        // Pawn
                        if (selPiece == WHITE_PAWN || selPiece == BLACK_PAWN) {
                            // Forward move
                            if (col == selectedCol) {
                                if (row == selectedRow + dir && board[row][col] == EMPTY)
                                    validMove = true;
                                else if (selectedRow == startRow && row == selectedRow + 2 * dir && board[selectedRow + dir][col] == EMPTY && board[row][col] == EMPTY)
                                    validMove = true;
                            }
                            // Capture diagonally
                            else if (abs(col - selectedCol) == 1 && row == selectedRow + dir) {
                                if (board[row][col] != EMPTY && !isSameColor(selPiece, board[row][col]))
                                    validMove = true;
                            }
                        }

                        // Rook
                        if (selPiece == WHITE_ROOK || selPiece == BLACK_ROOK) {
                            if (row == selectedRow || col == selectedCol) {
                                int rStep = (row > selectedRow) ? 1 : (row < selectedRow ? -1 : 0);
                                int cStep = (col > selectedCol) ? 1 : (col < selectedCol ? -1 : 0);
                                int r = selectedRow + rStep, c = selectedCol + cStep;
                                bool blocked = false;
                                while (r != row || c != col) {
                                    if (board[r][c] != EMPTY) { blocked = true; break; }
                                    r += rStep; c += cStep;
                                }
                                if (!blocked && (!isSameColor(selPiece, board[row][col])))
                                    validMove = true;
                            }
                            printf("Rook Move\n");
                        }

                        // Bishop
                        if (selPiece == WHITE_BISHOP || selPiece == BLACK_BISHOP) {
                            if (abs(row - selectedRow) == abs(col - selectedCol)) {
                                int rStep = (row > selectedRow) ? 1 : -1;
                                int cStep = (col > selectedCol) ? 1 : -1;
                                int r = selectedRow + rStep, c = selectedCol + cStep;
                                bool blocked = false;
                                while (r != row && c != col) {
                                    if (board[r][c] != EMPTY) { blocked = true; break; }
                                    r += rStep; c += cStep;
                                }
                                if (!blocked && (!isSameColor(selPiece, board[row][col])))
                                    validMove = true;
                            }
                            printf("Bishop Move\n");
                        }

                        // Knight
                        if (selPiece == WHITE_KNIGHT || selPiece == BLACK_KNIGHT) {
                            int dr = abs(row - selectedRow);
                            int dc = abs(col - selectedCol);
                            if ((dr == 2 && dc == 1) || (dr == 1 && dc == 2)) {
                                if (!isSameColor(selPiece, board[row][col]))
                                    validMove = true;
                            }   
                        }

                        // Queen
                        if (selPiece == WHITE_QUEEN || selPiece == BLACK_QUEEN) {
                            if (row == selectedRow || col == selectedCol || abs(row - selectedRow) == abs(col - selectedCol)) {
                                int rStep = (row > selectedRow) ? 1 : (row < selectedRow ? -1 : 0);
                                int cStep = (col > selectedCol) ? 1 : (col < selectedCol ? -1 : 0);
                                int r = selectedRow + rStep, c = selectedCol + cStep;
                                bool blocked = false;
                                while (r != row || c != col) {
                                    if (board[r][c] != EMPTY) { blocked = true; break; }
                                    r += rStep; c += cStep;
                                }
                                if (!blocked && (!isSameColor(selPiece, board[row][col])))
                                    validMove = true;
                            }
                        }

                        // King
                        if (selPiece == WHITE_KING || selPiece == BLACK_KING) {
                            int dr = abs(row - selectedRow);
                            int dc = abs(col - selectedCol);
                            if (dr <= 1 && dc <= 1) {
                                if (!isSameColor(selPiece, board[row][col]))
                                    validMove = true;
                            }
                        }

                        if (validMove) {
                            board[row][col] = selPiece;
                            board[selectedRow][selectedCol] = EMPTY;
                            whiteTurn = !whiteTurn;
                            
                            // Check if either king is captured
                            int whiteKingExists = 1, blackKingExists = 1;
                            for (int r = 0; r < 8; r++) {
                               for (int c = 0; c < 8; c++) {
                                  if (board[r][c] == WHITE_KING) whiteKingExists = 0;
                                  if (board[r][c] == BLACK_KING) blackKingExists = 0;
                                }
                            }
                             if (whiteKingExists || blackKingExists) {
                             printf("Game Ended\n");
                             running = 0;  // Exit main loop
                             }
                            
                        }

                        selectedRow = selectedCol = -1;
                    }
                }
            }
        }

        drawChessboard(renderer, pieces);
        SDL_Delay(16);
    }

    for (int i = 0; i < NUM_PIECES; i++)
        if (pieces[i]) SDL_DestroyTexture(pieces[i]);
    free(pieces);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}    