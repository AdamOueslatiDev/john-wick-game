#include "game.h"

void mouseToLogical(GameContext *ctx, int *mx, int *my) {
  int winW, winH, logW, logH, scaleN, scaleD, drawW, drawH, offX, offY;
  SDL_GetWindowSize(ctx->window, &winW, &winH);
  logW = ctx->cfg.screenW;
  logH = ctx->cfg.screenH;
  if (winW <= 0 || winH <= 0 || logW <= 0 || logH <= 0)
    return;
  if (winW * logH < logW * winH) {
    scaleN = winW;
    scaleD = logW;
  } else {
    scaleN = winH;
    scaleD = logH;
  }
  drawW = logW * scaleN / scaleD;
  drawH = logH * scaleN / scaleD;
  offX = (winW - drawW) / 2;
  offY = (winH - drawH) / 2;
  if (drawW > 0)
    *mx = (*mx - offX) * logW / drawW;
  if (drawH > 0)
    *my = (*my - offY) * logH / drawH;
}

void initConfig(GameConfig *cfg, int multiplayer, int screenW, int screenH) {
  cfg->screenW = screenW;
  cfg->screenH = screenH;
  cfg->halfW = screenW / 2;
  cfg->worldW = screenW * 4;
  cfg->worldH = screenH;

  cfg->playerDrawW = 72;
  cfg->playerDrawH = 90;
  cfg->collW = 40;
  cfg->collH = 80;
  cfg->groundY = screenH * 815 / 1000;

  cfg->frameCount = 4;
  cfg->maxObstacles = 24;
  cfg->maxPickups = 12;
  cfg->maxProjectiles = 16;
  cfg->maxEnemies = 4;
  cfg->maxLevels = 3;
  cfg->maxPuzzles = 5;
  cfg->maxQuestions = 64;

  cfg->obsCellW = 256;
  cfg->obsCellH = 256;
  cfg->itemCellW = 256;
  cfg->itemCellH = 256;

  cfg->obsRowH = 252;
  cfg->obsRowStride = 254;
  cfg->obsCrateX = 1;
  cfg->obsCrateW = 248;
  cfg->obsRoadX = 255;
  cfg->obsRoadW = 507;
  cfg->obsConeX = 796;
  cfg->obsConeW = 194;
  cfg->itemMedX = 0;
  cfg->itemMedW = 223;
  cfg->itemAmmoX = 288;
  cfg->itemAmmoW = 157;
  cfg->itemVestX = 525;
  cfg->itemVestW = 198;
  cfg->itemCoinX = 752;
  cfg->itemCoinW = 247;
  cfg->itemRowY = 1;
  cfg->itemRowH = 246;

  cfg->stateMenu = 0;
  cfg->statePlatformer = 1;
  cfg->stateEnigme1 = 2;
  cfg->stateEnigme2 = 3;
  cfg->stateChoice = 4;
  cfg->enigmeAttente = 0;
  cfg->enigmeCorrect = 1;
  cfg->enigmeFaux = 2;

  cfg->animStand = 0;
  cfg->animWalk = 1;
  cfg->animRun = 2;
  cfg->animJump = 3;
  cfg->animAttack = 4;
  cfg->animDead = 5;
  cfg->animCount = 6;

  cfg->dirRight = 0;
  cfg->dirLeft = 1;
  cfg->dirCount = 2;

  cfg->obsTypeCrate = 0;
  cfg->obsTypeRoadblock = 1;
  cfg->obsTypeCone = 2;

  cfg->itemMedkit = 0;
  cfg->itemAmmo = 1;
  cfg->itemVest = 2;
  cfg->itemCoin = 3;

  cfg->projW = 18;
  cfg->projH = 7;
  cfg->projSpeed = 14;
  cfg->projDamageNormal = 1;
  cfg->projDamageSuper = 3;

  cfg->gunOffsetX = 55;
  cfg->gunOffsetY = 38;

  cfg->multiplayer = multiplayer;
  cfg->volume = 80;
}

void loadSpriteSheet(SpriteSheet *s, SDL_Renderer *r, char *path,
                     int frameCount, int animSpeed) {
  SDL_Surface *surf;

  s->texture = NULL;
  s->frameW = 0;
  s->frameH = 0;
  s->frameCount = frameCount;
  s->currentFrame = 0;
  s->animSpeed = animSpeed;
  s->animTimer = 0;

  surf = IMG_Load(path);
  if (!surf) {
    fprintf(stderr, "loadSpriteSheet: %s -> %s\n", path, IMG_GetError());
    return;
  }
  s->texture = SDL_CreateTextureFromSurface(r, surf);
  if (frameCount > 0)
    s->frameW = surf->w / frameCount;
  else
    s->frameW = surf->w;
  s->frameH = surf->h;
  SDL_FreeSurface(surf);
}

void updateSpriteSheet(SpriteSheet *s, int dt) {
  if (!s->texture)
    return;
  if (s->animSpeed == 0)
    return;
  s->animTimer = s->animTimer + dt;
  if (s->animTimer >= s->animSpeed) {
    s->animTimer = 0;
    s->currentFrame = s->currentFrame + 1;
    if (s->currentFrame >= s->frameCount)
      s->currentFrame = 0;
  }
}

void renderSpriteSheet(SDL_Renderer *r, SpriteSheet *s, int x, int y, int w,
                       int h) {
  SDL_Rect src, dst;
  if (!s->texture)
    return;
  src.x = s->currentFrame * s->frameW;
  src.y = 0;
  src.w = s->frameW;
  src.h = s->frameH;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;
  SDL_RenderCopy(r, s->texture, &src, &dst);
}

void freeSpriteSheet(SpriteSheet *s) {
  if (s->texture)
    SDL_DestroyTexture(s->texture);
  s->texture = NULL;
}

SDL_Texture *makeText(SDL_Renderer *r, TTF_Font *f, char *text, SDL_Color col,
                      SDL_Rect *out) {
  SDL_Surface *surf;
  SDL_Texture *tex;
  surf = TTF_RenderUTF8_Blended(f, text, col);
  if (!surf)
    return NULL;
  tex = SDL_CreateTextureFromSurface(r, surf);
  if (out) {
    out->w = surf->w;
    out->h = surf->h;
  }
  SDL_FreeSurface(surf);
  return tex;
}

SDL_Texture *loadTexture(SDL_Renderer *r, char *path) {
  SDL_Surface *s;
  SDL_Texture *t;
  s = IMG_Load(path);
  if (!s) {
    fprintf(stderr, "loadTexture: %s -> %s\n", path, IMG_GetError());
    return NULL;
  }
  t = SDL_CreateTextureFromSurface(r, s);
  SDL_FreeSurface(s);
  return t;
}

void updateCamera(Camera *cam, SDL_Rect target, int worldW, int worldH,
                  GameConfig *cfg, int split) {
  int vw;
  if (split)
    vw = cfg->halfW;
  else
    vw = cfg->screenW;
  cam->w = vw;
  cam->h = cfg->screenH;
  cam->x = target.x + target.w / 2 - vw / 2;
  cam->y = 0;
  if (cam->x < 0)
    cam->x = 0;
  if (cam->x > worldW - vw)
    cam->x = worldW - vw;
  if (worldH > 0) {
  }
}

void initPersonnage(Personnage *p, SDL_Renderer *r, GameConfig *cfg, int x,
                    int y, int idx) {
  int d, s;
  char path[256];
  char *folder;
  char *dirs[2];
  char *anims[6];
  int speeds[6];

  memset(p, 0, sizeof(Personnage));
  p->drawW = cfg->playerDrawW;
  p->drawH = cfg->playerDrawH;
  p->x = x;
  p->y = y;
  p->vie = 5;
  p->maxVie = 5;
  p->direction = cfg->dirRight;
  p->etat = cfg->animStand;
  p->prevEtat = cfg->animStand;
  p->playerIndex = idx;

  p->posScreen.x = x;
  p->posScreen.y = y;
  p->posScreen.w = cfg->collW;
  p->posScreen.h = cfg->collH;

  if (idx == 0)
    folder = "images/Fighter";
  else
    folder = "images/Fighter2";

  dirs[cfg->dirRight] = "Right";
  dirs[cfg->dirLeft] = "Left";

  anims[cfg->animStand] = "Stand.png";
  anims[cfg->animWalk] = "Walk.png";
  anims[cfg->animRun] = "Walk.png";
  anims[cfg->animJump] = "Jump.png";
  anims[cfg->animAttack] = "Attack.png";
  anims[cfg->animDead] = "Dead.png";

  speeds[cfg->animStand] = 0;
  speeds[cfg->animWalk] = 120;
  speeds[cfg->animRun] = 75;
  speeds[cfg->animJump] = 100;
  speeds[cfg->animAttack] = 90;
  speeds[cfg->animDead] = 250;

  for (d = 0; d < cfg->dirCount; d++)
    for (s = 0; s < cfg->animCount; s++) {
      snprintf(path, 256, "%s/%s/%s", folder, dirs[d], anims[s]);
      loadSpriteSheet(&p->sheets[d][s], r, path, cfg->frameCount, speeds[s]);
    }
}

void movePersonnage(Personnage *p, Obstacle obs[], int nb, GameConfig *cfg) {
  int i;

  if (p->vie == 0) {
    p->vx = 0.0;
    p->vy = 0.0;
    return;
  }

  if (p->jumping) {
    p->vy = -13.0;
    p->onGround = 0;
    p->jumping = 0;
  }

  p->x = p->x + p->vx;
  p->y = p->y + p->vy;
  if (!p->onGround)
    p->vy = p->vy + 0.55;

  if (p->x < 0.0)
    p->x = 0.0;
  if (p->x > cfg->worldW - cfg->collW)
    p->x = cfg->worldW - cfg->collW;

  p->posScreen.x = p->x;
  p->posScreen.y = p->y;
  p->posScreen.w = cfg->collW;
  p->posScreen.h = cfg->collH;

  p->onGround = 0;

  if (p->y + cfg->collH >= cfg->groundY) {
    p->y = cfg->groundY - cfg->collH;
    p->onGround = 1;
    p->vy = 0.0;
  }

  p->posScreen.x = p->x;
  p->posScreen.y = p->y;

  for (i = 0; i < nb; i++) {
    int feetY, obsTop;
    int xOverlap;
    if (!obs[i].active)
      continue;
    feetY = p->posScreen.y + p->posScreen.h;
    obsTop = obs[i].pos.y;
    xOverlap = !(p->posScreen.x + p->posScreen.w <= obs[i].pos.x ||
                 p->posScreen.x >= obs[i].pos.x + obs[i].pos.w);
    if (xOverlap && p->vy >= 0.0 && feetY >= obsTop - 6 &&
        feetY <= obsTop + 12) {
      p->y = obsTop - cfg->collH;
      p->vy = 0.0;
      p->onGround = 1;
      p->posScreen.y = p->y;
    }
  }
  for (i = 0; i < nb; i++) {
    int pr, or2;
    if (!obs[i].active)
      continue;
    if (!SDL_HasIntersection(&p->posScreen, &obs[i].pos))
      continue;
    pr = p->posScreen.x + p->posScreen.w;
    or2 = obs[i].pos.x + obs[i].pos.w;
    if (p->vy == 0.0 && p->posScreen.y + p->posScreen.h <= obs[i].pos.y + 2) {
      p->posScreen.x = p->x;
      p->posScreen.y = p->y;
      continue;
    }
    if (p->vx > 0.0 && pr - obs[i].pos.x < obs[i].pos.w / 2) {
      p->x = obs[i].pos.x - cfg->collW;
      p->vx = 0.0;
    } else if (p->vx < 0.0 && or2 - p->posScreen.x < obs[i].pos.w / 2) {
      p->x = or2;
      p->vx = 0.0;
    }
    p->posScreen.x = p->x;
    p->posScreen.y = p->y;
  }
}

void animerPersonnage(Personnage *p, int dt) {
  SpriteSheet *s;

  if (p->etat != p->prevEtat) {
    p->sheets[p->direction][p->etat].currentFrame = 0;
    p->sheets[p->direction][p->etat].animTimer = 0;
    p->prevEtat = p->etat;
  }

  if (p->attackCooldown > 0) {
    p->attackCooldown = p->attackCooldown - dt;
    if (p->attackCooldown < 0)
      p->attackCooldown = 0;
  }

  s = &p->sheets[p->direction][p->etat];
  if (s->animSpeed == 0)
    return;

  s->animTimer = s->animTimer + dt;
  if (s->animTimer < s->animSpeed)
    return;
  s->animTimer = 0;
  s->currentFrame = s->currentFrame + 1;

  if (s->currentFrame >= s->frameCount) {
    s->currentFrame = 0;
    if (p->etat == 3) {
      p->etat = 0;
      p->prevEtat = -1;
    } else if (p->etat == 4) {
      p->etat = 0;
      p->prevEtat = -1;
    } else if (p->etat == 5) {
      s->currentFrame = s->frameCount - 1;
      s->animSpeed = 0;
    }
  }
}

void pollPlayerMovement(Personnage *p, GameConfig *cfg, int isP2) {
  int left, right, run;
  double speed;
  if (p->vie <= 0) {
    p->vx = 0.0;
    return;
  }
  if (isP2) {
    left = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT];
    right = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT];
    run = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RSHIFT];
  } else {
    left = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_Q] ||
           SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A] ||
           SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT];
    right = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D] ||
            SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT];
    run = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LSHIFT];
  }
  if (run)
    speed = 9.0;
  else
    speed = 5.0;
  if (left && !right) {
    p->vx = -speed;
    p->direction = cfg->dirLeft;
  } else if (right && !left) {
    p->vx = speed;
    p->direction = cfg->dirRight;
  } else {
    p->vx = 0.0;
  }
}

void syncAnimState(Personnage *p, GameConfig *cfg) {
  double avx;
  if (p->vie == 0)
    return;
  if (p->etat == cfg->animAttack && p->attackCooldown > 0)
    return;
  if (!p->onGround) {
    if (p->etat != cfg->animJump)
      p->etat = cfg->animJump;
    return;
  }
  avx = p->vx;
  if (avx < 0)
    avx = -avx;
  if (avx >= 8.0) {
    if (p->etat != cfg->animRun)
      p->etat = cfg->animRun;
  } else if (avx > 0.0) {
    if (p->etat != cfg->animWalk)
      p->etat = cfg->animWalk;
  } else {
    if (p->etat != cfg->animStand)
      p->etat = cfg->animStand;
  }
}

void updateGameMusic(GameContext *ctx, int anyAggro, int dt) {
  Mix_Music *target;
  int targetState;
  if (ctx->combatMusicCooldown > 0)
    ctx->combatMusicCooldown = ctx->combatMusicCooldown - dt;
  if (anyAggro) {
    targetState = 2;
    target = ctx->menu.musicCombat;
    ctx->combatMusicCooldown = 4000;
  } else if (ctx->combatMusicCooldown > 0) {
    return;
  } else {
    targetState = 1;
    target = ctx->menu.musicLevel;
  }
  if (ctx->currentMusicState == targetState)
    return;
  Mix_FadeOutMusic(400);
  if (target)
    Mix_FadeInMusic(target, -1, 600);
  ctx->currentMusicState = targetState;
}

void afficherPersonnage(Personnage *p, SDL_Renderer *r, Camera cam,
                        SDL_Rect vp) {
  int drawX, drawY;
  drawX = p->posScreen.x - cam.x + vp.x - (p->drawW - p->posScreen.w) / 2;
  drawY = p->posScreen.y - cam.y + vp.y - (p->drawH - p->posScreen.h);
  renderSpriteSheet(r, &p->sheets[p->direction][p->etat], drawX, drawY,
                    p->drawW, p->drawH);
}

void getGunBarrelPos(Personnage *p, GameConfig *cfg, int *outX, int *outY) {
  int drawX_world = p->posScreen.x - (p->drawW - p->posScreen.w) / 2;
  int drawY_world = p->posScreen.y - (p->drawH - p->posScreen.h);
  if (p->direction == cfg->dirRight)
    *outX = drawX_world + cfg->gunOffsetX;
  else
    *outX = drawX_world + p->drawW - cfg->gunOffsetX;
  *outY = drawY_world + cfg->gunOffsetY;
}

void freePersonnage(Personnage *p) {
  int d, s;
  for (d = 0; d < 2; d++)
    for (s = 0; s < 6; s++)
      freeSpriteSheet(&p->sheets[d][s]);
}

void fireProjectile(Projectile projs[], int max, int worldX, int worldY,
                    int facingRight, int isSuper, int fromPlayer,
                    GameConfig *cfg) {
  int i;
  for (i = 0; i < max; i++) {
    if (projs[i].active)
      continue;
    projs[i].x = worldX;
    projs[i].y = worldY;
    projs[i].active = 1;
    projs[i].isSuper = isSuper;
    projs[i].fromPlayer = fromPlayer;
    if (isSuper)
      projs[i].damage = cfg->projDamageSuper;
    else
      projs[i].damage = cfg->projDamageNormal;
    if (facingRight)
      projs[i].vx = cfg->projSpeed;
    else
      projs[i].vx = -cfg->projSpeed;
    projs[i].pos.w = cfg->projW;
    projs[i].pos.h = cfg->projH;
    break;
  }
}

void updateProjectiles(Projectile projs[], int max, Enemy *enemies[], int nbE,
                       Obstacle *obs[], int nbO, SDL_Rect playerBoxes[],
                       int nbPlayers, int playerVie[], int playerShield[],
                       GameConfig *cfg, int dt) {
  int i, j;
  if (dt > 0) {
  }
  for (i = 0; i < max; i++) {
    if (!projs[i].active)
      continue;
    projs[i].x = projs[i].x + projs[i].vx;
    projs[i].pos.x = projs[i].x;
    projs[i].pos.y = projs[i].y;
    if (projs[i].x < 0 || projs[i].x > cfg->worldW) {
      projs[i].active = 0;
      continue;
    }
    for (j = 0; j < nbO; j++) {
      if (!obs[j]->active)
        continue;
      if (SDL_HasIntersection(&projs[i].pos, &obs[j]->pos)) {
        projs[i].active = 0;
        break;
      }
    }
    if (!projs[i].active)
      continue;

    if (projs[i].fromPlayer) {
      for (j = 0; j < nbE; j++) {
        if (!enemies[j]->active || enemies[j]->dead)
          continue;
        if (SDL_HasIntersection(&projs[i].pos, &enemies[j]->posScreen)) {
          enemies[j]->vie = enemies[j]->vie - projs[i].damage;
          if (enemies[j]->vie <= 0)
            enemies[j]->dead = 1;
          projs[i].active = 0;
          break;
        }
      }
    } else {
      for (j = 0; j < nbPlayers; j++) {
        if (playerVie[j] <= 0)
          continue;
        if (SDL_HasIntersection(&projs[i].pos, &playerBoxes[j])) {
          if (playerShield[j]) {
          } else {
            playerVie[j] = playerVie[j] - projs[i].damage;
            if (playerVie[j] < 0)
              playerVie[j] = 0;
          }
          projs[i].active = 0;
          break;
        }
      }
    }
  }
}

void renderProjectiles(SDL_Renderer *r, Projectile projs[], int max, Camera cam,
                       SDL_Rect vp) {
  int i;
  SDL_Rect dst;
  for (i = 0; i < max; i++) {
    if (!projs[i].active)
      continue;
    dst.x = projs[i].pos.x - cam.x + vp.x;
    dst.y = projs[i].pos.y - cam.y + vp.y;
    dst.w = projs[i].pos.w;
    dst.h = projs[i].pos.h;
    if (projs[i].isSuper) {
      SDL_SetRenderDrawColor(r, 255, 60, 0, 255);
      SDL_RenderFillRect(r, &dst);
      SDL_SetRenderDrawColor(r, 255, 180, 0, 200);
      SDL_RenderDrawRect(r, &dst);
    } else {
      SDL_SetRenderDrawColor(r, 255, 230, 50, 255);
      SDL_RenderFillRect(r, &dst);
      SDL_SetRenderDrawColor(r, 255, 255, 180, 180);
      SDL_RenderDrawRect(r, &dst);
    }
  }
}

void loadBackground(Background *b, SDL_Renderer *r, char *path) {
  b->img = loadTexture(r, path);
}

