#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_rotozoom.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  int screenW, screenH, halfW;
  int worldW, worldH;
  int playerDrawW, playerDrawH;
  int collW, collH;
  int groundY;
  int frameCount;
  int maxObstacles, maxPickups, maxProjectiles;
  int maxEnemies, maxLevels, maxPuzzles, maxQuestions;
  int obsCellW, obsCellH;
  int itemCellW, itemCellH;
  int obsRowH, obsRowStride;
  int obsCrateX, obsCrateW;
  int obsRoadX, obsRoadW;
  int obsConeX, obsConeW;
  int itemMedX, itemMedW;
  int itemAmmoX, itemAmmoW;
  int itemVestX, itemVestW;
  int itemCoinX, itemCoinW;
  int itemRowY, itemRowH;

  int stateMenu;
  int statePlatformer;
  int stateEnigme1;
  int stateEnigme2;
  int stateChoice;

  int enigmeAttente, enigmeCorrect, enigmeFaux;

  int animStand, animWalk, animRun, animJump, animAttack, animDead, animCount;

  int dirRight, dirLeft, dirCount;

  int obsTypeCrate, obsTypeRoadblock, obsTypeCone;

  int itemMedkit, itemAmmo, itemVest, itemCoin;

  int projW, projH, projSpeed, projDamageNormal, projDamageSuper;

  int gunOffsetX, gunOffsetY;

  int multiplayer;

  int volume;
} GameConfig;

typedef struct {
  SDL_Texture *texture;
  int frameW, frameH;
  int frameCount, currentFrame;
  int animSpeed, animTimer;
} SpriteSheet;

typedef struct {
  int x, y, w, h;
} Camera;

typedef struct {
  SDL_Texture *sheetTex;
  int obsType, damageRow;
  SDL_Rect pos;
  int drawW, drawH, active;
} Obstacle;

typedef struct {
  SDL_Texture *sheetTex;
  int itemType;
  SDL_Rect pos;
  int drawW, drawH, collected;
} Pickup;

typedef struct {
  double x, y, vx;
  int active, isSuper, damage;

  int fromPlayer;
  SDL_Rect pos;
} Projectile;

typedef struct {
  double x, y, vx;
  int direction, active, vie, maxVie;
  int patrolLeft, patrolRight;
  int drawW, drawH;
  int enemyType;
  SpriteSheet walkRight, walkLeft;
  SpriteSheet attackRight, attackLeft;
  SpriteSheet deadSheet;
  int dead, deadTimer;
  int invincTimer;
  int attackCooldown, attacking, attackAnimTimer;
  SDL_Rect posScreen;
  int aggroed;
  int deAggroTimer;
  int fleeing;
  int aggroRange;
  int deAggroRange;
} Enemy;

typedef struct {
  double x, y, vx, vy;
  int onGround, jumping, direction, etat, prevEtat;
  int vie, maxVie, shield, score, superAmmo;
  int drawW, drawH;
  SpriteSheet sheets[2][6];
  SDL_Rect posScreen;
  int playerIndex, attackCooldown;
} Personnage;

typedef struct {
  SDL_Texture *img;
} Background;

typedef struct {
  SDL_Rect mapRect;
  SDL_Rect p1Dot, p2Dot;
} Minimap;

typedef struct {
  char q[200], r1[100], r2[100], r3[100];
  int ans;
} QuizBlock;
typedef struct {
  int shuffled[64], shufflePos, lastNb;
} ShuffleState;
typedef struct {
  char question[200], rep1_text[100], rep2_text[100], rep3_text[100];
  int numbr;
  SDL_Texture *questionTexture, *rep1Texture, *rep2Texture, *rep3Texture;
  SDL_Rect posQuestion, posRep1, posRep2, posRep3;
  SDL_Rect posRep1Text, posRep2Text, posRep3Text;
  SDL_Texture *btn[3], *btnHover[3];
  int etat;
} Enigme;

typedef struct {
  SDL_Texture *bgTex;
  SDL_Texture *answerTex[3];
  int correctAns;
  int bgW, bgH;
  int ansW[3], ansH[3];
} DragPuzzle;