void afficherBackground(Background *b, SDL_Renderer *r, Camera cam, SDL_Rect vp,
                        int worldW, int worldH) {
  SDL_Rect dst;
  int texW, texH;
  int scaledW;
  int firstTileWorldX, x;
  if (worldH > 0) {
  }
  if (worldW > 0) {
  }
  if (!b->img) {
    SDL_SetRenderDrawColor(r, 20, 20, 40, 255);
    SDL_RenderFillRect(r, &vp);
    return;
  }
  SDL_QueryTexture(b->img, NULL, NULL, &texW, &texH);
  if (texW <= 0 || texH <= 0) {
    SDL_SetRenderDrawColor(r, 20, 20, 40, 255);
    SDL_RenderFillRect(r, &vp);
    return;
  }

  scaledW = vp.h * texW / texH;
  if (scaledW <= 0) {
    SDL_RenderCopy(r, b->img, NULL, &vp);
    return;
  }
  firstTileWorldX = (cam.x / scaledW) * scaledW;
  if (firstTileWorldX > cam.x)
    firstTileWorldX = firstTileWorldX - scaledW;

  SDL_SetRenderDrawColor(r, 12, 14, 24, 255);
  SDL_RenderFillRect(r, &vp);

  for (x = firstTileWorldX; x < cam.x + vp.w; x = x + scaledW) {
    dst.x = vp.x + x - cam.x;
    dst.y = vp.y;
    dst.w = scaledW;
    dst.h = vp.h;
    SDL_RenderCopy(r, b->img, NULL, &dst);
  }
}

void freeBackground(Background *b) {
  if (b->img)
    SDL_DestroyTexture(b->img);
  b->img = NULL;
}

void initObstacleFromSheet(Obstacle *o, SDL_Texture *sheet, int obsType,
                           int damageRow, int worldX, int worldY, int drawW,
                           int drawH, GameConfig *cfg) {
  if (cfg->obsCellW > 0) {
  }
  memset(o, 0, sizeof(Obstacle));
  o->sheetTex = sheet;
  o->obsType = obsType;
  o->damageRow = damageRow;
  o->drawW = drawW;
  o->drawH = drawH;
  o->pos.x = worldX;
  o->pos.y = worldY;
  o->pos.w = drawW;
  o->pos.h = drawH;
  o->active = 1;
}

void afficherObstacle(Obstacle *o, SDL_Renderer *r, Camera cam, SDL_Rect vp,
                      GameConfig *cfg) {
  SDL_Rect src, dst;
  int sx, sy;
  if (!o->active || !o->sheetTex)
    return;
  src.y = o->damageRow * cfg->obsRowStride;
  src.h = cfg->obsRowH;
  if (o->obsType == cfg->obsTypeCrate) {
    src.x = cfg->obsCrateX;
    src.w = cfg->obsCrateW;
  } else if (o->obsType == cfg->obsTypeRoadblock) {
    src.x = cfg->obsRoadX;
    src.w = cfg->obsRoadW;
  } else {
    src.x = cfg->obsConeX;
    src.w = cfg->obsConeW;
  }
  sx = o->pos.x - cam.x + vp.x;
  sy = o->pos.y - cam.y + vp.y;
  dst.x = sx;
  dst.y = sy;
  dst.w = o->drawW;
  dst.h = o->drawH;
  SDL_RenderCopy(r, o->sheetTex, &src, &dst);
}

void initPickup(Pickup *pk, SDL_Texture *sheet, int itemType, int worldX,
                int worldY, GameConfig *cfg) {
  int dw, dh;
  dh = 38;
  if (itemType == cfg->itemMedkit) {
    dw = dh * cfg->itemMedW / cfg->itemRowH;
  } else if (itemType == cfg->itemAmmo) {
    dw = dh * cfg->itemAmmoW / cfg->itemRowH;
  } else if (itemType == cfg->itemVest) {
    dw = dh * cfg->itemVestW / cfg->itemRowH;
  } else {
    dw = dh * cfg->itemCoinW / cfg->itemRowH;
  }
  if (dw < 18)
    dw = 18;
  memset(pk, 0, sizeof(Pickup));
  pk->sheetTex = sheet;
  pk->itemType = itemType;
  pk->drawW = dw;
  pk->drawH = dh;
  pk->pos.x = worldX;
  pk->pos.y = worldY;
  pk->pos.w = dw;
  pk->pos.h = dh;
}

void afficherPickup(Pickup *pk, SDL_Renderer *r, Camera cam, SDL_Rect vp,
                    GameConfig *cfg) {
  SDL_Rect src, dst;
  int sx, sy;
  if (pk->collected || !pk->sheetTex)
    return;
  src.y = cfg->itemRowY;
  src.h = cfg->itemRowH;
  if (pk->itemType == cfg->itemMedkit) {
    src.x = cfg->itemMedX;
    src.w = cfg->itemMedW;
  } else if (pk->itemType == cfg->itemAmmo) {
    src.x = cfg->itemAmmoX;
    src.w = cfg->itemAmmoW;
  } else if (pk->itemType == cfg->itemVest) {
    src.x = cfg->itemVestX;
    src.w = cfg->itemVestW;
  } else {
    src.x = cfg->itemCoinX;
    src.w = cfg->itemCoinW;
  }
  sx = pk->pos.x - cam.x + vp.x;
  sy = pk->pos.y - cam.y + vp.y;
  dst.x = sx;
  dst.y = sy;
  dst.w = pk->drawW;
  dst.h = pk->drawH;
  SDL_RenderCopy(r, pk->sheetTex, &src, &dst);
}

int checkPickupCollision(SDL_Rect *playerBox, Pickup pickups[], int nb) {
  int i;
  for (i = 0; i < nb; i++) {
    if (pickups[i].collected)
      continue;
    if (SDL_HasIntersection(playerBox, &pickups[i].pos)) {
      pickups[i].collected = 1;
      return i;
    }
  }
  return -1;
}

void initEnemy(Enemy *e, SDL_Renderer *r, GameConfig *cfg, int worldX,
               int patrolLeft, int patrolRight, int enemyType) {
  char path[256];
  char *folder;

  memset(e, 0, sizeof(Enemy));
  e->x = worldX;
  e->y = cfg->groundY - cfg->collH;
  e->vx = 1.8;
  e->direction = cfg->dirRight;
  e->active = 1;
  e->maxVie = 5;
  e->vie = 5;
  e->patrolLeft = patrolLeft;
  e->patrolRight = patrolRight;
  e->drawW = cfg->playerDrawW;
  e->drawH = cfg->playerDrawH;
  e->enemyType = enemyType;
  e->posScreen.x = worldX;
  e->posScreen.y = e->y;
  e->posScreen.w = cfg->collW;
  e->posScreen.h = cfg->collH;

  e->aggroed = 0;
  e->deAggroTimer = 0;
  e->fleeing = 0;
  e->aggroRange = 350;
  e->deAggroRange = 550;

  if (enemyType == 0)
    folder = "images/Enemy";
  else
    folder = "images/Enemy2";

  snprintf(path, 256, "%s/Right/Walk.png", folder);
  loadSpriteSheet(&e->walkRight, r, path, 4, 120);
  snprintf(path, 256, "%s/Left/Walk.png", folder);
  loadSpriteSheet(&e->walkLeft, r, path, 4, 120);
  snprintf(path, 256, "%s/Right/Attack.png", folder);
  loadSpriteSheet(&e->attackRight, r, path, 4, 90);
  snprintf(path, 256, "%s/Left/Attack.png", folder);
  loadSpriteSheet(&e->attackLeft, r, path, 4, 90);
  snprintf(path, 256, "%s/Dead.png", folder);
  loadSpriteSheet(&e->deadSheet, r, path, 4, 180);
}

void getEnemyGunBarrelPos(Enemy *e, GameConfig *cfg, int *outX, int *outY) {
  int drawX_world = e->posScreen.x - (e->drawW - e->posScreen.w) / 2;
  int drawY_world = e->posScreen.y - (e->drawH - e->posScreen.h);
  if (e->direction == cfg->dirRight)
    *outX = drawX_world + cfg->gunOffsetX;
  else
    *outX = drawX_world + e->drawW - cfg->gunOffsetX;
  *outY = drawY_world + cfg->gunOffsetY;
}

void updateEnemy(Enemy *e, SDL_Rect playerBoxes[], int nbPlayers,
                 double playerVy[], int playerVie[], double playerX[],
                 int playerShield[], int playerScore[], double playerVxOut[],
                 double playerVyOut[], Obstacle obs[], int nbObs,
                 Projectile projs[], int maxProjs, GameConfig *cfg, int dt) {
  int i, closestPlayer, closestDist, dx, dmg, gx, gy;
  int hpLowThreshold;
  SpriteSheet *atkSheet;

  if (!e->active)
    return;
  if (e->invincTimer > 0)
    e->invincTimer = e->invincTimer - dt;
  if (e->attackCooldown > 0)
    e->attackCooldown = e->attackCooldown - dt;

  if (e->dead) {
    updateSpriteSheet(&e->deadSheet, dt);
    e->deadTimer = e->deadTimer + dt;
    if (e->deadTimer > 1200)
      e->active = 0;
    return;
  }

  hpLowThreshold = e->maxVie / 5;
  if (hpLowThreshold < 1)
    hpLowThreshold = 1;
  if (e->vie > 0 && e->vie <= hpLowThreshold)
    e->fleeing = 1;

  closestPlayer = -1;
  closestDist = 999999;
  for (i = 0; i < nbPlayers; i++) {
    if (playerVie[i] <= 0)
      continue;
    dx = playerBoxes[i].x - e->posScreen.x;
    if (dx < 0)
      dx = -dx;
    if (dx < closestDist) {
      closestDist = dx;
      closestPlayer = i;
    }
  }

  if (closestPlayer >= 0 && closestDist < e->aggroRange) {
    e->aggroed = 1;
    e->deAggroTimer = 0;
  } else if (e->aggroed) {
    if (closestPlayer < 0 || closestDist > e->deAggroRange) {
      if (e->deAggroTimer == 0)
        e->deAggroTimer = 2000;
      e->deAggroTimer = e->deAggroTimer - dt;
      if (e->deAggroTimer <= 0) {
        e->aggroed = 0;
        e->deAggroTimer = 0;
      }
    }
  }

  if (e->attacking) {
    if (e->direction == cfg->dirRight)
      atkSheet = &e->attackRight;
    else
      atkSheet = &e->attackLeft;
    e->attackAnimTimer = e->attackAnimTimer + dt;
    updateSpriteSheet(atkSheet, dt);
    if (atkSheet->currentFrame == 1 &&
        e->attackAnimTimer < atkSheet->animSpeed * 2) {
      getEnemyGunBarrelPos(e, cfg, &gx, &gy);
      fireProjectile(projs, maxProjs, gx, gy, e->direction == cfg->dirRight, 0,
                     0, cfg);
    }
    if (atkSheet->currentFrame == 0 &&
        e->attackAnimTimer > atkSheet->animSpeed * atkSheet->frameCount) {
      e->attacking = 0;
      e->attackAnimTimer = 0;
      e->attackCooldown = 2000;
      atkSheet->currentFrame = 0;
    }
    e->y = cfg->groundY - cfg->collH;
    e->posScreen.x = e->x;
    e->posScreen.y = e->y;
    e->posScreen.w = cfg->collW;
    e->posScreen.h = cfg->collH;
    return;
  }

  if (e->fleeing && closestPlayer >= 0) {
    double fleeSpeed = 2.6;
    if (playerBoxes[closestPlayer].x < e->posScreen.x) {
      e->vx = fleeSpeed;
      e->direction = cfg->dirRight;
    } else {
      e->vx = -fleeSpeed;
      e->direction = cfg->dirLeft;
    }
    e->x = e->x + e->vx;
    if (e->direction == cfg->dirRight)
      updateSpriteSheet(&e->walkRight, dt);
    else
      updateSpriteSheet(&e->walkLeft, dt);
  } else if (e->aggroed && closestPlayer >= 0) {
    double chaseSpeed = 2.4;
    int playerCx = playerBoxes[closestPlayer].x;
    if (playerCx < e->posScreen.x - 20) {
      e->vx = -chaseSpeed;
      e->direction = cfg->dirLeft;
    } else if (playerCx > e->posScreen.x + 20) {
      e->vx = chaseSpeed;
      e->direction = cfg->dirRight;
    } else {
      e->vx = 0;
    }
    if (closestDist < 200 && e->attackCooldown <= 0) {
      if (playerBoxes[closestPlayer].x < e->posScreen.x)
        e->direction = cfg->dirLeft;
      else
        e->direction = cfg->dirRight;
      e->attacking = 1;
      e->attackAnimTimer = 0;
      if (e->direction == cfg->dirRight) {
        e->attackRight.currentFrame = 0;
        e->attackRight.animTimer = 0;
      } else {
        e->attackLeft.currentFrame = 0;
        e->attackLeft.animTimer = 0;
      }
    } else {
      e->x = e->x + e->vx;
    }
    if (e->direction == cfg->dirRight)
      updateSpriteSheet(&e->walkRight, dt);
    else
      updateSpriteSheet(&e->walkLeft, dt);
  } else {
    if (e->vx > 1.8)
      e->vx = 1.8;
    else if (e->vx < -1.8)
      e->vx = -1.8;
    else if (e->vx == 0.0)
      e->vx = 1.8;
    e->x = e->x + e->vx;
    if (e->x <= e->patrolLeft) {
      e->x = e->patrolLeft;
      e->vx = 1.8;
      e->direction = cfg->dirRight;
    }
    if (e->x >= e->patrolRight) {
      e->x = e->patrolRight;
      e->vx = -1.8;
      e->direction = cfg->dirLeft;
    }
    e->posScreen.x = e->x;
    e->posScreen.y = e->y;
    for (i = 0; i < nbObs; i++) {
      if (!obs[i].active)
        continue;
      if (SDL_HasIntersection(&e->posScreen, &obs[i].pos)) {
        e->vx = -e->vx;
        e->direction = 1 - e->direction;
        e->x = e->x + e->vx * 4;
        break;
      }
    }
    if (e->direction == cfg->dirRight)
      updateSpriteSheet(&e->walkRight, dt);
    else
      updateSpriteSheet(&e->walkLeft, dt);
  }

  e->y = cfg->groundY - cfg->collH;
  e->posScreen.x = e->x;
  e->posScreen.y = e->y;
  e->posScreen.w = cfg->collW;
  e->posScreen.h = cfg->collH;

  for (i = 0; i < nbPlayers; i++) {
    int feetY, enemyTop, feetVsTop;
    if (playerVie[i] <= 0)
      continue;
    if (!SDL_HasIntersection(&e->posScreen, &playerBoxes[i]))
      continue;
    feetY = playerBoxes[i].y + playerBoxes[i].h;
    enemyTop = e->posScreen.y;
    feetVsTop = feetY - enemyTop;
    if (playerVy[i] > 1.0 && feetVsTop >= -2 && feetVsTop <= 8) {
      e->dead = 1;
      e->vie = 0;
      playerVyOut[i] = -9.0;
      playerScore[i] = playerScore[i] + 20;
    } else if (e->invincTimer <= 0) {
      dmg = 1;
      if (playerShield[i])
        dmg = 0;
      if (playerVie[i] > 0)
        playerVie[i] = playerVie[i] - dmg;
      if (playerX[i] > e->x)
        playerVxOut[i] = 6.0;
      else
        playerVxOut[i] = -6.0;
      playerVyOut[i] = -5.0;
      e->invincTimer = 1500;
    }
  }
}

void afficherEnemy(Enemy *e, SDL_Renderer *r, Camera cam, SDL_Rect vp) {
  SpriteSheet *sheet;
  int drawX, drawY;
  if (!e->active)
    return;
  drawX = e->posScreen.x - cam.x + vp.x - (e->drawW - e->posScreen.w) / 2;
  drawY = e->posScreen.y - cam.y + vp.y - (e->drawH - e->posScreen.h);
  if (e->dead)
    sheet = &e->deadSheet;
  else if (e->attacking) {
    if (e->direction == 0)
      sheet = &e->attackRight;
    else
      sheet = &e->attackLeft;
  } else {
    if (e->direction == 0)
      sheet = &e->walkRight;
    else
      sheet = &e->walkLeft;
  }
  renderSpriteSheet(r, sheet, drawX, drawY, e->drawW, e->drawH);

  if (!e->dead && e->vie > 0 && e->maxVie > 0) {
    SDL_Rect bgBar, fillBar;
    int barW = 50;
    int barH = 6;
    int barX = drawX + (e->drawW - barW) / 2;
    int barY = drawY - 12;
    int fillW = barW * e->vie / e->maxVie;
    if (barX + barW < vp.x || barX > vp.x + vp.w)
      return;
    bgBar.x = barX;
    bgBar.y = barY;
    bgBar.w = barW;
    bgBar.h = barH;
    SDL_SetRenderDrawColor(r, 20, 20, 20, 220);
    SDL_RenderFillRect(r, &bgBar);
    fillBar.x = barX;
    fillBar.y = barY;
    fillBar.w = fillW;
    fillBar.h = barH;
    if (e->fleeing) {
      SDL_SetRenderDrawColor(r, 230, 200, 50, 255);
    } else if (e->aggroed) {
      SDL_SetRenderDrawColor(r, 230, 40, 40, 255);
    } else {
      SDL_SetRenderDrawColor(r, 170, 40, 40, 255);
    }
    SDL_RenderFillRect(r, &fillBar);
    SDL_SetRenderDrawColor(r, 240, 240, 240, 220);
    SDL_RenderDrawRect(r, &bgBar);
  }
}

void freeEnemy(Enemy *e) {
  freeSpriteSheet(&e->walkRight);
  freeSpriteSheet(&e->walkLeft);
  freeSpriteSheet(&e->attackRight);
  freeSpriteSheet(&e->attackLeft);
  freeSpriteSheet(&e->deadSheet);
}

void initMinimap(Minimap *m, GameConfig *cfg) {
  m->mapRect.x = cfg->screenW - 185;
  m->mapRect.y = 10;
  m->mapRect.w = 175;
  m->mapRect.h = 90;
  m->p1Dot.w = 7;
  m->p1Dot.h = 7;
  m->p2Dot.w = 7;
  m->p2Dot.h = 7;
}

void updateMinimap(Minimap *m, SDL_Rect p1, SDL_Rect p2, int worldW, int worldH,
                   GameConfig *cfg) {
  float rx = (float)m->mapRect.w / (float)worldW;
  float ry = (float)m->mapRect.h / (float)worldH;
  m->p1Dot.x = m->mapRect.x + (int)(p1.x * rx);
  m->p1Dot.y = m->mapRect.y + (int)(p1.y * ry);
  m->p2Dot.x = m->mapRect.x + (int)(p2.x * rx);
  m->p2Dot.y = m->mapRect.y + (int)(p2.y * ry);
  if (cfg->screenW > 0) {
  }
}

void afficherMinimap(SDL_Renderer *r, Minimap *m, Obstacle obs[], int nbObs,
                     int worldW, int worldH, GameConfig *cfg) {
  float rx = (float)m->mapRect.w / (float)worldW;
  float ry = (float)m->mapRect.h / (float)worldH;
  SDL_Rect o;
  int i;
  if (cfg->screenW > 0) {
  }
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
  SDL_RenderFillRect(r, &m->mapRect);
  SDL_SetRenderDrawColor(r, 160, 110, 50, 220);
  for (i = 0; i < nbObs; i++) {
    if (!obs[i].active)
      continue;
    o.x = m->mapRect.x + (int)(obs[i].pos.x * rx);
    o.y = m->mapRect.y + (int)(obs[i].pos.y * ry);
    o.w = (int)(obs[i].pos.w * rx) + 1;
    o.h = (int)(obs[i].pos.h * ry) + 1;
    SDL_RenderFillRect(r, &o);
  }
  SDL_SetRenderDrawColor(r, 0, 255, 80, 255);
  SDL_RenderFillRect(r, &m->p1Dot);
  SDL_SetRenderDrawColor(r, 80, 160, 255, 255);
  SDL_RenderFillRect(r, &m->p2Dot);
  SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
  SDL_RenderDrawRect(r, &m->mapRect);
  if (worldH > 0) {
  }
}

void afficherHUD(SDL_Renderer *r, TTF_Font *f, int vie, int maxVie, int shield,
                 int superAmmo, int score, int level, int elapsedSec,
                 GameConfig *cfg, SDL_Rect vp, Minimap *minimap, Obstacle obs[],
                 int nbObs, int worldW, int worldH, SDL_Texture *heartTex,
                 SDL_Texture *heartEmptyTex) {
  int i, minutes, seconds;
  SDL_Color white, yellow;
  char buf[64];
  SDL_Rect rect, bar, heartRect, shieldBox;
  SDL_Texture *t;

  white.r = 255;
  white.g = 255;
  white.b = 255;
  white.a = 255;
  yellow.r = 255;
  yellow.g = 215;
  yellow.b = 0;
  yellow.a = 255;

  for (i = 0; i < maxVie; i++) {
    heartRect.x = vp.x + 10 + i * 30;
    heartRect.y = vp.y + 10;
    heartRect.w = 24;
    heartRect.h = 24;
    if (i < vie) {
      if (heartTex)
        SDL_RenderCopy(r, heartTex, NULL, &heartRect);
      else {
        SDL_SetRenderDrawColor(r, 220, 40, 60, 230);
        SDL_RenderFillRect(r, &heartRect);
      }
    } else {
      if (heartEmptyTex)
        SDL_RenderCopy(r, heartEmptyTex, NULL, &heartRect);
      else {
        SDL_SetRenderDrawColor(r, 60, 40, 40, 180);
        SDL_RenderFillRect(r, &heartRect);
      }
    }
  }

  if (shield) {
    shieldBox.x = vp.x + 10;
    shieldBox.y = vp.y + 40;
    shieldBox.w = 64;
    shieldBox.h = 18;
    SDL_SetRenderDrawColor(r, 60, 140, 255, 200);
    SDL_RenderFillRect(r, &shieldBox);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, f, "SHIELD", white, &rect);
    if (t) {
      rect.x = shieldBox.x + (shieldBox.w - rect.w) / 2;
      rect.y = shieldBox.y + (shieldBox.h - rect.h) / 2;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
  }

  snprintf(buf, 64, "x%d SUPER", superAmmo);
  rect.x = 0;
  rect.y = 0;
  rect.w = 0;
  rect.h = 0;
  t = makeText(r, f, buf, yellow, &rect);
  if (t) {
    rect.x = vp.x + 10;
    rect.y = vp.y + (shield ? 62 : 40);
    SDL_RenderCopy(r, t, NULL, &rect);
    SDL_DestroyTexture(t);
  }

  snprintf(buf, 64, "LVL %d", level);
  rect.x = 0;
  rect.y = 0;
  rect.w = 0;
  rect.h = 0;
  t = makeText(r, f, buf, white, &rect);
  if (t) {
    rect.x = vp.x + 10;
    rect.y = vp.y + vp.h - 50 - rect.h - 4;
    SDL_RenderCopy(r, t, NULL, &rect);
    SDL_DestroyTexture(t);
  }

  afficherMinimap(r, minimap, obs, nbObs, worldW, worldH, cfg);

  bar.x = vp.x;
  bar.y = vp.y + vp.h - 50;
  bar.w = vp.w;
  bar.h = 50;
  SDL_SetRenderDrawColor(r, 0, 0, 0, 220);
  SDL_RenderFillRect(r, &bar);
  SDL_SetRenderDrawColor(r, 60, 60, 60, 255);
  SDL_RenderDrawRect(r, &bar);

  snprintf(buf, 64, "SCORE: %d", score);
  rect.x = 0;
  rect.y = 0;
  rect.w = 0;
  rect.h = 0;
  t = makeText(r, f, buf, white, &rect);
  if (t) {
    rect.x = vp.x + 12;
    rect.y = bar.y + (bar.h - rect.h) / 2;
    SDL_RenderCopy(r, t, NULL, &rect);
    SDL_DestroyTexture(t);
  }

  minutes = elapsedSec / 60;
  seconds = elapsedSec - minutes * 60;
  snprintf(buf, 64, "%02d:%02d", minutes, seconds);
  rect.x = 0;
  rect.y = 0;
  rect.w = 0;
  rect.h = 0;
  t = makeText(r, f, buf, yellow, &rect);
  if (t) {
    rect.x = vp.x + vp.w / 2 - rect.w / 2;
    rect.y = bar.y + (bar.h - rect.h) / 2;
    SDL_RenderCopy(r, t, NULL, &rect);
    SDL_DestroyTexture(t);
  }

  snprintf(buf, 64, "SUPER AMMO: %d", superAmmo);
  rect.x = 0;
  rect.y = 0;
  rect.w = 0;
  rect.h = 0;
  t = makeText(r, f, buf, yellow, &rect);
  if (t) {
    rect.x = vp.x + vp.w - rect.w - 12;
    rect.y = bar.y + (bar.h - rect.h) / 2;
    SDL_RenderCopy(r, t, NULL, &rect);
    SDL_DestroyTexture(t);
  }
}

void addObs(Level *lv, GameConfig *cfg, int obsType, int x, int yFromGround,
            int w, int h) {
  if (lv->nbObstacles >= 24)
    return;
  initObstacleFromSheet(&lv->obstacles[lv->nbObstacles], lv->obsSpriteSheet,
                        obsType, lv->damageRow, x, lv->groundY - yFromGround, w,
                        h, cfg);
  lv->nbObstacles++;
}

void addPick(Level *lv, GameConfig *cfg, int itemType, int x, int yFromGround) {
  if (lv->nbPickups >= 12)
    return;
  initPickup(&lv->pickups[lv->nbPickups], lv->itemSpriteSheet, itemType, x,
             lv->groundY - yFromGround, cfg);
  lv->nbPickups++;
}

void addEnemy(Level *lv, SDL_Renderer *r, GameConfig *cfg, int x, int patrolL,
              int patrolR, int type) {
  if (lv->nbEnemies >= 4)
    return;
  initEnemy(&lv->enemies[lv->nbEnemies], r, cfg, x, patrolL, patrolR, type);
  lv->nbEnemies++;
}

void loadLevel(Level *lv, SDL_Renderer *r, GameConfig *cfg, int lvIdx) {
  int i;
  char *bgPaths[3];
  int CRW = 70, CRH = 70;
  int RBW = 120, RBH = 60;
  int CNW = 50, CNH = 70;

  memset(lv, 0, sizeof(Level));
  bgPaths[0] = "images/bg/level1.png";
  bgPaths[1] = "images/bg/level2.png";
  bgPaths[2] = "images/bg/level3.png";

  loadBackground(&lv->bg, r, bgPaths[lvIdx]);
  lv->obsSpriteSheet = loadTexture(r, "images/obstacles/obstacles.png");
  lv->itemSpriteSheet = loadTexture(r, "images/obstacles/items.png");

  lv->damageRow = lvIdx;
  lv->nbObstacles = 0;
  lv->nbPickups = 0;
  lv->nbEnemies = 0;

  if (lvIdx == 0) {
    lv->worldW = 5400;
    lv->groundY = 526;

    addObs(lv, cfg, cfg->obsTypeCrate, 400, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 475, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCone, 620, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 850, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCrate, 1150, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 1150, CRH * 2, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 1400, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCone, 1700, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeCone, 1780, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeCrate, 2050, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 2125, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 2400, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCrate, 2750, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 2750, CRH * 2, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCone, 3050, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 3350, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCrate, 3650, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 4000, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 4075, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCone, 4350, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 4650, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCrate, 4950, CRH, CRW, CRH);

    addPick(lv, cfg, cfg->itemAmmo, 550, 40);
    addPick(lv, cfg, cfg->itemMedkit, 1000, 40);
    addPick(lv, cfg, cfg->itemCoin, 1150, CRH * 2 + 50);
    addPick(lv, cfg, cfg->itemCoin, 1900, 40);
    addPick(lv, cfg, cfg->itemVest, 2600, 40);
    addPick(lv, cfg, cfg->itemAmmo, 2750, CRH * 2 + 50);
    addPick(lv, cfg, cfg->itemMedkit, 3500, 40);
    addPick(lv, cfg, cfg->itemCoin, 4300, 40);
    addPick(lv, cfg, cfg->itemCoin, 4800, 40);

    addEnemy(lv, r, cfg, 1900, 1700, 2200, 0);
    addEnemy(lv, r, cfg, 3200, 3000, 3600, 0);
    addEnemy(lv, r, cfg, 4500, 4300, 4900, 1);
  }

  if (lvIdx == 1) {
    lv->worldW = 5400;
    lv->groundY = 499;

    addObs(lv, cfg, cfg->obsTypeCrate, 350, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 425, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCone, 570, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 900, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCrate, 1080, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCone, 1350, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeCrate, 1650, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 1650, CRH * 2, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 1950, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 2120, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCone, 2400, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeCone, 2480, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeCrate, 2750, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 2825, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 3150, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCrate, 3500, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 3500, CRH * 2, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCone, 3800, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeCrate, 3950, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 4300, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCrate, 4600, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCone, 4900, CNH, CNW, CNH);

    addPick(lv, cfg, cfg->itemAmmo, 490, 40);
    addPick(lv, cfg, cfg->itemMedkit, 1200, 40);
    addPick(lv, cfg, cfg->itemCoin, 1650, CRH * 2 + 50);
    addPick(lv, cfg, cfg->itemVest, 2250, 40);
    addPick(lv, cfg, cfg->itemAmmo, 2900, 40);
    addPick(lv, cfg, cfg->itemMedkit, 3500, CRH * 2 + 50);
    addPick(lv, cfg, cfg->itemCoin, 4150, 40);
    addPick(lv, cfg, cfg->itemCoin, 4750, 40);

    addEnemy(lv, r, cfg, 1300, 1150, 1600, 0);
    addEnemy(lv, r, cfg, 2300, 2150, 2700, 1);
    addEnemy(lv, r, cfg, 4100, 3950, 4500, 0);
  }

  if (lvIdx == 2) {
    lv->worldW = 5400;
    lv->groundY = 536;

    addObs(lv, cfg, cfg->obsTypeRoadblock, 300, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCrate, 520, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCone, 700, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeCrate, 950, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 950, CRH * 2, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 950, CRH * 3, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 1300, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCone, 1550, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeCone, 1620, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeCrate, 1900, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 1900, CRH * 2, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 2100, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 2100, CRH * 2, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 2450, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCone, 2750, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeCrate, 3000, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 3300, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCrate, 3600, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 3600, CRH * 2, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCone, 3900, CNH, CNW, CNH);
    addObs(lv, cfg, cfg->obsTypeRoadblock, 4200, RBH, RBW, RBH);
    addObs(lv, cfg, cfg->obsTypeCrate, 4500, CRH, CRW, CRH);
    addObs(lv, cfg, cfg->obsTypeCrate, 4800, CRH, CRW, CRH);

    addPick(lv, cfg, cfg->itemMedkit, 400, 40);
    addPick(lv, cfg, cfg->itemCoin, 950, CRH * 3 + 50);
    addPick(lv, cfg, cfg->itemAmmo, 1450, 40);
    addPick(lv, cfg, cfg->itemCoin, 1900, CRH * 2 + 50);
    addPick(lv, cfg, cfg->itemVest, 2600, 40);
    addPick(lv, cfg, cfg->itemAmmo, 3150, 40);
    addPick(lv, cfg, cfg->itemMedkit, 3600, CRH * 2 + 50);
    addPick(lv, cfg, cfg->itemCoin, 4100, 40);
    addPick(lv, cfg, cfg->itemCoin, 4900, 40);

    addEnemy(lv, r, cfg, 1450, 1300, 1700, 1);
    addEnemy(lv, r, cfg, 2700, 2550, 3050, 0);
    addEnemy(lv, r, cfg, 4350, 4200, 4700, 1);
  }

  for (i = lv->nbObstacles; i < 24; i++)
    lv->obstacles[i].active = 0;
  for (i = lv->nbPickups; i < 12; i++)
    lv->pickups[i].collected = 1;
  for (i = lv->nbEnemies; i < 4; i++)
    lv->enemies[i].active = 0;
}

void updateLevel(Level *lv, SDL_Rect playerBoxes[], int nbPlayers,
                 double playerVy[], int playerVie[], double playerX[],
                 int playerShield[], int playerScore[], double playerVxOut[],
                 double playerVyOut[], Projectile projs[], int maxProjs,
                 GameConfig *cfg, int dt, int *anyAggroOut) {
  int i, anyAggro;
  Enemy *enArr[4];
  Obstacle *obsArr[24];
  for (i = 0; i < lv->nbEnemies; i++)
    enArr[i] = &lv->enemies[i];
  for (i = 0; i < lv->nbObstacles; i++)
    obsArr[i] = &lv->obstacles[i];
  for (i = 0; i < lv->nbEnemies; i++)
    updateEnemy(&lv->enemies[i], playerBoxes, nbPlayers, playerVy, playerVie,
                playerX, playerShield, playerScore, playerVxOut, playerVyOut,
                lv->obstacles, lv->nbObstacles, projs, maxProjs, cfg, dt);
  updateProjectiles(projs, maxProjs, enArr, lv->nbEnemies, obsArr,
                    lv->nbObstacles, playerBoxes, nbPlayers, playerVie,
                    playerShield, cfg, dt);
  anyAggro = 0;
  for (i = 0; i < lv->nbEnemies; i++) {
    if (lv->enemies[i].active && !lv->enemies[i].dead &&
        lv->enemies[i].aggroed && !lv->enemies[i].fleeing) {
      anyAggro = 1;
      break;
    }
  }
  if (anyAggroOut)
    *anyAggroOut = anyAggro;
}

void afficherLevel(Level *lv, SDL_Renderer *r, Camera cam, SDL_Rect vp,
                   GameConfig *cfg) {
  int i;
  afficherBackground(&lv->bg, r, cam, vp, cfg->worldW, cfg->worldH);
  for (i = 0; i < lv->nbObstacles; i++)
    afficherObstacle(&lv->obstacles[i], r, cam, vp, cfg);
  for (i = 0; i < lv->nbPickups; i++)
    afficherPickup(&lv->pickups[i], r, cam, vp, cfg);
  for (i = 0; i < lv->nbEnemies; i++)
    afficherEnemy(&lv->enemies[i], r, cam, vp);
}

void freeLevel(Level *lv) {
  int i;
  freeBackground(&lv->bg);
  if (lv->obsSpriteSheet)
    SDL_DestroyTexture(lv->obsSpriteSheet);
  if (lv->itemSpriteSheet)
    SDL_DestroyTexture(lv->itemSpriteSheet);
  for (i = 0; i < 4; i++)
    freeEnemy(&lv->enemies[i]);
}

void shuffleIndices(ShuffleState *s, int nb) {
  int i, j, tmp;
  for (i = 0; i < nb; i++)
    s->shuffled[i] = i;
  for (i = nb - 1; i > 0; i--) {
    j = rand() % (i + 1);
    tmp = s->shuffled[i];
    s->shuffled[i] = s->shuffled[j];
    s->shuffled[j] = tmp;
  }
  s->shufflePos = 0;
}

int loadQuizBlocks(char *file, QuizBlock blocks[], int max) {
  FILE *f;
  int nb, filled, len;
  char line[256];
  QuizBlock cur;
  f = fopen(file, "r");
  if (!f) {
    fprintf(stderr, "loadQuizBlocks: %s\n", file);
    return 0;
  }
  nb = 0;
  filled = 0;
  memset(&cur, 0, sizeof(cur));
  while (fgets(line, 256, f) && nb < max) {
    len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
      len--;
      line[len] = '\0';
    }
    if (strncmp(line, "QUESTION:", 9) == 0) {
      if (filled == 31) {
        blocks[nb] = cur;
        nb++;
      }
      memset(&cur, 0, sizeof(cur));
      filled = 0;
      strncpy(cur.q, line + 10, 199);
      filled = filled | 1;
    } else if (strncmp(line, "REP1:", 5) == 0) {
      strncpy(cur.r1, line + 6, 99);
      filled = filled | 2;
    } else if (strncmp(line, "REP2:", 5) == 0) {
      strncpy(cur.r2, line + 6, 99);
      filled = filled | 4;
    } else if (strncmp(line, "REP3:", 5) == 0) {
      strncpy(cur.r3, line + 6, 99);
      filled = filled | 8;
    } else if (strncmp(line, "ANSWER:", 7) == 0) {
      cur.ans = atoi(line + 8);
      filled = filled | 16;
    }
  }
  if (filled == 31 && nb < max) {
    blocks[nb] = cur;
    nb++;
  }
  fclose(f);
  return nb;
}

Enigme genererEnigme(QuizBlock blocks[], int nb, ShuffleState *s,
                     SDL_Renderer *r, TTF_Font *f, GameConfig *cfg) {
  Enigme e;
  QuizBlock *b;
  int idx;
  SDL_Color yellow, norm;
  int bw, bh, bx, by[3];
  memset(&e, 0, sizeof(e));
  e.etat = cfg->enigmeAttente;
  if (nb == 0)
    return e;
  if (s->lastNb != nb) {
    shuffleIndices(s, nb);
    s->lastNb = nb;
  }
  if (s->shufflePos >= nb)
    shuffleIndices(s, nb);
  idx = s->shuffled[s->shufflePos];
  s->shufflePos++;
  b = &blocks[idx];
  strncpy(e.question, b->q, 199);
  strncpy(e.rep1_text, b->r1, 99);
  strncpy(e.rep2_text, b->r2, 99);
  strncpy(e.rep3_text, b->r3, 99);
  e.numbr = b->ans;
  yellow.r = 255;
  yellow.g = 215;
  yellow.b = 0;
  yellow.a = 255;
  norm.r = 200;
  norm.g = 200;
  norm.b = 200;
  norm.a = 255;
  bw = 520;
  bh = 60;
  bx = (cfg->screenW - bw) / 2;
  by[0] = 220;
  by[1] = 300;
  by[2] = 380;
  e.questionTexture = makeText(r, f, e.question, yellow, &e.posQuestion);
  e.posQuestion.x = (cfg->screenW - e.posQuestion.w) / 2;
  e.posQuestion.y = 140;
  e.posRep1.x = bx;
  e.posRep1.y = by[0];
  e.posRep1.w = bw;
  e.posRep1.h = bh;
  e.posRep2.x = bx;
  e.posRep2.y = by[1];
  e.posRep2.w = bw;
  e.posRep2.h = bh;
  e.posRep3.x = bx;
  e.posRep3.y = by[2];
  e.posRep3.w = bw;
  e.posRep3.h = bh;
  e.rep1Texture = makeText(r, f, e.rep1_text, norm, &e.posRep1Text);
  e.rep2Texture = makeText(r, f, e.rep2_text, norm, &e.posRep2Text);
  e.rep3Texture = makeText(r, f, e.rep3_text, norm, &e.posRep3Text);
  e.posRep1Text.x = e.posRep1.x + (e.posRep1.w - e.posRep1Text.w) / 2;
  e.posRep1Text.y = e.posRep1.y + (e.posRep1.h - e.posRep1Text.h) / 2;
  e.posRep2Text.x = e.posRep2.x + (e.posRep2.w - e.posRep2Text.w) / 2;
  e.posRep2Text.y = e.posRep2.y + (e.posRep2.h - e.posRep2Text.h) / 2;
  e.posRep3Text.x = e.posRep3.x + (e.posRep3.w - e.posRep3Text.w) / 2;
  e.posRep3Text.y = e.posRep3.y + (e.posRep3.h - e.posRep3Text.h) / 2;
  e.btn[0] = loadTexture(r, "images/ui/btn1.png");
  e.btn[1] = loadTexture(r, "images/ui/btn2.png");
  e.btn[2] = loadTexture(r, "images/ui/btn3.png");
  e.btnHover[0] = loadTexture(r, "images/ui/btn1_hover.png");
  e.btnHover[1] = loadTexture(r, "images/ui/btn2_hover.png");
  e.btnHover[2] = loadTexture(r, "images/ui/btn3_hover.png");
  return e;
}