typedef struct {
  DragPuzzle puzzles[5];
  int count, order[5], orderPos;
  int active, currentPuzzle;
  SDL_Rect bgRect, slotRect, answerRects[3];
  int heldAnswer, dragOffX, dragOffY;
  SDL_Rect heldRect;
  int answered, correct;
  SDL_Texture *successTex;
  SDL_Texture *failTex;
} Enigme2;

typedef struct {
  Background bg;
  SDL_Texture *obsSpriteSheet, *itemSpriteSheet;
  Obstacle obstacles[24];
  int nbObstacles;
  Pickup pickups[12];
  int nbPickups;
  Enemy enemies[4];
  int nbEnemies;
  int groundY, damageRow;
  int worldW;
} Level;

typedef struct {
  char name[32];
  int score;
} ScoreEntry;

typedef struct {
  SDL_Texture *normal;
  SDL_Texture *hover;
  SDL_Rect rect;
  int hovered;
} MenuButton;

typedef struct {
  int menuScreen;

  SDL_Texture *bg[4];

  Mix_Music *musicMain;
  Mix_Music *musicSub;
  Mix_Music *musicVictory;
  Mix_Music *musicSuspense;
  Mix_Music *musicLevel;
  Mix_Music *musicCombat;

  Mix_Chunk *sfxHover;

  MenuButton btnJouer;
  MenuButton btnOptions;
  MenuButton btnScores;
  MenuButton btnHistoire;
  MenuButton btnQuitter;
  MenuButton btnVolUp;
  MenuButton btnVolDown;
  MenuButton btnFullscreen;
  MenuButton btnNormal;
  MenuButton btnRetourOpt;

  MenuButton btnOui;
  MenuButton btnNon;
  MenuButton btnCharger;
  MenuButton btnNouvelle;

  MenuButton btnMono;
  MenuButton btnMulti;
  MenuButton btnAvatar1;
  MenuButton btnAvatar2;
  MenuButton btnInput1;
  MenuButton btnInput2;
  MenuButton btnValiderPlayer;
  MenuButton btnRetourPlayer;

  MenuButton btnValiderScore;
  MenuButton btnRetourScore;
  MenuButton btnQuitterScore;

  MenuButton btnQuiz;
  MenuButton btnPuzzle;

  MenuButton btnRetourHistoire;
  MenuButton btnHistoireUp;
  MenuButton btnHistoireDown;

  MenuButton btnReprendre;
  MenuButton btnSauvegarder;
  MenuButton btnOptionsPause;
  MenuButton btnQuitterPartie;

  char nameInput[32];
  int nameLen;
  int nameActive;

  int chosenAvatar;
  int chosenInput;

  SDL_Texture *previewAvatar1;
  SDL_Texture *previewAvatar2;

  ScoreEntry topScores[10];
  int nbTopScores;

  int pendingScore;
  int hasPendingScore;

  int quizActive;
  int selectedKey;

  int optionsReturn;

  char histoireText[8192];
  int histoireLen;
  int histoireScroll;

  int hasSavedGame;
} MenuState;

typedef struct {
  GameConfig cfg;
  SDL_Window *window;
  SDL_Renderer *renderer;
  TTF_Font *font;
  TTF_Font *fontLarge;

  SDL_Texture *heartTex, *heartEmptyTex;

  Personnage player1, player2;
  Camera cam1, cam2;

  Level levels[3];
  int currentLevel, worldW, worldH;

  Minimap minimap;
  Projectile projectiles[16];

  int gameStartTick, elapsedSeconds;

  Enigme enigme1;
  QuizBlock quizBlocks[64];
  int quizCount, enigme1Active;
  SDL_Texture *enigme1Bg;
  ShuffleState shuffleState;

  Enigme2 enigme2;

  int gameState, running;
  int feedbackStart, enigmeStart, selectedKey;
  int enigmeFromCoin;
  int currentMusicState;
  int combatMusicCooldown;
  MenuState menu;

  int isFullscreen;
} GameContext;

void initConfig(GameConfig *cfg, int multiplayer, int screenW, int screenH);
void loadSpriteSheet(SpriteSheet *s, SDL_Renderer *r, char *path,
                     int frameCount, int animSpeed);