void afficherEnigme(Enigme *e, SDL_Renderer *r, int mx, int my,
                    GameConfig *cfg) {
  SDL_Rect *boxes[3];
  SDL_Texture *textTex[3];
  SDL_Rect *texts[3];
  int i, isHover;
  SDL_Rect *box;
  SDL_Texture *chosen;
  if (cfg->screenW > 0) {
  }
  boxes[0] = &e->posRep1;
  boxes[1] = &e->posRep2;
  boxes[2] = &e->posRep3;
  textTex[0] = e->rep1Texture;
  textTex[1] = e->rep2Texture;
  textTex[2] = e->rep3Texture;
  texts[0] = &e->posRep1Text;
  texts[1] = &e->posRep2Text;
  texts[2] = &e->posRep3Text;
  for (i = 0; i < 3; i++) {
    box = boxes[i];
    isHover = (mx > box->x && mx < box->x + box->w && my > box->y &&
               my < box->y + box->h);
    chosen = NULL;
    if (isHover && e->etat == 0 && e->btnHover[i])
      chosen = e->btnHover[i];
    else if (e->btn[i])
      chosen = e->btn[i];
    else {
      SDL_SetRenderDrawColor(r, 40, 40, 80, 220);
      SDL_RenderFillRect(r, box);
    }
    if (chosen)
      SDL_RenderCopy(r, chosen, NULL, box);
    if (textTex[i])
      SDL_RenderCopy(r, textTex[i], NULL, texts[i]);
  }
  if (e->questionTexture)
    SDL_RenderCopy(r, e->questionTexture, NULL, &e->posQuestion);
}

void afficherKeyboardHints(SDL_Renderer *r, TTF_Font *f, int selected,
                           GameConfig *cfg) {
  SDL_Color normal, active, c1, c2, c3;
  SDL_Rect r1, r2, r3;
  SDL_Texture *t1, *t2, *t3;
  if (cfg->screenW > 0) {
  }
  normal.r = 180;
  normal.g = 180;
  normal.b = 180;
  normal.a = 255;
  active.r = 255;
  active.g = 215;
  active.b = 0;
  active.a = 255;
  r1.x = 20;
  r1.y = 225;
  r1.w = 0;
  r1.h = 0;
  r2.x = 20;
  r2.y = 262;
  r2.w = 0;
  r2.h = 0;
  r3.x = 20;
  r3.y = 299;
  r3.w = 0;
  r3.h = 0;
  if (selected == 1)
    c1 = active;
  else
    c1 = normal;
  if (selected == 2)
    c2 = active;
  else
    c2 = normal;
  if (selected == 3)
    c3 = active;
  else
    c3 = normal;
  t1 = makeText(r, f, "A : Rep 1", c1, &r1);
  t2 = makeText(r, f, "B : Rep 2", c2, &r2);
  t3 = makeText(r, f, "C : Rep 3", c3, &r3);
  if (t1) {
    SDL_RenderCopy(r, t1, NULL, &r1);
    SDL_DestroyTexture(t1);
  }
  if (t2) {
    SDL_RenderCopy(r, t2, NULL, &r2);
    SDL_DestroyTexture(t2);
  }
  if (t3) {
    SDL_RenderCopy(r, t3, NULL, &r3);
    SDL_DestroyTexture(t3);
  }
}

void afficherFeedback(SDL_Renderer *r, TTF_Font *f, Enigme *e,
                      int feedbackStart, GameConfig *cfg) {
  int elapsed, alpha;
  char *msg;
  SDL_Color col;
  SDL_Rect rect;
  SDL_Texture *t;
  if (e->etat == 0)
    return;
  elapsed = SDL_GetTicks() - feedbackStart;
  if (elapsed > 1500)
    return;
  alpha = 255;
  if (elapsed > 800) {
    alpha = 255 - (255 * (elapsed - 800) / 700);
    if (alpha < 0)
      alpha = 0;
  }
  if (e->etat == 1) {
    msg = "Correct !";
    col.r = 60;
    col.g = 220;
    col.b = 100;
  } else {
    msg = "Mauvaise reponse !";
    col.r = 220;
    col.g = 60;
    col.b = 60;
  }
  col.a = alpha;
  rect.x = 0;
  rect.y = 0;
  rect.w = 0;
  rect.h = 0;
  t = makeText(r, f, msg, col, &rect);
  if (!t)
    return;
  rect.x = (cfg->screenW - rect.w) / 2;
  rect.y = 490;
  SDL_SetTextureAlphaMod(t, alpha);
  SDL_RenderCopy(r, t, NULL, &rect);
  SDL_DestroyTexture(t);
}

int resolveEnigme(int choix, Enigme *e, int *score, int *vie, GameConfig *cfg) {
  if (e->etat != cfg->enigmeAttente)
    return -1;
  if (choix == e->numbr) {
    e->etat = cfg->enigmeCorrect;
    *score = *score + 10;
    return 1;
  } else {
    e->etat = cfg->enigmeFaux;
    if (*vie > 0)
      *vie = *vie - 1;
    return 0;
  }
}

void updateTimerBar(SDL_Renderer *r, int startTime, GameConfig *cfg) {
  float ratio;
  int elapsed, fillW, rc, gc;
  SDL_Rect bg, fill;
  elapsed = SDL_GetTicks() - startTime;
  ratio = 1.0f - (float)elapsed / 10000.0f;
  if (ratio < 0.0f)
    ratio = 0.0f;
  bg.x = 0;
  bg.y = 0;
  bg.w = cfg->screenW;
  bg.h = 8;
  SDL_SetRenderDrawColor(r, 40, 40, 60, 220);
  SDL_RenderFillRect(r, &bg);
  fillW = (int)((float)cfg->screenW * ratio);
  rc = (int)(255.0f * (1.0f - ratio));
  gc = (int)(200.0f * ratio);
  fill.x = 0;
  fill.y = 0;
  fill.w = fillW;
  fill.h = 8;
  SDL_SetRenderDrawColor(r, rc, gc, 80, 240);
  SDL_RenderFillRect(r, &fill);
}

void libererEnigme(Enigme *e) {
  int i;
  if (e->questionTexture)
    SDL_DestroyTexture(e->questionTexture);
  if (e->rep1Texture)
    SDL_DestroyTexture(e->rep1Texture);
  if (e->rep2Texture)
    SDL_DestroyTexture(e->rep2Texture);
  if (e->rep3Texture)
    SDL_DestroyTexture(e->rep3Texture);
  for (i = 0; i < 3; i++) {
    if (e->btn[i])
      SDL_DestroyTexture(e->btn[i]);
    if (e->btnHover[i])
      SDL_DestroyTexture(e->btnHover[i]);
    e->btn[i] = NULL;
    e->btnHover[i] = NULL;
  }
  e->questionTexture = NULL;
  e->rep1Texture = NULL;
  e->rep2Texture = NULL;
  e->rep3Texture = NULL;
}

void playRotozoom(SDL_Renderer *ren, SDL_Texture *tex, GameConfig *cfg) {
  int w, h, i, scrW, scrH;
  SDL_Surface *surface, *rot;
  SDL_Texture *target, *t;
  SDL_Rect pos;
  double angle, scale;
  scrW = cfg->screenW;
  scrH = cfg->screenH;
  SDL_QueryTexture(tex, NULL, NULL, &w, &h);
  surface = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00,
                                 0x000000FF, 0xFF000000);
  target = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888,
                             SDL_TEXTUREACCESS_TARGET, w, h);
  SDL_SetRenderTarget(ren, target);
  SDL_RenderCopy(ren, tex, NULL, NULL);
  SDL_RenderReadPixels(ren, NULL, SDL_PIXELFORMAT_ARGB8888, surface->pixels,
                       surface->pitch);
  SDL_SetRenderTarget(ren, NULL);
  SDL_DestroyTexture(target);
  angle = 0;
  for (i = 0; i < 30; i++) {
    scale = 1.0 + i * 0.03;
    angle = angle + 8;
    rot = rotozoomSurface(surface, angle, scale, 1);
    t = SDL_CreateTextureFromSurface(ren, rot);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    pos.w = rot->w;
    pos.h = rot->h;
    pos.x = (scrW - pos.w) / 2;
    pos.y = (scrH - pos.h) / 2;
    SDL_RenderCopy(ren, t, NULL, &pos);
    SDL_RenderPresent(ren);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(rot);
    SDL_Delay(20);
  }
  for (i = 30; i > 0; i--) {
    scale = 1.0 + i * 0.03;
    angle = angle + 8;
    rot = rotozoomSurface(surface, angle, scale, 1);
    t = SDL_CreateTextureFromSurface(ren, rot);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    pos.w = rot->w;
    pos.h = rot->h;
    pos.x = (scrW - pos.w) / 2;
    pos.y = (scrH - pos.h) / 2;
    SDL_RenderCopy(ren, t, NULL, &pos);
    SDL_RenderPresent(ren);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(rot);
    SDL_Delay(20);
  }
  SDL_FreeSurface(surface);
}

void playEnigme2Result(SDL_Renderer *ren, SDL_Texture *bgTex,
                       SDL_Texture *resultTex, GameConfig *cfg) {
  int w, h, i, scrW, scrH;
  SDL_Surface *bgSurf, *rotSurf;
  SDL_Surface *resSurf;
  SDL_Texture *target, *t;
  SDL_Rect pos;
  double angle, scale;

  scrW = cfg->screenW;
  scrH = cfg->screenH;

  if (!bgTex)
    return;

  SDL_QueryTexture(bgTex, NULL, NULL, &w, &h);
  bgSurf = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF,
                                0xFF000000);
  target = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888,
                             SDL_TEXTUREACCESS_TARGET, w, h);
  SDL_SetRenderTarget(ren, target);
  SDL_RenderCopy(ren, bgTex, NULL, NULL);
  SDL_RenderReadPixels(ren, NULL, SDL_PIXELFORMAT_ARGB8888, bgSurf->pixels,
                       bgSurf->pitch);
  SDL_SetRenderTarget(ren, NULL);
  SDL_DestroyTexture(target);

  resSurf = NULL;
  if (resultTex) {
    int rw, rh;
    SDL_QueryTexture(resultTex, NULL, NULL, &rw, &rh);
    resSurf = SDL_CreateRGBSurface(0, rw, rh, 32, 0x00FF0000, 0x0000FF00,
                                   0x000000FF, 0xFF000000);
    target = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888,
                               SDL_TEXTUREACCESS_TARGET, rw, rh);
    SDL_SetRenderTarget(ren, target);
    SDL_RenderCopy(ren, resultTex, NULL, NULL);
    SDL_RenderReadPixels(ren, NULL, SDL_PIXELFORMAT_ARGB8888, resSurf->pixels,
                         resSurf->pitch);
    SDL_SetRenderTarget(ren, NULL);
    SDL_DestroyTexture(target);
  }

  angle = 0;
  for (i = 0; i < 30; i++) {
    scale = 0.5 + i * 0.05;
    angle = angle + 12.0;
    rotSurf = rotozoomSurface(bgSurf, angle, scale, 1);
    t = SDL_CreateTextureFromSurface(ren, rotSurf);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    pos.w = rotSurf->w;
    pos.h = rotSurf->h;
    pos.x = (scrW - pos.w) / 2;
    pos.y = (scrH - pos.h) / 2;
    SDL_RenderCopy(ren, t, NULL, &pos);
    SDL_RenderPresent(ren);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(rotSurf);
    SDL_Delay(20);
  }

  for (i = 0; i < 30; i++) {
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    if (resSurf) {
      int rw = resSurf->w;
      int rh = resSurf->h;
      double targetW = scrW * 0.7;
      double targetH = scrH * 0.7;
      double sx = targetW / rw;
      double sy = targetH / rh;
      double base;
      double pulse;
      double s;
      if (sx < sy)
        base = sx;
      else
        base = sy;
      if (((i / 5) % 2) == 0)
        pulse = 1.04;
      else
        pulse = 0.96;
      s = base * pulse;
      pos.w = rw * s;
      pos.h = rh * s;
      pos.x = (scrW - pos.w) / 2;
      pos.y = (scrH - pos.h) / 2;
      SDL_RenderCopy(ren, resultTex, NULL, &pos);
    }
    SDL_RenderPresent(ren);
    SDL_Delay(25);
  }

  if (resSurf) {
    for (i = 30; i > 0; i--) {
      scale = 0.5 + i * 0.05;
      angle = angle - 12.0; /* reverse direction */
      rotSurf = rotozoomSurface(resSurf, angle, scale, 1);
      t = SDL_CreateTextureFromSurface(ren, rotSurf);
      SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
      SDL_RenderClear(ren);
      pos.w = rotSurf->w;
      pos.h = rotSurf->h;
      pos.x = (scrW - pos.w) / 2;
      pos.y = (scrH - pos.h) / 2;
      SDL_RenderCopy(ren, t, NULL, &pos);
      SDL_RenderPresent(ren);
      SDL_DestroyTexture(t);
      SDL_FreeSurface(rotSurf);
      SDL_Delay(20);
    }
    SDL_FreeSurface(resSurf);
  } else {
    for (i = 30; i > 0; i--) {
      scale = 0.5 + i * 0.05;
      angle = angle - 12.0;
      rotSurf = rotozoomSurface(bgSurf, angle, scale, 1);
      t = SDL_CreateTextureFromSurface(ren, rotSurf);
      SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
      SDL_RenderClear(ren);
      pos.w = rotSurf->w;
      pos.h = rotSurf->h;
      pos.x = (scrW - pos.w) / 2;
      pos.y = (scrH - pos.h) / 2;
      SDL_RenderCopy(ren, t, NULL, &pos);
      SDL_RenderPresent(ren);
      SDL_DestroyTexture(t);
      SDL_FreeSurface(rotSurf);
      SDL_Delay(20);
    }
  }

  SDL_FreeSurface(bgSurf);
}

SDL_Rect fitRect(int boxX, int boxY, int boxW, int boxH, int srcW, int srcH) {
  SDL_Rect out;
  float scaleW, scaleH, scale;
  int fitW, fitH;
  if (srcW <= 0 || srcH <= 0) {
    out.x = boxX;
    out.y = boxY;
    out.w = boxW;
    out.h = boxH;
    return out;
  }
  scaleW = (float)boxW / (float)srcW;
  scaleH = (float)boxH / (float)srcH;
  if (scaleW < scaleH)
    scale = scaleW;
  else
    scale = scaleH;
  fitW = (int)((float)srcW * scale);
  fitH = (int)((float)srcH * scale);
  out.x = boxX + (boxW - fitW) / 2;
  out.y = boxY + (boxH - fitH) / 2;
  out.w = fitW;
  out.h = fitH;
  return out;
}

SDL_Texture *loadAndQuery(SDL_Renderer *r, char *path, int *outW, int *outH) {
  SDL_Texture *t = loadTexture(r, path);
  if (t)
    SDL_QueryTexture(t, NULL, NULL, outW, outH);
  else {
    *outW = 1;
    *outH = 1;
  }
  return t;
}

void computeEnigme2Layout(Enigme2 *e2, GameConfig *cfg) {
  int i, puz, bgBoxW, bgBoxH, bgBoxX, bgBoxY, ansBoxW, ansBoxH, totalW, startX,
      ansY, bx;
  DragPuzzle *p;
  puz = e2->currentPuzzle;
  p = &e2->puzzles[puz];
  bgBoxW = 320;
  bgBoxH = 400;
  bgBoxX = (cfg->screenW - bgBoxW) / 2;
  bgBoxY = 30;
  e2->bgRect = fitRect(bgBoxX, bgBoxY, bgBoxW, bgBoxH, p->bgW, p->bgH);
  e2->slotRect.w = e2->bgRect.w * 60 / 100;
  e2->slotRect.h = e2->bgRect.h * 42 / 100;
  e2->slotRect.x = e2->bgRect.x + (e2->bgRect.w - e2->slotRect.w) / 2;
  e2->slotRect.y = e2->bgRect.y + e2->bgRect.h * 2 / 100;
  ansBoxW = 180;
  ansBoxH = 130;
  totalW = 3 * ansBoxW + 2 * 20;
  startX = (cfg->screenW - totalW) / 2;
  ansY = cfg->screenH - ansBoxH - 18;
  for (i = 0; i < 3; i++) {
    bx = startX + i * (ansBoxW + 20);
    e2->answerRects[i] =
        fitRect(bx, ansY, ansBoxW, ansBoxH, p->ansW[i], p->ansH[i]);
  }
  e2->heldRect = e2->answerRects[0];
}

void initEnigme2(Enigme2 *e2, SDL_Renderer *r, GameConfig *cfg) {
  int i, j;
  char pathBg[128], pathA[128];
  int correctAnswers[5];
  memset(e2, 0, sizeof(Enigme2));
  e2->active = 0;
  e2->heldAnswer = -1;
  e2->orderPos = 5;
  e2->count = 5;
  correctAnswers[0] = 1;
  correctAnswers[1] = 0;
  correctAnswers[2] = 2;
  correctAnswers[3] = 0;
  correctAnswers[4] = 1;
  for (i = 0; i < 5; i++) {
    snprintf(pathBg, 128, "images/enigme2/q%d_bg.png", i + 1);
    e2->puzzles[i].bgTex =
        loadAndQuery(r, pathBg, &e2->puzzles[i].bgW, &e2->puzzles[i].bgH);
    for (j = 0; j < 3; j++) {
      snprintf(pathA, 128, "images/enigme2/q%d_a%d.png", i + 1, j + 1);
      e2->puzzles[i].answerTex[j] = loadAndQuery(
          r, pathA, &e2->puzzles[i].ansW[j], &e2->puzzles[i].ansH[j]);
    }
    e2->puzzles[i].correctAns = correctAnswers[i];
  }
  e2->successTex = loadTexture(r, "images/enigme2/success.png");
  e2->failTex = loadTexture(r, "images/enigme2/fail.png");
  if (cfg->screenW > 0) {
  }
}

void triggerEnigme2(Enigme2 *e2, SDL_Renderer *r, GameConfig *cfg) {
  int i, j, tmp;
  if (r) {
  }
  if (e2->orderPos >= e2->count) {
    for (i = 0; i < e2->count; i++)
      e2->order[i] = i;
    for (i = e2->count - 1; i > 0; i--) {
      j = rand() % (i + 1);
      tmp = e2->order[i];
      e2->order[i] = e2->order[j];
      e2->order[j] = tmp;
    }
    e2->orderPos = 0;
  }
  e2->currentPuzzle = e2->order[e2->orderPos];
  e2->orderPos = e2->orderPos + 1;
  e2->answered = 0;
  e2->correct = 0;
  e2->heldAnswer = -1;
  e2->active = 1;
  computeEnigme2Layout(e2, cfg);
}

void afficherEnigme2(Enigme2 *e2, SDL_Renderer *r, TTF_Font *f,
                     GameConfig *cfg) {
  SDL_Color white, yellow, green, red, fbCol;
  SDL_Rect fs, rect, bounding, drawn;
  SDL_Texture *t;
  int i;
  DragPuzzle *puz;
  char *fbMsg;
  if (!e2->active)
    return;
  white.r = 255;
  white.g = 255;
  white.b = 255;
  white.a = 255;
  yellow.r = 255;
  yellow.g = 215;
  yellow.b = 0;
  yellow.a = 255;
  green.r = 60;
  green.g = 220;
  green.b = 100;
  green.a = 255;
  red.r = 220;
  red.g = 60;
  red.b = 60;
  red.a = 255;
  fs.x = 0;
  fs.y = 0;
  fs.w = cfg->screenW;
  fs.h = cfg->screenH;
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(r, 10, 10, 30, 245);
  SDL_RenderFillRect(r, &fs);
  puz = &e2->puzzles[e2->currentPuzzle];
  if (puz->bgTex)
    SDL_RenderCopy(r, puz->bgTex, NULL, &e2->bgRect);
  else {
    SDL_SetRenderDrawColor(r, 60, 60, 60, 255);
    SDL_RenderFillRect(r, &e2->bgRect);
  }
  SDL_SetRenderDrawColor(r, 255, 255, 255, 220);
  SDL_RenderDrawRect(r, &e2->slotRect);
  rect.x = 0;
  rect.y = 0;
  rect.w = 0;
  rect.h = 0;
  t = makeText(r, f, "GLISSER LE BON VISAGE !", yellow, &rect);
  if (t) {
    rect.x = (cfg->screenW - rect.w) / 2;
    rect.y = 6;
    SDL_RenderCopy(r, t, NULL, &rect);
    SDL_DestroyTexture(t);
  }
  for (i = 0; i < 3; i++) {
    if (i == e2->heldAnswer || e2->answered)
      continue;
    bounding = e2->answerRects[i];
    if (puz->answerTex[i]) {
      drawn = fitRect(bounding.x, bounding.y, bounding.w, bounding.h,
                      puz->ansW[i], puz->ansH[i]);
      SDL_RenderCopy(r, puz->answerTex[i], NULL, &drawn);
    } else {
      SDL_SetRenderDrawColor(r, 80, 80, 80, 255);
      SDL_RenderFillRect(r, &bounding);
    }
    SDL_SetRenderDrawColor(r, 180, 180, 180, 200);
    SDL_RenderDrawRect(r, &bounding);
  }
  if (e2->heldAnswer >= 0 && !e2->answered) {
    if (puz->answerTex[e2->heldAnswer]) {
      drawn = fitRect(e2->heldRect.x, e2->heldRect.y, e2->heldRect.w,
                      e2->heldRect.h, puz->ansW[e2->heldAnswer],
                      puz->ansH[e2->heldAnswer]);
      SDL_RenderCopy(r, puz->answerTex[e2->heldAnswer], NULL, &drawn);
    }
    SDL_SetRenderDrawColor(r, 255, 215, 0, 220);
    SDL_RenderDrawRect(r, &e2->heldRect);
  }
  if (e2->answered) {
    if (e2->correct) {
      fbCol = green;
      fbMsg = "Correct ! +10 points";
    } else {
      fbCol = red;
      fbMsg = "Mauvaise reponse ! -1 vie";
    }
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, f, fbMsg, fbCol, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = cfg->screenH - 80;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, f, "Appuyez ESPACE pour continuer", white, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = cfg->screenH - 50;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
  }
}

void handleEnigme2Mouse(GameContext *ctx, Enigme2 *e2, SDL_Event *ev) {
  int i, mx, my;
  SDL_Rect *ar;
  if (e2->answered)
    return;
  if (ev->type == SDL_MOUSEBUTTONDOWN && ev->button.button == SDL_BUTTON_LEFT) {
    SDL_GetMouseState(&mx, &my);
    mouseToLogical(ctx, &mx, &my);
    for (i = 0; i < 3; i++) {
      ar = &e2->answerRects[i];
      if (mx >= ar->x - 8 && mx <= ar->x + ar->w + 8 && my >= ar->y - 8 &&
          my <= ar->y + ar->h + 8) {
        e2->heldAnswer = i;
        e2->dragOffX = mx - ar->x;
        e2->dragOffY = my - ar->y;
        e2->heldRect = *ar;
        break;
      }
    }
  } else if (ev->type == SDL_MOUSEBUTTONUP &&
             ev->button.button == SDL_BUTTON_LEFT) {
    if (e2->heldAnswer >= 0) {
      if (SDL_HasIntersection(&e2->heldRect, &e2->slotRect)) {
        e2->answered = 1;
        e2->correct =
            (e2->heldAnswer == e2->puzzles[e2->currentPuzzle].correctAns);
      } else {
        e2->heldRect = e2->answerRects[e2->heldAnswer];
        e2->heldAnswer = -1;
      }
    }
  }
}

tracks the cursor smoothly even if MOUSEMOTION events lag.* /
    void updateEnigme2Drag(GameContext *ctx, Enigme2 *e2) {
  int mx, my, buttons;
  if (e2->answered || e2->heldAnswer < 0)
    return;
  buttons = SDL_GetMouseState(&mx, &my);
  if (!(buttons & SDL_BUTTON(SDL_BUTTON_LEFT))) {
    if (SDL_HasIntersection(&e2->heldRect, &e2->slotRect)) {
      e2->answered = 1;
      e2->correct =
          (e2->heldAnswer == e2->puzzles[e2->currentPuzzle].correctAns);
    } else {
      e2->heldRect = e2->answerRects[e2->heldAnswer];
      e2->heldAnswer = -1;
    }
    return;
  }
  mouseToLogical(ctx, &mx, &my);
  e2->heldRect.x = mx - e2->dragOffX;
  e2->heldRect.y = my - e2->dragOffY;
}

void freeEnigme2(Enigme2 *e2) {
  int i, j;
  for (i = 0; i < 5; i++) {
    if (e2->puzzles[i].bgTex)
      SDL_DestroyTexture(e2->puzzles[i].bgTex);
    for (j = 0; j < 3; j++)
      if (e2->puzzles[i].answerTex[j])
        SDL_DestroyTexture(e2->puzzles[i].answerTex[j]);
  }
  if (e2->successTex)
    SDL_DestroyTexture(e2->successTex);
  if (e2->failTex)
    SDL_DestroyTexture(e2->failTex);
}

int loadTopScores(ScoreEntry scores[], int max) {
  FILE *f;
  int nb = 0, score;
  char name[32];
  f = fopen("scores.txt", "r");
  if (!f)
    return 0;
  while (nb < max && fscanf(f, "%31s %d", name, &score) == 2) {
    strncpy(scores[nb].name, name, 31);
    scores[nb].name[31] = '\0';
    scores[nb].score = score;
    nb++;
  }
  fclose(f);
  {
    int i, j;
    ScoreEntry tmp;
    for (i = 0; i < nb - 1; i++)
      for (j = 0; j < nb - 1 - i; j++)
        if (scores[j].score < scores[j + 1].score) {
          tmp = scores[j];
          scores[j] = scores[j + 1];
          scores[j + 1] = tmp;
        }
  }
  return nb;
}

void saveScore(char *name, int score) {
  FILE *f = fopen("scores.txt", "a");
  if (!f)
    return;
  fprintf(f, "%s %d\n", name, score);
  fclose(f);
}

void saveGameState(GameContext *ctx) {
  FILE *f;
  Personnage *p1, *p2;
  int p1x, p1y, p2x, p2y;
  f = fopen("save.txt", "w");
  if (!f)
    return;
  p1 = &ctx->player1;
  p2 = &ctx->player2;
  p1x = p1->x;
  p1y = p1->y;
  p2x = p2->x;
  p2y = p2->y;
  fprintf(f, "%d\n", ctx->currentLevel);
  fprintf(f, "%d\n", ctx->cfg.multiplayer);
  fprintf(f, "%d\n", ctx->menu.chosenAvatar);
  fprintf(f, "%d\n", ctx->menu.chosenInput);
  fprintf(f, "%d %d\n", p1x, p1y);
  fprintf(f, "%d %d %d %d\n", p1->vie, p1->score, p1->superAmmo, p1->shield);
  fprintf(f, "%d %d\n", p2x, p2y);
  fprintf(f, "%d %d %d %d\n", p2->vie, p2->score, p2->superAmmo, p2->shield);
  fprintf(f, "%d\n", ctx->elapsedSeconds);
  fclose(f);
  ctx->menu.hasSavedGame = 1;
}

int loadGameState(GameContext *ctx) {
  FILE *f;
  Personnage *p1, *p2;
  int level, mp, av, inp;
  int p1x, p1y, p1v, p1s, p1a, p1sh;
  int p2x, p2y, p2v, p2s, p2a, p2sh;
  int elapsed;
  f = fopen("save.txt", "r");
  if (!f)
    return 0;
  if (fscanf(f, "%d", &level) != 1) {
    fclose(f);
    return 0;
  }
  if (fscanf(f, "%d", &mp) != 1) {
    fclose(f);
    return 0;
  }
  if (fscanf(f, "%d", &av) != 1) {
    fclose(f);
    return 0;
  }
  if (fscanf(f, "%d", &inp) != 1) {
    fclose(f);
    return 0;
  }
  if (fscanf(f, "%d %d", &p1x, &p1y) != 2) {
    fclose(f);
    return 0;
  }
  if (fscanf(f, "%d %d %d %d", &p1v, &p1s, &p1a, &p1sh) != 4) {
    fclose(f);
    return 0;
  }
  if (fscanf(f, "%d %d", &p2x, &p2y) != 2) {
    fclose(f);
    return 0;
  }
  if (fscanf(f, "%d %d %d %d", &p2v, &p2s, &p2a, &p2sh) != 4) {
    fclose(f);
    return 0;
  }
  if (fscanf(f, "%d", &elapsed) != 1) {
    fclose(f);
    return 0;
  }
  fclose(f);

  if (level < 0 || level >= ctx->cfg.maxLevels)
    return 0;

  /* re-init players with saved avatar */
  freePersonnage(&ctx->player1);
  freePersonnage(&ctx->player2);
  ctx->cfg.multiplayer = mp;
  ctx->menu.chosenAvatar = av;
  ctx->menu.chosenInput = inp;
  initPersonnage(&ctx->player1, ctx->renderer, &ctx->cfg, p1x, p1y,
                 av == 0 ? 0 : 1);
  initPersonnage(&ctx->player2, ctx->renderer, &ctx->cfg, p2x, p2y,
                 av == 0 ? 1 : 0);
  p1 = &ctx->player1;
  p2 = &ctx->player2;
  p1->x = p1x;
  p1->y = p1y;
  p1->vie = p1v;
  p1->score = p1s;
  p1->superAmmo = p1a;
  p1->shield = p1sh;
  p2->x = p2x;
  p2->y = p2y;
  p2->vie = p2v;
  p2->score = p2s;
  p2->superAmmo = p2a;
  p2->shield = p2sh;
  ctx->currentLevel = level;
  /* reset gameStartTick so elapsedSeconds resumes from saved value */
  ctx->gameStartTick = SDL_GetTicks() - elapsed * 1000;
  ctx->elapsedSeconds = elapsed;
  return 1;
}

int hasSavedGame() {
  FILE *f = fopen("save.txt", "r");
  if (!f)
    return 0;
  fclose(f);
  return 1;
}

int loadHistoireText(char *buf, int bufLen) {
  FILE *f;
  int n;
  f = fopen("histoire.txt", "r");
  if (!f) {
    n = snprintf(buf, bufLen,
                 "L'HISTOIRE DE JOHN WICK\n"
                 "\n"
                 "Apres la mort de sa femme, John Wick recoit un dernier\n"
                 "cadeau : un chiot nomme Daisy.\n"
                 "\n"
                 "Lorsque des truands tuent Daisy et volent sa voiture,\n"
                 "John reprend les armes et plonge a nouveau dans le monde\n"
                 "des assassins qu'il avait quitte.\n"
                 "\n"
                 "Son voyage le mene a traverser trois lieux :\n"
                 "  - Niveau 1 : Les rues de la ville\n"
                 "  - Niveau 2 : L'hotel Continental\n"
                 "  - Niveau 3 : Les catacombes\n"
                 "\n"
                 "(Creer un fichier histoire.txt a cote du jeu pour\n"
                 " remplacer ce texte par votre propre histoire.)\n");
    return n;
  }
  n = fread(buf, 1, bufLen - 1, f);
  if (n < 0)
    n = 0;
  buf[n] = '\0';
  fclose(f);
  return n;
}

void menuButtonCheck(MenuButton *btn, int mx, int my, Mix_Chunk *sfx,
                     int *wasHovered) {
  int isHover = (mx >= btn->rect.x && mx <= btn->rect.x + btn->rect.w &&
                 my >= btn->rect.y && my <= btn->rect.y + btn->rect.h);
  if (isHover && !btn->hovered && sfx)
    Mix_PlayChannel(-1, sfx, 0);
  btn->hovered = isHover;
  if (wasHovered)
    *wasHovered = isHover;
}

void renderMenuButton(SDL_Renderer *r, MenuButton *btn) {
  SDL_Texture *tex;
  if (btn->hovered && btn->hover)
    tex = btn->hover;
  else if (btn->normal)
    tex = btn->normal;
  else {
    SDL_SetRenderDrawColor(r, 60, 60, 80, 220);
    SDL_RenderFillRect(r, &btn->rect);
    return;
  }
  SDL_RenderCopy(r, tex, NULL, &btn->rect);
}

void makeMenuButton(MenuButton *btn, SDL_Renderer *r, char *pathNormal,
                    char *pathHover, int x, int y, int w, int h) {
  btn->normal = loadTexture(r, pathNormal);
  btn->hover = loadTexture(r, pathHover);
  btn->rect.x = x;
  btn->rect.y = y;
  btn->rect.w = w;
  btn->rect.h = h;
  btn->hovered = 0;
}

void initMenu(MenuState *m, SDL_Renderer *r, TTF_Font *f, GameConfig *cfg) {
  int bw = 200, bh = 50, bx = 50, by;

  memset(m, 0, sizeof(MenuState));
  m->menuScreen = 0;

  m->bg[0] = loadTexture(r, "images/menu/bg1.png");
  m->bg[1] = loadTexture(r, "images/menu/bg2.png");
  m->bg[2] = loadTexture(r, "images/menu/bg3.png");
  m->bg[3] = loadTexture(r, "images/menu/bg4.png");

  m->musicMain = Mix_LoadMUS("audio/menu_main.mp3");
  m->musicSub = Mix_LoadMUS("audio/menu_sub.mp3");
  m->musicVictory = Mix_LoadMUS("audio/victory.mp3");
  m->musicSuspense = Mix_LoadMUS("audio/suspense.mp3");
  m->musicLevel = Mix_LoadMUS("audio/level.mp3");
  m->musicCombat = Mix_LoadMUS("audio/combat.mp3");
  m->sfxHover = Mix_LoadWAV("audio/hover.wav");

  Mix_VolumeMusic(cfg->volume);

  by = 120;
  makeMenuButton(&m->btnJouer, r, "images/menu/btn_jouer.png",
                 "images/menu/btn_jouer_h.png", bx, by, bw, bh);
  by += 70;
  makeMenuButton(&m->btnOptions, r, "images/menu/btn_options.png",
                 "images/menu/btn_options_h.png", bx, by, bw, bh);
  by += 70;
  makeMenuButton(&m->btnScores, r, "images/menu/btn_scores.png",
                 "images/menu/btn_scores_h.png", bx, by, bw, bh);
  by += 70;
  makeMenuButton(&m->btnHistoire, r, "images/menu/btn_histoire.png",
                 "images/menu/btn_histoire_h.png", bx, by, bw, bh);
  by += 70;
  makeMenuButton(&m->btnQuitter, r, "images/menu/btn_quitter.png",
                 "images/menu/btn_quitter_h.png", cfg->screenW - bw - 30,
                 cfg->screenH - bh - 20, bw, bh);

  makeMenuButton(&m->btnVolDown, r, "images/menu/btn_diminuer.png",
                 "images/menu/btn_diminuer_h.png", 150, 200, bw, bh);
  makeMenuButton(&m->btnVolUp, r, "images/menu/btn_augmenter.png",
                 "images/menu/btn_augmenter_h.png", 400, 200, bw, bh);
  makeMenuButton(&m->btnNormal, r, "images/menu/btn_normal.png",
                 "images/menu/btn_normal_h.png", 150, 300, bw, bh);
  makeMenuButton(&m->btnFullscreen, r, "images/menu/btn_pleinecran.png",
                 "images/menu/btn_pleinecran_h.png", 400, 300, bw, bh);
  makeMenuButton(&m->btnRetourOpt, r, "images/menu/btn_retour.png",
                 "images/menu/btn_retour_h.png", cfg->screenW - bw - 30,
                 cfg->screenH - bh - 20, bw, bh);

  makeMenuButton(&m->btnOui, r, "images/menu/btn_oui.png",
                 "images/menu/btn_oui_h.png", 150, 300, bw, bh);
  makeMenuButton(&m->btnNon, r, "images/menu/btn_non.png",
                 "images/menu/btn_non_h.png", 400, 300, bw, bh);
  makeMenuButton(&m->btnCharger, r, "images/menu/btn_charger.png",
                 "images/menu/btn_charger_h.png", (cfg->screenW - bw) / 2, 200,
                 bw, bh);
  makeMenuButton(&m->btnNouvelle, r, "images/menu/btn_nouvelle.png",
                 "images/menu/btn_nouvelle_h.png", (cfg->screenW - bw) / 2, 290,
                 bw, bh);

  makeMenuButton(&m->btnMono, r, "images/menu/btn_mono.png",
                 "images/menu/btn_mono_h.png", 150, 250, bw, bh);
  makeMenuButton(&m->btnMulti, r, "images/menu/btn_multi.png",
                 "images/menu/btn_multi_h.png", 400, 250, bw, bh);
  makeMenuButton(&m->btnAvatar1, r, "images/menu/btn_avatar1.png",
                 "images/menu/btn_avatar1_h.png", 150, 180, bw, bh);
  makeMenuButton(&m->btnAvatar2, r, "images/menu/btn_avatar2.png",
                 "images/menu/btn_avatar2_h.png", 400, 180, bw, bh);
  makeMenuButton(&m->btnInput1, r, "images/menu/btn_input1.png",
                 "images/menu/btn_input1_h.png", 150, 260, bw, bh);
  makeMenuButton(&m->btnInput2, r, "images/menu/btn_input2.png",
                 "images/menu/btn_input2_h.png", 400, 260, bw, bh);
  makeMenuButton(&m->btnValiderPlayer, r, "images/menu/btn_valider.png",
                 "images/menu/btn_valider_h.png", (cfg->screenW - bw) / 2, 360,
                 bw, bh);
  makeMenuButton(&m->btnRetourPlayer, r, "images/menu/btn_retour.png",
                 "images/menu/btn_retour_h.png", cfg->screenW - bw - 30,
                 cfg->screenH - bh - 20, bw, bh);

  makeMenuButton(&m->btnValiderScore, r, "images/menu/btn_valider.png",
                 "images/menu/btn_valider_h.png", (cfg->screenW - bw) / 2, 340,
                 bw, bh);
  makeMenuButton(&m->btnRetourScore, r, "images/menu/btn_retour.png",
                 "images/menu/btn_retour_h.png", cfg->screenW - bw - 30,
                 cfg->screenH - bh - 20, bw, bh);
  makeMenuButton(&m->btnQuitterScore, r, "images/menu/btn_quitter.png",
                 "images/menu/btn_quitter_h.png", 30, cfg->screenH - bh - 20,
                 bw, bh);

  makeMenuButton(&m->btnQuiz, r, "images/menu/btn_quiz.png",
                 "images/menu/btn_quiz_h.png", 150, 250, bw, bh);
  makeMenuButton(&m->btnPuzzle, r, "images/menu/btn_puzzle.png",
                 "images/menu/btn_puzzle_h.png", 400, 250, bw, bh);
  makeMenuButton(&m->btnRetourHistoire, r, "images/menu/btn_retour.png",
                 "images/menu/btn_retour_h.png", cfg->screenW - bw - 30,
                 cfg->screenH - bh - 20, bw, bh);

  makeMenuButton(&m->btnHistoireUp, r, "images/menu/btn_up.png",
                 "images/menu/btn_up_h.png", cfg->screenW - 70, 100, 50, 50);
  makeMenuButton(&m->btnHistoireDown, r, "images/menu/btn_down.png",
                 "images/menu/btn_down_h.png", cfg->screenW - 70,
                 cfg->screenH - 130, 50, 50);

  by = 140;
  makeMenuButton(&m->btnReprendre, r, "images/menu/btn_reprendre.png",
                 "images/menu/btn_reprendre_h.png", (cfg->screenW - bw) / 2, by,
                 bw, bh);
  by += 70;
  makeMenuButton(&m->btnSauvegarder, r, "images/menu/btn_sauvegarder.png",
                 "images/menu/btn_sauvegarder_h.png", (cfg->screenW - bw) / 2,
                 by, bw, bh);
  by += 70;
  makeMenuButton(&m->btnOptionsPause, r, "images/menu/btn_options.png",
                 "images/menu/btn_options_h.png", (cfg->screenW - bw) / 2, by,
                 bw, bh);
  by += 70;
  makeMenuButton(&m->btnQuitterPartie, r, "images/menu/btn_quitter.png",
                 "images/menu/btn_quitter_h.png", (cfg->screenW - bw) / 2, by,
                 bw, bh);

  m->nameLen = 0;
  memset(m->nameInput, 0, 32);
  m->nameActive = 1;
  m->chosenAvatar = 0;
  m->chosenInput = 0;
  m->hasPendingScore = 0;
  m->optionsReturn = 0;
  m->previewAvatar1 = loadTexture(r, "images/Fighter/Right/Stand.png");
  m->previewAvatar2 = loadTexture(r, "images/Fighter2/Right/Stand.png");

  m->histoireLen = loadHistoireText(m->histoireText, 8192);
  m->histoireScroll = 0;

  m->hasSavedGame = hasSavedGame();

  m->nbTopScores = loadTopScores(m->topScores, 10);

  if (f) {
  }

  if (m->musicMain)
    Mix_PlayMusic(m->musicMain, -1);
}