void updateSpriteSheet(SpriteSheet *s, int dt);
void renderSpriteSheet(SDL_Renderer *r, SpriteSheet *s, int x, int y, int w,
                       int h);
void freeSpriteSheet(SpriteSheet *s);
SDL_Texture *makeText(SDL_Renderer *r, TTF_Font *f, char *text, SDL_Color col,
                      SDL_Rect *out);
SDL_Texture *loadTexture(SDL_Renderer *r, char *path);
void updateCamera(Camera *cam, SDL_Rect target, int worldW, int worldH,
                  GameConfig *cfg, int split);

/* player */
void initPersonnage(Personnage *p, SDL_Renderer *r, GameConfig *cfg, int x,
                    int y, int idx);
void movePersonnage(Personnage *p, Obstacle obs[], int nb, GameConfig *cfg);
void animerPersonnage(Personnage *p, int dt);
void pollPlayerMovement(Personnage *p, GameConfig *cfg, int isP2);
void syncAnimState(Personnage *p, GameConfig *cfg);
void updateGameMusic(GameContext *ctx, int anyAggro, int dt);
void afficherPersonnage(Personnage *p, SDL_Renderer *r, Camera cam,
                        SDL_Rect vp);
void getGunBarrelPos(Personnage *p, GameConfig *cfg, int *outX, int *outY);
void freePersonnage(Personnage *p);

/* projectile */
void fireProjectile(Projectile projs[], int max, int worldX, int worldY,
                    int facingRight, int isSuper, int fromPlayer,
                    GameConfig *cfg);
void updateProjectiles(Projectile projs[], int max, Enemy *enemies[], int nbE,
                       Obstacle *obs[], int nbO, SDL_Rect playerBoxes[],
                       int nbPlayers, int playerVie[], int playerShield[],
                       GameConfig *cfg, int dt);
void renderProjectiles(SDL_Renderer *r, Projectile projs[], int max, Camera cam,
                       SDL_Rect vp);

/* background */
void loadBackground(Background *b, SDL_Renderer *r, char *path);
void afficherBackground(Background *b, SDL_Renderer *r, Camera cam, SDL_Rect vp,
                        int worldW, int worldH);
void freeBackground(Background *b);

/* obstacle et item */
void initObstacleFromSheet(Obstacle *o, SDL_Texture *sheet, int obsType,
                           int damageRow, int worldX, int worldY, int drawW,
                           int drawH, GameConfig *cfg);
void afficherObstacle(Obstacle *o, SDL_Renderer *r, Camera cam, SDL_Rect vp,
                      GameConfig *cfg);
void initPickup(Pickup *pk, SDL_Texture *sheet, int itemType, int worldX,
                int worldY, GameConfig *cfg);
void afficherPickup(Pickup *pk, SDL_Renderer *r, Camera cam, SDL_Rect vp,
                    GameConfig *cfg);
int checkPickupCollision(SDL_Rect *playerBox, Pickup pickups[], int nb);

/* enemy */
void initEnemy(Enemy *e, SDL_Renderer *r, GameConfig *cfg, int worldX,
               int patrolLeft, int patrolRight, int enemyType);
void updateEnemy(Enemy *e, SDL_Rect playerBoxes[], int nbPlayers,
                 double playerVy[], int playerVie[], double playerX[],
                 int playerShield[], int playerScore[], double playerVxOut[],
                 double playerVyOut[], Obstacle obs[], int nbObs,
                 Projectile projs[], int maxProjs, GameConfig *cfg, int dt);
void afficherEnemy(Enemy *e, SDL_Renderer *r, Camera cam, SDL_Rect vp);
void freeEnemy(Enemy *e);
void getEnemyGunBarrelPos(Enemy *e, GameConfig *cfg, int *outX, int *outY);

/* minimap */
void initMinimap(Minimap *m, GameConfig *cfg);
void updateMinimap(Minimap *m, SDL_Rect p1, SDL_Rect p2, int worldW, int worldH,
                   GameConfig *cfg);