void renderMenuBg(SDL_Renderer *r, SDL_Window *win, MenuState *m, int bgIdx,
                  GameConfig *cfg) {
  SDL_Rect full;
  int winW, winH;
  if (win) {
    SDL_GetWindowSize(win, &winW, &winH);
  } else {
    winW = cfg->screenW;
    winH = cfg->screenH;
  }
  SDL_RenderSetLogicalSize(r, 0, 0);
  full.x = 0;
  full.y = 0;
  full.w = winW;
  full.h = winH;
  if (bgIdx >= 0 && bgIdx < 4 && m->bg[bgIdx])
    SDL_RenderCopy(r, m->bg[bgIdx], NULL, &full);
  else {
    SDL_SetRenderDrawColor(r, 10, 10, 20, 255);
    SDL_RenderFillRect(r, &full);
  }
  SDL_RenderSetLogicalSize(r, cfg->screenW, cfg->screenH);
}

void renderMenu(GameContext *ctx) {
  SDL_Renderer *r = ctx->renderer;
  TTF_Font *f = ctx->font;
  MenuState *m = &ctx->menu;
  GameConfig *cfg = &ctx->cfg;
  int mx, my, i;
  SDL_Color white, yellow, gold;
  SDL_Rect rect;
  SDL_Texture *t;
  char buf[128];

  SDL_GetMouseState(&mx, &my);
  mouseToLogical(ctx, &mx, &my);
  white.r = 255;
  white.g = 255;
  white.b = 255;
  white.a = 255;
  yellow.r = 255;
  yellow.g = 215;
  yellow.b = 0;
  yellow.a = 255;
  gold.r = 255;
  gold.g = 200;
  gold.b = 50;
  gold.a = 255;

  SDL_SetRenderDrawColor(r, 10, 10, 20, 255);
  SDL_RenderClear(r);

  if (m->menuScreen == 0) {
    renderMenuBg(r, ctx->window, m, 0, cfg);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, ctx->fontLarge, "WICKLINE 2D", gold, &rect);
    if (t) {
      rect.x = cfg->screenW - rect.w - 40;
      rect.y = 60;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, f, "[ LOGO ]", white, &rect);
    if (t) {
      rect.x = cfg->screenW - rect.w - 40;
      rect.y = 20;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }

    menuButtonCheck(&m->btnJouer, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnOptions, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnScores, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnHistoire, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnQuitter, mx, my, m->sfxHover, NULL);

    renderMenuButton(r, &m->btnJouer);
    renderMenuButton(r, &m->btnOptions);
    renderMenuButton(r, &m->btnScores);
    renderMenuButton(r, &m->btnHistoire);
    renderMenuButton(r, &m->btnQuitter);
  } else if (m->menuScreen == 1) {
    renderMenuBg(r, ctx->window, m, 1, cfg);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, ctx->fontLarge, "OPTIONS", yellow, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = 60;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }

    snprintf(buf, 128, "Volume: %d", cfg->volume);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, f, buf, white, &rect);
    if (t) {
      rect.x = 280;
      rect.y = 210;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }

    menuButtonCheck(&m->btnVolDown, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnVolUp, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnNormal, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnFullscreen, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnRetourOpt, mx, my, m->sfxHover, NULL);

    renderMenuButton(r, &m->btnVolDown);
    renderMenuButton(r, &m->btnVolUp);
    renderMenuButton(r, &m->btnNormal);
    renderMenuButton(r, &m->btnFullscreen);
    renderMenuButton(r, &m->btnRetourOpt);

    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, f, "Volume", white, &rect);
    if (t) {
      rect.x = 50;
      rect.y = 215;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, f, "Affichage", white, &rect);
    if (t) {
      rect.x = 50;
      rect.y = 315;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
  } else if (m->menuScreen == 2) {
    renderMenuBg(r, ctx->window, m, 1, cfg);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, ctx->fontLarge, "Voulez-vous sauvegarder votre jeu ?",
                 yellow, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = 160;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    menuButtonCheck(&m->btnOui, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnNon, mx, my, m->sfxHover, NULL);
    renderMenuButton(r, &m->btnOui);
    renderMenuButton(r, &m->btnNon);
  } else if (m->menuScreen == 3) {
    renderMenuBg(r, ctx->window, m, 1, cfg);
    menuButtonCheck(&m->btnCharger, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnNouvelle, mx, my, m->sfxHover, NULL);
    renderMenuButton(r, &m->btnCharger);
    renderMenuButton(r, &m->btnNouvelle);
  } else if (m->menuScreen == 4) {
    renderMenuBg(r, ctx->window, m, 1, cfg);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, ctx->fontLarge, "Choisir le mode", yellow, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = 80;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    menuButtonCheck(&m->btnMono, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnMulti, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnRetourPlayer, mx, my, m->sfxHover, NULL);
    renderMenuButton(r, &m->btnMono);
    renderMenuButton(r, &m->btnMulti);
    renderMenuButton(r, &m->btnRetourPlayer);
  } else if (m->menuScreen == 5) {
    renderMenuBg(r, ctx->window, m, 1, cfg);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, ctx->fontLarge, "Choisir un avatar", yellow, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = 80;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    {
      char *hint;
      if (cfg->multiplayer)
        hint =
            "P1 : Z Q S D / Maj. / Espace      P2 : Fleches / Maj.D. / Ctrl.D.";
      else
        hint = "Touches : Z Q S D   Maj. = courir   Espace = tirer";
      rect.x = 0;
      rect.y = 0;
      rect.w = 0;
      rect.h = 0;
      t = makeText(r, f, hint, white, &rect);
      if (t) {
        rect.x = (cfg->screenW - rect.w) / 2;
        rect.y = cfg->screenH - 90;
        SDL_RenderCopy(r, t, NULL, &rect);
        SDL_DestroyTexture(t);
      }
    }

    menuButtonCheck(&m->btnAvatar1, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnAvatar2, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnValiderPlayer, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnRetourPlayer, mx, my, m->sfxHover, NULL);
    renderMenuButton(r, &m->btnAvatar1);
    renderMenuButton(r, &m->btnAvatar2);
    renderMenuButton(r, &m->btnValiderPlayer);
    renderMenuButton(r, &m->btnRetourPlayer);

    {
      SDL_Rect pSrc, pDst;
      SDL_Texture *tex;
      int j;
      int ptexW, ptexH;
      for (j = 0; j < 2; j++) {
        MenuButton *btn;
        if (j == 0) {
          tex = m->previewAvatar1;
          btn = &m->btnAvatar1;
        } else {
          tex = m->previewAvatar2;
          btn = &m->btnAvatar2;
        }
        if (!tex)
          continue;
        SDL_QueryTexture(tex, NULL, NULL, &ptexW, &ptexH);
        pSrc.x = 0;
        pSrc.y = 0;
        if (cfg->frameCount > 0)
          pSrc.w = ptexW / cfg->frameCount;
        else
          pSrc.w = ptexW;
        pSrc.h = ptexH;
        pDst.w = 96;
        pDst.h = 120;
        pDst.x = btn->rect.x + (btn->rect.w - pDst.w) / 2;
        pDst.y = btn->rect.y + btn->rect.h + 12;
        {
          SDL_Rect frame = pDst;
          frame.x = frame.x - 6;
          frame.y = frame.y - 6;
          frame.w = frame.w + 12;
          frame.h = frame.h + 12;
          SDL_SetRenderDrawColor(r, 18, 18, 30, 200);
          SDL_RenderFillRect(r, &frame);
          if (m->chosenAvatar == j)
            SDL_SetRenderDrawColor(r, 255, 215, 0, 255);
          else
            SDL_SetRenderDrawColor(r, 100, 100, 110, 255);
          SDL_RenderDrawRect(r, &frame);
        }
        SDL_RenderCopy(r, tex, &pSrc, &pDst);
      }
    }

    SDL_SetRenderDrawColor(r, 255, 215, 0, 200);
    if (m->chosenAvatar == 0)
      SDL_RenderDrawRect(r, &m->btnAvatar1.rect);
    else
      SDL_RenderDrawRect(r, &m->btnAvatar2.rect);
  } else if (m->menuScreen == 6) {
    renderMenuBg(r, ctx->window, m, 2, cfg);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, ctx->fontLarge, "Entrer votre nom", yellow, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = 100;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }

    {
      SDL_Rect box = {250, 220, 300, 50};
      SDL_SetRenderDrawColor(r, 40, 40, 60, 220);
      SDL_RenderFillRect(r, &box);
      SDL_SetRenderDrawColor(r, 180, 180, 255, 255);
      SDL_RenderDrawRect(r, &box);
      rect.x = 0;
      rect.y = 0;
      rect.w = 0;
      rect.h = 0;
      t = makeText(r, f, m->nameInput, white, &rect);
      if (t) {
        rect.x = box.x + 10;
        rect.y = box.y + (box.h - rect.h) / 2;
        SDL_RenderCopy(r, t, NULL, &rect);
        SDL_DestroyTexture(t);
      }
    }
    menuButtonCheck(&m->btnValiderScore, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnRetourScore, mx, my, m->sfxHover, NULL);
    renderMenuButton(r, &m->btnValiderScore);
    renderMenuButton(r, &m->btnRetourScore);
  } else if (m->menuScreen == 7) {
    renderMenuBg(r, ctx->window, m, 2, cfg);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, ctx->fontLarge, "Meilleurs Scores", gold, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = 60;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }

    for (i = 0; i < m->nbTopScores; i++) {
      snprintf(buf, 128, "%2d.  %-20s  %d", i + 1, m->topScores[i].name,
               m->topScores[i].score);
      rect.x = 0;
      rect.y = 0;
      rect.w = 0;
      rect.h = 0;
      t = makeText(r, f, buf, white, &rect);
      if (t) {
        rect.x = (cfg->screenW - rect.w) / 2;
        rect.y = 130 + i * 32;
        SDL_RenderCopy(r, t, NULL, &rect);
        SDL_DestroyTexture(t);
      }
    }
    if (m->nbTopScores == 0) {
      rect.x = 0;
      rect.y = 0;
      rect.w = 0;
      rect.h = 0;
      t = makeText(r, f, "Aucun score enregistre — jouez !", white, &rect);
      if (t) {
        rect.x = (cfg->screenW - rect.w) / 2;
        rect.y = 260;
        SDL_RenderCopy(r, t, NULL, &rect);
        SDL_DestroyTexture(t);
      }
    }
    menuButtonCheck(&m->btnRetourScore, mx, my, m->sfxHover, NULL);
    renderMenuButton(r, &m->btnRetourScore);
  } else if (m->menuScreen == 8) {
    renderMenuBg(r, ctx->window, m, 3, cfg);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, ctx->fontLarge, "Enigme", yellow, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = 80;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    menuButtonCheck(&m->btnQuiz, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnPuzzle, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnRetourHistoire, mx, my, m->sfxHover, NULL);
    renderMenuButton(r, &m->btnQuiz);
    renderMenuButton(r, &m->btnPuzzle);
    renderMenuButton(r, &m->btnRetourHistoire);
  } else if (m->menuScreen == 9) {
    renderMenuBg(r, ctx->window, m, 3, cfg);
    afficherEnigme(&ctx->enigme1, r, mx, my, cfg);
    afficherKeyboardHints(r, f, m->selectedKey, cfg);
    afficherFeedback(r, f, &ctx->enigme1, ctx->feedbackStart, cfg);
  } else if (m->menuScreen == 10) {
    renderMenuBg(r, ctx->window, m, 1, cfg);
    {
      SDL_Rect veil;
      veil.x = 0;
      veil.y = 0;
      veil.w = cfg->screenW;
      veil.h = cfg->screenH;
      SDL_SetRenderDrawColor(r, 0, 0, 0, 140);
      SDL_RenderFillRect(r, &veil);
    }
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, ctx->fontLarge, "Pause", yellow, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = 70;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    menuButtonCheck(&m->btnReprendre, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnSauvegarder, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnOptionsPause, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnQuitterPartie, mx, my, m->sfxHover, NULL);
    renderMenuButton(r, &m->btnReprendre);
    renderMenuButton(r, &m->btnSauvegarder);
    renderMenuButton(r, &m->btnOptionsPause);
    renderMenuButton(r, &m->btnQuitterPartie);
  } else if (m->menuScreen == 12) {
    renderMenuBg(r, ctx->window, m, 3, cfg);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(r, ctx->fontLarge, "Histoire", gold, &rect);
    if (t) {
      rect.x = (cfg->screenW - rect.w) / 2;
      rect.y = 30;
      SDL_RenderCopy(r, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    {
      SDL_Rect panel;
      panel.x = 60;
      panel.y = 90;
      panel.w = cfg->screenW - 150;
      panel.h = cfg->screenH - 180;
      SDL_SetRenderDrawColor(r, 20, 20, 35, 200);
      SDL_RenderFillRect(r, &panel);
      SDL_SetRenderDrawColor(r, 180, 160, 80, 255);
      SDL_RenderDrawRect(r, &panel);
      SDL_RenderSetClipRect(r, &panel);
      {
        char line[256];
        int lineY, lineH, len, j, ln;
        lineY = panel.y + 12 - m->histoireScroll;
        lineH = 22;
        len = m->histoireLen;
        ln = 0;
        j = 0;
        while (ln < len) {
          int end = ln;
          while (end < len && m->histoireText[end] != '\n' && (end - ln) < 255)
            end++;
          j = end - ln;
          if (j > 255)
            j = 255;
          memcpy(line, m->histoireText + ln, j);
          line[j] = '\0';
          if (lineY + lineH >= panel.y && lineY <= panel.y + panel.h &&
              line[0] != '\0') {
            rect.x = 0;
            rect.y = 0;
            rect.w = 0;
            rect.h = 0;
            t = makeText(r, f, line, white, &rect);
            if (t) {
              rect.x = panel.x + 14;
              rect.y = lineY;
              SDL_RenderCopy(r, t, NULL, &rect);
              SDL_DestroyTexture(t);
            }
          }
          lineY += lineH;
          if (end < len && m->histoireText[end] == '\n')
            end++;
          ln = end;
          if (lineY > panel.y + panel.h + 100)
            break;
        }
      }
      SDL_RenderSetClipRect(r, NULL);
    }
    menuButtonCheck(&m->btnHistoireUp, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnHistoireDown, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnRetourHistoire, mx, my, m->sfxHover, NULL);
    renderMenuButton(r, &m->btnHistoireUp);
    renderMenuButton(r, &m->btnHistoireDown);
    renderMenuButton(r, &m->btnRetourHistoire);
  }

  SDL_RenderPresent(r);
}

void handleMenuEvents(GameContext *ctx, SDL_Event *ev) {
  MenuState *m = &ctx->menu;
  GameConfig *cfg = &ctx->cfg;
  int mx, my;

  SDL_GetMouseState(&mx, &my);
  mouseToLogical(ctx, &mx, &my);

  if (ev->type == SDL_QUIT) {
    ctx->running = 0;
    return;
  }

  if (ev->type == SDL_KEYDOWN) {
    if (ev->key.keysym.sym == SDLK_ESCAPE) {
      ctx->running = 0;
      return;
    }

    if (m->menuScreen == 0) {
      if (ev->key.keysym.sym == SDLK_j) {
        if (m->hasSavedGame)
          m->menuScreen = 3;
        else
          m->menuScreen = 4;
        return;
      }
      if (ev->key.keysym.sym == SDLK_o) {
        m->optionsReturn = 0;
        m->menuScreen = 1;
        return;
      }
      if (ev->key.keysym.sym == SDLK_h) {
        m->histoireScroll = 0;
        m->menuScreen = 12;
        return;
      }
      if (ev->key.keysym.sym == SDLK_s) {
        m->nbTopScores = loadTopScores(m->topScores, 10);
        m->menuScreen = 7;
        return;
      }
    }

    if (m->menuScreen == 1) {
      if (ev->key.keysym.sym == SDLK_PLUS ||
          ev->key.keysym.sym == SDLK_KP_PLUS) {
        cfg->volume = cfg->volume + 8;
        if (cfg->volume > 128)
          cfg->volume = 128;
        Mix_VolumeMusic(cfg->volume);
        return;
      }
      if (ev->key.keysym.sym == SDLK_MINUS ||
          ev->key.keysym.sym == SDLK_KP_MINUS) {
        cfg->volume = cfg->volume - 8;
        if (cfg->volume < 0)
          cfg->volume = 0;
        Mix_VolumeMusic(cfg->volume);
        return;
      }
    }

    if (m->menuScreen == 4) {
      if (ev->key.keysym.sym == SDLK_n) {
        m->menuScreen = 5;
        return;
      }
    }

    if (m->menuScreen == 5) {
      if (ev->key.keysym.sym == SDLK_RETURN) {
        m->menuScreen = 6;
        Mix_HaltMusic();
        return;
      }
    }

    if (m->menuScreen == 6) {
      if (ev->key.keysym.sym == SDLK_RETURN) {
        if (m->nameLen > 0) {
          if (m->hasPendingScore) {
            saveScore(m->nameInput, m->pendingScore);
            m->hasPendingScore = 0;
            m->pendingScore = 0;
          }
          m->nbTopScores = loadTopScores(m->topScores, 10);
          m->menuScreen = 7;
          Mix_HaltMusic();
          if (m->musicVictory)
            Mix_PlayMusic(m->musicVictory, 1);
        }
        return;
      }
      if (ev->key.keysym.sym == SDLK_BACKSPACE && m->nameLen > 0) {
        m->nameLen--;
        m->nameInput[m->nameLen] = '\0';
        return;
      }
    }

    if (m->menuScreen == 8) {
      if (ev->key.keysym.sym == SDLK_e) {
        m->menuScreen = 9;
        return;
      }
    }

    if (m->menuScreen == 12) {
      int step = 40;
      if (ev->key.keysym.sym == SDLK_DOWN ||
          ev->key.keysym.sym == SDLK_PAGEDOWN ||
          ev->key.keysym.sym == SDLK_SPACE) {
        m->histoireScroll = m->histoireScroll + step;
        return;
      }
      if (ev->key.keysym.sym == SDLK_UP || ev->key.keysym.sym == SDLK_PAGEUP) {
        m->histoireScroll = m->histoireScroll - step;
        if (m->histoireScroll < 0)
          m->histoireScroll = 0;
        return;
      }
    }

    if (m->menuScreen == 9) {
      if (ctx->enigme1.etat == cfg->enigmeAttente) {
        int choice = 0;
        if (ev->key.keysym.sym == SDLK_a)
          choice = 1;
        if (ev->key.keysym.sym == SDLK_b)
          choice = 2;
        if (ev->key.keysym.sym == SDLK_c)
          choice = 3;
        if (choice != 0) {
          m->selectedKey = choice;
          resolveEnigme(choice, &ctx->enigme1, &ctx->player1.score,
                        &ctx->player1.vie, cfg);
          ctx->feedbackStart = SDL_GetTicks();
        }
      }
    }
  }

  if (ev->type == SDL_TEXTINPUT && m->menuScreen == 6) {
    if (m->nameLen < 31) {
      m->nameInput[m->nameLen] = ev->text.text[0];
      m->nameLen++;
      m->nameInput[m->nameLen] = '\0';
    }
    return;
  }

  if (ev->type == SDL_MOUSEBUTTONDOWN && ev->button.button == SDL_BUTTON_LEFT) {
    if (m->menuScreen == 0) {
      if (m->btnJouer.hovered) {
        if (m->hasSavedGame)
          m->menuScreen = 3;
        else
          m->menuScreen = 4;
        return;
      }
      if (m->btnOptions.hovered) {
        m->optionsReturn = 0;
        m->menuScreen = 1;
        return;
      }
      if (m->btnScores.hovered) {
        m->nbTopScores = loadTopScores(m->topScores, 10);
        m->menuScreen = 7;
        return;
      }
      if (m->btnHistoire.hovered) {
        m->histoireScroll = 0;
        m->menuScreen = 12;
        return;
      }
      if (m->btnQuitter.hovered) {
        ctx->running = 0;
        return;
      }
    } else if (m->menuScreen == 1) {
      if (m->btnVolDown.hovered) {
        cfg->volume = cfg->volume - 8;
        if (cfg->volume < 0)
          cfg->volume = 0;
        Mix_VolumeMusic(cfg->volume);
        return;
      }
      if (m->btnVolUp.hovered) {
        cfg->volume = cfg->volume + 8;
        if (cfg->volume > 128)
          cfg->volume = 128;
        Mix_VolumeMusic(cfg->volume);
        return;
      }
      if (m->btnFullscreen.hovered) {
        SDL_SetWindowFullscreen(ctx->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        ctx->isFullscreen = 1;
        return;
      }
      if (m->btnNormal.hovered) {
        SDL_SetWindowFullscreen(ctx->window, 0);
        ctx->isFullscreen = 0;
        return;
      }
      if (m->btnRetourOpt.hovered) {
        m->menuScreen = m->optionsReturn;
        return;
      }
    } else if (m->menuScreen == 2) {
      if (m->btnOui.hovered) {
        saveGameState(ctx);
        m->hasSavedGame = 1;
        m->menuScreen = 10;
        return;
      }
      if (m->btnNon.hovered) {
        m->menuScreen = 10;
        return;
      }
    } else if (m->menuScreen == 3) {
      if (m->btnNouvelle.hovered) {
        m->menuScreen = 4;
        return;
      }
      if (m->btnCharger.hovered) {
        if (loadGameState(ctx)) {
          Mix_HaltMusic();
          ctx->gameState = cfg->statePlatformer;
        } else {
          m->hasSavedGame = 0;
          m->menuScreen = 4;
        }
        return;
      }
      if (m->btnRetourPlayer.hovered) {
        m->menuScreen = 0;
        return;
      }
    } else if (m->menuScreen == 4) {
      if (m->btnMono.hovered) {
        cfg->multiplayer = 0;
        m->menuScreen = 5;
        return;
      }
      if (m->btnMulti.hovered) {
        cfg->multiplayer = 1;
        m->menuScreen = 5;
        return;
      }
      if (m->btnRetourPlayer.hovered) {
        m->menuScreen = 0;
        return;
      }
    } else if (m->menuScreen == 5) {
      if (m->btnAvatar1.hovered) {
        m->chosenAvatar = 0;
        return;
      }
      if (m->btnAvatar2.hovered) {
        m->chosenAvatar = 1;
        return;
      }
      if (m->btnRetourPlayer.hovered) {
        m->menuScreen = 4;
        return;
      }
      if (m->btnValiderPlayer.hovered) {
        Mix_HaltMusic();
        ctx->currentLevel = 0;
        ctx->gameState = cfg->statePlatformer;
        freePersonnage(&ctx->player1);
        freePersonnage(&ctx->player2);
        initPersonnage(&ctx->player1, ctx->renderer, cfg, 80,
                       cfg->groundY - cfg->collH, m->chosenAvatar == 0 ? 0 : 1);
        initPersonnage(&ctx->player2, ctx->renderer, cfg, 160,
                       cfg->groundY - cfg->collH, m->chosenAvatar == 0 ? 1 : 0);
        ctx->gameStartTick = SDL_GetTicks();
        return;
      }
    } else if (m->menuScreen == 6) {
      if (m->btnValiderScore.hovered && m->nameLen > 0) {
        if (m->hasPendingScore) {
          saveScore(m->nameInput, m->pendingScore);
          m->hasPendingScore = 0;
          m->pendingScore = 0;
        }
        m->nbTopScores = loadTopScores(m->topScores, 10);
        m->menuScreen = 7;
        Mix_HaltMusic();
        if (m->musicVictory)
          Mix_PlayMusic(m->musicVictory, 1);
        return;
      }
      if (m->btnRetourScore.hovered) {
        m->menuScreen = 0;
        return;
      }
    } else if (m->menuScreen == 7) {
      /* ===== LEADERBOARD ===== */
      if (m->btnRetourScore.hovered) {
        Mix_HaltMusic();
        if (m->musicMain)
          Mix_PlayMusic(m->musicMain, -1);
        m->menuScreen = 0;
        return;
      }
    } else if (m->menuScreen == 8) {
      if (m->btnQuiz.hovered) {
        if (m->musicSuspense)
          Mix_PlayMusic(m->musicSuspense, -1);
        m->menuScreen = 9;
        if (ctx->quizCount > 0) {
          libererEnigme(&ctx->enigme1);
          ctx->enigme1 =
              genererEnigme(ctx->quizBlocks, ctx->quizCount, &ctx->shuffleState,
                            ctx->renderer, ctx->font, cfg);
        }
        return;
      }
      if (m->btnPuzzle.hovered) {
        ctx->gameState = cfg->stateEnigme2;
        triggerEnigme2(&ctx->enigme2, ctx->renderer, cfg);
        return;
      }
      if (m->btnRetourHistoire.hovered) {
        m->menuScreen = 0;
        return;
      }
    } else if (m->menuScreen == 9) {
      if (ctx->enigme1.etat == cfg->enigmeAttente) {
        int clicked = 0;
        if (mx > ctx->enigme1.posRep1.x &&
            mx < ctx->enigme1.posRep1.x + ctx->enigme1.posRep1.w &&
            my > ctx->enigme1.posRep1.y &&
            my < ctx->enigme1.posRep1.y + ctx->enigme1.posRep1.h)
          clicked = 1;
        if (mx > ctx->enigme1.posRep2.x &&
            mx < ctx->enigme1.posRep2.x + ctx->enigme1.posRep2.w &&
            my > ctx->enigme1.posRep2.y &&
            my < ctx->enigme1.posRep2.y + ctx->enigme1.posRep2.h)
          clicked = 2;
        if (mx > ctx->enigme1.posRep3.x &&
            mx < ctx->enigme1.posRep3.x + ctx->enigme1.posRep3.w &&
            my > ctx->enigme1.posRep3.y &&
            my < ctx->enigme1.posRep3.y + ctx->enigme1.posRep3.h)
          clicked = 3;
        if (clicked) {
          resolveEnigme(clicked, &ctx->enigme1, &ctx->player1.score,
                        &ctx->player1.vie, cfg);
          ctx->feedbackStart = SDL_GetTicks();
        }
      }
    } else if (m->menuScreen == 10) {
      if (m->btnReprendre.hovered) {
        Mix_HaltMusic();
        ctx->gameState = cfg->statePlatformer;
        return;
      }
      if (m->btnSauvegarder.hovered) {
        m->menuScreen = 2;
        return;
      }
      if (m->btnOptionsPause.hovered) {
        m->optionsReturn = 10;
        m->menuScreen = 1;
        return;
      }
      if (m->btnQuitterPartie.hovered) {
        Mix_HaltMusic();
        if (m->musicMain)
          Mix_PlayMusic(m->musicMain, -1);
        m->menuScreen = 0;
        return;
      }
    } else if (m->menuScreen == 12) {
      if (m->btnHistoireUp.hovered) {
        m->histoireScroll = m->histoireScroll - 40;
        if (m->histoireScroll < 0)
          m->histoireScroll = 0;
        return;
      }
      if (m->btnHistoireDown.hovered) {
        m->histoireScroll = m->histoireScroll + 40;
        return;
      }
      if (m->btnRetourHistoire.hovered) {
        m->menuScreen = 0;
        return;
      }
    }
  }

  if (ev->type == SDL_MOUSEWHEEL && m->menuScreen == 12) {
    m->histoireScroll = m->histoireScroll - ev->wheel.y * 30;
    if (m->histoireScroll < 0)
      m->histoireScroll = 0;
  }
}

void updateMenu(GameContext *ctx) { ctx->running = ctx->running; }

void freeMenu(MenuState *m) {
  int i;
  for (i = 0; i < 4; i++) {
    if (m->bg[i])
      SDL_DestroyTexture(m->bg[i]);
  }
  if (m->musicMain)
    Mix_FreeMusic(m->musicMain);
  if (m->musicSub)
    Mix_FreeMusic(m->musicSub);
  if (m->musicVictory)
    Mix_FreeMusic(m->musicVictory);
  if (m->musicSuspense)
    Mix_FreeMusic(m->musicSuspense);
  if (m->musicLevel)
    Mix_FreeMusic(m->musicLevel);
  if (m->musicCombat)
    Mix_FreeMusic(m->musicCombat);
  if (m->sfxHover)
    Mix_FreeChunk(m->sfxHover);
  if (m->previewAvatar1)
    SDL_DestroyTexture(m->previewAvatar1);
  if (m->previewAvatar2)
    SDL_DestroyTexture(m->previewAvatar2);
#define FB(btn)                                                                \
  {                                                                            \
    if (m->btn.normal)                                                         \
      SDL_DestroyTexture(m->btn.normal);                                       \
    if (m->btn.hover)                                                          \
      SDL_DestroyTexture(m->btn.hover);                                        \
  }
  FB(btnJouer)
  FB(btnOptions)
  FB(btnScores)
  FB(btnHistoire)
  FB(btnQuitter)
  FB(btnVolUp)
  FB(btnVolDown)
  FB(btnNormal)
  FB(btnFullscreen)
  FB(btnRetourOpt)
  FB(btnOui)
  FB(btnNon)
  FB(btnCharger) FB(btnNouvelle) FB(btnMono) FB(btnMulti) FB(btnAvatar1)
      FB(btnAvatar2) FB(btnInput1) FB(btnInput2) FB(btnValiderPlayer)
          FB(btnRetourPlayer) FB(btnValiderScore) FB(btnRetourScore)
              FB(btnQuitterScore) FB(btnQuiz) FB(btnPuzzle)
                  FB(btnRetourHistoire) FB(btnHistoireUp) FB(btnHistoireDown)
                      FB(btnReprendre) FB(btnSauvegarder) FB(btnOptionsPause)
                          FB(btnQuitterPartie)
#undef FB
}

void resetPlayer(Personnage *p, GameConfig *cfg) {
  int d, s;
  p->x = 80.0;
  p->y = cfg->groundY - cfg->collH;
  p->vx = 0.0;
  p->vy = 0.0;
  p->onGround = 1;
  p->etat = cfg->animStand;
  p->prevEtat = cfg->animStand;
  p->posScreen.x = 80;
  p->posScreen.y = cfg->groundY - cfg->collH;
  p->posScreen.w = cfg->collW;
  p->posScreen.h = cfg->collH;
  for (d = 0; d < 2; d++)
    for (s = 0; s < 6; s++) {
      p->sheets[d][s].currentFrame = 0;
      p->sheets[d][s].animTimer = 0;
    }
}

void applyPickupEffect(int itemType, Personnage *p, GameConfig *cfg) {
  if (itemType == cfg->itemMedkit) {
    p->vie = p->vie + 2;
    if (p->vie > p->maxVie)
      p->vie = p->maxVie;
  } else if (itemType == cfg->itemAmmo) {
    p->superAmmo = p->superAmmo + 3;
  } else if (itemType == cfg->itemVest) {
    p->shield = 1;
  } else if (itemType == cfg->itemCoin) {
    p->score = p->score + 5;
  }
}

int initGame(GameContext *ctx, int multiplayer) {
  int i;
  memset(ctx, 0, sizeof(GameContext));
  initConfig(&ctx->cfg, multiplayer, 800, 600);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return 0;
  }
  if (IMG_Init(IMG_INIT_PNG) == 0) {
    fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
    return 0;
  }
  if (TTF_Init() < 0) {
    fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
    return 0;
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    fprintf(stderr, "Mix_OpenAudio: %s\n", Mix_GetError());
    return 0;
  }

  srand(time(NULL));

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

  ctx->window = SDL_CreateWindow("John Wick 2D", SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, ctx->cfg.screenW,
                                 ctx->cfg.screenH, SDL_WINDOW_RESIZABLE);
  if (!ctx->window)
    return 0;

  ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
  if (!ctx->renderer)
    return 0;

  SDL_RenderSetLogicalSize(ctx->renderer, ctx->cfg.screenW, ctx->cfg.screenH);
  SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

  ctx->font = TTF_OpenFont("arial.ttf", 20);
  if (!ctx->font) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return 0;
  }

  ctx->fontLarge = TTF_OpenFont("arial.ttf", 38);
  if (!ctx->fontLarge)
    ctx->fontLarge = ctx->font;

  ctx->heartTex = loadTexture(ctx->renderer, "images/ui/heart.png");
  ctx->heartEmptyTex = loadTexture(ctx->renderer, "images/ui/heart_empty.png");

  ctx->worldW = ctx->cfg.worldW;
  ctx->worldH = ctx->cfg.worldH;

  for (i = 0; i < ctx->cfg.maxLevels; i++)
    loadLevel(&ctx->levels[i], ctx->renderer, &ctx->cfg, i);

  ctx->currentLevel = 0;

  initPersonnage(&ctx->player1, ctx->renderer, &ctx->cfg, 80,
                 ctx->cfg.groundY - ctx->cfg.collH, 0);
  initPersonnage(&ctx->player2, ctx->renderer, &ctx->cfg, 160,
                 ctx->cfg.groundY - ctx->cfg.collH, 1);

  ctx->cam1.w = ctx->cfg.screenW;
  ctx->cam1.h = ctx->cfg.screenH;
  ctx->cam2.w = ctx->cfg.halfW;
  ctx->cam2.h = ctx->cfg.screenH;

  initMinimap(&ctx->minimap, &ctx->cfg);

  for (i = 0; i < ctx->cfg.maxProjectiles; i++)
    ctx->projectiles[i].active = 0;

  ctx->quizCount =
      loadQuizBlocks("enigmes.txt", ctx->quizBlocks, ctx->cfg.maxQuestions);
  if (ctx->quizCount > 0) {
    ctx->shuffleState.lastNb = -1;
    ctx->enigme1 =
        genererEnigme(ctx->quizBlocks, ctx->quizCount, &ctx->shuffleState,
                      ctx->renderer, ctx->font, &ctx->cfg);
  }
  ctx->enigme1Active = 0;
  ctx->enigmeFromCoin = 0;
  ctx->currentMusicState = 0;
  ctx->combatMusicCooldown = 0;
  ctx->enigme1Bg = loadTexture(ctx->renderer, "images/ui/enigme_bg.png");

  initEnigme2(&ctx->enigme2, ctx->renderer, &ctx->cfg);

  ctx->gameState = ctx->cfg.stateMenu;
  ctx->running = 1;
  ctx->feedbackStart = 0;
  ctx->enigmeStart = 0;
  ctx->selectedKey = 0;
  ctx->gameStartTick = SDL_GetTicks();
  ctx->elapsedSeconds = 0;
  ctx->isFullscreen = 0;

  initMenu(&ctx->menu, ctx->renderer, ctx->font, &ctx->cfg);

  SDL_StartTextInput();
  return 1;
}