void afficherMinimap(SDL_Renderer *r, Minimap *m, Obstacle obs[], int nbObs,
                     int worldW, int worldH, GameConfig *cfg);

/* HUD */
void afficherHUD(SDL_Renderer *r, TTF_Font *f, int vie, int maxVie, int shield,
                 int superAmmo, int score, int level, int elapsedSec,
                 GameConfig *cfg, SDL_Rect vp, Minimap *minimap, Obstacle obs[],
                 int nbObs, int worldW, int worldH, SDL_Texture *heartTex,
                 SDL_Texture *heartEmptyTex);

/* level */
void loadLevel(Level *lv, SDL_Renderer *r, GameConfig *cfg, int lvIdx);
void updateLevel(Level *lv, SDL_Rect playerBoxes[], int nbPlayers,
                 double playerVy[], int playerVie[], double playerX[],
                 int playerShield[], int playerScore[], double playerVxOut[],
                 double playerVyOut[], Projectile projs[], int maxProjs,
                 GameConfig *cfg, int dt, int *anyAggroOut);
void afficherLevel(Level *lv, SDL_Renderer *r, Camera cam, SDL_Rect vp,
                   GameConfig *cfg);
void freeLevel(Level *lv);

/* enigme 1 */
void shuffleIndices(ShuffleState *s, int nb);
int loadQuizBlocks(char *file, QuizBlock blocks[], int max);
Enigme genererEnigme(QuizBlock blocks[], int nb, ShuffleState *s,
                     SDL_Renderer *r, TTF_Font *f, GameConfig *cfg);
void afficherEnigme(Enigme *e, SDL_Renderer *r, int mx, int my,
                    GameConfig *cfg);
void afficherKeyboardHints(SDL_Renderer *r, TTF_Font *f, int selected,
                           GameConfig *cfg);
void afficherFeedback(SDL_Renderer *r, TTF_Font *f, Enigme *e,
                      int feedbackStart, GameConfig *cfg);
int resolveEnigme(int choix, Enigme *e, int *score, int *vie, GameConfig *cfg);
void updateTimerBar(SDL_Renderer *r, int startTime, GameConfig *cfg);
void libererEnigme(Enigme *e);

/* enigme 2 */
void playRotozoom(SDL_Renderer *ren, SDL_Texture *tex, GameConfig *cfg);
void playEnigme2Result(SDL_Renderer *ren, SDL_Texture *bgTex,
                       SDL_Texture *resultTex, GameConfig *cfg);
SDL_Rect fitRect(int boxX, int boxY, int boxW, int boxH, int srcW, int srcH);
void initEnigme2(Enigme2 *e2, SDL_Renderer *r, GameConfig *cfg);
void triggerEnigme2(Enigme2 *e2, SDL_Renderer *r, GameConfig *cfg);
void afficherEnigme2(Enigme2 *e2, SDL_Renderer *r, TTF_Font *f,
                     GameConfig *cfg);
void handleEnigme2Mouse(GameContext *ctx, Enigme2 *e2, SDL_Event *ev);
void updateEnigme2Drag(GameContext *ctx, Enigme2 *e2);
void freeEnigme2(Enigme2 *e2);

/* menu */
void initMenu(MenuState *m, SDL_Renderer *r, TTF_Font *f, GameConfig *cfg);
void handleMenuEvents(GameContext *ctx, SDL_Event *ev);
void updateMenu(GameContext *ctx);
void renderMenu(GameContext *ctx);
void freeMenu(MenuState *m);
int loadTopScores(ScoreEntry scores[], int max);
void saveScore(char *name, int score);
void menuButtonCheck(MenuButton *btn, int mx, int my, Mix_Chunk *sfx,
                     int *wasHovered);

/* save-game */
void saveGameState(GameContext *ctx);
int loadGameState(GameContext *ctx);
int hasSavedGame();

/* histoire */
int loadHistoireText(char *buf, int bufLen);

/* game */
int initGame(GameContext *ctx, int multiplayer);
void handleEvents(GameContext *ctx, SDL_Event *ev);
void updateGame(GameContext *ctx, int dt);
void renderGame(GameContext *ctx);
void cleanupGame(GameContext *ctx);

#endif