void handleEvents(GameContext *ctx, SDL_Event *ev) {
  Personnage *p1 = &ctx->player1, *p2 = &ctx->player2;
  int gx, gy, isSuper;

  if (ev->type == SDL_QUIT) {
    ctx->running = 0;
    return;
  }

  if (ctx->gameState == ctx->cfg.stateMenu) {
    handleMenuEvents(ctx, ev);
    return;
  }

  if (ctx->gameState == ctx->cfg.stateChoice) {
    if (ev->type == SDL_KEYDOWN && ev->key.keysym.sym == SDLK_ESCAPE) {
      ctx->gameState = ctx->cfg.statePlatformer;
      return;
    }
    if (ev->type == SDL_MOUSEBUTTONDOWN &&
        ev->button.button == SDL_BUTTON_LEFT) {
      int mx, my;
      SDL_GetMouseState(&mx, &my);
      mouseToLogical(ctx, &mx, &my);
      if (mx >= ctx->menu.btnQuiz.rect.x &&
          mx <= ctx->menu.btnQuiz.rect.x + ctx->menu.btnQuiz.rect.w &&
          my >= ctx->menu.btnQuiz.rect.y &&
          my <= ctx->menu.btnQuiz.rect.y + ctx->menu.btnQuiz.rect.h) {
        if (ctx->quizCount > 0) {
          libererEnigme(&ctx->enigme1);
          ctx->enigme1 =
              genererEnigme(ctx->quizBlocks, ctx->quizCount, &ctx->shuffleState,
                            ctx->renderer, ctx->font, &ctx->cfg);
        }
        ctx->enigme1Active = 1;
        ctx->enigme1.etat = ctx->cfg.enigmeAttente;
        ctx->enigmeFromCoin = 1;
        ctx->gameState = ctx->cfg.stateEnigme1;
        ctx->enigmeStart = SDL_GetTicks();
        ctx->feedbackStart = 0;
        return;
      }
      if (mx >= ctx->menu.btnPuzzle.rect.x &&
          mx <= ctx->menu.btnPuzzle.rect.x + ctx->menu.btnPuzzle.rect.w &&
          my >= ctx->menu.btnPuzzle.rect.y &&
          my <= ctx->menu.btnPuzzle.rect.y + ctx->menu.btnPuzzle.rect.h) {
        triggerEnigme2(&ctx->enigme2, ctx->renderer, &ctx->cfg);
        ctx->gameState = ctx->cfg.stateEnigme2;
        return;
      }
    }
    if (ev->type == SDL_KEYDOWN) {
      if (ev->key.keysym.sym == SDLK_q) {
        if (ctx->quizCount > 0) {
          libererEnigme(&ctx->enigme1);
          ctx->enigme1 =
              genererEnigme(ctx->quizBlocks, ctx->quizCount, &ctx->shuffleState,
                            ctx->renderer, ctx->font, &ctx->cfg);
        }
        ctx->enigme1Active = 1;
        ctx->enigme1.etat = ctx->cfg.enigmeAttente;
        ctx->enigmeFromCoin = 1;
        ctx->gameState = ctx->cfg.stateEnigme1;
        ctx->enigmeStart = SDL_GetTicks();
        return;
      }
      if (ev->key.keysym.sym == SDLK_p) {
        triggerEnigme2(&ctx->enigme2, ctx->renderer, &ctx->cfg);
        ctx->gameState = ctx->cfg.stateEnigme2;
        return;
      }
    }
    return;
  }

  if (ctx->gameState == ctx->cfg.stateEnigme2) {
    if (ev->type == SDL_KEYDOWN && ev->key.keysym.sym == SDLK_ESCAPE) {
      ctx->enigme2.active = 0;
      ctx->enigme2.answered = 0;
      ctx->enigme2.heldAnswer = -1;
      ctx->gameState = ctx->cfg.statePlatformer;
      return;
    }
    handleEnigme2Mouse(ctx, &ctx->enigme2, ev);
    if (ev->type == SDL_KEYDOWN && ev->key.keysym.sym == SDLK_SPACE &&
        ctx->enigme2.answered) {
      SDL_Texture *resultTex;
      SDL_Texture *bgTex =
          ctx->enigme2.puzzles[ctx->enigme2.currentPuzzle].bgTex;
      if (ctx->enigme2.correct) {
        p1->score = p1->score + 10;
        resultTex = ctx->enigme2.successTex;
      } else {
        if (p1->vie > 0)
          p1->vie = p1->vie - 1;
        resultTex = ctx->enigme2.failTex;
      }
      playEnigme2Result(ctx->renderer, bgTex, resultTex, &ctx->cfg);
      ctx->enigme2.active = 0;
      ctx->gameState = ctx->cfg.statePlatformer;
    }
    return;
  }

  if (ctx->gameState == ctx->cfg.stateEnigme1) {
    if (ev->type == SDL_MOUSEBUTTONDOWN &&
        ctx->enigme1.etat == ctx->cfg.enigmeAttente) {
      int mx, my, clicked;
      SDL_GetMouseState(&mx, &my);
      mouseToLogical(ctx, &mx, &my);
      clicked = 0;
      if (mx > ctx->enigme1.posRep1.x &&
          mx < ctx->enigme1.posRep1.x + ctx->enigme1.posRep1.w &&
          my > ctx->enigme1.posRep1.y &&
          my < ctx->enigme1.posRep1.y + ctx->enigme1.posRep1.h)
        clicked = 1;
      if (mx > ctx->enigme1.posRep2.x &&
          mx < ctx->enigme1.posRep2.x + ctx->enigme1.posRep2.w &&
          my > ctx->enigme1.posRep2.y &&
          my < ctx->enigme1.posRep2.y + ctx->enigme1.posRep2.h)
        clicked = 2;
      if (mx > ctx->enigme1.posRep3.x &&
          mx < ctx->enigme1.posRep3.x + ctx->enigme1.posRep3.w &&
          my > ctx->enigme1.posRep3.y &&
          my < ctx->enigme1.posRep3.y + ctx->enigme1.posRep3.h)
        clicked = 3;
      if (clicked) {
        resolveEnigme(clicked, &ctx->enigme1, &p1->score, &p1->vie, &ctx->cfg);
        ctx->feedbackStart = SDL_GetTicks();
      }
    }
    if (ev->type == SDL_KEYDOWN &&
        ctx->enigme1.etat == ctx->cfg.enigmeAttente) {
      int choice = 0;
      if (ev->key.keysym.sym == SDLK_a)
        choice = 1;
      if (ev->key.keysym.sym == SDLK_b)
        choice = 2;
      if (ev->key.keysym.sym == SDLK_c)
        choice = 3;
      if (choice != 0) {
        ctx->selectedKey = choice;
        resolveEnigme(choice, &ctx->enigme1, &p1->score, &p1->vie, &ctx->cfg);
        ctx->feedbackStart = SDL_GetTicks();
      }
    }
    return;
  }

  if (ev->type == SDL_KEYDOWN) {
    switch (ev->key.keysym.sym) {
    case SDLK_d:
    case SDLK_RIGHT:
      if (p1->vie > 0) {
        p1->vx = 5.0;
        p1->etat = ctx->cfg.animWalk;
        p1->direction = ctx->cfg.dirRight;
      }
      break;
    case SDLK_q:
    case SDLK_LEFT:
      if (p1->vie > 0) {
        p1->vx = -5.0;
        p1->etat = ctx->cfg.animWalk;
        p1->direction = ctx->cfg.dirLeft;
      }
      break;
    case SDLK_LSHIFT:
      if (p1->vie > 0) {
        if (p1->direction == ctx->cfg.dirRight)
          p1->vx = 9.0;
        else
          p1->vx = -9.0;
        p1->etat = ctx->cfg.animRun;
      }
      break;
    case SDLK_z:
    case SDLK_UP:
      if (p1->vie > 0 && p1->onGround) {
        p1->jumping = 1;
        p1->etat = ctx->cfg.animJump;
      }
      break;
    case SDLK_SPACE:
      if (p1->vie > 0 && p1->attackCooldown <= 0) {
        isSuper = 0;
        if (p1->superAmmo > 0) {
          isSuper = 1;
          p1->superAmmo--;
        }
        getGunBarrelPos(p1, &ctx->cfg, &gx, &gy);
        fireProjectile(ctx->projectiles, ctx->cfg.maxProjectiles, gx, gy,
                       p1->direction == ctx->cfg.dirRight, isSuper, 1,
                       &ctx->cfg);
        p1->attackCooldown = 300;
        p1->etat = ctx->cfg.animAttack;
      }
      break;
    case SDLK_ESCAPE:
      ctx->gameState = ctx->cfg.stateMenu;
      ctx->menu.menuScreen = 10;
      Mix_HaltMusic();
      if (ctx->menu.musicSub)
        Mix_PlayMusic(ctx->menu.musicSub, -1);
      p1->vx = 0.0;
      if (ctx->cfg.multiplayer)
        p2->vx = 0.0;
      break;
    default:
      break;
    }

    if (ctx->cfg.multiplayer) {
      switch (ev->key.keysym.sym) {
      case SDLK_KP_6:
        if (p2->vie > 0) {
          p2->vx = 5.0;
          p2->etat = ctx->cfg.animWalk;
          p2->direction = ctx->cfg.dirRight;
        }
        break;
      case SDLK_KP_4:
        if (p2->vie > 0) {
          p2->vx = -5.0;
          p2->etat = ctx->cfg.animWalk;
          p2->direction = ctx->cfg.dirLeft;
        }
        break;
      case SDLK_RSHIFT:
        if (p2->vie > 0) {
          if (p2->direction == ctx->cfg.dirRight)
            p2->vx = 9.0;
          else
            p2->vx = -9.0;
          p2->etat = ctx->cfg.animRun;
        }
        break;
      case SDLK_KP_8:
        if (p2->vie > 0 && p2->onGround) {
          p2->jumping = 1;
          p2->etat = ctx->cfg.animJump;
        }
        break;
      case SDLK_RCTRL:
        if (p2->vie > 0 && p2->attackCooldown <= 0) {
          isSuper = 0;
          if (p2->superAmmo > 0) {
            isSuper = 1;
            p2->superAmmo--;
          }
          getGunBarrelPos(p2, &ctx->cfg, &gx, &gy);
          fireProjectile(ctx->projectiles, ctx->cfg.maxProjectiles, gx, gy,
                         p2->direction == ctx->cfg.dirRight, isSuper, 1,
                         &ctx->cfg);
          p2->attackCooldown = 300;
          p2->etat = ctx->cfg.animAttack;
        }
        break;
      default:
        break;
      }
    }
  }

  if (ev->type == SDL_KEYUP) {
    switch (ev->key.keysym.sym) {
    case SDLK_d:
    case SDLK_RIGHT:
    case SDLK_q:
    case SDLK_LEFT:
    case SDLK_LSHIFT:
      if (p1->vie > 0 && p1->etat != ctx->cfg.animAttack &&
          p1->etat != ctx->cfg.animJump && p1->etat != ctx->cfg.animDead) {
        p1->vx = 0.0;
        p1->etat = ctx->cfg.animStand;
      }
      break;
    default:
      break;
    }
    if (ctx->cfg.multiplayer) {
      switch (ev->key.keysym.sym) {
      case SDLK_KP_4:
      case SDLK_KP_6:
      case SDLK_RSHIFT:
        if (p2->vie > 0 && p2->etat != ctx->cfg.animAttack &&
            p2->etat != ctx->cfg.animJump && p2->etat != ctx->cfg.animDead) {
          p2->vx = 0.0;
          p2->etat = ctx->cfg.animStand;
        }
        break;
      default:
        break;
      }
    }
  }
}

void updateGame(GameContext *ctx, int dt) {
  Level *lv;
  Personnage *p1, *p2;
  int elapsed, timerDone, feedbackDone, nbPlayers, pickIdx, pi;
  SDL_Rect pBoxes[2];
  double pVy[2], pX[2], pVxOut[2], pVyOut[2];
  int pVie[2], pShield[2], pScore[2];

  if (ctx->gameState == ctx->cfg.stateMenu) {
    updateMenu(ctx);
    return;
  }

  lv = &ctx->levels[ctx->currentLevel];
  p1 = &ctx->player1;
  p2 = &ctx->player2;
  ctx->elapsedSeconds = (SDL_GetTicks() - ctx->gameStartTick) / 1000;

  if (lv->worldW > 0)
    ctx->worldW = lv->worldW;
  else
    ctx->worldW = ctx->cfg.worldW;
  ctx->cfg.worldW = ctx->worldW;
  if (lv->groundY > 0)
    ctx->cfg.groundY = lv->groundY;

  if (ctx->gameState != ctx->cfg.statePlatformer) {
    ctx->currentMusicState = 0;
    ctx->combatMusicCooldown = 0;
  }

  if (ctx->gameState == ctx->cfg.stateEnigme2)
    updateEnigme2Drag(ctx, &ctx->enigme2);

  if (ctx->gameState == ctx->cfg.stateEnigme1) {
    elapsed = SDL_GetTicks() - ctx->enigmeStart;
    timerDone = (elapsed > 10000);
    feedbackDone = (ctx->enigme1.etat != ctx->cfg.enigmeAttente &&
                    SDL_GetTicks() - ctx->feedbackStart > 1500);
    if (timerDone || feedbackDone) {
      ctx->selectedKey = 0;
      ctx->enigme1Active = 0;
      ctx->gameState = ctx->cfg.statePlatformer;
      ctx->enigmeStart = 0;
      ctx->enigme1.etat = ctx->cfg.enigmeAttente;
      if (ctx->quizCount > 0) {
        libererEnigme(&ctx->enigme1);
        ctx->enigme1 =
            genererEnigme(ctx->quizBlocks, ctx->quizCount, &ctx->shuffleState,
                          ctx->renderer, ctx->font, &ctx->cfg);
      }
      if (ctx->enigmeFromCoin) {
        ctx->enigmeFromCoin = 0;
        return;
      }
      resetPlayer(p1, &ctx->cfg);
      ctx->currentLevel = ctx->currentLevel + 1;
      if (ctx->currentLevel >= ctx->cfg.maxLevels) {
        int totalScore = p1->score;
        if (ctx->cfg.multiplayer && p2->score > totalScore)
          totalScore = p2->score;
        ctx->menu.pendingScore = totalScore;
        ctx->menu.hasPendingScore = 1;
        ctx->menu.nameLen = 0;
        memset(ctx->menu.nameInput, 0, 32);
        ctx->menu.menuScreen = 6;
        ctx->gameState = ctx->cfg.stateMenu;
        Mix_HaltMusic();
        if (ctx->menu.musicVictory)
          Mix_PlayMusic(ctx->menu.musicVictory, 1);
        remove("save.txt");
        ctx->menu.hasSavedGame = 0;
        ctx->currentLevel = 0;
        return;
      }
      ctx->gameStartTick = SDL_GetTicks();
      resetPlayer(p1, &ctx->cfg);
      if (ctx->cfg.multiplayer)
        resetPlayer(p2, &ctx->cfg);
      for (pi = 0; pi < ctx->cfg.maxProjectiles; pi++)
        ctx->projectiles[pi].active = 0;
    }
    return;
  }

  if (ctx->gameState != ctx->cfg.statePlatformer)
    return;

  pollPlayerMovement(p1, &ctx->cfg, 0);
  movePersonnage(p1, lv->obstacles, lv->nbObstacles, &ctx->cfg);
  if (p1->vie == 0)
    p1->etat = ctx->cfg.animDead;
  syncAnimState(p1, &ctx->cfg);
  animerPersonnage(p1, dt);

  nbPlayers = 1;
  if (ctx->cfg.multiplayer) {
    pollPlayerMovement(p2, &ctx->cfg, 1);
    movePersonnage(p2, lv->obstacles, lv->nbObstacles, &ctx->cfg);
    if (p2->vie == 0)
      p2->etat = ctx->cfg.animDead;
    syncAnimState(p2, &ctx->cfg);
    animerPersonnage(p2, dt);
    nbPlayers = 2;
  }

  pBoxes[0] = p1->posScreen;
  pBoxes[1] = p2->posScreen;
  pVy[0] = p1->vy;
  pVy[1] = p2->vy;
  pX[0] = p1->x;
  pX[1] = p2->x;
  pVie[0] = p1->vie;
  pVie[1] = p2->vie;
  pShield[0] = p1->shield;
  pShield[1] = p2->shield;
  pScore[0] = p1->score;
  pScore[1] = p2->score;
  pVxOut[0] = p1->vx;
  pVxOut[1] = p2->vx;
  pVyOut[0] = p1->vy;
  pVyOut[1] = p2->vy;

  {
    int anyAggro = 0;
    updateLevel(lv, pBoxes, nbPlayers, pVy, pVie, pX, pShield, pScore, pVxOut,
                pVyOut, ctx->projectiles, ctx->cfg.maxProjectiles, &ctx->cfg,
                dt, &anyAggro);
    updateGameMusic(ctx, anyAggro, dt);
  }

  p1->vx = pVxOut[0];
  p1->vy = pVyOut[0];
  p1->vie = pVie[0];
  p1->score = pScore[0];
  if (ctx->cfg.multiplayer) {
    p2->vx = pVxOut[1];
    p2->vy = pVyOut[1];
    p2->vie = pVie[1];
    p2->score = pScore[1];
  }

  updateCamera(&ctx->cam1, p1->posScreen, ctx->worldW, ctx->worldH, &ctx->cfg,
               ctx->cfg.multiplayer);
  if (ctx->cfg.multiplayer)
    updateCamera(&ctx->cam2, p2->posScreen, ctx->worldW, ctx->worldH, &ctx->cfg,
                 1);
  updateMinimap(&ctx->minimap, p1->posScreen, p2->posScreen, ctx->worldW,
                ctx->worldH, &ctx->cfg);

  pickIdx = checkPickupCollision(&p1->posScreen, lv->pickups, lv->nbPickups);
  if (pickIdx < 0 && ctx->cfg.multiplayer)
    pickIdx = checkPickupCollision(&p2->posScreen, lv->pickups, lv->nbPickups);
  if (pickIdx >= 0) {
    int itype = lv->pickups[pickIdx].itemType;
    if (itype == ctx->cfg.itemCoin) {
      p1->score++;
      ctx->gameState = ctx->cfg.stateChoice;
    } else
      applyPickupEffect(itype, p1, &ctx->cfg);
  }

  if (p1->posScreen.x >= ctx->worldW - ctx->cfg.screenW / 2) {
    if (!ctx->enigme1Active && ctx->enigme1.etat == ctx->cfg.enigmeAttente) {
      ctx->enigme1Active = 1;
      ctx->enigmeFromCoin = 0;
      ctx->gameState = ctx->cfg.stateEnigme1;
      ctx->enigmeStart = SDL_GetTicks();
    }
  }
}

void renderGame(GameContext *ctx) {
  Level *lv = &ctx->levels[ctx->currentLevel];
  int mx, my;
  SDL_Rect vp1, vp2, divider, fs;

  if (ctx->gameState == ctx->cfg.stateMenu) {
    renderMenu(ctx);
    return;
  }

  SDL_GetMouseState(&mx, &my);
  mouseToLogical(ctx, &mx, &my);
  SDL_SetRenderDrawColor(ctx->renderer, 10, 10, 20, 255);
  SDL_RenderClear(ctx->renderer);

  if (ctx->gameState == ctx->cfg.stateChoice) {
    SDL_Color yellow, white;
    SDL_Rect veil, rect;
    SDL_Texture *t;
    MenuState *m = &ctx->menu;
    yellow.r = 255;
    yellow.g = 215;
    yellow.b = 0;
    yellow.a = 255;
    white.r = 255;
    white.g = 255;
    white.b = 255;
    white.a = 255;
    veil.x = 0;
    veil.y = 0;
    veil.w = ctx->cfg.screenW;
    veil.h = ctx->cfg.screenH;
    SDL_SetRenderDrawColor(ctx->renderer, 8, 8, 20, 230);
    SDL_RenderFillRect(ctx->renderer, &veil);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(ctx->renderer, ctx->fontLarge, "Choisissez votre defi", yellow,
                 &rect);
    if (t) {
      rect.x = (ctx->cfg.screenW - rect.w) / 2;
      rect.y = 100;
      SDL_RenderCopy(ctx->renderer, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
    t = makeText(ctx->renderer, ctx->font,
                 "Q = Quiz   P = Puzzle   ESC = annuler", white, &rect);
    if (t) {
      rect.x = (ctx->cfg.screenW - rect.w) / 2;
      rect.y = 170;
      SDL_RenderCopy(ctx->renderer, t, NULL, &rect);
      SDL_DestroyTexture(t);
    }
    menuButtonCheck(&m->btnQuiz, mx, my, m->sfxHover, NULL);
    menuButtonCheck(&m->btnPuzzle, mx, my, m->sfxHover, NULL);
    renderMenuButton(ctx->renderer, &m->btnQuiz);
    renderMenuButton(ctx->renderer, &m->btnPuzzle);
    SDL_RenderPresent(ctx->renderer);
    return;
  }

  if (ctx->gameState == ctx->cfg.stateEnigme2) {
    afficherEnigme2(&ctx->enigme2, ctx->renderer, ctx->font, &ctx->cfg);
    SDL_RenderPresent(ctx->renderer);
    return;
  }

  if (ctx->gameState == ctx->cfg.stateEnigme1) {
    fs.x = 0;
    fs.y = 0;
    fs.w = ctx->cfg.screenW;
    fs.h = ctx->cfg.screenH;
    if (ctx->enigme1Bg)
      SDL_RenderCopy(ctx->renderer, ctx->enigme1Bg, NULL, &fs);
    updateTimerBar(ctx->renderer, ctx->enigmeStart, &ctx->cfg);
    afficherEnigme(&ctx->enigme1, ctx->renderer, mx, my, &ctx->cfg);
    afficherKeyboardHints(ctx->renderer, ctx->font, ctx->selectedKey,
                          &ctx->cfg);
    afficherFeedback(ctx->renderer, ctx->font, &ctx->enigme1,
                     ctx->feedbackStart, &ctx->cfg);
    SDL_RenderPresent(ctx->renderer);
    return;
  }

  if (!ctx->cfg.multiplayer) {
    vp1.x = 0;
    vp1.y = 0;
    vp1.w = ctx->cfg.screenW;
    vp1.h = ctx->cfg.screenH;
    SDL_RenderSetClipRect(ctx->renderer, &vp1);
    afficherLevel(lv, ctx->renderer, ctx->cam1, vp1, &ctx->cfg);
    afficherPersonnage(&ctx->player1, ctx->renderer, ctx->cam1, vp1);
    renderProjectiles(ctx->renderer, ctx->projectiles, ctx->cfg.maxProjectiles,
                      ctx->cam1, vp1);
    afficherHUD(ctx->renderer, ctx->font, ctx->player1.vie, ctx->player1.maxVie,
                ctx->player1.shield, ctx->player1.superAmmo, ctx->player1.score,
                ctx->currentLevel + 1, ctx->elapsedSeconds, &ctx->cfg, vp1,
                &ctx->minimap, lv->obstacles, lv->nbObstacles, ctx->worldW,
                ctx->worldH, ctx->heartTex, ctx->heartEmptyTex);
    SDL_RenderSetClipRect(ctx->renderer, NULL);
  } else {
    vp1.x = 0;
    vp1.y = 0;
    vp1.w = ctx->cfg.halfW;
    vp1.h = ctx->cfg.screenH;
    vp2.x = ctx->cfg.halfW;
    vp2.y = 0;
    vp2.w = ctx->cfg.halfW;
    vp2.h = ctx->cfg.screenH;

    SDL_RenderSetClipRect(ctx->renderer, &vp1);
    afficherLevel(lv, ctx->renderer, ctx->cam1, vp1, &ctx->cfg);
    afficherPersonnage(&ctx->player1, ctx->renderer, ctx->cam1, vp1);
    renderProjectiles(ctx->renderer, ctx->projectiles, ctx->cfg.maxProjectiles,
                      ctx->cam1, vp1);
    afficherHUD(ctx->renderer, ctx->font, ctx->player1.vie, ctx->player1.maxVie,
                ctx->player1.shield, ctx->player1.superAmmo, ctx->player1.score,
                ctx->currentLevel + 1, ctx->elapsedSeconds, &ctx->cfg, vp1,
                &ctx->minimap, lv->obstacles, lv->nbObstacles, ctx->worldW,
                ctx->worldH, ctx->heartTex, ctx->heartEmptyTex);

    SDL_RenderSetClipRect(ctx->renderer, &vp2);
    afficherLevel(lv, ctx->renderer, ctx->cam2, vp2, &ctx->cfg);
    afficherPersonnage(&ctx->player2, ctx->renderer, ctx->cam2, vp2);
    renderProjectiles(ctx->renderer, ctx->projectiles, ctx->cfg.maxProjectiles,
                      ctx->cam2, vp2);
    afficherHUD(ctx->renderer, ctx->font, ctx->player2.vie, ctx->player2.maxVie,
                ctx->player2.shield, ctx->player2.superAmmo, ctx->player2.score,
                ctx->currentLevel + 1, ctx->elapsedSeconds, &ctx->cfg, vp2,
                &ctx->minimap, lv->obstacles, lv->nbObstacles, ctx->worldW,
                ctx->worldH, ctx->heartTex, ctx->heartEmptyTex);

    SDL_RenderSetClipRect(ctx->renderer, NULL);
    divider.x = ctx->cfg.halfW - 1;
    divider.y = 0;
    divider.w = 2;
    divider.h = ctx->cfg.screenH;
    SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 200);
    SDL_RenderFillRect(ctx->renderer, &divider);
  }

  SDL_RenderPresent(ctx->renderer);
}

void cleanupGame(GameContext *ctx) {
  int i;
  SDL_StopTextInput();
  freePersonnage(&ctx->player1);
  freePersonnage(&ctx->player2);
  libererEnigme(&ctx->enigme1);
  freeEnigme2(&ctx->enigme2);
  for (i = 0; i < ctx->cfg.maxLevels; i++)
    freeLevel(&ctx->levels[i]);
  if (ctx->heartTex)
    SDL_DestroyTexture(ctx->heartTex);
  if (ctx->heartEmptyTex)
    SDL_DestroyTexture(ctx->heartEmptyTex);
  if (ctx->enigme1Bg)
    SDL_DestroyTexture(ctx->enigme1Bg);
  freeMenu(&ctx->menu);
  if (ctx->fontLarge && ctx->fontLarge != ctx->font)
    TTF_CloseFont(ctx->fontLarge);
  if (ctx->font)
    TTF_CloseFont(ctx->font);
  SDL_DestroyRenderer(ctx->renderer);
  SDL_DestroyWindow(ctx->window);
  Mix_CloseAudio();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}
